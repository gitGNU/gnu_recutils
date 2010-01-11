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

  /* Forward references for parsing routines.  */
  bool rec_sex_eql (rec_sex_val_t res, rec_sex_val_t val1, rec_sex_val_t val2, bool ci);
  bool rec_sex_neq (rec_sex_val_t res, rec_sex_val_t val1, rec_sex_val_t val2, bool ci);
  bool rec_sex_mat (rec_sex_val_t res, rec_sex_val_t val1, rec_sex_val_t val2, bool ci);
  bool rec_sex_add (rec_sex_val_t res, rec_sex_val_t val1, rec_sex_val_t val2);
  bool rec_sex_sub (rec_sex_val_t res, rec_sex_val_t val1, rec_sex_val_t val2);
  bool rec_sex_mul (rec_sex_val_t res, rec_sex_val_t val1, rec_sex_val_t val2);
  bool rec_sex_div (rec_sex_val_t res, rec_sex_val_t val1, rec_sex_val_t val2);
  bool rec_sex_mod (rec_sex_val_t res, rec_sex_val_t val1, rec_sex_val_t val2);
  bool rec_sex_bt (rec_sex_val_t res, rec_sex_val_t val1, rec_sex_val_t val2);
  bool rec_sex_lt (rec_sex_val_t res, rec_sex_val_t val1, rec_sex_val_t val2);
  bool rec_sex_not (rec_sex_val_t res, rec_sex_val_t val);
  bool rec_sex_and (rec_sex_val_t res, rec_sex_val_t val1, rec_sex_val_t val2);
  bool rec_sex_or (rec_sex_val_t res, rec_sex_val_t val1, rec_sex_val_t val2);
  bool rec_sex_group (rec_sex_val_t res, rec_sex_val_t val);

%}

/* Bison declarations.  */

%union {
  struct rec_sex_val_s sexval;
}
 
%token <sexval> REC_SEX_TOK_INT
%token <sexval> REC_SEX_TOK_STR
%left <sexval> REC_SEX_TOK_MAT 
%left <sexval> REC_SEX_TOK_AND REC_SEX_TOK_OR
%left <sexval> REC_SEX_TOK_EQL REC_SEX_TOK_NEQ REC_SEX_TOK_LT REC_SEX_TOK_BT
%left <sexval> REC_SEX_TOK_SUB REC_SEX_TOK_ADD
%left <sexval> REC_SEX_TOK_MUL REC_SEX_TOK_DIV REC_SEX_TOK_MOD
%left REC_SEX_TOK_NEG  REC_SEX_TOK_MIN /* negation--unary minus */
%right <sexval> REC_SEX_TOK_NOT
%token REC_SEX_TOK_BP REC_SEX_TOK_EP
%token REC_SEX_TOK_ERR

%type <sexval> input
%type <sexval> exp

%% /* The grammar follows.  */

input: /* Empty */ { sex_ctx->result = 0; }
     | exp
     {
       if ($1.type == REC_SEX_INT)
         {
           sex_ctx->result = ($1.int_val != 0);
         }
       else
         {
           /* Nonempty string => true. */
           sex_ctx->result = ($1.str_val[0] != 0);
         }
     }
     ;

exp : REC_SEX_TOK_INT          { $$.type = REC_SEX_INT; $$.int_val = $1.int_val; }
    | REC_SEX_TOK_STR          { $$.type = REC_SEX_STR; $$.str_val = $1.str_val; }
    | exp REC_SEX_TOK_EQL exp  { if (!rec_sex_eql (&$$, &$1, &$3, sex_ctx->case_insensitive)) YYABORT;}
    | exp REC_SEX_TOK_NEQ exp  { if (!rec_sex_neq (&$$, &$1, &$3, sex_ctx->case_insensitive)) YYABORT; }
    | REC_SEX_TOK_STR REC_SEX_TOK_MAT REC_SEX_TOK_STR  { if (!rec_sex_mat (&$$, &$1, &$3, sex_ctx->case_insensitive)) YYABORT; }
    | exp REC_SEX_TOK_ADD exp  { if (!rec_sex_add (&$$, &$1, &$3)) YYABORT; }
    | exp REC_SEX_TOK_SUB exp  { if (!rec_sex_sub (&$$, &$1, &$3)) YYABORT; }
    | exp REC_SEX_TOK_MUL exp  { if (!rec_sex_mul (&$$, &$1, &$3)) YYABORT; }
    | exp REC_SEX_TOK_DIV exp  { if (!rec_sex_div (&$$, &$1, &$3)) YYABORT; }
    | exp REC_SEX_TOK_MOD exp  { if (!rec_sex_mod (&$$, &$1, &$3)) YYABORT; }
    | exp REC_SEX_TOK_BT exp   { if (!rec_sex_bt (&$$, &$1, &$3)) YYABORT; }
    | exp REC_SEX_TOK_LT exp   { if (!rec_sex_lt (&$$, &$1, &$3)) YYABORT; }
    | REC_SEX_TOK_NOT exp %prec REC_SEX_TOK_NEG { if (!rec_sex_not (&$$, &$2)) YYABORT; }
    | exp REC_SEX_TOK_AND exp  { if (!rec_sex_and (&$$, &$1, &$3)) YYABORT; }
    | exp REC_SEX_TOK_OR exp   { if (!rec_sex_or (&$$, &$1, &$3)) YYABORT; }
    | REC_SEX_TOK_BP exp REC_SEX_TOK_EP { if (!rec_sex_group (&$$, &$2)) YYABORT; }

