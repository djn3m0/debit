/*
 * Copyright (C) 2006, 2007 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 *
 * This file is part of debit.
 *
 * Debit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Debit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with debit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <glib.h>
#include <glib/gstdio.h>

#undef HAVE_MMAP

#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif /* HAVE_MMAP */

/* Dirty hack for w32 support */
#ifndef O_NDELAY
#define O_NDELAY 0
#endif /* O_NDELAY */

#include <unistd.h>

#include "bitstream_parser.h"
#include "bitheader.h"
#include "bitstream_packets.h"
#include "design.h"
#include "bitstream_write.h"

/* State necessary for bitstream computation */
typedef struct _bitstream_writer {
  int fd;
  uint32_t crc;
  const bitstream_parsed_t *bit;
} bitstream_writer_t;

static inline void write_s(int fd, const void *buf, size_t count) {
  const char *dat = buf;
  do {
    ssize_t written = write(fd, dat, count);
    if (written < 0) {
      perror("writing buffer");
      return;
    }
    count -= written;
    dat += written;
  } while (count != 0);
}

static inline void write_u8(int fd, const uint8_t dat) {
  write_s(fd, &dat, sizeof(dat));
}

static inline void write_u16(int fd, const uint16_t dat) {
  const uint16_t wdat = GUINT16_TO_BE(dat);
  write_s(fd, &wdat, sizeof(wdat));
}

static inline void write_u32(int fd, const uint32_t dat) {
  const uint32_t wdat = GUINT32_TO_BE(dat);
  write_s(fd, &wdat, sizeof(wdat));
}

/* Write data buffer with host to bitstream conversion of words */
static inline void
write_words(int fd, const uint32_t *buf, const size_t wordc) {
  size_t i;
  for (i = 0; i < wordc; i++)
    write_u32(fd, buf[i]);
}

/* Write data buffer without host to bitstream conversion of words */
static inline void
write_buf(int fd, const void *buf, const size_t len) {
  write_s(fd, buf, len);
}

/*
 * Header writing functions. This is the wrapper around the real
 * bitstream data. Not really usefull, but necessary.
 */

static void
bs_write_option(const int fd, const uint8_t code,
		const void *payload, const uint16_t length) {
  write_u8(fd,code);
  write_u16(fd,length);
  write_s(fd, payload, length);
}

static void
bs_write_long_option(const int fd, const uint8_t code,
		     const void *payload, const uint32_t length) {
  write_u8(fd,code);
  write_u32(fd,length);
  write_s(fd, payload, length);
}

static void
bs_write_header(const int fd, const bitstream_parsed_t *bit) {
  const parsed_header_t *header = &bit->header;

#define REWRITE(opt)   do {				\
  const header_option_p *hopt = get_option(header,opt); \
  bs_write_option(fd, opt,				\
		  hopt->data, hopt->len); }		\
  while(0)

  REWRITE(FILENAME);
  REWRITE(DEVICE_TYPE);
  REWRITE(BUILD_DATE);
  REWRITE(BUILD_TIME);
#undef REWRITE
}

static void
bs_write_magic(const int fd) {
  write_u16(fd, 9);
  write_u16(fd, 0x0ff0);
  write_u16(fd, 0x0ff0);
  write_u16(fd, 0x0ff0);
  write_u16(fd, 0x0ff0);
  write_u16(fd, 0x0);
  write_u8(fd,  0x1);
}

#ifdef HAVE_MMAP
static void
bs_write_data(int fd, int fd_data, const char *tmp) {
  struct stat statistics;
  void *data_buf;
  off_t size;
  (void) tmp;

  fstat(fd_data, &statistics);
  size = statistics.st_size;

  data_buf = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd_data, 0);
  if (!data_buf) {
    fprintf(stderr, "MMAP FAILED");
    return;
  }
  bs_write_long_option(fd, CODE, data_buf, size);
  munmap(data_buf, size);
}
#else /* HAVE_MMAP */
/* Non-mmap suboptimal implementation, which goes through the file
   path -- which must not have been unlinked. */
