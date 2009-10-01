/* -*- mode: C -*- Time-stamp: "09/10/01 22:56:00 jemarch"
 *
 *       File:         rec-rset.c
 *       Date:         Thu Mar  5 18:12:10 2009
 *
 *       GNU rec - Record Sets
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

#include <gl_array_list.h>
#include <gl_list.h>

#include <rec.h>

/* Record Set Data Structure.
 *
 * A record set is a set of zero or more non-special records maybe
 * preceded by a record descriptor.
 */
struct rec_rset_s
{
  int size;                   /* Number of records stored in the
                                 record set. The record descriptor is
                                 not included in the count. */
  rec_record_t descriptor;    /* Record descriptor of this record
                                 set. */
  gl_list_t record_list;      /* List of ordinary records. */
};

/* Set of names for special fields */

#define REC_NAME_REC "%rec"
#define REC_NAME_KEY "%key"
#define REC_NAME_MANDATORY "%mandatory"
#define REC_NAME_UNIQUE "%unique:"

static char *special_fields[] =
  {
    REC_NAME_REC,
    REC_NAME_KEY,
    REC_NAME_MANDATORY,
    REC_NAME_UNIQUE,
    /* Centinel */
    ""
  };

static bool
rec_rset_record_equals_fn (const void *elt1, const void *elt2);
static void
rec_rset_record_dispose_fn (const void *elt);
static bool
rec_rset_record_can_be_inserted_p (rec_rset_t rset, rec_record_t record);

rec_rset_t
rec_rset_new (void)
{
  rec_rset_t rset;
  rec_record_t descriptor;
  rec_field_t field;

  /* Allocate memory for the new record set */
  rset = malloc (sizeof (struct rec_rset_s));
  
  if (rset != NULL)
    {
      rset->descriptor = NULL;
      rset->size = 0;

      /* Initialize the record list, allowing duplicates */
      rset->record_list = gl_list_create_empty (GL_ARRAY_LIST,
                                                rec_rset_record_equals_fn,
                                                NULL,
                                                rec_rset_record_dispose_fn,
                                                true);
    }

  return rset;
}

void
rec_rset_destroy (rec_rset_t rset)
{
  rec_record_destroy (rset->descriptor);
  gl_list_free (rset->record_list);
}

int
rec_rset_size (rec_rset_t rset)
{
  return rset->size;
}

rec_record_t
rec_rset_get_record_at (rec_rset_t rset,
                        int position)
{
  rec_record_t result;

  /* position sanity check */
  if ((position < 0) ||
      (position >= gl_list_size (rset->record_list)))
    {
      result = NULL;
    }
  else
    {
      result = (rec_record_t) gl_list_get_at (rset->record_list,
                                              position);
    }
  
  return result;
}

bool
rec_rset_insert_record_at (rec_rset_t rset,
                           rec_record_t record,
                           int position)
{
  bool inserted;
  int number_of_records;
  gl_list_node_t node;

  /* position sanity check */
  number_of_records = gl_list_size (rset->record_list);
  if (position < 0)
    {
      position = 0;
    }
  if (position >= number_of_records)
    {
      position = number_of_records;
    }

  inserted = false;
  if (rec_rset_record_can_be_inserted_p (rset, record))
    {
      /* Field insertion */
      node = gl_list_set_at (rset->record_list,
                             position,
                             (void *) record);
      inserted = true;
    }

  return inserted;
}

bool
rec_rset_remove_record_at (rec_rset_t rset,
                           int position)
{
  bool removed;

  /* position sanity check */
  if ((position < 0) ||
      (position >= gl_list_size (rset->record_list)))
    {
      removed = false;
    }
  else
    {
      removed = gl_list_remove_at (rset->record_list,
                                   position);
    }

  return removed;
}

bool
rec_rset_insert_record (rec_rset_t rset,
                        rec_record_t record)
{
  bool inserted;
  gl_list_node_t list_node;

  inserted = false;
  if (rec_rset_record_can_be_inserted_p (rset, record))
    {
      list_node = gl_list_add_last (rset->record_list,
                                    (void *) record);
      
      if (list_node != NULL)
        {
          inserted = true;
          rset->size++;
        }
    }

  return inserted;
}

rec_record_t
rec_rset_get_descriptor (rec_rset_t rset)
{
  return rset->descriptor;
}

void
rec_rset_set_descriptor (rec_rset_t rset, rec_record_t record)
{
  rec_record_destroy (rset->descriptor);
  rset->descriptor = record;
}

/*
 * Private functions
 */

static bool
rec_rset_record_equals_fn (const void *elt1,
                           const void *elt2)
{
  rec_record_t record1;
  rec_record_t record2;

  record1 = (rec_record_t) elt1;
  record2 = (rec_record_t) elt2;

  return rec_record_equal_p (record1,
                             record2);
}

static void
rec_rset_record_dispose_fn (const void *elt)
{
  rec_record_t record;

  record = (rec_record_t) elt;
  rec_record_destroy (record);
}

static bool
rec_rset_record_can_be_inserted_p (rec_rset_t rset,
                                   rec_record_t record)
{
  /* The record can be inserted if and only if it
   *
   * - It does not contain a field defined as key having a value
   *   already present in some record in the set.
   *
   * - It does not contain duplicated fields defined as unique.
   *
   * - It does not contain a field defined as mandatory.
   */

  bool can_be_inserted;
  rec_record_t descriptor;
  rec_field_t field;
  int index;

  can_be_inserted = true;

  /* Get the special record of the record set and iterate on its
     fields */
  descriptor = rec_rset_get_descriptor (rset);

  for (index = 0;
       index < rec_record_size (descriptor);
       index++)
    {
      field = rec_record_get_field_at (descriptor,
                                       index);

      if (strcmp (rec_field_get_name (field),
                  REC_NAME_MANDATORY) == 0)
        {
          /* Mandatory field => the record should contain this
             field */
          if (!rec_record_field_p (record, rec_field_get_value (field)))
            {
              can_be_inserted = false;
              break;
            }
        }
      if (strcmp (rec_field_get_name (field),
                  REC_NAME_UNIQUE) == 0)
        {
          /* Unique field => if the record has a field with this name
             then no other record in the set can have the same
             NAME->VALUE field */
          if (rec_record_field_p (record, rec_field_get_value (field)))
            {
              /* XXX */
            }
        }
    }

  return can_be_inserted;
}

/* End of rec-rset.c */
