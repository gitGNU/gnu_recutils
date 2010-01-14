/* -*- mode: C -*- Time-stamp: "10/01/14 14:07:47 jemarch"
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

   /* Forward prototypes for functions defined below.  */
   char *rec_sex_lex_extract_name (char *str);
   bool rec_sex_lex_extract_index (char *str, int *num);
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

{FIELD_NAME}(\[[0-9]+\])? {
  int res;
  char *match;
  char *name;
  char *name_str;
  int index = -1;

  match = strdup (yytext);
  rec_sex_lex_extract_index (match, &index);
  name = rec_sex_lex_extract_name (match);

  /* Make sure the field name finish with ':'.  */
  name_str = malloc (strlen(name) + 2);
  strncpy (name_str, name, strlen(name));
  if (yytext[strlen(name) - 1] != ':')
    {
      name_str[strlen(name)] = ':';
      name_str[strlen(name) + 1] = 0;
    }
  else
    {
      name_str[strlen(name)] = 0;
    }

  /* Create a name node.  */
  yylval->node = rec_sex_ast_node_new ();
  rec_sex_ast_node_set_name (yylval->node, name_str);
  rec_sex_ast_node_set_index (yylval->node, index);
  res = REC_SEX_TOK_NAM;

  free (name);
  free (match);

  return res;
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

char *
rec_sex_lex_extract_name (char *str)
{
  size_t size;
  char *res;
  char *p;

  p = str;
  while ((*p != '[') && (*p != 0))
    {
      p++;
    }

  size = (p - str) + 1;
  res = malloc (size);
  strncpy (res, str, size);
  res[size] = 0;

  return res;
}

bool
rec_sex_lex_extract_index (char *str,
                           int *num)
{
  bool res;
  char *p, *b;
  char aux[100];
  int aux_size;

  /* Note that this function assumes syntax correctness in STR if a
     '[' is found.  */  

  aux_size = 0;
  p = str;
  while ((*p != 0) && (*p != '['))
    {
      p++;
    }

  if (*p == 0)
    {
      res = false;
    }
  else
    {
      p++; /* Pass the [.  */
      while (*p != ']')
        {
          aux[aux_size++] = *p;
          p++;
        }
      aux[aux_size] = 0;

      *num = atoi (aux);
      res = true;
    }

  return res;
}

/* End of rec-sex.l */