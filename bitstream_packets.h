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

typedef enum _cmd_pkt_ver {
  TYPE_V1 = 1, TYPE_V2 = 2,
} cmd_pkt_ver_t;

typedef enum _special_words {
  SYNCHRO = 0xAA995566U,
  NOOP    = 0x20000000U,
  NULLPKT = 0x00000000U,
} special_word_t;

typedef struct _xil_register {
  guint32 value;
} xil_register_t;

/* Building packets */

static inline uint32_t
build_pkt2(const unsigned wordc, const unsigned rd, const unsigned wr) {
  return (
    ((TYPE_V2 << V2_PKT_TYPE_OFFSET) & V2_PKT_TYPE_MASK) |
    ((wr << V2_PKT_WR_OFFSET) & V2_PKT_WR_MASK) |
    ((rd << V2_PKT_RD_OFFSET) & V2_PKT_RD_MASK) |
    ((wordc << V2_PKT_WORDC_OFFSET) & V2_PKT_WORDC_MASK)
    );
}

static inline uint32_t
build_pkt1(const unsigned wordc, const unsigned rega, const unsigned rd, const unsigned wr) {
  return (
    ((TYPE_V1 << V1_PKT_TYPE_OFFSET) & V1_PKT_TYPE_MASK) |
    ((wr << V1_PKT_WR_OFFSET) & V1_PKT_WR_MASK) |
    ((rd << V1_PKT_RD_OFFSET) & V1_PKT_RD_MASK) |
    ((rega << V1_PKT_REGA_OFFSET) & V1_PKT_REGA_MASK) |
    ((wordc << V1_PKT_WORDC_OFFSET) & V1_PKT_WORDC_MASK)
    );
}

#endif /* _BITSTREAM_PACKETS_H */
