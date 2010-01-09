/* -*- mode: C -*- Time-stamp: ""
 *
 *       File:         rec-sex.y
 *       Date:         Sat Jan  9 16:36:55 2010
 *
 *       GNU Records - Selection Expressions parser
 *
 */

/* Copyright (C) 2010 Jose E. Marchesi */

/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

%pure-parser
%name-prefix="sex"
%parse-param {struct rec_sex_ctx_s *sex_ctx}
%lex-param { void *scanner }

%{
   #include <config.h>

   #include <stdlib.h>
   #include <stdio.h>
   #include <string.h>
   #include <regex.h>

   #include <rec.h>
   #include <rec-sex-ctx.h>

  void sexerror (struct rec_sex_ctx_s *context, const char *err)
  {
    /* Do nothing.  */
  }

   #define scanner sex_ctx->scanner
%}

/* Bison declarations.  */

%union {
  int int_val;
  char *str_val;
}
 
%token <int_val> REC_SEX_TOK_INT
%token <str_val> REC_SEX_TOK_STR
%left <int_val> REC_SEX_TOK_SUB REC_SEX_TOK_ADD
%left <int_val> REC_SEX_TOK_MUL REC_SEX_TOK_DIV REC_SEX_TOK_MOD
%left REC_SEX_TOK_NEG  REC_SEX_TOK_MIN /* negation--unary minus */
%left <int_val> REC_SEX_TOK_EQL REC_SEX_TOK_NEQ REC_SEX_TOK_MAT REC_SEX_TOK_LT REC_SEX_TOK_BT
%left <int_val> REC_SEX_TOK_AND REC_SEX_TOK_OR
%right <int_val> REC_SEX_TOK_NOT
%token REC_SEX_TOK_BP REC_SEX_TOK_EP
%token REC_SEX_TOK_ERR

%type <int_val> input
%type <int_val> exp

%% /* The grammar follows.  */

input: /* Empty */
     {
       sex_ctx->result = 0;
     }
     | exp
     {
       sex_ctx->result = ($1 != 0);
     }
     ;

exp : REC_SEX_TOK_INT
    {
      $$ = $1;
    }
    | exp REC_SEX_TOK_EQL exp       
    {
      $$ = ($1 == $3);
    }
    | exp REC_SEX_TOK_NEQ exp
    {
      $$ = ($1 != $3);
    }
    | REC_SEX_TOK_STR REC_SEX_TOK_EQL REC_SEX_TOK_STR 
    {
      $$ = (strcmp ($1, $3) == 0);
    }
    | REC_SEX_TOK_STR REC_SEX_TOK_NEQ REC_SEX_TOK_STR
    {
      $$ = (strcmp ($1, $3) != 0);
    }
    | REC_SEX_TOK_STR REC_SEX_TOK_MAT REC_SEX_TOK_STR
    {
      {
        regex_t regexp;

        if (regcomp (&regexp, $3, REG_EXTENDED) == 0)
          {
            $$ = (regexec (&regexp,
                           $1,
                           0,
                           NULL,
                           0) == 0);
          }
        else
          {
            /* Error compiling the regexp.  */
            YYABORT;
          }
      }
    }
    | exp REC_SEX_TOK_ADD exp       
   {
      $$ = $1 + $3;
    }
    | exp REC_SEX_TOK_SUB exp       
    {
      $$ = $1 + $3;
    }
    | exp REC_SEX_TOK_MUL exp       
    {
      $$ = $1 * $3;
    }
    | exp REC_SEX_TOK_DIV exp       
    {
      $$ = $1 / $3;
    }
    | exp REC_SEX_TOK_MOD exp       
    {
      $$ = $1 % $3;
    }
    | exp REC_SEX_TOK_BT exp       
    {
      $$ = ($1 > $3);
    }
    | exp REC_SEX_TOK_LT exp       
    {
      $$ = ($1 < $3);
    }
    | REC_SEX_TOK_MIN exp %prec REC_SEX_TOK_NEG
    {
      $$ = -$2;
    }
    | REC_SEX_TOK_NOT exp           
    {
      $$ = !$1;
    }
    | exp REC_SEX_TOK_AND exp
    {
      $$ = ($1 && $3);
    }
    | exp REC_SEX_TOK_OR exp
    {
      $$ = ($1 || $3);
    }
    | REC_SEX_TOK_BP exp REC_SEX_TOK_EP
    {
      $$ = $2;
    }

%%

/* End of rec-sex.y */
