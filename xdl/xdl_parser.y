/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

/* XDL Parser */

%{

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "wiring.h"

#define YYPARSE_PARAM yyparm

void yyerror(char *err) {
	fprintf(stderr, "XDL parser error: %s", err);
}

static void treat_pip(parser_t *parser,
		      const char *start,
		      const char *end,
		      const char *site) {
	const wire_db_t *wdb = parser->pipdb->wiredb;
	int err;
	wire_atom_t swire, ewire;
	parser->pip_counter++;

	/* Obviously this parsing should be factored in
	   the lexer, before we even have to duplicate and pass strings
	   around */
	err = parse_wire_simple(wdb, &swire, start);
	if (err) {
		printf("unknown wire %s @%s\n", start, site);
		goto out_err;
	}
	err = parse_wire_simple(wdb, &ewire, end);
	if (err) {
		printf("unknown wire %s @%s\n", end, site);
		goto out_err;
	}

	printf("pip %s to %s @ %s\n", wire_name(wdb,swire), wire_name(wdb,ewire), site);
out_err:
	return;
}

static void treat_design(parser_t *parser,
			 const char *design_name,
			 const char *device) {
	/* Load the wire/pip db at this time. Obviously, should use the
	 * type of device here... */
	parser->pipdb = get_pipdb(parser->datadir);
	parser->design = strdup(design_name);
	printf("Design %s on device %s",design_name,device);
}

%}

/* Options */
%pure-parser

%union {
	char *name;
}

/* Bison declaration */
%token STRING
%token IDENTIFIER
%token VERSION
%token DESIGN

%token INSTANCE
%token PLACED
%token UNPLACED
%token TOK_QUOTE
%token TOK_CFG_SEP
%token TOK_WS

%token NET
%token INPIN
%token OUTPIN
%token PIP
%token CONFIG
%token CONNECTION

%type <name> IDENTIFIER
%type <name> wire0
%type <name> wire1
%type <name> tile
%type <name> part
%type <name> STRING
%type <name> design_name

%% /* Grammar follows */

input:   /* empty */
       | design instancelist netlist
;

/* Xilinx file format is really dumb... the comas are not even handled */
/* properly. And i'm not gonna change that. */

/* Design */
design_name: STRING ;
part: IDENTIFIER ;
ncd_version: VERSION ;

device: IDENTIFIER ;
package: IDENTIFIER ;
speed: IDENTIFIER ;

design_header: DESIGN design_name part ncd_version { treat_design(yyparm, $2, $3); free($2); free($3); }
             | DESIGN design_name device package speed ncd_version ;

design: design_header ',' config ';' ;

/* Instances */
name: STRING ;
sitedef: STRING ;
tile: IDENTIFIER { $$ = $1; } ;
site: IDENTIFIER ;

cfgattr: IDENTIFIER ;
cfgval:  | IDENTIFIER ; /* Configuration values / names can be empty (MUXF) */
whitespace: | TOK_WS ;  /* Whitespace can be empty */

cfgvallist: TOK_CFG_SEP cfgval | cfgvallist TOK_CFG_SEP cfgval ;
cfgitem: cfgattr cfgvallist ;
cfglist: cfgitem | cfglist TOK_WS cfgitem ;
cfgstring: TOK_QUOTE whitespace cfglist whitespace TOK_QUOTE ;
placement: PLACED tile site | UNPLACED ;
config: CONFIG cfgstring ;
instance: INSTANCE name sitedef ',' placement ',' config ';' ;

instancelist: instance | instancelist instance;

/* Nets */
inst_name: STRING ;
inst_pin: IDENTIFIER ;
outpin: OUTPIN inst_name inst_pin ',' ;
inpin: INPIN inst_name inst_pin ',' ;
inpinlist: inpin | inpinlist inpin ;
iopinlist: outpin inpinlist ;

wire0: IDENTIFIER { $$ = $1; };
wire1: IDENTIFIER { $$ = $1; };
dir: CONNECTION;
pip: PIP tile wire0 dir wire1 ',' {
	treat_pip(yyparm, $3, $5, $2);
	free($3); free($5); free($2);
 } ;
piplist: pip | piplist pip ;

/* Some nets have the vcc qualifier appended after the name */
qualifier:
   | IDENTIFIER ;
net_header: NET STRING qualifier ;
net: net_header ',' iopinlist piplist ';'
   | net_header ',' config ',' ';' ;

netlist: net | netlist net ;