static void
bs_write_data(int fd, int fd_data, const char *tmp) {
  GError *error = NULL;
  GMappedFile *file = NULL;
  (void) fd_data;

  file = g_mapped_file_new(tmp, FALSE, &error);
  if (error != NULL) {
    g_warning("could not map file %s: %s", tmp, error->message);
    g_error_free (error);
    return;
  }

  bs_write_long_option(fd, CODE,
		       g_mapped_file_get_contents (file),
		       g_mapped_file_get_length (file));

  g_mapped_file_free(file);
}
#endif /* HAVE_MMAP */

/* Only depends on ? */
/* A bit complicated, as this must be computed after the file output */
static void
bs_add_header(const bitstream_writer_t *writer,
	      int fd, int data, const char *tmp) {
  (void) writer;
  bs_write_magic(fd);
  bs_write_header(fd, writer->bit);
  bs_write_data(fd, data, tmp);
}

/* packet writing functions */
#if defined(VIRTEX2) || defined(SPARTAN3)

typedef enum _io_type {
  PKT_READ = 0,
  PKT_WRITE = 1,
} io_type_t;

typedef enum _registers_index {
  /* from xilinx ug002.pdf */
  CRC = 0, FAR, FDRI, FDRO,
  CMD, CTL, MASK,
  STAT, LOUT, COR,
  MFWR, FLR, KEY,
  CBC, IDCODE,
  __NUM_REGISTERS,
} register_index_t;

typedef enum _cmd_code {
  C_NULL = 0, WCFG,
  C_MFWR,
  DGHIGH_LFRM, RCFG, START, RCAP, RCRC,
  AGHIGH, SWITCH, GRESTORE,
  SHUTDOWN, GCAPTURE,
  DESYNCH,
  __NUM_CMD_CODE,
} cmd_code_t;

/* Other shitty registers */
/* XXX Duplicated, along with above */

typedef struct _bitfield_t {
  /* length is implicit as the difference of lenghts */
  unsigned char off;
} bitfield_t;

typedef enum _cor_fields {
  GWE_CYCLE = 0,
  GTS_CYCLE, LOCK_CYCLE,
  MATCH_CYCLE, DONE_CYCLE,
  SSCLKSRC, OSCFSEL,
  SINGLE, DRIVE_DONE, DONE_PIPE,
  SHUT_RST_DCM, RSV, CRC_BYPASS,
  COR_FIELD_LAST,
} cor_bits_t;

static const bitfield_t cor_bitfields[COR_FIELD_LAST+1] = {
  [GWE_CYCLE] = { .off = 0 },
  [GTS_CYCLE] = { .off = 3 },
  [LOCK_CYCLE] = { .off = 6 },
  [MATCH_CYCLE] = { .off = 9 },
  [DONE_CYCLE] = { .off = 12 },
  [SSCLKSRC] = { .off = 15 },
  [OSCFSEL] = { .off = 17 },
  [SINGLE] = { .off = 23 },
  [DRIVE_DONE] = { .off = 24 },
  [DONE_PIPE] = { .off = 25 },
  [SHUT_RST_DCM] = { .off = 26 },
  [RSV] = { .off = 27 },
  [CRC_BYPASS] = { .off = 29 },
  [COR_FIELD_LAST] = { .off = 30 },
};

#elif defined(VIRTEX4)

typedef enum _io_type {
  PKT_READ = 0,
  PKT_WRITE = 1,
} io_type_t;

typedef enum _registers_index {
  CRC = 0, FAR, FDRI, FDRO,
  CMD, CTL, MASK,
  STAT, LOUT, COR,
  /* error in ug071 */
  MFWR, CBC, IDCODE, AXSS,
  __NUM_REGISTERS,
} register_index_t;

