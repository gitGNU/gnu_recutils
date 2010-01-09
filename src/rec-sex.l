/* -*- mode: C -*- Time-stamp: "10/01/09 22:41:31 jemarch"
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
%option extra-type="struct rec_sex_ctx_s *"

%{
#include <config.h>

#include <rec.h>
#include <rec-sex-ctx.h>
#include <rec-sex.tab.h>

#define YY_INPUT(buf,result,max_size)           \
   {                                            \
   char c;                                      \
   if ((yyextra->index) < strlen(yyextra->in))  \
     {                                          \
       buf[0] = yyextra->in[yyextra->index];    \
       result = 1;                              \
       (yyextra->index)++;                      \
     }                                          \
     else                                       \
     {                                          \
       result = YY_NULL;                        \
     }                                          \
   }
%}

NEWLINE            \n
BLANK              [ \t\n]
DIGIT              [0-9]
LETTER             [a-zA-Z]
FIELD_NAME_PART    {LETTER}[a-zA-Z0-9_]+":"?
FIELD_NAME         {FIELD_NAME_PART}+
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
">"      { return REC_SEX_TOK_BT; }
"<"      { return REC_SEX_TOK_LT; }
"("      { return REC_SEX_TOK_BP; }
")"      { return REC_SEX_TOK_EP; }
"&&"     { return REC_SEX_TOK_AND; }
"||"     { return REC_SEX_TOK_OR; }

{DIGIT}+ {
    yylval->int_val = atoi (yytext);
    return REC_SEX_TOK_INT;    
}

#{FIELD_NAME} {
  /* Get the number of fields with this name */
/*  {
    int i;
    int num_records = 0;
    rec_parser_t parser;
    rec_field_name_t field_name;

    for (i = 0; i < rec_record_size (yyextra); i++)
      {
        rec_field_t field;
  
        field = rec_record_get_field (yyextra, i);
        if (strcmp (rec_field_name (field), yytext) == 0)
          {
            num_records++;
          }
      }
  }
*/
  yylval->int_val = 0;
  return REC_SEX_TOK_INT;
}

{FIELD_NAME} {
  /* Get the value of the first field with this name, or error */
  {
    rec_field_t field;

    /* Get the first field with the given name in RECORD,
       if any.  */
    field = rec_record_get_field_name (yyextra->record, yytext);
    if (field)
      {
        /* Return the field value.  */
        yylval->str_val = strdup (rec_field_value (field));
      }
    else
      {
        /* No field with such a name: return the empty string.  */
        yylval->str_val = "";
      }
  }

  return REC_SEX_TOK_STR;
}

{STRING} {
  /* Strip the " characters */
  yytext[strlen(yytext) - 1] = 0;
  yylval->str_val = strdup (yytext + 1);
  return REC_SEX_TOK_STR;
}

.        { printf ("Unrecognized character: %s\n", yytext); }

%%

/* End of rec-sex.l */
