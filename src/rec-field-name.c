/* -*- mode: C -*- Time-stamp: "2010-04-09 16:25:54 jco"
 *
 *       File:         rec-field-name.c
 *       Date:         Fri Dec 25 17:27:05 2009
 *
 *       GNU recutils - Field names.
 *
 */

/* Copyright (C) 2009 Jose E. Marchesi */

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

#include <rec.h>

/* Field name Data Structure
 *
 * A field name is an ordered set of strings.
 */

 /* parts[0] => Type
    parts[1] => Name
    parts[2] => Role */
#define NAME_MAX_PARTS 3

struct rec_field_name_s
{
  char *parts[NAME_MAX_PARTS];
  int size;
};

rec_field_name_t
rec_field_name_new ()
{
  int i;
  rec_field_name_t new;

  new = malloc (sizeof (struct rec_field_name_s));

  if (new)
    {
      new->size = 0;
      for (i = 0; i < NAME_MAX_PARTS; i++)
        {
          new->parts[i] = NULL;
        }
    }

  return new;
}

void
rec_field_name_destroy (rec_field_name_t fname)
{
  int i;

  for (i = 0;
       i < fname->size;
       i++)
    {
      free (fname->parts[i]);
    }
      
  free (fname);
}

int
rec_field_name_size (rec_field_name_t fname)
{
  return fname->size;
}

const char *
rec_field_name_get (rec_field_name_t fname,
                    int index)
{
  const char *ret;

  if ((index < 0)
      || (index >= fname->size))
    {
      ret = NULL;
    }
  else
    {
      ret = fname->parts[index];
    }

  return ret;
}

bool
rec_field_name_set (rec_field_name_t fname,
                    int index,
                    const char *str)
{
  bool ret;

  ret = true;

  if ((index < 0) 
      || (index >= NAME_MAX_PARTS))
    {
      ret = false;
    }
  else
    {
      if (fname->parts[index])
        {
          free (fname->parts[index]);
          fname->parts[index] = NULL;
        }
      
      if (str)
        {
          fname->parts[index] = strdup (str);
          fname->size = index + 1;
        }
    }

  return ret;
}

rec_field_name_t
rec_field_name_dup (rec_field_name_t fname)
{
  rec_field_name_t new;
  int i;

  new = rec_field_name_new ();
  if (new)
    {
      for (i = 0; i < rec_field_name_size (fname); i++)
        {
          /* Note that 'i' cannot overflow in the following call */
          rec_field_name_set (new,
                              i,
                              rec_field_name_get (fname, i));
        }
    }

  return new;
}

bool
rec_field_name_eql_p (rec_field_name_t fname1,
                      rec_field_name_t fname2)
{
  int i;
  bool ret;

  ret = true;

  if (rec_field_name_size (fname1) 
      == rec_field_name_size (fname2))
    {
      for (i = 0; i < rec_field_name_size (fname1); i++)
        {
          if (strcmp (rec_field_name_get (fname1, i),
                      rec_field_name_get (fname2, i)) != 0)
            {
              ret = false;
              break;
            }
        }
    }
  else
    {
      ret = false;
    }

  return ret;
}

bool
rec_field_name_equal_p (rec_field_name_t fname1,
                        rec_field_name_t fname2)
{
  int i;
  bool ret;
  char *role1;
  char *role2;

  ret = true;

  if ((rec_field_name_size (fname1) == 1)
      && (rec_field_name_size (fname2) == 3))
    {
      role1 = fname1->parts[0];
      role2 = fname2->parts[2];

      ret = (strcmp (role1, role2) == 0);
    }
  else if ((rec_field_name_size (fname1) == 3)
           && (rec_field_name_size (fname2) == 1))
    {
      role1 = fname1->parts[2];
      role2 = fname2->parts[0];

      ret = (strcmp (role1, role2) == 0);
    }
  else
    {
      return rec_field_name_eql_p (fname1, fname2);
    }

  return ret;
}

/* End of rec-field-name.c */