%%

bool
rec_sex_eql (rec_sex_val_t res,
             rec_sex_val_t val1,
             rec_sex_val_t val2,
             bool case_insensitive)
{
  bool ret;

  ret = true;

  res->type = REC_SEX_INT;

  if ((val1->type == REC_SEX_INT)
      && (val2->type == REC_SEX_INT))
    {
      res->int_val = val1->int_val == val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_STR))
    {
      if (case_insensitive)
        {
          res->int_val =  strcasecmp (val1->str_val, val2->str_val) == 0;
        }
      else
        {
          res->int_val =  strcmp (val1->str_val, val2->str_val) == 0;
        }
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_INT))
    {
      val1->int_val = atoi (val1->str_val);
      res->int_val = val1->int_val == val2->int_val;
    }
  else if ((val1->type == REC_SEX_INT)
           && (val2->type == REC_SEX_STR))
    {
      val2->int_val = atoi (val2->str_val);
      res->int_val = val1->int_val == val2->int_val;
    }

  return ret;
}

bool
rec_sex_neq (rec_sex_val_t res,
             rec_sex_val_t val1,
             rec_sex_val_t val2,
             bool case_insensitive)
{
  bool ret;

  ret = true;

  res->type = REC_SEX_INT;

  if ((val1->type == REC_SEX_INT)
      && (val2->type == REC_SEX_INT))
    {
      res->int_val = val1->int_val != val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_STR))
    {
      if (case_insensitive)
        {
          res->int_val =  strcasecmp (val1->str_val, val2->str_val) != 0;
        }
      else
        {
          res->int_val =  strcmp (val1->str_val, val2->str_val) != 0;
        }
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_INT))
    {
      val1->int_val = atoi (val1->str_val);
      res->int_val = val1->int_val != val2->int_val;
    }
  else if ((val1->type == REC_SEX_INT)
           && (val2->type == REC_SEX_STR))
    {
      val2->int_val = atoi (val2->str_val);
      res->int_val = val1->int_val != val2->int_val;
    }

  return ret;
}

bool
rec_sex_mat (rec_sex_val_t res,
             rec_sex_val_t val1,
             rec_sex_val_t val2,
             bool case_insensitive)
{
  bool ret;
  regex_t regexp;
  int flags;
 
  flags = 0;
  ret = true;
  res->type = REC_SEX_INT;

  flags = REG_EXTENDED;
  if (case_insensitive)
    {
      flags |= REG_ICASE;
    }

  if ((val1->type == REC_SEX_STR)
      && (val2->type == REC_SEX_STR))
    {
      if (regcomp (&regexp, val2->str_val, flags) == 0)
        {
          int flags = 0;

          res->int_val = (regexec (&regexp,
                                   val1->str_val,
                                   0,
                                   NULL,
                                   0) == 0);
        }
      else
        {
          /* Error compiling the regexp.  */
          ret = false;
        }
    }
  else
    {
      ret = false;
    }

  return ret;
}