typedef enum _cmd_code {
  C_NULL = 0, WCFG,
  C_MFWR,
  C_LFRM, RCFG, START, RCAP, RCRC,
  AGHIGH, SWITCH, GRESTORE,
  SHUTDOWN, GCAPTURE,
  DESYNCH,
  __NUM_CMD_CODE,
} cmd_code_t;

/* Other shitty registers */
/* XXX Duplicated, along with above */

typedef struct _bitfield_t {
  /* length is implicit as the difference of lengths */
  unsigned char off;
} bitfield_t;

typedef enum _cor_fields {
  GWE_CYCLE = 0,
  GTS_CYCLE, LOCK_CYCLE,
  MATCH_CYCLE, DONE_CYCLE,
  SSCLKSRC, OSCFSEL,
  SINGLE, DRIVE_DONE, DONE_PIPE,
  RSV, CRC_BYPASS, COR_FIELD_LAST,
} cor_bits_t;

static const bitfield_t cor_bitfields[COR_FIELD_LAST+1] = {
  [GWE_CYCLE] = { .off = 0 },
  [GTS_CYCLE] = { .off = 3 },
  [LOCK_CYCLE] = { .off = 6 },
  [MATCH_CYCLE] = { .off = 9 },
  [DONE_CYCLE] = { .off = 12 },
  [SSCLKSRC] = { .off = 15 },
  [OSCFSEL] = { .off = 17 },
  [SINGLE] = { .off = 23 },
  [DRIVE_DONE] = { .off = 24 },
  [DONE_PIPE] = { .off = 25 },
  [RSV] = { .off = 26 },
  [CRC_BYPASS] = { .off = 28 },
  [COR_FIELD_LAST] = { .off = 29 },
};

typedef enum _ctl_fields {
  GTS_USR_B = 0,
  RSV0,
  PERSIST, SBITS,
  RSV1,
  GLUTMASK_B,
  RSV2, ICAPSEL,
  CTL_FIELD_LAST,
} ctl_bits_t;

static const bitfield_t ctl_bitfields[CTL_FIELD_LAST+1] = {
  [GTS_USR_B] = { .off = 0 },
  [RSV0] = { .off = 1 },
  [PERSIST] = { .off = 3 },
  [SBITS] = { .off = 4 },
  [RSV1] = { .off = 6 },
  [GLUTMASK_B] = { .off = 8 },
  [RSV2] = { .off = 9 },
  [ICAPSEL] = { .off = 30 },
  [CTL_FIELD_LAST] = { .off = 31 },
};

#endif

#define MSK(x) ((1<<x)-1)
static inline uint32_t
setfiel(const uint32_t val,
	const unsigned field,
	const bitfield_t bitfields[]) {
  unsigned offset = bitfields[field].off;
  unsigned flen = bitfields[field+1].off - offset;
  assert((val & MSK(flen)) == val);
  return val << offset;
}

#define COR_F(name, val) \
  setfiel(val, name, cor_bitfields)

#define CTL_F(name, val) \
  setfiel(val, name, ctl_bitfields)

#if defined(VIRTEX2) || defined (SPARTAN3)
#include "codes/crc-ibm.h"
#define crc_byte crc_ibm_byte
#define crc_addr5 crc_ibm_addr5
#else
#if defined (VIRTEX4)
#include "codes/crc32-c.h"
#define crc_byte crc32c_byte
#define crc_addr5 crc32c_addr5
#endif
#endif

/* crc update function, from host-interpreted data.
   TODO: Share with update function in parser. */
static inline void
update_crc_h(bitstream_writer_t *writer, const register_index_t reg,
	     const uint32_t *dat, const size_t wordc) {
  guint32 bcc = writer->crc;
  size_t i;

  for (i = 0; i < wordc; i++) {
    uint32_t val = dat[i];
    bcc = crc_byte(bcc, val);
    bcc = crc_byte(bcc, val >> 8);
    bcc = crc_byte(bcc, val >> 16);
    bcc = crc_byte(bcc, val >> 24);
    /* the 5 bits of register address */
    bcc = crc_addr5(bcc, reg);
  }

  /* write back the new CRC register */
  writer->crc = bcc;

  /* writes to the CRC should yield a zero value */
  if (reg == CRC) {
    debit_log(L_WRITE,"write to CRC register yielded %04x", bcc);
  }
}

