#ifndef _HAS_PARSER_H
#define _HAS_PARSER_H

#include "localpips.h"

typedef struct _parser_t {
/* 	yyscan_t scanner; */
	const char *datadir;
	/* written by the scanner */
	unsigned pip_counter;
	pip_db_t *pipdb;
	/* chip descr too */
	char *design;
} parser_t;

static inline void
free_parser(parser_t *parser) {
	pip_db_t *pipdb= parser->pipdb;
	char *design= parser->design;
	if (design) {
		parser->design = NULL;
		free(design);
	}
	if (pipdb) {
		parser->pipdb = NULL;
		free_pipdb(pipdb);
	}
}

#endif /* _HAS_PARSER_H */