bool
rec_sex_add (rec_sex_val_t res,
             rec_sex_val_t val1,
             rec_sex_val_t val2)
{
  bool ret;

  ret = true;

  res->type = REC_SEX_INT;

  if ((val1->type == REC_SEX_INT)
      && (val2->type == REC_SEX_INT))
    {
      res->int_val = val1->int_val + val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_STR))
    {
      val1->int_val = atoi (val1->str_val);
      val2->int_val = atoi (val2->str_val);
      res->int_val =  val1->int_val + val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_INT))
    {
      val1->int_val = atoi (val1->str_val);
      res->int_val = val1->int_val + val2->int_val;
    }
  else if ((val1->type == REC_SEX_INT)
           && (val2->type == REC_SEX_STR))
    {
      val2->int_val = atoi (val2->str_val);
      res->int_val = val1->int_val + val2->int_val;
    }

  return ret;
}

bool
rec_sex_sub (rec_sex_val_t res,
             rec_sex_val_t val1,
             rec_sex_val_t val2)
{
  bool ret;

  ret = true;

  res->type = REC_SEX_INT;

  if ((val1->type == REC_SEX_INT)
      && (val2->type == REC_SEX_INT))
    {
      res->int_val = val1->int_val - val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_STR))
    {
      val1->int_val = atoi (val1->str_val);
      val2->int_val = atoi (val2->str_val);
      res->int_val =  val1->int_val - val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_INT))
    {
      val1->int_val = atoi (val1->str_val);
      res->int_val = val1->int_val - val2->int_val;
    }
  else if ((val1->type == REC_SEX_INT)
           && (val2->type == REC_SEX_STR))
    {
      val2->int_val = atoi (val2->str_val);
      res->int_val = val1->int_val - val2->int_val;
    }

  return ret;
}

bool
rec_sex_mul (rec_sex_val_t res,
             rec_sex_val_t val1,
             rec_sex_val_t val2)
{
  bool ret;

  ret = true;

  res->type = REC_SEX_INT;

  if ((val1->type == REC_SEX_INT)
      && (val2->type == REC_SEX_INT))
    {
      res->int_val = val1->int_val * val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_STR))
    {
      val1->int_val = atoi (val1->str_val);
      val2->int_val = atoi (val2->str_val);
      res->int_val =  val1->int_val * val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_INT))
    {
      val1->int_val = atoi (val1->str_val);
      res->int_val = val1->int_val * val2->int_val;
    }
  else if ((val1->type == REC_SEX_INT)
           && (val2->type == REC_SEX_STR))
    {
      val2->int_val = atoi (val2->str_val);
      res->int_val = val1->int_val * val2->int_val;
    }

  return ret;
}

bool
rec_sex_div (rec_sex_val_t res,
             rec_sex_val_t val1,
             rec_sex_val_t val2)
{
  bool ret;

  ret = true;

  res->type = REC_SEX_INT;

  if ((val1->type == REC_SEX_INT)
      && (val2->type == REC_SEX_INT))
    {
      res->int_val = val1->int_val / val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_STR))
    {
      val1->int_val = atoi (val1->str_val);
      val2->int_val = atoi (val2->str_val);
      res->int_val =  val1->int_val / val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_INT))
    {
      val1->int_val = atoi (val1->str_val);
      res->int_val = val1->int_val / val2->int_val;
    }
  else if ((val1->type == REC_SEX_INT)
           && (val2->type == REC_SEX_STR))
    {
      val2->int_val = atoi (val2->str_val);
      res->int_val = val1->int_val / val2->int_val;
    }

  return ret;
}

bool
rec_sex_mod (rec_sex_val_t res,
             rec_sex_val_t val1,
             rec_sex_val_t val2)
{
  bool ret;

  ret = true;

  res->type = REC_SEX_INT;

  if ((val1->type == REC_SEX_INT)
      && (val2->type == REC_SEX_INT))
    {
      res->int_val = val1->int_val % val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_STR))
    {
      val1->int_val = atoi (val1->str_val);
      val2->int_val = atoi (val2->str_val);
      res->int_val =  val1->int_val % val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_INT))
    {
      val1->int_val = atoi (val1->str_val);
      res->int_val = val1->int_val % val2->int_val;
    }
  else if ((val1->type == REC_SEX_INT)
           && (val2->type == REC_SEX_STR))
    {
      val2->int_val = atoi (val2->str_val);
      res->int_val = val1->int_val % val2->int_val;
    }

  return ret;
}

