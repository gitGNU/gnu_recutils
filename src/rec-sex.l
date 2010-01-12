/* -*- mode: C -*- Time-stamp: "10/01/12 22:27:12 jemarch"
 *
 *       File:         rec-sex.l
 *       Date:         Sat Jan  9 16:35:18 2010
 *
 *       GNU Records - Selection Expressions lexer
 *
 */

%option outfile="rec-sex.lex.c"
%option header-file="rec-sex.lex.h"
%option prefix="sex"
%option pointer
%option noyywrap
%option reentrant
%option bison-bridge
%option extra-type="void *"

%{
#include <config.h>

#include <rec-sex-ast.h>
#include <rec-sex-parser.h>
#include <rec-sex.tab.h>

#define YY_INPUT(buf,result,max_size)                                   \
   {                                                                    \
     int ci;                                                            \
     if ((ci = rec_sex_parser_getc ((rec_sex_parser_t) yyextra)) == -1)     \
       {                                                                \
         result = YY_NULL;                                              \
       }                                                                \
     else                                                               \
       {                                                                \
         buf[0] = (char) ci;                                            \
         result = 1;                                                    \
     }                                                                  \
   }
%}

NEWLINE            \n
BLANK              [ \t\n]
DIGIT              [0-9]
LETTER             [a-zA-Z]
FIELD_NAME_PART    {LETTER}[a-zA-Z0-9_]+":"?
FIELD_NAME         ({FIELD_NAME_PART})+
STRING             '([^'\n]|\\(.|\n))*'

/* Please do not touch this comment' */

%%

{BLANK}  { /* Ignore whitespace */ }

"+"      { return REC_SEX_TOK_ADD; }
"-"      { return REC_SEX_TOK_SUB; }
"/"      { return REC_SEX_TOK_DIV; }
"%"      { return REC_SEX_TOK_MOD; }
"*"      { return REC_SEX_TOK_MUL; }
"="      { return REC_SEX_TOK_EQL; }
"!="     { return REC_SEX_TOK_NEQ; }
"!"      { return REC_SEX_TOK_NOT; }
"~"      { return REC_SEX_TOK_MAT; }
">"      { return REC_SEX_TOK_GT; }
"<"      { return REC_SEX_TOK_LT; }
"("      { return REC_SEX_TOK_BP; }
")"      { return REC_SEX_TOK_EP; }
"&&"     { return REC_SEX_TOK_AND; }
"||"     { return REC_SEX_TOK_OR; }
"#"      { return REC_SEX_TOK_SHARP; }

{DIGIT}+ {
  /*
   * Create an integer node.
   */
  yylval->node = rec_sex_ast_node_new ();
  rec_sex_ast_node_set_int (yylval->node, atoi (yytext));

  return REC_SEX_TOK_INT;    
}

{FIELD_NAME} {
  /*
   * Create a name node.
   */
  {
    char *name_str;

    /* Make sure the field name finish with ':'.  */
    name_str = malloc (strlen(yytext) + 2);
    strncpy (name_str, yytext, strlen(yytext));
    if (yytext[strlen(yytext) - 1] != ':')
      {
        name_str[strlen(yytext)] = ':';
        name_str[strlen(yytext) + 1] = 0;
      }
    else
      {
        name_str[strlen(yytext)] = 0;
      }

    yylval->node = rec_sex_ast_node_new ();
    rec_sex_ast_node_set_name (yylval->node, name_str);

    free (name_str);
  }

  return REC_SEX_TOK_NAM;
}

{STRING} {
  /*
   * Create a string node.
   */

  /* Strip the " characters */
  yytext[strlen(yytext) - 1] = 0;

  yylval->node = rec_sex_ast_node_new ();
  rec_sex_ast_node_set_str (yylval->node, yytext + 1);

  return REC_SEX_TOK_STR;
}

.        { return REC_SEX_TOK_ERR; }

%%

/*
#{FIELD_NAME} {
  {
    int i;
    int num_fields = 0;
    rec_field_name_t field_name;
    char *field_name_str;

    field_name_str = malloc (strlen (yytext));
    strncpy (field_name_str, yytext + 1, strlen (yytext) - 1);
    field_name_str[strlen (yytext) - 1] = ':';
    field_name_str[strlen (yytext)] = 0;

    field_name = rec_parse_field_name_str (field_name_str);
    for (i = 0; i < rec_record_size (yyextra->record); i++)
      {
        rec_field_t field;
  
        field = rec_record_get_field (yyextra->record, i);
        if (rec_field_name_equal_p (rec_field_name (field),
                                    field_name))
          {
            num_fields++;
          }
      }

    yylval->sexval.type = REC_SEX_INT;
    yylval->sexval.int_val = num_fields;
    yylval->sexval.tag = NULL;

    free (field_name_str);
  }

  return REC_SEX_TOK_INT;
}
*/

/* End of rec-sex.l */
