/* -*- mode: C -*-
 *
 *       File:         rec-fex.c
 *       Date:         Sun Apr 11 21:31:32 2010
 *
 *       GNU recutils - Field Expressions
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

#include <malloc.h>
#include <string.h>
#include <regex.h>

#include <rec.h>

/*
 * Data types.
 */

struct rec_fex_elem_s
{
  char *str;

  rec_field_name_t field_name;
  char prefix;
  int max;
  int min;
};

#define REC_FEX_MAX_ELEMS 20

struct rec_fex_s
{
  int num_elems;
  rec_fex_elem_t elems[REC_FEX_MAX_ELEMS];
};

/*
 * Static function declarations.
 */

static bool rec_fex_parse_str (rec_fex_t new, char *str);

/*
 * Public functions.
 */

rec_fex_t
rec_fex_new (char *str)
{
  rec_fex_t new;
  int i;

  new = malloc (sizeof (struct rec_fex_s));
  if (new)
    {
      /* Initially the FEX is empty.  */
      new->num_elems = 0;
      for (i = 0; i < REC_FEX_MAX_ELEMS; i++)
        {
          new->elems[i] = 0;
        }

      /* Parse the string.  */
      if (!rec_fex_parse_str (new, str))
        {
          free (new);
          new = NULL;
        }
    }
  
  return new;
}

void
rec_fex_destroy (rec_fex_t fex)
{
  rec_field_name_destroy (fex->field_name);
  free (fex->str);
  free (fex);
}

bool
rec_fex_check (char *str)
{
  int ret;
  static char *regexp_str = "(/?%?[a-zA-Z][a-zA-Z0-9_])+";
  regex_t regexp;

  /* Compile the regexp.  */
  if ((ret = regcomp (&regexp, regexp_str, REG_EXTENDED)) != 0)
    {
      fprintf (stderr, "rec_resolver_check: error compiling regexp.\n");
      return false;
    }

  /* Check.  */
  return (regexec (&regexp, regexp_str, 0, NULL, 0) == 0);
}

int
rec_fex_size (rec_fex_t fex)
{
  return fex->num_elems;
}

rec_fex_elem_t
rec_fex_get (rec_fex_t fex,
             int position)
{
  if ((position < 0) || (position >= fex->num_elems))
    {
      return NULL;
    }

  return fex->elems[position];
}

bool
rec_fex_elem_prefix (rec_fex_elem_t elem)
{
  return elem->prefix;
}

rec_field_name_t
rec_fex_elem_field_name (rec_fex_elem_t elem)
{
  return elem->field_name;
}

int
rec_fex_elem_min (rec_fex_elem_t elem)
{
  return elem->min;
}

int
rec_fex_elem_max (rec_fex_elem_t elem)
{
  return elem->max;
}

/*
 * Private functions.
 */

static bool
rec_fex_parse_str (rec_fex_t new,
                   char *str)
{
  bool res;
  char *elem_str;
  char *fex_str;
  rec_fex_elem_t elem;
  int i;

  res = true;

  fex_str = strdup (str);
  if (!fex_str)
    {
      return false;
    }

  elem_str = strsep (&fex_str, ",");
  do
    {
      elem = malloc (sizeof (struct rec_fex_elem_s));
      if (!rec_fex_parse_elem (elem, elem_str))
        {
          /* Parse error.  */
          for (i = 0; i < fex->num_elems; i++)
            {
              free (fex->elem[i]);
            }

          res = false;
          break;
        }

      /* Add the elem to the FEX.  */
      fex->elems[fex->num_elems] = elem;
      fex->num_elems++;
    }
  while ((elem_str = strsep (&fex_str, ",")));

  if (res)
    {
      fex->str = strdup (str);
    }

  free (fex_str);
  free (elem_str);

  return res;
}

static bool
rec_fex_parse_elem (rec_fex_elem_t elem,
                    char *str)
{


}

/* End of rec-fex.c */
