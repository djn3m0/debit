#ifndef _HAS_PARSER_H
#define _HAS_PARSER_H

#include "localpips.h"
#include "sites.h"

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
} parser_t;

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
