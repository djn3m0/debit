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
	char *design;
} parser_t;

static inline void
free_parser(parser_t *parser) {
	chip_descr_t *chip = (void *)parser->chip;
	pip_db_t *pipdb= (void *)parser->pipdb;
	char *design= (void *)parser->design;
	if (design) {
		parser->design = NULL;
		free(design);
	}
	if (pipdb) {
		parser->pipdb = NULL;
		free_pipdb(pipdb);
	}
	if (chip) {
		parser->chip = NULL;
		release_chip(chip);
	}
}

#endif /* _HAS_PARSER_H */
