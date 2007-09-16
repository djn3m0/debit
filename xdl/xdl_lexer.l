/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 */

/* XDL lexer */

/* Definitions */
%{
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "debitlog.h"
#include "xdl_parser.h"

extern void yyerror(char *);

#define copy_and_return(token_type)		\
	{					\
		yylval->name = strdup(yytext);	\
		return(token_type);		\
	}
%}

/* options */
%option noyywrap
/* Reentrancy is a huge performance hit.
   Is this expected ? */
/* %option reentrant */
%option bison-bridge
%option header-file="xdl_lexer.h"
/* %option reentrant */

/* States */
%x S_STRING
%x S_CONFIG
%x S_CSTRING
%x S_CIDENTIFIER
%x S_CLUT

%%

design { return DESIGN; }
inst   { return INSTANCE; }
cfg    { BEGIN(S_CONFIG); return(CONFIG); }
placed { return PLACED; }
unplaced { return UNPLACED; }

net    { return NET; }
outpin { return OUTPIN; }
inpin  { return INPIN; }
pip    { return PIP; }
"=="|"=>"|"=-"|"->"     { return CONNECTION; }

[a-zA-Z0-9_\-]* { copy_and_return(IDENTIFIER); } /* Should do otherwise */
v[0-9]+.[0-9]+  { return VERSION; }

<S_CONFIG>{
[ \t\n]+     /* eat up whitespace */
\"           { BEGIN(S_CSTRING); return (TOK_QUOTE); }
}

<S_CSTRING>{
([^\\\n\t \":]*|\\[\": ])* { copy_and_return(IDENTIFIER); }
[ \t\n]+      { return(TOK_WS); } /* whitespace is significant is this context.
                                     It helps resolve shift-reduce conflicts */
:             { return(TOK_CFG_SEP); }
:D=           { BEGIN(S_CLUT);
                return(TOK_CFG_SEP); } /* magic string for LUT cfg data */
\"            { BEGIN(INITIAL); return (TOK_QUOTE); }
.             { g_warning("Unrecognized character: %s\n", yytext ); }
}

<S_CLUT>{
A[1-6]   {
	/* I definitely love this */
	static const uint64_t vars[6] = {
	  [0] = 0xaaaaaaaaaaaaaaaaULL, [1] = 0xccccccccccccccccULL,
	  [2] = 0xf0f0f0f0f0f0f0f0ULL, [3] = 0xff00ff00ff00ff00ULL,
	  [4] = 0xffff0000ffff0000ULL, [5] = 0xffffffff00000000ULL,
	};
	unsigned index = yytext[1]-'1';
	assert(index < ARRAY_SIZE(vars));
	debit_log(L_LEXER, "Lookup variable %s, index %i", yytext, index);
	yylval->val = vars[index];
	return(TOK_E_VAL);
}
0x[a-fA-F0-9]+ { yylval->val = 0; /* XXX */
	         return(TOK_E_VAL); }
0        { yylval->val = 0;
	   return(TOK_E_VAL); }
[()+*@~] { return(yytext[0]); }
[ \t\n]+ { BEGIN(S_CSTRING); return(TOK_WS); } /* end-of-thing */
}

\" BEGIN(S_STRING);
<S_STRING>{
[^\\\n\"]* { yymore(); }
\n { yymore(); } /* Handle multi-line string */
\\\n { yymore(); } /* Handle multi-line string with escaped NL */
\\[\": ] { yymore(); } /* Handle string-within-string and other escaped characters */
\"  {
         yytext[yyleng-1] = '\0';
         BEGIN(INITIAL);
         copy_and_return(STRING);
    }
}

","            { return ','; }
";"            { return ';'; }

#[^\n]*\n      /* eat up one-line and end-of-line comments */
[ \t\n]+       /* eat up whitespace */

.              { g_warning("Unrecognized character: %s\n", yytext ); }

%%