bool
rec_sex_bt (rec_sex_val_t res,
             rec_sex_val_t val1,
             rec_sex_val_t val2)
{
  bool ret;

  ret = true;

  res->type = REC_SEX_INT;

  if ((val1->type == REC_SEX_INT)
      && (val2->type == REC_SEX_INT))
    {
      res->int_val = val1->int_val > val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_STR))
    {
      val1->int_val = atoi (val1->str_val);
      val2->int_val = atoi (val2->str_val);
      res->int_val =  val1->int_val > val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_INT))
    {
      val1->int_val = atoi (val1->str_val);
      res->int_val = val1->int_val > val2->int_val;
    }
  else if ((val1->type == REC_SEX_INT)
           && (val2->type == REC_SEX_STR))
    {
      val2->int_val = atoi (val2->str_val);
      res->int_val = val1->int_val > val2->int_val;
    }

  return ret;
}     

bool
rec_sex_lt (rec_sex_val_t res,
            rec_sex_val_t val1,
            rec_sex_val_t val2)
{
  bool ret;

  ret = true;

  res->type = REC_SEX_INT;

  if ((val1->type == REC_SEX_INT)
      && (val2->type == REC_SEX_INT))
    {
      res->int_val = val1->int_val < val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_STR))
    {
      val1->int_val = atoi (val1->str_val);
      val2->int_val = atoi (val2->str_val);
      res->int_val =  val1->int_val < val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_INT))
    {
      val1->int_val = atoi (val1->str_val);
      res->int_val = val1->int_val < val2->int_val;
    }
  else if ((val1->type == REC_SEX_INT)
           && (val2->type == REC_SEX_STR))
    {
      val2->int_val = atoi (val2->str_val);
      res->int_val = val1->int_val < val2->int_val;
    }

  return ret;
}     

bool
rec_sex_not (rec_sex_val_t res,
             rec_sex_val_t val)
{
  bool ret;

  ret = true;

  res->type = REC_SEX_INT;
  if (val->type == REC_SEX_INT)
    {
      res->int_val = -val->int_val;
    }
  else
    {
      val->int_val = atoi (val->str_val);
      res->int_val = -val->int_val;
    }

  return ret;
}

bool
rec_sex_and (rec_sex_val_t res,
            rec_sex_val_t val1,
            rec_sex_val_t val2)
{
  bool ret;

  ret = true;

  res->type = REC_SEX_INT;

  if ((val1->type == REC_SEX_INT)
      && (val2->type == REC_SEX_INT))
    {
      res->int_val = val1->int_val && val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_STR))
    {
      val1->int_val = atoi (val1->str_val);
      val2->int_val = atoi (val2->str_val);
      res->int_val =  val1->int_val && val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_INT))
    {
      val1->int_val = atoi (val1->str_val);
      res->int_val = val1->int_val && val2->int_val;
    }
  else if ((val1->type == REC_SEX_INT)
           && (val2->type == REC_SEX_STR))
    {
      val2->int_val = atoi (val2->str_val);
      res->int_val = val1->int_val && val2->int_val;
    }

  return ret;
}     

bool
rec_sex_or (rec_sex_val_t res,
            rec_sex_val_t val1,
            rec_sex_val_t val2)
{
  bool ret;

  ret = true;

  res->type = REC_SEX_INT;

  if ((val1->type == REC_SEX_INT)
      && (val2->type == REC_SEX_INT))
    {
      res->int_val = val1->int_val || val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_STR))
    {
      val1->int_val = atoi (val1->str_val);
      val2->int_val = atoi (val2->str_val);
      res->int_val =  val1->int_val || val2->int_val;
    }
  else if ((val1->type == REC_SEX_STR)
           && (val2->type == REC_SEX_INT))
    {
      val1->int_val = atoi (val1->str_val);
      res->int_val = val1->int_val || val2->int_val;
    }
  else if ((val1->type == REC_SEX_INT)
           && (val2->type == REC_SEX_STR))
    {
      val2->int_val = atoi (val2->str_val);
      res->int_val = val1->int_val || val2->int_val;
    }

  return ret;
}

bool
rec_sex_group (rec_sex_val_t res,
               rec_sex_val_t val)
{
  bool ret;
  
  ret = true;
  if (val->type == REC_SEX_INT)
    {
      res->type = REC_SEX_INT;
      res->int_val = val->int_val;
    }
  else
    {
      res->type = REC_SEX_STR;
      res->str_val = val->str_val;
    }

  return ret;
}



/* End of rec-sex.y */