static inline void
update_crc_w(bitstream_writer_t *writer,
	     const register_index_t reg, const uint32_t word) {
  update_crc_h(writer, reg, &word, 1);
}

/* crc update function from bitstream-ordered data */
static inline void
update_crc_b(bitstream_writer_t *bit,
	     const register_index_t reg,
	     const char *val, const size_t len) {
  guint32 bcc = bit->crc;
  size_t i;
  assert(len % 4 == 0);

  for (i = 0; i < len; i+=4) {
    bcc = crc_byte(bcc, val[i+3]);
    bcc = crc_byte(bcc, val[i+2]);
    bcc = crc_byte(bcc, val[i+1]);
    bcc = crc_byte(bcc, val[i+0]);
    /* the 5 bits of register address */
    bcc = crc_addr5(bcc, reg);
  }

  /* write back the new CRC register */
  bit->crc = bcc;

  /* writes to the CRC should yield a zero value. */
  if (reg == CRC) {
    debit_log(L_WRITE,"write to CRC register yielded %04x", bcc);
  }
}

/* Low-level packet write functions */

static inline void
write_pkt1(const int fd, const unsigned wordc, const unsigned rega, const io_type_t io) {
  const uint32_t pkt1 = build_pkt1(wordc, rega, (io == PKT_READ), (io == PKT_WRITE));
  write_u32(fd, pkt1);
}

static inline void
write_pkt2(const int fd, const unsigned wordc, const io_type_t io) {
  const uint32_t pkt2 = build_pkt2(wordc, (io == PKT_READ), (io == PKT_WRITE));
  write_u32(fd, pkt2);
}

static inline void
bs_write_noop(const int fd) {
  write_u32(fd, NOOP);
}

static inline void
bs_write_synchro(int fd) {
  write_u32(fd, (unsigned)-1);
  write_u32(fd, SYNCHRO);
}

/* High-level register operation functions */
static void
bs_write_wreg(const int fd, const cmd_pkt_ver_t type, const unsigned rega, const unsigned wordc) {
  switch(type) {
  case TYPE_V1:
    write_pkt1(fd, wordc, rega, PKT_WRITE);
    break;
  case TYPE_V2:
    write_pkt1(fd, 0, rega, PKT_WRITE);
    /* XXX hotfix for pkt2 generation: invert read and write.
       See in ug002 where the correct fix should happen */
    write_pkt2(fd, wordc, PKT_READ);
    break;
  }
  /* Register rega for future CRC updates */
}

static void
bs_write_wreg_data(bitstream_writer_t *writer,
		   const int fd, const unsigned rega, const unsigned wordc, const uint32_t *data) {
  /* Actually we could decide here on what type of write we'd need.
   * For now simply do an heuristic of what Xlx does.
   */
  const cmd_pkt_ver_t type = ((wordc >> V1_PKT_WORDC_LEN) == 0) ? TYPE_V1 : TYPE_V2;
  bs_write_wreg(fd, type, rega, wordc);
  /* Actually write the data, with host-to-bitstream reordering */
  write_words(fd, data, wordc);
  /* Update the running CRC */
  update_crc_h(writer, rega, data, wordc);
}

static void
bs_write_wreg_u32(bitstream_writer_t *writer,
		  const int fd, const unsigned rega, const uint32_t word) {
  bs_write_wreg_data(writer, fd, rega, 1, &word);
}

static inline void
bs_write_padding(bitstream_writer_t *writer, const unsigned rega, const unsigned count) {
  const int fd = writer->fd;
  unsigned i;
  for (i = 0; i < count; i++) {
    write_u32(fd, 0);
    update_crc_w(writer, rega, 0);
  }
}

