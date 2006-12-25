/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "keyfile.h"

#include "sites.h"
#include "design.h"

/*
 * File site control description is of form
 * [XC2V2000]
 * X_WIDTH=;
 * Y_WIDTH=;
 * File site description data is of this form
 * [SITENAME]
 * ID=NUM;
 * XINTERVALS=1,2,3,4
 * YINTERVALS=2,3,7,7
 * This means: sites in ([1,2] u [3,4]) x ([2,3] u [7,7]) are of type ID
 * Xilinx is cool: the names are okay so that this description works and
 * we don't have to repeat sitename several times
 */

#if defined(VIRTEX2)

static const gchar *
chipfiles[XC2__NUM] = {
  [XC2V40] = "xc2v40",
  [XC2V80] = "xc2v80",
  [XC2V250] = "xc2v250",
  [XC2V500] = "xc2v500",
  [XC2V1000] = "xc2v1000",
  [XC2V1500] = "xc2v1500",
  [XC2V2000] = "xc2v2000",
  [XC2V3000] = "xc2v3000",
  [XC2V4000] = "xc2v4000",
  [XC2V6000] = "xc2v6000",
  [XC2V8000] = "xc2v8000",
};

#elif defined(VIRTEX4)

static const gchar *
chipfiles[XC4VLX__NUM] = {
  [XC4VLX15] = "xc4vlx15",
  [XC4VLX25] = "xc4vlx25",
  [XC4VLX40] = "xc4vlx40",
  [XC4VLX60] = "xc4vlx60",
  [XC4VLX80] = "xc4vlx80",
  [XC4VLX100] = "xc4vlx100",
  [XC4VLX160] = "xc4vlx160",
  [XC4VLX200] = "xc4vlx200",
};

#endif

/* iterate over intervals */
typedef void (* interval_iterator_t)(unsigned i, void *dat);

static void
iterate_over_intervals(interval_iterator_t iter, void *dat,
		       const gint *intervals, const gsize len) {
  unsigned i;
  for (i = 0; i < len-1; i+=2) {
    unsigned j,
      min = intervals[i],
      max = intervals[i+1];
    /* iterate over the interval */
    for (j = min; j < max; j++)
      iter(j, dat);
  }
}

typedef struct _line_iterator {
  chip_descr_t *chip;
  site_type_t type;
  const gint *intervals_x;
  unsigned xlen;
  /* current line we're operating at */
  unsigned y;
} line_iterator_t;

static void
pos_setting(unsigned x, void *dat) {
  line_iterator_t *data = dat;
  unsigned y = data->y;
  chip_descr_t *chip = data->chip;
  site_type_t type = data->type;
  csite_descr_t *descr = get_global_site(chip,x,y);

  g_assert(descr->type == SITE_TYPE_NEUTRAL);
  descr->type = type;
}

static void
iterate_over_lines(unsigned y, void *dat) {
  /* dat contains the y-coordinate */
  line_iterator_t *data = dat;
  data->y = y;
  iterate_over_intervals(pos_setting, dat,
			 data->intervals_x, data->xlen);
}

static void
init_chip_type(chip_descr_t *chip, site_type_t type,
	       const gint *intervals_x, const gsize xlen,
	       const gint *intervals_y, const gsize ylen) {
  line_iterator_t x_arg = {
    .type = type, .chip = chip,
    .intervals_x = intervals_x, .xlen = xlen,
  };
  iterate_over_intervals(iterate_over_lines, &x_arg,
			 intervals_y, ylen);
}

static inline void
alloc_chip(chip_descr_t *descr) {
  unsigned nelems = descr->width * descr->height;
  descr->data = g_new0(csite_descr_t, nelems);
}

