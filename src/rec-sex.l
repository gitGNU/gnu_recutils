/* -*- mode: C -*- Time-stamp: "10/01/12 14:57:42 jemarch"
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
">"      { return REC_SEX_TOK_BT; }
"<"      { return REC_SEX_TOK_LT; }
"("      { return REC_SEX_TOK_BP; }
")"      { return REC_SEX_TOK_EP; }
"&&"     { return REC_SEX_TOK_AND; }
"||"     { return REC_SEX_TOK_OR; }

{DIGIT}+ {
  yylval->sexval.type = REC_SEX_INT;
  yylval->sexval.int_val = atoi (yytext);
  yylval->sexval.tag = NULL;

  return REC_SEX_TOK_INT;    
}

#{FIELD_NAME} {
  /* Get the number of fields with this name */
  {
    int i;
    int num_fields = 0;
    rec_field_name_t field_name;
    char *field_name_str;

    /* Parse a field name.  */
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

{FIELD_NAME} {
  /* Get the value of the first field with this name, or error */
  {
    char *field_str;
    rec_field_t field;
    rec_field_name_t fname;

    yylval->sexval.type = REC_SEX_STR;

    /* Make sure the field name finish with ':'.  */
    field_str = malloc (strlen(yytext) + 2);
    strncpy (field_str, yytext, strlen(yytext));
    if (yytext[strlen(yytext) - 1] != ':')
      {
        field_str[strlen(yytext)] = ':';
        field_str[strlen(yytext) + 1] = 0;
      }
    else
      {
        field_str[strlen(yytext)] = 0;
      }

    /* Save the tag.  */
    yylval->sexval.tag = field_str;

    /* Get the first field with the given name in RECORD,
       if any.  */
    fname = rec_parse_field_name_str (field_str);
    field = rec_record_get_field_by_name (yyextra->record, fname, 0);
    if (field)
      {
        /* Return the field value.  */
        yylval->sexval.str_val = strdup (rec_field_value (field));
      }
    else
      {
        /* No field with such a name: return the empty string.  */
        yylval->sexval.str_val = "";
      }
  }

  return REC_SEX_TOK_STR;
}

{STRING} {
  /* Strip the " characters */
  yytext[strlen(yytext) - 1] = 0;

  yylval->sexval.type = REC_SEX_STR;
  yylval->sexval.str_val = strdup (yytext + 1);
  yylval->sexval.tag = NULL;

  return REC_SEX_TOK_STR;
}

.        { return REC_SEX_TOK_ERR; }

%%

/* End of rec-sex.l */
