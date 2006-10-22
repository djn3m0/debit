/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

#ifndef _BITSTREAM_PACKETS_H
#define _BITSTREAM_PACKETS_H

/*
 * V1 packet type
 */

typedef struct {
  unsigned word_count :11;
  unsigned __rsvd :2;
  unsigned reg_addr :14;
  unsigned rd :1;
  unsigned wr :1;
  unsigned type :3;
} v2_packet1_t;

#define V1_PKT_WORDC_OFFSET 0
#define V1_PKT_WORDC_LEN 11
#define V1_PKT_WORDC_MASK ((1<<V1_PKT_WORDC_LEN) - 1) << V1_PKT_WORDC_OFFSET

#define V1_PKT_RSVD_OFFSET (V1_PKT_WORDC_OFFSET + V1_PKT_WORDC_LEN)
#define V1_PKT_RSVD_LEN 2
#define V1_PKT_RSVD_MASK ((1<<V1_PKT_RSVD_LEN) - 1) << V1_PKT_RSVD_OFFSET

#define V1_PKT_REGA_OFFSET (V1_PKT_RSVD_OFFSET + V1_PKT_RSVD_LEN)
#define V1_PKT_REGA_LEN 14
#define V1_PKT_REGA_MASK ((1<<V1_PKT_REGA_LEN) - 1) << V1_PKT_REGA_OFFSET

#define V1_PKT_RD_OFFSET (V1_PKT_REGA_OFFSET + V1_PKT_REGA_LEN)
#define V1_PKT_RD_LEN 1
#define V1_PKT_RD_MASK ((1<<V1_PKT_RD_LEN) - 1) << V1_PKT_RD_OFFSET

#define V1_PKT_WR_OFFSET (V1_PKT_RD_OFFSET + V1_PKT_RD_LEN)
#define V1_PKT_WR_LEN 1
#define V1_PKT_WR_MASK ((1<<V1_PKT_WR_LEN) - 1) << V1_PKT_WR_OFFSET

#define V1_PKT_TYPE_OFFSET (V1_PKT_WR_OFFSET + V1_PKT_WR_LEN)
#define V1_PKT_TYPE_LEN 3
#define V1_PKT_TYPE_MASK ((1<<V1_PKT_TYPE_LEN) - 1) << V1_PKT_TYPE_OFFSET

static inline unsigned
wordc_of_pkt1(const uint32_t pkt1) {
  return (pkt1 & V1_PKT_WORDC_MASK) >> V1_PKT_WORDC_OFFSET;
}

static inline unsigned
rsvd_of_pkt1(const uint32_t pkt1) {
  return (pkt1 & V1_PKT_RSVD_MASK) >> V1_PKT_RSVD_OFFSET;
}

static inline unsigned
rega_of_pkt1(const uint32_t pkt1) {
  return (pkt1 & V1_PKT_REGA_MASK) >> V1_PKT_REGA_OFFSET;
}

static inline unsigned
rd_of_pkt1(const uint32_t pkt1) {
  return (pkt1 & V1_PKT_RD_MASK) >> V1_PKT_RD_OFFSET;
}

static inline unsigned
wr_of_pkt1(const uint32_t pkt1) {
  return (pkt1 & V1_PKT_WR_MASK) >> V1_PKT_WR_OFFSET;
}

static inline unsigned
type_of_pkt1(const uint32_t pkt1) {
  return (pkt1 & V1_PKT_TYPE_MASK) >> V1_PKT_TYPE_OFFSET;
}

/*
 * V2 packet type
 */
typedef struct {
  unsigned word_count :27;
  unsigned rd :1;
  unsigned wr :1;
  unsigned type :3;
} v2_packet2_t;

#define V2_PKT_WORDC_OFFSET 0
#define V2_PKT_WORDC_LEN 27
#define V2_PKT_WORDC_MASK ((1<<V2_PKT_WORDC_LEN) - 1) << V2_PKT_WORDC_OFFSET

#define V2_PKT_WR_OFFSET (V2_PKT_WORDC_OFFSET + V2_PKT_WORDC_LEN)
#define V2_PKT_WR_LEN 1
#define V2_PKT_WR_MASK ((1<<V2_PKT_WR_LEN) - 1) << V2_PKT_WR_OFFSET

#define V2_PKT_RD_OFFSET (V2_PKT_WR_OFFSET + V2_PKT_WR_LEN)
#define V2_PKT_RD_LEN 1
#define V2_PKT_RD_MASK ((1<<V2_PKT_RD_LEN) - 1) << V2_PKT_RD_OFFSET

#define V2_PKT_TYPE_OFFSET (V2_PKT_RD_OFFSET + V2_PKT_RD_LEN)
#define V2_PKT_TYPE_LEN 3
#define V2_PKT_TYPE_MASK ((1<<V2_PKT_TYPE_LEN) - 1) << V2_PKT_TYPE_OFFSET

static inline unsigned
wordc_of_v2pkt(const uint32_t v2pkt) {
  return (v2pkt & V2_PKT_WORDC_MASK) >> V2_PKT_WORDC_OFFSET;
}

static inline unsigned
rd_of_v2pkt(const uint32_t v2pkt) {
  return (v2pkt & V2_PKT_RD_MASK) >> V2_PKT_RD_OFFSET;
}

static inline unsigned
wr_of_v2pkt(const uint32_t v2pkt) {
  return (v2pkt & V2_PKT_WR_MASK) >> V2_PKT_WR_OFFSET;
}

static inline unsigned
type_of_v2pkt(const uint32_t v2pkt) {
  return (v2pkt & V2_PKT_TYPE_MASK) >> V2_PKT_TYPE_OFFSET;
}

#endif /* _BITSTREAM_PACKETS_H */