static void
write_frame(const char *frame, guint type, guint index, guint frameidx, void *data) {
  bitstream_writer_t *writer = data;
  const chip_struct_t *chip = writer->bit->chip_struct;
  const unsigned frame_len = chip->framelen * sizeof(uint32_t);
  (void) type; (void) index; (void) frameidx;

  /* unlikely */
  if (!frame) {
    bs_write_padding(writer, FDRI, chip->framelen);
    return;
  }

  write_buf(writer->fd, frame, frame_len);
  update_crc_b(writer, FDRI, frame, frame_len);
}

#if defined(VIRTEX2) || defined(SPARTAN3)

/* Scatter-gather data write, for frames */

#define AUTOCRC_NONE 0xffff

static inline unsigned nframes(const chip_struct_t *chip_struct) {
  const unsigned *col_count = chip_struct->col_count;
  const unsigned *frame_count = chip_struct->frame_count;
  unsigned total = 0;
  int type;
  for (type = 0; type < V2C__NB_CFG; type++)
    total += frame_count[type] * col_count[type];
  return total;
}

#define S2U(x) GUINT32_FROM_BE(*((uint32_t *)x))

static void
bs_fdri_write_frames(bitstream_writer_t *writer) {
  int fd = writer->fd;
  const bitstream_parsed_t *bit = writer->bit;
  const chip_struct_t *chip = bit->chip_struct;
  /* Compute total word count */
  const unsigned framec = nframes(chip);
  const unsigned wordc = (framec + 1) * chip->framelen;

  /* prepare for FRDI write */
  /* FAR initialization. For non-compressed bitstream, it is set to be zero */
  bs_write_wreg_u32(writer, fd, FAR, 0);
  bs_write_wreg_u32(writer, fd, CMD, WCFG);

  /* Prepare long FDRI write */
  bs_write_wreg(fd, TYPE_V2, FDRI, wordc);

  /* simply write *all* frames, in FAR order */
  iterate_over_frames_far(bit, write_frame, writer);

  /* padding frame */
  bs_write_padding(writer, FDRI, chip->framelen);

  /* write AutoCRC word and update CRC accordingly */

  debit_log(L_WRITE,"ACRC is %04x", writer->crc);
  write_u32(fd, writer->crc);
  update_crc_w(writer, CRC, writer->crc);
  /* This yields zero in CRC register */
}

static void
bs_write_cmd_header(bitstream_writer_t *writer) {
  int fd = writer->fd;
  const bitstream_parsed_t *bit = writer->bit;
  const chip_struct_t *chip = bit->chip_struct;

  bs_write_wreg_u32(writer, fd, CMD, RCRC);
  /* reset the CRC appropriately */
  writer->crc = 0;

  bs_write_wreg_u32(writer, fd, FLR, chip->framelen-1);
  /* These values are meaningless for me now */
  bs_write_wreg_u32(writer, fd, COR,
		    COR_F(GWE_CYCLE, 5) | COR_F(GTS_CYCLE, 4) |
		    COR_F(LOCK_CYCLE, 7) | COR_F(MATCH_CYCLE, 7) |
		    COR_F(DONE_CYCLE, 3) | COR_F(OSCFSEL, 2));
  bs_write_wreg_u32(writer, fd, IDCODE, chip->idcode);
  bs_write_wreg_u32(writer, fd, MASK, 0);
  bs_write_wreg_u32(writer, fd, CMD, SWITCH);

  /* Start-of-write */
}

static void
bs_write_cmd_footer(bitstream_writer_t *writer) {
  int fd = writer->fd;
  const bitstream_parsed_t *bit = writer->bit;
  const chip_struct_t *chip = bit->chip_struct;
  unsigned i;
  (void) chip;
  /* All frames have been written */
  bs_write_wreg_u32(writer, fd, CMD, GRESTORE);
  bs_write_wreg_u32(writer, fd, CMD, DGHIGH_LFRM);

  /* Series of noop packets, waiting for flush of the last written
     frame */
  for (i = 0; i < chip->framelen; i++)
    bs_write_noop(fd);

  bs_write_wreg_u32(writer, fd, CMD, START);
  bs_write_wreg_u32(writer, fd, CTL, 0);

  /* CRC check. We should fuck this up big time intentionally. We don't
     want anyone to load our bitstreams for now... */
  debit_log(L_WRITE,"CRC is %04x", writer->crc);
  bs_write_wreg_u32(writer, fd, CRC, writer->crc);

  bs_write_wreg_u32(writer, fd, CMD, DESYNCH);
  for (i = 0; i < 4; i++)
    bs_write_noop(fd);
}

