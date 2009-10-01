/* -*- mode: C -*- Time-stamp: "09/10/01 13:42:48 jemarch"
 *
 *       File:         rec-parser.y
 *       Date:         Thu Jun 11 18:35:49 2009
 *
 *       GNU rec - Bison parser
 *
 */

%pure-parser
%name-prefix="rec_parser_"
%locations
%defines
%error-verbose
%parse-param { rec_parser_ctx_t context }
%lex-param { void *scanner }

%union {
  char *value;
}

%token REC_NEWLINE
%token <value> REC_COMMENT
%token <value> REC_FIELD_NAME
%token <value> REC_FIELD_VALUE

%{
  #include <stdio.h>
  #include <rec-parser-ctx.h>

  int rec_parser_lex (YYSTYPE *lvalp, YYLTYPE *llocp, void *scanner);

  void rec_parser_error (YYLTYPE *locp,
                        rec_parser_ctx_t context,
                        const char* err)
  {
    fprintf (stderr, "error:%d: %s\n", locp->first_line, err);
  }

#define scanner context->scanner
  %}

%%

input: REC_COMMENT input
| REC_NEWLINE input
| REC_FIELD_NAME REC_FIELD_VALUE input
| /* empty */
;

/* End of rec-parser.y */
