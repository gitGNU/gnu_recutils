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
#include <libintl.h>
#define _(str) dgettext (PACKAGE, str)

#include <rec-utils.h>
#include <rec.h>

/*
 * Data types.
 */

struct rec_fex_elem_s
{
  char *str;

  rec_field_name_t field_name;
  int max;
  int min;
};

#define REC_FEX_MAX_ELEMS 20

struct rec_fex_s
{
  int num_elems;
  char *str;
  rec_fex_elem_t elems[REC_FEX_MAX_ELEMS];
};

/*
 * Static function declarations.
 */

static bool rec_fex_parse_str_simple (rec_fex_t new, char *str);
static bool rec_fex_parse_str_subscripts (rec_fex_t new, char *str);
static bool rec_fex_parse_elem (rec_fex_elem_t elem, char *str);

/*
 * Public functions.
 */

rec_fex_t
rec_fex_new (char *str,
             enum rec_fex_kind_e kind)
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
      if (kind == REC_FEX_SUBSCRIPTS)
        {
          if (!rec_fex_parse_str_subscripts (new, str))
            {
              free (new);
              new = NULL;
            }
        }
      else
        {
          /* Simple FEX.  */
          if (!rec_fex_parse_str_simple (new, str))
            {
              free (new);
              new = NULL;
            }
        }
    }
  
  return new;
}

void
rec_fex_destroy (rec_fex_t fex)
{
  int i;
  
  for (i = 0; i < fex->num_elems; i++)
    {
      if (fex->elems[i]->field_name)
        {
          rec_field_name_destroy (fex->elems[i]->field_name);
        }
      if (fex->elems[i]->str)
        {
          free (fex->elems[i]->str);
        }
      free (fex->elems[i]);
    }
  if (fex->str)
    {
      free (fex->str);
    }
  free (fex);
}