static void
bs_write_body(bitstream_writer_t *writer) {
  /* write all raw bitstream data to disk */
  bs_write_synchro(writer->fd);
  bs_write_cmd_header(writer);
  bs_fdri_write_frames(writer);
  bs_write_cmd_footer(writer);
}

#elif defined(VIRTEX4)

#include "design_v4.h"

static inline unsigned nframes(const chip_struct_t *chip) {
  const unsigned *frame_count = chip->frame_count;
  unsigned total = 0;
  design_col_t type;
  for (type = 0; type < V4C__NB_CFG; type++) {
    /* cast the type to a col_type_t */
    total += frame_count[type] * num_of_type(chip, type);
  }
  return total * (chip->row_count << 1);
}

static void
bs_write_cmd_header(bitstream_writer_t *writer) {
  int fd = writer->fd;
  const bitstream_parsed_t *bit = writer->bit;
  const chip_struct_t *chip = bit->chip_struct;
  int nop;

  /* V4 inserts noops */
  bs_write_noop(fd);

  bs_write_wreg_u32(writer, fd, CMD, RCRC);
  /* reset the CRC appropriately */
  writer->crc = 0;

  bs_write_noop(fd);
  bs_write_noop(fd);

  /* These values are meaningless for me now */
  bs_write_wreg_u32(writer, fd, COR,
		    COR_F(GWE_CYCLE, 5) | COR_F(GTS_CYCLE, 4) |
		    COR_F(LOCK_CYCLE, 7) | COR_F(MATCH_CYCLE, 0) |
		    COR_F(DONE_CYCLE, 3) | COR_F(OSCFSEL, 2));
  bs_write_wreg_u32(writer, fd, IDCODE, chip->idcode);
  bs_write_wreg_u32(writer, fd, CMD, SWITCH);
  bs_write_noop(fd);

  /* Set unknown bits */
  bs_write_wreg_u32(writer, fd, MASK, CTL_F(RSV2, 0x3));
  bs_write_wreg_u32(writer, fd, CTL, CTL_F(RSV2, 0x3));

  /* 1150 for xc4vlx100
   * 1150 for xc4vls40
   */
#define NOOPS_SYNC 1150
  for(nop = 0; nop < NOOPS_SYNC; nop++)
    bs_write_noop(fd);

  /* Clear those same bits */
  bs_write_wreg_u32(writer, fd, MASK, CTL_F(RSV2, 0x3));
  bs_write_wreg_u32(writer, fd, CTL, 0);

  bs_write_wreg_u32(writer, fd, CMD, C_NULL);
  bs_write_noop(fd);

  /* Start-of-write */
}

static void
bs_fdri_write_frames(bitstream_writer_t *writer) {
  int fd = writer->fd;
  const bitstream_parsed_t *bit = writer->bit;
  const chip_struct_t *chip = bit->chip_struct;
  /* Compute total word count */
  const unsigned framec = nframes(chip);
  const unsigned wordc = framec * chip->framelen;

  /* prepare for FRDI write */
  /* FAR initialization. For non-compressed bitstream, it is set to be zero */
  bs_write_wreg_u32(writer, fd, FAR, 0);
  bs_write_wreg_u32(writer, fd, CMD, WCFG);
  bs_write_noop(fd);

  /* Prepare long FDRI write */
  bs_write_wreg(fd, TYPE_V2, FDRI, wordc);

  /* simply write *all* frames, in FAR order. Along with noop padding */
  iterate_over_frames_far(bit, write_frame, writer);

  /* explicit CRC check for v4 */
  bs_write_wreg_u32(writer, fd, CRC, writer->crc);
}

