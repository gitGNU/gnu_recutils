/* -*- mode: C -*- Time-stamp: "09/10/01 13:42:17 jemarch"
 *
 *       File:         rec-lexer.l
 *       Date:         Sat May 30 07:01:55 2009
 *
 *       GNU rec - Flex lexer
 *
 */

%option reentrant
%option prefix="rec_parser_"
%option bison-bridge
%option bison-locations
%option noyywrap
%option yylineno

%{
#include <string.h>
#include <rec-parser-cxt.h>
#include <rec-parser.h>

#define YY_EXTRA_TYPE rec_parser_cxt_t
#define YY_USER_ACTION yylloc->first_line = yylineno;

%}

%x FIELD_VALUE

%%

(%?[a-zA-Z_-]+:)+  {
       
  yylval->value = strdup (yytext);
  BEGIN(FIELD_VALUE);
  return REC_FIELD_NAME;
 }

^#.*               {
  yylval->value = strdup (yytext);
  return REC_COMMENT;
}

\n                 {
  yylval->value = NULL;
  return REC_NEWLINE;
}

<FIELD_VALUE>\n\+ { }

<FIELD_VALUE>\n    {
  yylval->value = "a_value";
  BEGIN(INITIAL);
  return REC_FIELD_VALUE;
}

<FIELD_VALUE>.   { }

