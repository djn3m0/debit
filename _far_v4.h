static const unsigned
v4_frame_count[V4C__NB_CFG] = {
  [V4C_IOB] = 30,
  [V4C_GCLK] = 3,
  [V4C_CLB] = 22,
  [V4C_DSP48] = 21,
  [V4C_BRAM] = 64,
  [V4C_BRAM_INT] = 20,
  /* the padding frames are seen as a compulsory column of two frames at
     the end of any row for all three types of columns */
  [V4C_PAD] = 2,
};

static const
chip_struct_t bitdescr[XC4VLX__NUM] = {
  /* FLR is always 41 for virtex-4 */
  [XC4VLX15] = { .idcode = 0x01658093U,
		 .chip = XC4VLX15,
		 .framelen = 41,
		 .frame_count = v4_frame_count,
		 .col_count = {
		   [V4_TYPE_CLB] = 29,
		   [V4_TYPE_BRAM] = 3,
		   [V4_TYPE_BRAM_INT] = 3,
		 },
		 .row_count = 2, },
  [XC4VLX25] = { .idcode = 0x0167C093,
		 .chip = XC4VLX25,
		 .framelen = 41,
		 .frame_count = v4_frame_count,
		 .col_count = {
		   [V4_TYPE_CLB] = 33,
		   [V4_TYPE_BRAM] = 3,
		   [V4_TYPE_BRAM_INT] = 3,
		 },
		 .row_count = 3, },
  [XC4VLX40] = { .idcode = 0x016A4093U,
		 .chip = XC4VLX40,
		 .framelen = 41,
		 .frame_count = v4_frame_count,
		 .col_count = {
		   [V4_TYPE_CLB] = 41,
		   [V4_TYPE_BRAM] = 3,
		   [V4_TYPE_BRAM_INT] = 3,
		 },
		 .row_count = 4, },
  [XC4VLX60] = { .idcode = 0x016B4093U,
		 .chip = XC4VLX60,
		 .framelen = 41,
		 .frame_count = v4_frame_count,
		 .col_count = {
		   [V4_TYPE_CLB] = 57,
		   [V4_TYPE_BRAM] = 5,
		   [V4_TYPE_BRAM_INT] = 5,
		 },
		 .row_count = 4, },
  [XC4VLX80] = { .idcode = 0x016D8093U,
		 .chip = XC4VLX80,
		 .framelen = 41,
		 .frame_count = v4_frame_count,
		 .col_count = {
		   [V4_TYPE_CLB] = 61,
		   [V4_TYPE_BRAM] = 5,
		   [V4_TYPE_BRAM_INT] = 5,
		 },
		 .row_count = 5, },
  [XC4VLX100] = { .idcode = 0x01700093U,
		  .chip = XC4VLX100,
		  .framelen = 41,
		  .frame_count = v4_frame_count,
		  .col_count = {
		    [V4_TYPE_CLB] = 69,
		    [V4_TYPE_BRAM] = 5,
		    [V4_TYPE_BRAM_INT] = 5,
		  },
		  .row_count = 6, },
  [XC4VLX160] = { .idcode = 0x01718093U,
		  .chip = XC4VLX160,
		  .framelen = 41,
		  .frame_count = v4_frame_count,
		  .col_count = {
		    [V4_TYPE_CLB] = 93,
		    [V4_TYPE_BRAM] = 6,
		    [V4_TYPE_BRAM_INT] = 6,
		  },
		  .row_count = 6, },
  [XC4VLX200] = { .idcode = 0x01734093U,
		  .chip = XC4VLX200,
		  .framelen = 41,
		  .frame_count = v4_frame_count,
		  .col_count = {
		    [V4_TYPE_CLB] = 121,
		    [V4_TYPE_BRAM] = 7,
		    [V4_TYPE_BRAM_INT] = 7,
		  },
		  .row_count = 6, },
};

typedef enum _ba_v4_col_type {
  BA_TYPE_CLB = 0,
  BA_TYPE_BRAM_INT,
  BA_TYPE_BRAM,
} ba_v4_col_type_t;

typedef enum parser_state {
  STATE_IDLE = 0,
  STATE_UNSYNCHED,
  STATE_WAITING_CTRL,
  STATE_WAITING_DATA,
} parser_state_t;

typedef enum _v4_registers_index {
  /* from xilinx ug071.pdf */
  CRC = 0, FAR, FDRI, FDRO,
  CMD, CTL, MASK,
  STAT, LOUT, COR,
  MFWR, CBC, IDCODE,
  AXSS,
  __V4_NUM_REGISTERS,
} register_index_t;

typedef enum _cmd_code {
  CMD_NULL = 0,
  WCFG, MFW,
  LFRM, RCFG, START, RCAP, RCRC,
  AGHIGH, SWITCH, GRESTORE,
  SHUTDOWN, GCAPTURE,
  DESYNCH,
  __NUM_CMD_CODE,
} cmd_code_t;

#if DEBIT_DEBUG > 0
static const
char *reg_names[__V4_NUM_REGISTERS] = {
  [CRC] = "CRC",
  [FAR] = "FAR",
  [FDRI] = "FDRI",
  [FDRO] = "FDRO",
  [CMD] = "CMD",
  [CTL] = "CTL",
  [MASK] = "MASK",
  [STAT] = "STAT",
  [LOUT] = "LOUT",
  [COR] = "COR",
  [MFWR] = "MFWR",
  [CBC] = "CBC",
  [IDCODE] = "IDCODE",
  [AXSS] = "AXSS",
};

