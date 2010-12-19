/* -*- mode: C -*-
 *
 *       File:         rec-field.c
 *       Date:         Fri Feb 27 20:40:26 2009
 *
 *       GNU recutils - Fields
 *
 */

/* Copyright (C) 2009, 2010 Jose E. Marchesi */

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

  /* Localization.  */
  char *source;
  size_t location;
  char *location_str;
  size_t char_location;
  char *char_location_str;
};

rec_field_name_t
rec_field_name (rec_field_t field)
{
  return field->name;
}

char *
rec_field_name_str (rec_field_t field)
{
  return rec_write_field_name_str (rec_field_name (field),
                                   REC_WRITER_NORMAL);
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

      /* Localization is unused by default.  */
      field->source = NULL;
      field->location = 0;
      field->location_str = NULL;
      field->char_location = 0;
      field->char_location_str = NULL;
    }
  
  return field;
}

rec_field_t
rec_field_new_str (const char *name,
                   const char *value)
{
  rec_field_t field;
  rec_field_name_t field_name;
  
  field = NULL;

  field_name = rec_parse_field_name_str ((char *) name);
  if (field_name)
    {
      field = rec_field_new (field_name, value);
    }

  return field;
}

rec_field_t
rec_field_dup (rec_field_t field)
{
  rec_field_t new_field;

  new_field = rec_field_new (rec_field_name_dup (rec_field_name (field)),
                             rec_field_value (field));


  new_field->location = field->location;
  new_field->char_location = field->char_location;

  if (field->source)
    {
      new_field->source = strdup (field->source);
    }

  if (field->location_str)
    {
      new_field->location_str = strdup (field->location_str);
    }

  if (field->char_location_str)
    {
      new_field->char_location_str = strdup (field->char_location_str);
    }

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

  free (field->source);
  free (field->location_str);
  free (field->char_location_str);
  free (field);
}

rec_comment_t
rec_field_to_comment (rec_field_t field)
{
  rec_comment_t res;
  char *comment_str;
  
  comment_str = rec_write_field_str (field,
                                     REC_WRITER_NORMAL);

  /* Remove a trailing newline.  */
  if (comment_str[strlen (comment_str) - 1] == '\n')
    {
      comment_str[strlen (comment_str) - 1] = '\0';
    }

  res = rec_comment_new (comment_str);
  free (comment_str);
  
  return res;
}

char *
rec_field_source (rec_field_t field)
{
  return field->source;
}

void
rec_field_set_source (rec_field_t field,
                      char *source)
{
  if (field->source)
    {
      free (field->source);
      field->source = NULL;
    }

  field->source = strdup (source);
}

size_t
rec_field_location (rec_field_t field)
{
  return field->location;
}

void
rec_field_set_location (rec_field_t field,
                        size_t location)
{
  field->location = location;

  if (field->location_str)
    {
      free (field->location_str);
      field->location_str = NULL;
    }

  field->location_str = malloc (30);
  asprintf (&(field->location_str), "%zu", field->location);
}

char *
rec_field_location_str (rec_field_t field)
{
  char *res;

  if (field->location_str)
    {
      res = field->location_str;
    }
  else
    {
      res = "";
    }

  return res;
}

size_t
rec_field_char_location (rec_field_t field)
{
  return field->char_location;
}

void
rec_field_set_char_location (rec_field_t field,
                             size_t location)
{
  field->char_location = location;

  if (field->char_location_str)
    {
      free (field->char_location_str);
      field->char_location_str = NULL;
    }

  asprintf (&(field->char_location_str), "%zu", field->char_location);
}

char *
rec_field_char_location_str (rec_field_t field)
{
  char *res;

  if (field->char_location_str)
    {
      res = field->char_location_str;
    }
  else
    {
      res = "";
    }
  
  return res;
}

/* End of rec-field.c */