static void
init_group_chip_type(GKeyFile *file, const gchar *group,
		     gpointer data) {
  chip_descr_t *chip = data;
  GError *error = NULL;
  gint *intervals[2] = {0, 0};
  gsize sizes[2];
  unsigned type;

  /* get group name & al from the group, fill in with this */
  intervals[0] = g_key_file_get_integer_list(file, group, "x", &sizes[0], &error);
  if (error)
    goto out_err;
  intervals[1] = g_key_file_get_integer_list(file, group, "y", &sizes[1], &error);
  if (error)
    goto out_err;
  type = g_key_file_get_integer(file, group, "type", &error);
  if (error)
    goto out_err;
  g_assert(type < NR_SITE_TYPE);
  init_chip_type(chip, type, intervals[0], sizes[0], intervals[1], sizes[1]);

 out_err:
  if (intervals[0])
    g_free(intervals[0]);
  if (intervals[1])
    g_free(intervals[1]);
  if (error) {
    g_warning("Error treating group %s: %s",
	      group, error->message);
    g_error_free(error);
  }
  return;
}

void
iterate_over_sites(const chip_descr_t *chip,
		   site_iterator_t fun, gpointer data) {
  unsigned x, y;
  unsigned xmax = chip->width, ymax = chip->height;
  csite_descr_t *site = chip->data;

  for (y = 0; y < ymax; y++)
    for (x = 0; x < xmax; x++)
      fun(x,y,site++,data);
}

void
iterate_over_typed_sites(const chip_descr_t *chip, site_type_t type,
			 site_iterator_t fun, gpointer data) {
  unsigned x, y;
  unsigned xmax = chip->width, ymax = chip->height;
  csite_descr_t *site = chip->data;

  for (y = 0; y < ymax; y++)
    for (x = 0; x < xmax; x++) {
      if (site->type == type)
	fun(x,y,site,data);
      site++;
    }
}

typedef struct _local_counter {
  gint x;
  gint y;
  gint y_global;
} local_counter_t;

static void
init_site_coord(unsigned site_x, unsigned site_y,
		csite_descr_t *site, gpointer dat) {
  local_counter_t *coords = dat;
  site_type_t type = site->type;
  local_counter_t *count = coords + type;
  gint x = count->x, y = count->y, y_global = count->y_global;
  gboolean newline = y_global < (int)site_y ? TRUE : FALSE;

  if (newline) {
    y++;
    x = 0;
  } else {
    x++;
  }

  site->type_coord.x = x;
  site->type_coord.y = y;

  if (newline)
    count->y_global = site_y;
  count->x = x;
  count->y = y;
}

static void
init_local_coordinates(chip_descr_t *chip) {
  /* local counters */
  local_counter_t coords[NR_SITE_TYPE];
  unsigned i;
  for (i = 0; i < NR_SITE_TYPE; i++) {
    coords[i].y = -1;
    coords[i].y_global = -1;
  }
  iterate_over_sites(chip, init_site_coord, coords);
}

static void
init_chip(chip_descr_t *chip, GKeyFile *file) {
  /* for each of the types, call the init functions */
  iterate_over_groups(file, init_group_chip_type, chip);
  init_local_coordinates(chip);
}

/* exported alloc and destroy functions */
chip_descr_t *
get_chip(const gchar *dirname, const unsigned chipid) {
  chip_descr_t *chip = g_new0(chip_descr_t, 1);
  const gchar *chipname = chipfiles[chipid];
  GKeyFile *keyfile;
  GError *error = NULL;
  gchar *filename;
  int err;

  filename = g_build_filename(dirname, chipname, "chip_control", NULL);
  err = read_keyfile(&keyfile,filename);
  g_free(filename);
  if (err)
    goto out_err;

#define DIM "DIMENTIONS"
  /* Get width and height */
  chip->width  = g_key_file_get_integer(keyfile, DIM, "WIDTH", &error);
  if (error)
    goto out_err_free_err_keyfile;
  chip->height = g_key_file_get_integer(keyfile, DIM, "HEIGHT", &error);
  if (error)
    goto out_err_free_err_keyfile;

  alloc_chip(chip);
  g_key_file_free(keyfile);

  filename = g_build_filename(dirname, chipname, "chip_data", NULL);
  err = read_keyfile(&keyfile,filename);
  g_free(filename);
  if (err)
    goto out_err;

  init_chip(chip, keyfile);
  g_key_file_free(keyfile);

  return chip;

 out_err_free_err_keyfile:
  g_warning("error reading chip description: %s", error->message);
  g_error_free(error);
  g_key_file_free(keyfile);
 out_err:
  g_free(chip);
  return NULL;
}

