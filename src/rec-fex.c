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

#include <stdlib.h>
#include <string.h>
#include <gettext.h>
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

#define REC_FEX_MAX_ELEMS 256

struct rec_fex_s
{
  int num_elems;
  char *str;
  rec_fex_elem_t elems[REC_FEX_MAX_ELEMS];
};

/*
 * Static function declarations.
 */

static bool rec_fex_parse_str_simple (rec_fex_t new, char *str, char *sep);
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
      new->str = NULL;
      for (i = 0; i < REC_FEX_MAX_ELEMS; i++)
        {
          new->elems[i] = 0;
        }

      if (str != NULL)
        {
          /* Parse the string.  */
          if (kind == REC_FEX_SUBSCRIPTS)
            {
              if (!rec_fex_parse_str_subscripts (new, str))
                {
                  free (new);
                  new = NULL;
                }
            }
          else if (kind == REC_FEX_SIMPLE)
            {
              /* Simple FEX.  */
              if (!rec_fex_parse_str_simple (new, str, " \t\n"))
                {
                  free (new);
                  new = NULL;
                }
            }
          else /* REC_FEX_CSV */
            {
              /* Simple FEX with fields separated by commas.  */
              if (!rec_fex_parse_str_simple (new, str, ","))
                {
                  free (new);
                  new = NULL;
                }
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

      free (fex->elems[i]->str);
      free (fex->elems[i]);
    }

  free (fex->str);
  free (fex);
}

bool
rec_fex_check (char *str, enum rec_fex_kind_e kind)
{
  int ret;
  char *regexp_str;

  switch (kind)
    {
    case REC_FEX_SIMPLE:
      {
        regexp_str = "^" REC_FNAME_LIST_RE "$";
        break;
      }
    case REC_FEX_CSV:
      {
        regexp_str = "^" REC_FNAME_LIST_CS_RE "$";
        break;
      }
    case REC_FEX_SUBSCRIPTS:
      {
        regexp_str = "^" REC_FNAME_LIST_SUB_RE "$";
        break;
      }
    default:
      {
        regexp_str = NULL;
        break;
      }
    }

  return rec_match (str, regexp_str);
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

void
rec_fex_elem_set_field_name (rec_fex_elem_t elem,
                             rec_field_name_t fname)
{
  if (elem->field_name)
    {
      rec_field_name_destroy (elem->field_name);
      elem->field_name = NULL;
    }

  elem->field_name = rec_field_name_dup (fname);
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

char *
rec_fex_str (rec_fex_t fex,
             enum rec_fex_kind_e kind)
{
  char *result;
  size_t result_size;
  rec_buf_t buf;
  size_t i;
  char *field_str;
  char *tmp;

  result = NULL;
  buf = rec_buf_new (&result, &result_size);
  if (buf)
    {
      for (i = 0; i < fex->num_elems; i++)
        {
          if (i != 0)
            {
              if (kind == REC_FEX_SIMPLE)
                {
                  rec_buf_putc (' ', buf);
                }
              else
                {
                  rec_buf_putc (',', buf);
                }
            }
          
          field_str = rec_write_field_name_str (fex->elems[i]->field_name,
                                                REC_WRITER_NORMAL);

          /* People usually don't write the final ':' in fexes, so
             remove it.  */
          if (field_str[strlen(field_str) - 1] == ':')
            {
              field_str[strlen(field_str) - 1] = '\0';
            }
          
          rec_buf_puts (field_str, buf);
          free (field_str);

          if (kind == REC_FEX_SUBSCRIPTS)
            {
              if ((fex->elems[i]->min != -1)
                  || (fex->elems[i]->max != -1))
                {
                  rec_buf_putc ('[', buf);
                  if (fex->elems[i]->min != -1)
                    {
                      asprintf (&tmp, "%d", fex->elems[i]->min);
                      rec_buf_puts (tmp, buf);
                      free (tmp);
                    }
                  if (fex->elems[i]->max != -1)
                    {
                      asprintf (&tmp, "-%d", fex->elems[i]->max);
                      rec_buf_puts (tmp, buf);
                      free (tmp);
                    }

                  rec_buf_putc (']', buf);
                }
            }
        }
    }

  rec_buf_close (buf);

  return result;
}

bool
rec_fex_member_p (rec_fex_t fex,
                  rec_field_name_t fname,
                  int min,
                  int max)
{
  bool res = false;
  int i;
  
  for (i = 0; i < fex->num_elems; i++)
    {
      if (rec_field_name_equal_p (fname,
                                  fex->elems[i]->field_name)
          && ((min == -1) || (fex->elems[i]->min == min))
          && ((max == -1) || (fex->elems[i]->max == max)))
        {
          res = true;
          break;
        }
    }

  return res;
}

void
rec_fex_append (rec_fex_t fex,
                rec_field_name_t fname,
                int min,
                int max)
{
  rec_fex_elem_t new_elem;

  if (fex->num_elems >= REC_FEX_MAX_ELEMS)
    {
      fprintf (stderr, _("internal error: REC_FEX_MAX_ELEMS exceeded.  Please report this.\n"));
      return;
    }

  new_elem = malloc (sizeof (struct rec_fex_elem_s));
  if (new_elem)
    {
      new_elem->field_name = rec_field_name_dup (fname);
      new_elem->str = rec_write_field_name_str (fname, REC_WRITER_NORMAL);
      new_elem->min = min;
      new_elem->max = max;
      fex->elems[fex->num_elems++] = new_elem;
    }
}

/*
 * Private functions.
 */

static bool
rec_fex_parse_str_simple (rec_fex_t new,
                          char *str,
                          char *sep)
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

  elem_str = strsep (&fex_str, sep);
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
  while ((elem_str = strsep (&fex_str, sep)));

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

  return res;
}

static bool
rec_fex_parse_elem (rec_fex_elem_t elem,
                    char *str)
{
  bool ret;
  const char *p;

  ret = true;
  p = str;

  /* 'Empty' part.  */
  elem->field_name = NULL;
  elem->str = NULL;
  elem->min = -1;
  elem->max = -1;


  /* Get the field name.  */
  if (!rec_parse_regexp (&p,
                         "^" REC_FNAME_RE,
                         &(elem->str)))
    {
      /* Parse error.  */
      return false;
    }
  elem->field_name = rec_parse_field_name_str (elem->str);

  /* Get the subscripts if they are present.  */
  if (*p == '[')
    {
      p++;
      /* First subscript in range.  */
      if (!rec_parse_int (&p, &(elem->min)))
        {
          /* Parse error.  */
          free (elem->str);
          rec_field_name_destroy (elem->field_name);
          return false;
        }

      if (*p == '-')
        {
          p++;
          /* Second subscript in range.  */
          if (!rec_parse_int (&p, &(elem->max)))
            {
              /* Parse error.  */
              free (elem->str);
              rec_field_name_destroy (elem->field_name);
              return false;
            }
        }

      if (*p != ']')
        {
          /* Parse error.  */
          free (elem->str);
          rec_field_name_destroy (elem->field_name);
          return false;
        }
      p++; /* Skip the ]  */
    }

  if (*p != '\0')
    {
      free (elem->str);
      rec_field_name_destroy (elem->field_name);
      return false;
    }

  return ret;
}

/* End of rec-fex.c */
