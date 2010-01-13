/* -*- mode: C -*- Time-stamp: "10/01/13 16:38:29 jemarch"
 *
 *       File:         rec-field.c
 *       Date:         Fri Feb 27 20:40:26 2009
 *
 *       GNU rec library - Fields
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
#include <stdio.h>

#include <rec.h>

/* Field Data Structure.
 *
 * A field is a name-value pair.
 */
struct rec_field_s
{
  rec_field_name_t name;
  char *value; /* NULL-terminated string containing the text value of
                  the field. */
};

rec_field_name_t
rec_field_name (rec_field_t field)
{
  return field->name;
}

void
rec_field_set_name (rec_field_t field,
                    rec_field_name_t fname)
{
  if (field->name != NULL)
    {
      /* Release previously used memory */
      rec_field_name_destroy (field->name);
      field->name = NULL;
    }

  field->name = fname;
}

char *
rec_field_value (rec_field_t field)
{
  return field->value;
}

void
rec_field_set_value (rec_field_t field,
                     const char *value)
{
  if (field->value != NULL)
    {
      /* Release previously used memory */
      free (field->value);
      field->value = NULL;
    }

  field->value = strdup (value);
}

rec_field_t
rec_field_new (rec_field_name_t name,
               const char *value)
{
  rec_field_t field;

  field = malloc (sizeof (struct rec_field_s));

  if (field != NULL)
    {
      field->name = NULL;
      field->value = NULL;

      rec_field_set_name (field, name);
      rec_field_set_value (field, value);
    }
  
  return field;
}

rec_field_t
rec_field_dup (rec_field_t field)
{
  rec_field_t new_field;

  new_field = rec_field_new (rec_field_name_dup (rec_field_name (field)),
                             rec_field_value (field));

  return new_field;
}

bool
rec_field_equal_p (rec_field_t field1,
                   rec_field_t field2)
{
  return (rec_field_name_equal_p (field1->name,
                                  field2->name));
}

void
rec_field_destroy (rec_field_t field)
{
  if (field->name)
    {
      rec_field_name_destroy (field->name);
    }

  free (field);
}

/* End of rec-field.c */