void
release_chip(chip_descr_t *chip) {
  g_free(chip->data);
  chip->data = NULL;
  g_free(chip);
}

/*
 * Utility function
 */

/*
 * This is the compact form of the site printing function.  We could
 * also do only one call with the "%0.0i" trick, and doing some tricky
 * initialization. This other solution is, however, a bit more
 * complicated, so we'll leave this as-is for now.
 */

typedef enum _site_print_type {
  PRINT_BOTH = 0,
  PRINT_X,
  PRINT_Y,
} site_print_t;

#if defined(VIRTEX2)

static const site_print_t print_type[NR_SITE_TYPE] = {
  [TIOI] = PRINT_X,
  [BIOI] = PRINT_X,
  [LIOI] = PRINT_Y,
  [RIOI] = PRINT_Y,
  [RTERM] = PRINT_Y,
  [LTERM] = PRINT_Y,
  [TTERM] = PRINT_X,
  [BTERM] = PRINT_X,
  [TTERMBRAM] = PRINT_X,
  [BTERMBRAM] = PRINT_X,
  [TIOIBRAM] = PRINT_X,
  [BIOIBRAM] = PRINT_X,
};

static const char *print_str[NR_SITE_TYPE] = {
  [SITE_TYPE_NEUTRAL] = "GLOBALR%iC%i",
  [CLB] = "R%iC%i",
  [RTERM] = "RTERMR%i",
  [LTERM] = "LTERMR%i",
  [TTERM] = "TTERMC%i",
  [BTERM] = "BTERMC%i",
  [TIOI] = "TIOIC%i",
  [BIOI] = "BIOIC%i",
  [LIOI] = "LIOIR%i",
  [RIOI] = "RIOIR%i",
  [TTERMBRAM] = "TTERMBRAMC%i",
  [BTERMBRAM] = "BTERMBRAMC%i",
  [TIOIBRAM] = "TIOIBRAMC%i",
  [BIOIBRAM] = "BIOIBRAMC%i",
  [BRAM] = "BRAMR%iC%i",
};

#elif defined(VIRTEX4)

static const site_print_t print_type[NR_SITE_TYPE];

static const char *print_str[NR_SITE_TYPE] = {
  [SITE_TYPE_NEUTRAL] = "INTR%iC%i",
  [IOB] = "IOBR%iC%i",
  [CLB] = "CLBR%iC%i",
  [DSP48] = "DSP48R%iC%i",
  [GCLKC] = "GCLKCR%iC%i",
  [BRAM] = "BRAMR%iC%i",
};

#endif

void
sprint_csite(gchar *data, const csite_descr_t *site) {
  /* Use a string chunk ? */
  const char *str = print_str[site->type];
  const site_print_t strtype = print_type[site->type];
  const guint x = site->type_coord.x + 1;
  const guint y = site->type_coord.y + 1;

  if (!str)
    str = print_str[SITE_TYPE_NEUTRAL];

  switch (strtype) {
    case PRINT_BOTH:
      sprintf(data, str, y, x);
      break;
    case PRINT_X:
      sprintf(data, str, x);
      break;
    case PRINT_Y:
      sprintf(data, str, y);
      break;
  }

}

static void
print_iterator(unsigned x, unsigned y,
	       csite_descr_t *site, gpointer dat) {
  gchar name[32];
  sprint_csite(name, site);
  (void) dat;
  g_print("global site (%i,%i) is %s\n", x, y, name);
}

void
print_chip(chip_descr_t *chip) {
  iterate_over_sites(chip, print_iterator, NULL);
}