static void
bs_write_cmd_footer(bitstream_writer_t *writer) {
  int fd = writer->fd;
  const bitstream_parsed_t *bit = writer->bit;
  const chip_struct_t *chip = bit->chip_struct;
  unsigned i;
  (void) chip;
  /* All frames have been written */

  bs_write_wreg_u32(writer, fd, CMD, GRESTORE);
  bs_write_noop(fd);

  bs_write_wreg_u32(writer, fd, CMD, C_LFRM);
  bs_write_noop(fd);

  /* Series of noop packets, waiting for flush of the last written
     frame */
  for (i = 0; i < chip->framelen; i++)
    bs_write_noop(fd);

  bs_write_wreg_u32(writer, fd, CMD, GRESTORE);
  bs_write_noop(fd);

  bs_write_wreg_u32(writer, fd, CMD, C_NULL);
  bs_write_noop(fd);

  bs_write_wreg_u32(writer, fd, FAR, 0xffff);

  bs_write_wreg_u32(writer, fd, CMD, START);
  bs_write_noop(fd);

  bs_write_wreg_u32(writer, fd, MASK, 0xffff);
  bs_write_wreg_u32(writer, fd, CTL, 0xffff);

  /* CRC check. We should fuck this up big time intentionally. We don't
     want anyone to load our bitstreams for now... */
  debit_log(L_WRITE,"CRC is %04x", writer->crc);
  bs_write_wreg_u32(writer, fd, CRC, writer->crc);

  bs_write_wreg_u32(writer, fd, CMD, DESYNCH);
  for (i = 0; i < 16; i++)
    bs_write_noop(fd);
}

static void
bs_write_body(bitstream_writer_t *writer) {
  /* write all raw bitstream data to disk */
  bs_write_synchro(writer->fd);
  bs_write_cmd_header(writer);
  bs_fdri_write_frames(writer);
  bs_write_cmd_footer(writer);
}

#endif

int
bitstream_write(const bitstream_parsed_t *bit,
		const char *output_dir,
		const char *ofile) {
  int file, data, err = 0;
  bitstream_writer_t writer = { .bit = bit };
  gchar *tmpname = NULL;
  (void) output_dir;

  /* open bitstream data file */
  data = g_file_open_tmp(NULL,&tmpname,NULL);
  if (data < 0)
    return data;
  debit_log(L_WRITE,"Temporary datafile is %s", tmpname);
  writer.fd = data;

  /* unlink the temp file from the filesystem early if we can directly
     mmap from a fd instead of a filename */
#ifdef HAVE_MMAP
  (void) unlink(tmpname);
  g_free(tmpname);
  tmpname = NULL;
#endif /* HAVE_MMAP */

  bs_write_body(&writer);

  /* open bitstream itself */
  file = g_open(ofile, O_CREAT | O_TRUNC | O_NDELAY | O_WRONLY, S_IRWXU);
  if (file < 0) {
    err = file;
    perror("Opening bitstream file");
    goto err_data;
  }

  /* Close the fd_data so that data will appear in mmap */
#ifndef HAVE_MMAP
  err = close(data);
  data = -1;
  if (err) {
    perror("Closing tmp data file");
    goto err_data;
  }
#endif

  /* Prepend header to raw bitstream data */
  bs_add_header(&writer, file, data, tmpname);
  err = close(file);
  if (err) {
    perror("Closing bitstream file");
    goto err_data;
  }

 err_data:
#ifdef HAVE_MMAP
  err = close(data);
  data = -1;
  if (err) {
    perror("Closing bitstream file");
    goto err_data;
  }
#else
  (void) unlink(tmpname);
  g_free(tmpname);
  tmpname = NULL;
#endif /* HAVE_MMAP */
  return err;
}


/* XXX no FreezeDCI option for now */
