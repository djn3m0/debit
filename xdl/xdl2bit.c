#include <stdio.h>
#include "xdl_parser.h"
#include "parser.h"

extern int yyparse (void *);

void yyerror(char *err) {
	fprintf(stderr, "%s", err);
}

extern int yydebug;

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;
	parser_t parser = { .pip_counter = 0 };

	yyparse(&parser);

	printf("Processed %u pips\n", parser.pip_counter);

	return 0;
}
