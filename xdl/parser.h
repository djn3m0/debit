#ifndef _HAS_PARSER_H
#define _HAS_PARSER_H

#include <assert.h>
#include "localpips.h"
#include "sites.h"

#define PROP_MAX 5

typedef struct _parser_t {
/* 	yyscan_t scanner; */
  const char *datadir;
  const pip_db_t *pipdb;
  const chip_descr_t *chip;
  /* written by the scanner */
  unsigned pip_counter;
  /* yummy bitstream */
  bitstream_parsed_t bit;

  /* Very small state */
  const csite_descr_t *current_site;
  slice_index_t slice_idx;

  /* Properties */
  /* first propery should be decoded in the lexer, others are duplicated
     in the array. In an ideal world, the last property would be looked
     up in the lexer too, avoiding yet another string copy */
  //unsigned propid;
  int propidx;
  char *proplist[PROP_MAX];
} parser_t;

static inline void
free_properties(parser_t *parser) {
  const unsigned idmax = parser->propidx;
  char **proplist = parser->proplist;
  unsigned i;
  for (i = 0; i < idmax; i++) {
    if (!proplist[i])
      continue;
    free(proplist[i]);
    proplist[i] = NULL;
  }
  parser->propidx = 0;
}

static inline char *
peek_property(const parser_t *parser) {
  return parser->proplist[parser->propidx - 1];
}

static inline char *
pop_property(parser_t *parser) {
  char *p = parser->proplist[--parser->propidx];
  assert(parser->propidx >= 0);
  return p;
}

static inline void
push_property(parser_t *parser, char *p) {
  debit_log(L_PARSER, "pushing config val %s", p);
  parser->proplist[parser->propidx++] = p;
  assert(parser->propidx <= PROP_MAX);
}

static inline void
free_parser(parser_t *parser) {
	chip_descr_t *chip = (void *)parser->chip;
	pip_db_t *pipdb= (void *)parser->pipdb;
	if (pipdb) {
		parser->pipdb = NULL;
		free_pipdb(pipdb);
	}
	if (chip) {
		parser->chip = NULL;
		release_chip(chip);
	}
	/* XXX Free bitstream, header */
}

#endif /* _HAS_PARSER_H */