static const
char *cmd_names[__NUM_CMD_CODE] = {
  [CMD_NULL] = "NULL",
  [WCFG] = "WCFG",
  [MFW] = "MFWR",
  [LFRM] = "LFRM",
  [RCFG] = "RCFG",
  [START] = "START",
  [RCAP] = "RCAP",
  [RCRC] = "RCRC",
  [AGHIGH] = "AGHIGH",
  [SWITCH] = "SWITCH",
  [GRESTORE] = "GRESTORE",
  [SHUTDOWN] = "SHUTDOWN",
  [GCAPTURE] = "GCAPTURE",
  [DESYNCH] = "DESYNCH",
};
#endif

#define sw_far_t sw_far_v4_t
#define id_vlx_t id_v4vlx_t
#define col_type_t v4_col_type_t
#define design_col_t v4_design_col_t
#define XC_VLX__NUM XC4VLX__NUM
#define VC__NB_CFG V4C__NB_CFG
#define get_hwfar get_hwfar_v4
#define fill_swfar fill_swfar_v4
#define frame_count_v v4_frame_count

/* This structure contains the internal structure of the parser */
typedef struct _bitstream_parser {
  /* state of the parser */
  parser_state_t state;
  /* state of the register engine */
  register_index_t active_register;
  gint write__read;
  gint active_length;

  /* Actual state of the various registers */
  xil_register_t registers[__V4_NUM_REGISTERS];

  /* detailed view of some registers */
  id_vlx_t type;

  /* Specific FDRI quirks */
  const void *last_frame;

  /* Bitstream proper */
  bytearray_t ba;
} bitstream_parser_t;

/*
 * Length, in words, of a frame.
 */

#define frame_length 41

/***
 * FAR handling
 */

static inline void _far_increment_type(sw_far_t *addr);
static inline gboolean _far_is_pad(const bitstream_parser_t *bitstream,
				   const sw_far_t *addr);

static inline void
_far_increment_tb(sw_far_t *addr) {
  v4_tb_t tb = addr->tb;
  switch (tb) {
  case V4_TB_TOP:
    addr->tb = V4_TB_BOTTOM;
    break;
  case V4_TB_BOTTOM:
    addr->tb = V4_TB_TOP;
    /* increase type */
    _far_increment_type(addr);
    break;
  default:
    g_assert_not_reached();
  }
}

int
snprintf_far(char *buf, const size_t buf_len,
	     const uint32_t hwfar) {
  return snprintf(buf, buf_len,
		  "%i_%i_%i_%i_%i",
		  v4_tb_of_far(hwfar),
		  v4_type_of_far(hwfar),
		  v4_row_of_far(hwfar),
		  v4_col_of_far(hwfar),
		  v4_mna_of_far(hwfar));
}

/* Type is a bit strange -- it watches the type from the far and the col
   count, so as to get a mixed type which depends on both these
   parameters. */
#define DSP_V4_OF_END(x) ((x) > 50 ? 13 : 9)

static inline design_col_t
_type_of_far(const bitstream_parser_t *bitstream,
	     const sw_far_t *addr) {
  const id_vlx_t chiptype = bitstream->type;
  const col_type_t type = addr->type;

  /* unlikely */
  if (_far_is_pad(bitstream, addr))
    return V4C_PAD;

  switch (type) {
  case V4_TYPE_CLB:
    {
      const unsigned col = addr->col;
      const unsigned end = bitdescr[chiptype].col_count[V4_TYPE_CLB] - 1;
      const unsigned middle = end >> 1;
      const unsigned dsp = DSP_V4_OF_END(end);
      /* Let's be more intelligent.
	 Middle, extremities: IO.
	 DSP is at fixed position and the rest is CLB */
      if (col == 0 || col == end || col == middle)
	return V4C_IOB;
      if (col == (middle + 1))
	return V4C_GCLK;
      if (col == dsp)
	return V4C_DSP48;
      return V4C_CLB;
    }
  case V4_TYPE_BRAM:
    return V4C_BRAM;
  case V4_TYPE_BRAM_INT:
    return V4C_BRAM_INT;
  case V4_TYPE_CFG_CLB:
  case V4_TYPE_CFG_BRAM:
    g_print("Unknown frame type, please report your bitstream");
  default:
    g_assert_not_reached();
  }

  return -1;
}

static inline unsigned
_typed_col_of_far(const bitstream_parser_t *bitstream,
		  const sw_far_t *addr) {
  const id_vlx_t chiptype = bitstream->type;
  const col_type_t type = addr->type;
  const unsigned col = addr->col;

  /* unlikely, move this out of main exec flow */
  if (_far_is_pad(bitstream, addr))
    return type;

  switch (type) {
  case V4_TYPE_CLB:
    {
      const unsigned col = addr->col;
      const unsigned end = bitdescr[chiptype].col_count[V4_TYPE_CLB] - 1;
      const unsigned middle = end >> 1;
      const unsigned dsp = DSP_V4_OF_END(end);

      if (col == 0)
	return 0;
      if (col == dsp)
	return 0;
      if (col == middle)
	return 1;
      if (col == end)
	return 2;
      if (col == (middle + 1))
	return 0;
      if (col < dsp)
	return col-1;
      if (col < middle)
	return col-2;
      return col-4;
    }
  default:
    return col;
  }

  return -1;
}
