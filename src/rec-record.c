/* -*- mode: C -*- Time-stamp: "09/10/01 23:04:33 jemarch"
 *
 *       File:         rec-record.c
 *       Date:         Thu Mar  5 17:11:41 2009
 *
 *       GNU rec - Records
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

#include <gl_array_list.h>
#include <gl_list.h>

#include <rec.h>

/* Static functions defined in this file */
static bool rec_record_field_equals_fn (const void *elt1,
                                        const void *elt2);
static void rec_record_field_dispose_fn (const void *elt);


/* Record Data Structure.
 *
 * A record is an ordered set of one or more fields.
 */
struct rec_record_s
{
  int size;             /* Number of fields contained in this
                           record */
  gl_list_t field_list; /* List of fields contained in this record */
};

rec_record_t
rec_record_new (void)
{
  rec_record_t record;

  record = malloc (sizeof (struct rec_record_s));

  if (record != NULL)
    {
      record->size = 0;
      record->field_list = gl_list_create_empty (GL_ARRAY_LIST,
                                                 rec_record_field_equals_fn,
                                                 NULL,
                                                 rec_record_field_dispose_fn,
                                                 true);
      
      if (record->field_list == NULL)
        {
          /* Out of memory */
          free (record);
          record = NULL;
        }
    }

  return record;
}

void
rec_record_destroy (rec_record_t record)
{
  gl_list_free (record->field_list);
  free (record);
}

int
rec_record_size (rec_record_t record)
{
  return record->size;
}

bool
rec_record_field_p (rec_record_t record,
                    const char *field_name)
{
  bool found;
  gl_list_node_t list_node;
  rec_field_t field;

  found = false;

  /* Create a dummy field to use with gl_list_search. Unfortunately
     this means that we are allocating memory here. An alternative
     would be to use a list iterator - jemarch */
  field = rec_field_new (field_name, "");

  if (field != NULL)
    {

      list_node = gl_list_search (record->field_list,
                                  field);

      if (list_node != NULL)
        {
          found = true;
        }

      rec_field_destroy (field);
    }

  return found;
}

bool
rec_record_insert_field (rec_record_t record,
                         rec_field_t field)
{
  gl_list_node_t list_node;

  list_node = gl_list_add_last (record->field_list,
                                (void *) field);
  record->size++;

  return true;
}

bool
rec_record_remove_field (rec_record_t record,
                         const char *field_name)
{
  bool removed;
  rec_field_t field;
  gl_list_node_t list_node;

  removed = false;

  /* Create a dummy field to use with gl_list_search. Unfortunately
     this is allocating memory. An alternative would be to use list
     iterators -jemarch */
  field = rec_field_new (field_name, "");

  if (field != NULL)
    {
      removed = gl_list_remove (record->field_list,
                                (void *) field);
      rec_field_destroy (field);
    }

  if (removed)
    {
      record->size--;
    }

  return removed;
}

rec_field_t
rec_record_get_field (rec_record_t record,
                      const char *field_name)
{
  rec_field_t result;
  rec_field_t field;
  gl_list_node_t list_node;

  result = NULL;

  /* Create a dummy rec_field_t to use with gl_list_search */
  field = rec_field_new (field_name, "");
  if (field != NULL)
    {
      list_node = gl_list_search (record->field_list,
                                  field);
      if (list_node != NULL)
        {
          result = (rec_field_t)
            gl_list_node_value (record->field_list,
                                list_node);
        }

      rec_field_destroy (field);
    }

  return result;
}

rec_field_t
rec_record_get_field_at (rec_record_t record,
                         int position)
{
  rec_field_t result;

  /* position sanity check */
  if ((position < 0) ||
      (position >= gl_list_size (record->field_list)))
    {
      result = NULL;
    }
  else
    {
      result = (rec_field_t) gl_list_get_at (record->field_list,
                                             position);
    }

  return result;
}

bool
rec_record_insert_field_at (rec_record_t record,
                            rec_field_t field,
                            int position)
{
  bool inserted;
  int number_of_fields;
  gl_list_node_t node;
    
  /* position sanity check */
  number_of_fields = gl_list_size (record->field_list);
  if (position < 0)
    {
      position = 0;
    }
  if (position >= number_of_fields)
    {
      position = number_of_fields;
    }

  /* Field insertion */
  node = gl_list_set_at (record->field_list,
                         position,
                         (void *) field);

  inserted = true;
  return inserted;
}

bool
rec_record_equal_p (rec_record_t record1,
                    rec_record_t record2)
{
  return ((rec_record_subset_p (record1, record2)) &&
          (rec_record_subset_p (record2, record1)));
}

bool
rec_record_subset_p (rec_record_t record1,
                     rec_record_t record2)
{
  bool result;
  bool field_found;
  int index1;
  int index2;
  rec_field_t field1;
  rec_field_t field2;

  result = true;
  for (index1 = 0; index1 < rec_record_size (record1); index1++)
    {
      field_found = false;
      field1 = rec_record_get_field_at (record1, index1);

      for (index2 = 0; index2 < rec_record_size (record2); index2++)
        {
          field2 = rec_record_get_field_at (record2, index2);

          if (rec_field_equal_p (field1, field2))
            {
              field_found = true;
              break;
            }
        }

      if (!field_found)
        {
          result = false;
          break;
        }
    }

  return result;
}

rec_record_t
rec_record_dup (rec_record_t record)
{
  rec_record_t new_record;
  int index;
  rec_field_t field;
  rec_field_t new_field;

  new_record = rec_record_new ();
  for (index = 0; index < rec_record_size (record); index++)
    {
      field = rec_record_get_field_at (record, index);
      
      new_field = rec_field_new (rec_field_get_name (field),
                                 rec_field_get_value (field));

      rec_record_insert_field (new_record, new_field);
    }

  return new_record;
}

/*
 * Private functions
 */

static bool
rec_record_field_equals_fn (const void *elt1,
                            const void *elt2)
{
  rec_field_t field1;
  rec_field_t field2;

  field1 = (rec_field_t) elt1;
  field2 = (rec_field_t) elt2;

  return rec_field_equal_p (field1,
                            field2);
}

static void
rec_record_field_dispose_fn (const void *elt)
{
  rec_field_t field;

  field = (rec_field_t) elt;
  rec_field_destroy (field);
}

/* End of rec-record.c */
