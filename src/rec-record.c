/* -*- mode: C -*- Time-stamp: "2010-04-08 18:20:08 jemarch"
 *
 *       File:         rec-record.c
 *       Date:         Thu Mar  5 17:11:41 2009
 *
 *       GNU recutils - Records
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
#include <malloc.h>
#include <string.h>

#include <rec-mset.h>
#include <rec.h>

/*
 * Record data structures.
 */

struct rec_record_s
{
  /* type ids for the elements stored in the mset.  */
  int field_type;
  int comment_type;

  /* The mset.  */
  rec_mset_t mset;
};

/* Static functions implemented below.  */

static void rec_record_field_disp_fn (void *data);
static bool rec_record_field_equal_fn (void *data1, void *data2);
static void *rec_record_field_dup_fn (void *data);
static void rec_record_comment_disp_fn (void *data);
static bool rec_record_comment_equal_fn (void *data1, void *data2);
static void *rec_record_comment_dup_fn (void *data);


/*
 * Public functions.
 */

rec_record_t
rec_record_new (void)
{
  rec_record_t record;

  record = malloc (sizeof (struct rec_record_s));

  if (record)
    {
      /* Create the mset.  */
      record->mset = rec_mset_new ();
      if (record->mset)
        {
          /* Register the types.  */
          record->field_type = rec_mset_register_type (record->mset,
                                                       "field",
                                                       rec_record_field_disp_fn,
                                                       rec_record_field_equal_fn,
                                                       rec_record_field_dup_fn);

          record->comment_type = rec_mset_register_type (record->mset,
                                                         "comment",
                                                         rec_record_comment_disp_fn,
                                                         rec_record_comment_equal_fn,
                                                         rec_record_comment_dup_fn);
        }
      else
        {
          /* Error.  */
          free (record);
          record = NULL;
        }
    }

  return record;
}

void
rec_record_destroy (rec_record_t record)
{
  rec_mset_destroy (record->mset);
  free (record);
}

rec_record_t
rec_record_dup (rec_record_t record)
{
  rec_record_t new;

  new = malloc (sizeof (struct rec_record_s));
  if (new)
    {
      new->field_type = record->field_type;
      new->comment_type = record->comment_type;
      new->mset = rec_mset_dup (record->mset);
    }

  return new;
}

bool
rec_record_subset_p (rec_record_t record1,
                     rec_record_t record2)
{
  bool result;
  bool elem_found;
  rec_mset_elem_t elem1;
  rec_mset_elem_t elem2;

  result = true;

  elem1 = NULL;
  while (elem1 = rec_mset_next (record1->mset, elem1, MSET_ANY))
    {
      elem_found = false;

      elem2 = NULL;
      while (elem2 = rec_mset_next (record2->mset, elem2, MSET_ANY))
        {
          if (rec_mset_elem_equal_p (elem1, elem2))
            {
              elem_found = true;
              break;
            }
        }

      if (!elem_found)
        {
          result = false;
          break;
        }
    }

  return result;
}

bool
rec_record_equal_p (rec_record_t record1,
                    rec_record_t record2)
{
  return ((rec_record_subset_p (record1, record2)) &&
          (rec_record_subset_p (record2, record1)));
}

int
rec_record_num_elems (rec_record_t record)
{
  return rec_mset_count (record->mset,
                         MSET_ANY);
}

int
rec_record_num_fields (rec_record_t record)
{
  return rec_mset_count (record->mset,
                         record->field_type);
}

int
rec_record_num_comments (rec_record_t record)
{
  return rec_mset_count (record->mset,
                         record->comment_type);
}

rec_record_elem_t
rec_record_get_elem (rec_record_t record,
                     int position)
{
  rec_record_elem_t elem;

  elem.mset_elem = rec_mset_get (record->mset,
                                 MSET_ANY,
                                 position);

  return elem;
}

rec_record_elem_t
rec_record_get_field (rec_record_t record,
                      int position)
{
  rec_record_elem_t elem;

  elem.mset_elem = rec_mset_get (record->mset,
                                 record->field_type,
                                 position);

  return elem;
}

rec_record_elem_t
rec_record_get_comment (rec_record_t record,
                        int position)
{
  rec_record_elem_t elem;

  elem.mset_elem = rec_mset_get (record->mset,
                                 record->comment_type,
                                 position);

  return elem;
}

bool
rec_record_remove_at (rec_record_t record,
                      int position)
{
  return rec_mset_remove_at (record->mset, position);
}

void
rec_record_insert_at (rec_record_t record,
                      rec_record_elem_t elem,
                      int position)
{
  rec_mset_insert_at (record->mset,
                      elem.mset_elem,
                      position);
}

