#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <unistd.h>

#include "bitstream_parser.h"
#include "codes/crc-ibm.h"
#include "bitheader.h"
#include "bitstream_packets.h"
#include "design.h"

/* State necessary for bitstream computation */
typedef struct _bitstream_writer {
  int fd;
  uint32_t crc;
} bitstream_writer_t;

static inline void write_u8(int fd, const uint8_t dat) {
  (void) write(fd, &dat, sizeof(dat));
}

static inline void write_u16(int fd, const uint16_t dat) {
  const uint16_t wdat = GUINT16_TO_BE(dat);
  (void) write(fd, &wdat, sizeof(wdat));
}

static inline void write_u32(int fd, const uint32_t dat) {
  const uint32_t wdat = GUINT32_TO_BE(dat);
  (void) write(fd, &wdat, sizeof(wdat));
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
  (void) write(fd, buf, len);
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
  write(fd, payload, length);
}

static void
bs_write_long_option(const int fd, const uint8_t code,
		     const void *payload, const uint32_t length) {
  write_u8(fd,code);
  write_u32(fd,length);
  write(fd, payload, length);
}

static void
bs_write_header(const int fd) {
  const char *filename = "essai.ncd";
  const char *device = "2v40cs144";
  const char *date = "2006/10/ 5";
  const char *time = "06:06:06";
  bs_write_option(fd, FILENAME, filename, strlen(filename)+1);
  bs_write_option(fd, DEVICE_TYPE, device, strlen(filename)+1);
  bs_write_option(fd, BUILD_DATE, date, strlen(date)+1);
  bs_write_option(fd, BUILD_TIME, time, strlen(time)+1);
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

static void
bs_write_data(int fd, int fd_data) {
  struct stat statistics;
  void *data_buf;
  off_t size;

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

/* Only depends on ? */
/* A bit complicated, as this must be computed after the file output */
static void
bs_add_header(const bitstream_writer_t *writer, int fd, int data) {
  (void) writer;
  bs_write_magic(fd);
  bs_write_header(fd);
  bs_write_data(fd, data);
}

/* packet writing functions */

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

/* crc update function, from host-interpreted data.
   TODO: Share with update function in parser. */
static inline void
update_crc_h(bitstream_writer_t *writer, const register_index_t reg,
	     const uint32_t *dat, const size_t wordc) {
  guint32 bcc = writer->crc;
  size_t i;

  for (i = 0; i < wordc; i++) {
    uint32_t val = dat[i];
    bcc = crc_ibm_byte(bcc, val);
    bcc = crc_ibm_byte(bcc, val >> 8);
    bcc = crc_ibm_byte(bcc, val >> 16);
    bcc = crc_ibm_byte(bcc, val >> 24);
    /* the 5 bits of register address */
    bcc = crc_ibm_addr5(bcc, reg);
  }

  /* write back the new CRC register */
/*   thing->bcc = bcc; */

  /* writes to the CRC should yield a zero value */
  if (reg == CRC)
    debit_log(L_BITSTREAM,"write to CRC register yielded %04x", bcc);
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

  for (i = 0; i < len; i++) {
    bcc = crc_ibm_byte(bcc, val[i]);
    bcc = crc_ibm_byte(bcc, val[i+1]);
    bcc = crc_ibm_byte(bcc, val[i+2]);
    bcc = crc_ibm_byte(bcc, val[i+3]);
    /* the 5 bits of register address */
    bcc = crc_ibm_addr5(bcc, reg);
  }

  /* write back the new CRC register */
  //  crcreg->value = bcc;

  /* writes to the CRC should yield a zero value. */
  if (reg == CRC)
    debit_log(L_BITSTREAM,"write to CRC register yielded %04x", bcc);
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
    write_pkt2(fd, wordc, PKT_WRITE);
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

/* Scatter-gather data write, for frames */

#define AUTOCRC_NONE 0xffff

/*
typedef void (*frame_iterator_t)(const char *frame,
				 guint type,
				 guint index,
				 guint frameidx,
				 void *data);
 */

static void
write_frame(const char *frame, guint type, guint index, guint frameidx, void *data) {
  bitstream_writer_t *writer = data;
  /* XXX */
  unsigned frame_len = 25;
  (void) type; (void) index; (void) frameidx;
  write_buf(writer->fd, frame, frame_len);
  //  update_crc_b(writer, FDRI, frame, frame_len);
}

static void
bs_fdri_write_frames(bitstream_writer_t *bit, int fd,
		     const bitstream_parsed_t *parsed) {
  bitstream_writer_t bitstream_writer;
  /* Compute total word count */
  unsigned wordc = 2000;

  /* prepare for FRDI write */
  bs_write_wreg(fd, TYPE_V2, FDRI, wordc);

  /* simply write *all* frames, to be refined */
  iterate_over_frames(parsed, write_frame, &bitstream_writer);

  /* Update CRC */

  /* write AutoCRC word and update CRC accordingly */
  update_crc_w(bit, CRC, bit->crc);
  write_u32(fd, bit->crc);

}

#define S2U(x) GUINT32_FROM_BE(*((uint32_t *)x))

static void
bs_write_cmd_header(bitstream_writer_t *writer,
		    int fd, const bitstream_parsed_t *bit) {
  const chip_struct_t *chip = bit->chip_struct;
  /* CRC reset */
  bs_write_wreg_u32(writer, fd, CMD, RCRC);
  /* FLR setting -- from bitstream type */
  bs_write_wreg_u32(writer, fd, FLR, chip->framelen-1);
  /* COR setting, default value */
  bs_write_wreg_u32(writer, fd, COR, S2U("cor"));
  /* IDCODE setting */
  bs_write_wreg_u32(writer, fd, IDCODE, chip->idcode);
  /* MASK setting (what is this ?) */
  bs_write_wreg_u32(writer, fd, MASK, S2U("mask"));
  /* CMD first command */
  bs_write_wreg_u32(writer, fd, CMD, SWITCH);

  /* Start-of-write */
  /* FAR initialization. For non-compressed bitstream, it should be zero */
  bs_write_wreg_u32(writer, fd, FAR, S2U("far"));
  /* CMD second command */
  bs_write_wreg_u32(writer, fd, CMD, WCFG);
  /* Then the data, into the FDRI */
}

static void
bs_write_cmd_footer(bitstream_writer_t *writer, int fd, const bitstream_parsed_t *bit) {
  const chip_struct_t *chip = bit->chip_struct;
  unsigned i;
  (void) chip;
  /* All frames have been written */
  bs_write_wreg_u32(writer, fd, CMD, GRESTORE);
  bs_write_wreg_u32(writer, fd, CMD, DGHIGH_LFRM);
  /* Series of noop packets. How many are there ? */
  for (i = 0; i < 26; i++)
    bs_write_noop(fd);
  /* Again, commands */
  bs_write_wreg_u32(writer, fd, CMD, START);
  bs_write_wreg_u32(writer, fd, CTL, 0xffff);
  /* CRC check, if need be. We fuck this up big time intentionally. We
     don't want anyone to load our bitstreams for now... */
  bs_write_wreg_u32(writer, fd, CRC, 0xffff);
  bs_write_wreg_u32(writer, fd, CMD, DESYNCH);
  /* Last series of noops */
  for (i = 0; i < 4; i++)
    bs_write_noop(fd);
}

int
bitstream_write(const bitstream_parsed_t *bit,
		const char *output_dir,
		const char *ofile) {
  int file, data, err = 0;
  bitstream_writer_t writer;
  gchar *tmpname = NULL;
  (void) output_dir;

  /* open bitstream data file */
  data = g_file_open_tmp(NULL,&tmpname,NULL);
  if (data < 0)
    return data;
  writer.fd = data;

  /* unlink it from the filesystem */
  unlink(tmpname);
  g_free(tmpname);

  bs_write_synchro(data);
  bs_write_cmd_header(&writer, data, bit);
  bs_fdri_write_frames(&writer, data, bit);
  bs_write_cmd_footer(&writer, data, bit);

  /* open bitstream itself */
  file = open(ofile, O_CREAT | O_NONBLOCK | O_WRONLY, S_IRWXU | S_IRWXG);
  if (file < 0) {
    err = file;
    goto err_data;
  }

  /* Then assemble the whole thing */
  bs_add_header(&writer, file, data);

  close(file);
 err_data:
  close(data);
  return err;
}


/* XXX no FreezeDCI option for now */
