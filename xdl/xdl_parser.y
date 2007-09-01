/* XDL Parser */

%{

#include <assert.h>
#include <stdio.h>
#include "parser.h"

#define YYPARSE_PARAM yyparm
extern void yyerror(char *);

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

design_header: DESIGN design_name part ncd_version
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
	((parser_t *)yyparm)->pip_counter++;
	printf("pip %s to %s @ %s\n", $3, $5, $2);
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