void
rec_record_append (rec_record_t record,
                   rec_record_elem_t elem)
{
  rec_mset_append (record->mset, elem.mset_elem);
}

void
rec_record_append_field (rec_record_t record,
                         rec_field_t field)
{
  rec_record_elem_t elem;

  elem = rec_record_elem_field_new (record, field);
  rec_mset_append (record->mset, elem.mset_elem);
}

void
rec_record_append_comment (rec_record_t record,
                           rec_comment_t comment)
{
  rec_record_elem_t elem;

  elem = rec_record_elem_comment_new (record, comment);
  rec_mset_append (record->mset,
                   elem.mset_elem);
}

rec_record_elem_t
rec_record_remove (rec_record_t record,
                   rec_record_elem_t elem)
{
  elem.mset_elem = rec_mset_remove (record->mset,
                                    elem.mset_elem);
  return elem;
}

void
rec_record_insert_after (rec_record_t record,
                         rec_record_elem_t elem,
                         rec_record_elem_t new_elem)
{
  rec_mset_insert_after (record->mset,
                         elem.mset_elem,
                         new_elem.mset_elem);
}

rec_record_elem_t
rec_record_search_field (rec_record_t record,
                         rec_field_t field)
{
  rec_record_elem_t elem;

  elem.mset_elem = rec_mset_search (record->mset,
                                    (void *) field);

  return elem;
}

rec_record_elem_t
rec_record_search_field_name (rec_record_t record,
                              rec_field_name_t field_name,
                              int n)
{
  rec_record_elem_t elem;
  rec_field_t field;
  int found;

  found = 0;
  elem.mset_elem = NULL;
  while (elem.mset_elem =
         rec_mset_next (record->mset, elem.mset_elem, record->field_type))
    {
      field = (rec_field_t) rec_mset_elem_data (elem.mset_elem);
      if (rec_field_name_equal_p (field_name,
                                  rec_field_name (field)))
        {
          found++;
          if (found == n)
            {
              /* This is the Nth field with the given field name.  */
              break;
            }
        }
    }

  return elem;
}

rec_record_elem_t
rec_record_first (rec_record_t record)
{
  rec_record_elem_t elem;

  elem.mset_elem = rec_mset_first (record->mset, MSET_ANY);
  return elem;
}

rec_record_elem_t
rec_record_first_field (rec_record_t record)
{
  rec_record_elem_t elem;

  elem.mset_elem = rec_mset_first (record->mset,
                                   record->field_type);

  return elem;
}

rec_record_elem_t
rec_record_first_comment (rec_record_t record)
{
  rec_record_elem_t elem;
  
  elem.mset_elem = rec_mset_first (record->mset,
                                   record->comment_type);

  return elem;
}

rec_record_elem_t
rec_record_next (rec_record_t record,
                 rec_record_elem_t elem)
{
  elem.mset_elem = rec_mset_next (record->mset,
                                  elem.mset_elem,
                                  MSET_ANY);

  return elem;
}

rec_record_elem_t
rec_record_next_field (rec_record_t record,
                       rec_record_elem_t elem)
{
  elem.mset_elem = rec_mset_next (record->mset,
                                  elem.mset_elem,
                                  record->field_type);

  return elem;
}

rec_record_elem_t
rec_record_next_comment (rec_record_t record,
                         rec_record_elem_t elem)
{
  elem.mset_elem = rec_mset_next (record->mset,
                                  elem.mset_elem,
                                  record->comment_type);

  return elem;
}

bool
rec_record_field_p (rec_record_t record,
                    rec_field_name_t field_name)
{
  return (rec_record_get_num_fields_by_name (record, field_name) > 0);
}

int
rec_record_get_num_fields_by_name (rec_record_t record,
                                   rec_field_name_t field_name)
{
  rec_mset_elem_t elem;
  int num_fields;
  rec_field_t field;

  num_fields = 0;

  elem = NULL;
  while (elem = rec_mset_next (record->mset, elem, record->field_type))
    {
      field = (rec_field_t) rec_mset_elem_data (elem);
      if (rec_field_name_equal_p (rec_field_name (field),
                                  field_name))
        {
          num_fields++;
        }
    }

  return num_fields;
}

rec_field_t
rec_record_get_field_by_name (rec_record_t record,
                              rec_field_name_t field_name,
                              int n)
{
  rec_mset_elem_t elem;
  int num_fields;
  rec_field_t field;

  num_fields = 0;
  elem = NULL;

  while (elem = rec_mset_next (record->mset, elem, record->field_type))
    {
      field = (rec_field_t) rec_mset_elem_data (elem);
      if (rec_field_name_equal_p (rec_field_name (field), field_name))
        {
          if (n == num_fields)
            {
              return field;
            }

          num_fields++;
        }
    }

  return NULL;
}

