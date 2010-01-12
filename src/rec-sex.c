/* -*- mode: C -*- Time-stamp: "10/01/12 23:04:45 jemarch"
 *
 *       File:         rec-sex.c
 *       Date:         Sat Jan  9 20:28:43 2010
 *
 *       GNU Records - Sexy expressions
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

#include <config.h>

#include <rec.h>

#include <rec-sex-ast.h>
#include <rec-sex-parser.h>

#include <rec-sex.tab.h>
#include <rec-sex.lex.h>

/*
 * Data structures
 */

struct rec_sex_s
{
  rec_sex_ast_t ast;
  rec_sex_parser_t parser;
};

/*
 * Public functions.
 */

rec_sex_t
rec_sex_new (bool case_insensitive)
{
  rec_sex_t new;

  new = malloc (sizeof (struct rec_sex_s));
  if (new)
    {
      /* Initialize a new parser.  */
      new->parser = rec_sex_parser_new ();
      rec_sex_parser_set_case_insensitive (new->parser,
                                           case_insensitive);

      /* Initialize a new AST.  */
      new->ast = rec_sex_ast_new ();
    }

  return new;
}

void
rec_sex_destroy (rec_sex_t sex)
{
  if (sex->parser)
    {
      rec_sex_parser_destroy (sex->parser);
    }

  if (sex->ast)
    {
      rec_sex_ast_destroy (sex->ast);
    }

  free (sex);  /* yeah! :D */
}

bool
rec_sex_compile (rec_sex_t sex,
                 char *expr)
{
  bool res;

  res = rec_sex_parser_run (sex->parser, expr);
  return res;
}

bool
rec_sex_apply (rec_sex_t sex,
               rec_record_t record,
               bool *status)
{
  /* XXX: write me.  */
  *status = true;
  return true;
}

void
rec_sex_print_ast (rec_sex_t sex)
{
  rec_sex_parser_print_ast (sex->parser);
}
 
/* End of rec-sex.c */
