/*
 * XDL printing library
 */

#include "xdlout.h"

#include <time.h>
#include "bitheader.h"

/**
 * Design printing function
 **/

void print_design(parsed_header_t *header) {
  const unsigned ncdv1 = 3, ncdv2 = 1;
  const header_option_t *devopt = get_option(header, DEVICE_TYPE);
  const header_option_t *nameopt = get_option(header, FILENAME);
  char *name = nameopt->payload;
  unsigned namel = get_option_len(nameopt);
  char *chp = devopt->payload;
  unsigned chpl = get_option_len(devopt);
  time_t timestamp;

  g_print("design \"%.*s\" %.*s v%i.%i ,\n",
	  namel, name, chpl, chp, ncdv1, ncdv2);

  /* At some point get the timestamp from the bitfile */
  timestamp = time(NULL);
  g_print("  cfg \"\n");
  g_print("       _DESIGN_PROP::PK_NGMTIMESTAMP:%lu\n",timestamp);
  g_print("      \";\n");
}

/**
 * NET Printing functions
 **/
struct _print_net {
  const wire_db_t *wiredb;
  const chip_descr_t *chipdb;
};

#ifdef VIRTEX2

#define STDNAME(x) [x] = #x

static const gchar *typenames[NR_WIRE_TYPE] = {
  STDNAME(BX), STDNAME(BY),
  STDNAME(X), STDNAME(XB), STDNAME(XQ),
  STDNAME(Y), STDNAME(YB), STDNAME(YQ),
  STDNAME(F1), STDNAME(F2), STDNAME(F3), STDNAME(F4),
  STDNAME(G1), STDNAME(G2), STDNAME(G3), STDNAME(G4),
  STDNAME(CE),
  STDNAME(SR),
  STDNAME(CLK),
  STDNAME(O0), STDNAME(O1), STDNAME(O2), STDNAME(O3), STDNAME(O4), STDNAME(O5), STDNAME(O6), STDNAME(O7),
  STDNAME(I0), STDNAME(I1), STDNAME(I2), STDNAME(I3), STDNAME(I4), STDNAME(I5), STDNAME(I6), STDNAME(I7),
  STDNAME(IQ1), STDNAME(IQ2),
};

#undef STDNAME

static inline
const char *typename(const wire_type_t wt) {
  const gchar *name = typenames[wt];
  return name ? name : "unknown";
}

typedef enum iopin_dir {
  IO_INPUT,
  IO_OUTPUT,
  IO_END,
} iopin_dir_t;

const char *ioname[IO_END] = {
  [IO_INPUT] = "inpin",
  [IO_OUTPUT] = "outpin",
};

static void
print_iopin(const iopin_dir_t iodir,
	    const sited_pip_t *spip,
	    const wire_db_t *wiredb,
	    const chip_descr_t *chip) {
  /* Use type to get the name of the wire in the instance */
  const wire_t *wire = get_wire(wiredb, spip->pip.target);
  const csite_descr_t *site = get_site(chip, spip->site);
  gchar slicen[MAX_SITE_NLEN];
  snprint_slice(slicen, MAX_SITE_NLEN, chip, site, wire->situation - ZERO);
  /* Combine the situation and site to get the location */
  g_print("  %s \"%s\" %s , #%s\n", ioname[iodir], slicen,
	  typename(wire->type), wire_name(wiredb,spip->pip.target));
}

static gboolean
print_inpin(GNode *net,
	    gpointer data) {
  const sited_pip_t *spip = net->data;
  struct _print_net *arg = data;
  print_iopin(IO_INPUT, spip, arg->wiredb, arg->chipdb);
  return FALSE;
}

static gboolean
print_outpin(GNode *net,
	     gpointer data) {
  const sited_pip_t *spip = net->data;
  struct _print_net *arg = data;
  print_iopin(IO_OUTPUT, spip, arg->wiredb, arg->chipdb);
  return FALSE;
}

#else

static gboolean
print_inpin(GNode *net,
	    gpointer data) {
  (void) net;
  (void) data;
  return FALSE;
}

static gboolean
print_outpin(GNode *net,
	     gpointer data) {
  (void) net;
  (void) data;
  return FALSE;
}

#endif

static gboolean
print_wire(GNode *net,
	   gpointer data) {
  struct _print_net *arg = data;
  const wire_db_t *wiredb = arg->wiredb;
  const chip_descr_t *chip = arg->chipdb;
  const sited_pip_t *spip = net->data;
  gchar buf[64];

  /* This is how wire start are indicated -- this is actually redundant
     with positioning in the tree... */
  if (spip->pip.source == WIRE_EP_END)
    return FALSE;

  sprint_spip(buf, wiredb, chip, net->data);
  g_print("  %s ,\n", buf);
  return FALSE;
}

static void
print_net(GNode *net, gpointer data) {
  static unsigned netnum = 0;
  g_print("net \"net_%i\" , \n", netnum++);
  /* print input -- this should be the output pin of a logical bloc */
  print_outpin(net, data);
  /* print outputs -- these should be input pins to some logical blocs */
  g_node_traverse (net, G_IN_ORDER, G_TRAVERSE_LEAVES, -1, print_inpin, data);
  g_node_traverse (net, G_PRE_ORDER, G_TRAVERSE_ALL, -1, print_wire, data);
  g_print("  ;\n");
}

void print_nets(nets_t *net,
		const pip_db_t *pipdb,
		const chip_descr_t *cdb) {
  struct _print_net arg = { .wiredb = pipdb->wiredb, .chipdb = cdb };
  /* Iterate through nets */
  g_node_children_foreach (net->head, G_TRAVERSE_ALL, print_net, &arg);
}