void
rec_record_remove_field_by_name (rec_record_t record,
                                 rec_field_name_t field_name,
                                 int index)
{
  rec_record_elem_t elem;
  rec_field_t field;
  int num_fields;

  num_fields = 0;
  elem = rec_record_first_field (record);
  while (rec_record_elem_p (elem))
    {
      field = rec_record_elem_field (elem);
      if (rec_field_name_equal_p (rec_field_name (field),
                                  field_name))
          
        {
          if ((index == -1) || (index == num_fields))
            {
              elem = rec_record_remove (record, elem);
              if (rec_record_elem_p (elem) &&
                  !rec_record_elem_field_p (record, elem))
                {
                  elem = rec_record_next_field (record, elem);
                }
            }

          num_fields++;
        }
      else
        {
          elem = rec_record_next_field (record, elem);
        }
    }
}

rec_record_elem_t
rec_record_null_elem (void)
{
  rec_record_elem_t elem;

  elem.mset_elem = NULL;

  return elem;
}

rec_record_elem_t
rec_record_elem_field_new (rec_record_t record,
                           rec_field_t field)
{
  rec_record_elem_t elem;

  elem.mset_elem = rec_mset_elem_new (record->mset, record->field_type);
  rec_mset_elem_set_data (elem.mset_elem, (void *) field);

  return elem;
}

rec_record_elem_t
rec_record_elem_comment_new (rec_record_t record,
                             rec_comment_t comment)
{
  rec_record_elem_t elem;

  elem.mset_elem = rec_mset_elem_new (record->mset, record->comment_type);
  rec_mset_elem_set_data (elem.mset_elem, (void *) comment);

  return elem;
}

bool
rec_record_elem_p (rec_record_elem_t elem)
{
  return (elem.mset_elem != NULL);
}

bool
rec_record_elem_field_p (rec_record_t record,
                         rec_record_elem_t elem)
{
  return (rec_mset_elem_type (elem.mset_elem) == record->field_type);
}

bool
rec_record_elem_comment_p (rec_record_t record,
                           rec_record_elem_t elem)
{
  return (rec_mset_elem_type (elem.mset_elem)
          == record->comment_type);
}

rec_field_t
rec_record_elem_field (rec_record_elem_t elem)
{
  return (rec_field_t) rec_mset_elem_data (elem.mset_elem);
}

rec_comment_t
rec_record_elem_comment (rec_record_elem_t elem)
{
  return (rec_comment_t) rec_mset_elem_data (elem.mset_elem);
}

rec_comment_t
rec_record_to_comment (rec_record_t record)
{
  FILE *stm;
  rec_comment_t res;
  char *comment_str;
  size_t comment_str_size;
  rec_record_elem_t elem;

  stm = open_memstream (&comment_str, &comment_str_size);

  elem = rec_record_null_elem ();
  while (rec_record_elem_p (elem = rec_record_next (record, elem)))
    {
      if (rec_record_elem_field_p (record, elem))
        {
          /* Field.  */
          fputs (rec_write_field_str (rec_record_elem_field (elem)), stm);
        }
      else
        {
          /* Comment.  */
          fputs (rec_write_comment_str (rec_comment_text (rec_record_elem_comment (elem))),
                 stm);
        }
    }

  fclose (stm);

  /* Remove a trailing newline.  */
  if (comment_str[comment_str_size - 1] == '\n')
    {
      comment_str[comment_str_size - 1] = '\0';
    }

  res = rec_comment_new (comment_str);
  free (comment_str);

  return res;
}

/*
 * Private functions
 */

static void
rec_record_field_disp_fn (void *data)
{
  rec_field_destroy ((rec_field_t) data);
}

static bool
rec_record_field_equal_fn (void *data1,
                           void *data2)
{
  return (data1 == data2);
  /*  return rec_field_equal_p ((rec_field_t) data1,
      (rec_field_t) data2);*/
}

static void *
rec_record_field_dup_fn (void *data)
{
  rec_field_t copy;

  copy = rec_field_dup ((rec_field_t) data);
  return (void *) copy;
}

static void
rec_record_comment_disp_fn (void *data)
{
  rec_comment_destroy ((rec_comment_t) data);
}

static bool
rec_record_comment_equal_fn (void *data1,
                             void *data2)
{
  return (data1 == data2);
/*  return rec_comment_equal_p ((rec_comment_t) data1,
(rec_comment_t) data2);*/
}

static void *
rec_record_comment_dup_fn (void *data)
{
  rec_comment_t copy;
  
  copy = rec_comment_dup ((rec_comment_t) data);
  return (void *) copy;
}

/* End of rec-record.c */