bool
rec_fex_check (char *str)
{
  int ret;
  static char *regexp_str =
    "^" /* Beginning of the string.  */
    "[a-zA-Z%][a-zA-Z0-9_-]*(\\[[0-9]\\])?"     /* First element name.  */
    "(,[a-zA-Z%][a-zA-Z0-9_-]*(\\[[0-9]\\])?)*" /* Subsequent element names. */
    "$" /* End of the string.  */
    ;
  regex_t regexp;

  /* Compile the regexp.  */
  if ((ret = regcomp (&regexp, regexp_str, REG_EXTENDED)) != 0)
    {
      fprintf (stderr, _("internal error: rec_resolver_check: error compiling regexp.\n"));
      return false;
    }

  /* Check.  */
  ret = regexec (&regexp, str, 0, NULL, 0);
  if (ret != 0)
    {
      char *str_error;
      size_t str_error_size;

      str_error_size = regerror (ret, &regexp, 0, 0);
      str_error = malloc (str_error_size + 1);
      if (str_error)
        {
          regerror (ret, &regexp, str_error, str_error_size);
          fprintf (stderr, _("error: invalid field expression in -p: %s.\n"), str_error);
        }
    }
  
  return (ret == 0);
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

rec_field_name_t
rec_fex_elem_field_name (rec_fex_elem_t elem)
{
  return elem->field_name;
}

char *
rec_fex_elem_field_name_str (rec_fex_elem_t elem)
{
  return elem->str;
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

void
rec_fex_sort (rec_fex_t fex)
{
  bool done;
  rec_fex_elem_t aux;
  int i, j;

  /* XXX: this code only works when 'max' is not specified.  */

  for (i = 1; i < fex->num_elems; i++)
    {
      aux = fex->elems[i];
      j = i - 1;
      done = false;

      while (!done)
       {
         /* If elems[j] > aux  */
         if ((fex->elems[j]->min == -1) || (fex->elems[j]->min > aux->min))
           {
             fex->elems[j + 1] = fex->elems[j];
             j = j - 1;
             if (j < 0)
               {
                 done = true;
               }
           }
         else
           {
             done = true;
           }
       }

      fex->elems[j + 1] = aux;
    }
}

/*
 * Private functions.
 */

static bool
rec_fex_parse_str_simple (rec_fex_t new,
                          char *str)
{
  bool res;
  rec_fex_elem_t elem;
  rec_field_name_t field_name;
  char *fex_str;
  char *elem_str;
  size_t i;

  if (!str)
    {
      return false;
    }

  fex_str = strdup (str);
  if (!fex_str)
    {
      return false;
    }

  res = true;

  elem_str = strsep (&fex_str, " \t\n");
  do
    {
      if (strlen (elem_str) > 0)
        {
          if ((field_name = rec_parse_field_name_str (elem_str))
              && (elem = malloc (sizeof (struct rec_fex_elem_s))))
            {
              /* Add an element to the FEX.  */
              elem->field_name = field_name;
              elem->str = strdup (elem_str);
              elem->min = -1;
              elem->max = -1;
              new->elems[new->num_elems++] = elem;              
            }
          else
            {
              res = false;
              break;
            }
        }
    }
  while ((elem_str = strsep (&fex_str, " \t\n")));

  if (new->num_elems == 0)
    {
      /* No elements were recognized.  */
      res = false;
    }

  if (res)
    {
      new->str = strdup (str);
    }
  else
    {
      /* Destroy parsed elements.  */
      for (i = 0; i < new->num_elems; i++)
        {
          rec_field_name_destroy (new->elems[i]->field_name);
          free (new->elems[i]->str);
          free (new->elems[i]);
        }
    }

  return res;
}

static bool
rec_fex_parse_str_subscripts (rec_fex_t new,
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
      if (!elem)
        {
          /* Out of memory.  */
          res = false;
          break;
        }
      if (!rec_fex_parse_elem (elem, elem_str))
        {
          /* Parse error.  */
          for (i = 0; i < new->num_elems; i++)
            {
              free (new->elems[i]);
            }

          res = false;
          break;
        }

      /* Add the elem to the FEX.  */
      new->elems[new->num_elems++] = elem;
    }
  while ((elem_str = strsep (&fex_str, ",")));

  if (res)
    {
      new->str = strdup (str);
    }

  free (fex_str);
  free (elem_str);

  return res;
}

static bool
rec_fex_parse_elem (rec_fex_elem_t elem,
                    char *str)
{
  bool ret;
  char *b, *p;
  char *field_name_str;

  ret = true;
  p = str;

  /* 'Empty' part.  */
  elem->field_name = NULL;
  elem->min = -1;
  elem->max = -1;

  /* Syntax:
   *
   *    [/]FNAME[min-max]
   */

  /* Get the field name.  */
  b = p;
  while ((*p != 0) && (*p != '['))
    {
      p++;
    }

  if ((p - b) > 0)
    {
      size_t size = (p - b) + 1;

      field_name_str = malloc (size + 1);
      if (!field_name_str)
        {
          /* End of memory.  */
          return false;
        }

      strncpy (field_name_str, b, size - 1);
      field_name_str[size - 1] = ':';
      field_name_str[size] = '\0';

      elem->field_name = rec_parse_field_name_str (field_name_str);
      elem->str = strdup (field_name_str);
    }

  /* Get the subscript, if any.  */
  if ((elem->field_name)
      && (*p == '['))
    {
      char number[100];
      size_t number_size = 0;

      p++;
      while ((*p != 0) && (*p <= '9') && (*p >= '0'))
        {
          number[number_size++] = *p;
          p++;
        }
      number[number_size] = 0;

      if (*p == ']')
        {
          /* The following call cannot fail.  */
          rec_atoi (number, &(elem->min));
        }
      else 
        {
          /* Expected ]: parse error. */
          ret = false;
        }
    }
  

  if (!(elem->field_name))
    {
      /* No field name: parse error.  */
      ret = false;
    }

  return ret;
}

/* End of rec-fex.c */
