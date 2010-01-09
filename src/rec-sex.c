/* -*- mode: C -*- Time-stamp: "10/01/09 22:51:28 jemarch"
 *
 *       File:         rec-sex.c
 *       Date:         Sat Jan  9 20:28:43 2010
 *
 *       GNU Records - Selection Expressions
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
#include <rec-sex-ctx.h>
#include <rec-sex.tab.h>
#include <rec-sex.lex.h>

/*
 * Data structures
 */

struct rec_sex_s
{
  struct rec_sex_ctx_s *parser_ctx;
};

/*
 * Public functions.
 */

rec_sex_t
rec_sex_new ()
{
  rec_sex_t new;

  new = malloc (sizeof (struct rec_sex_s));
  if (new)
    {
      new->parser_ctx = malloc (sizeof (struct rec_sex_ctx_s));
      if (new->parser_ctx)
        {
          new->parser_ctx->in = NULL;
          new->parser_ctx->index = 0;
          new->parser_ctx->result = false;
        }
      else
        {
          free (new);
          new = NULL;
        }
    }

  return new;
}

void
rec_sex_destroy (rec_sex_t sex)
{
  free (sex->parser_ctx);
  free (sex);
}

bool
rec_sex_apply (rec_sex_t sex,
               char *expr,
               rec_record_t record)
{
  bool res;

  /* Set the context.  */
  sex->parser_ctx->in = expr;
  sex->parser_ctx->index = 0;
  sex->parser_ctx->record = record;
  sex->parser_ctx->result = false;

  /* Initialize the sexy scanner.  */
  sexlex_init (&(sex->parser_ctx->scanner));
  sexset_extra (sex->parser_ctx, sex->parser_ctx->scanner);

  if (!sexparse (sex->parser_ctx))
    {
      res = sex->parser_ctx->result;
    }
  else
    {
      /* Parse error.  XXX.*/
      printf("Parser error in selection expression.\n");
      res = false;
    }

  /* Deallocate memory.  */
  sexlex_destroy (sex->parser_ctx->scanner);

  return res;
}
 
/* End of rec-sex.c */
