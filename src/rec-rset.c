/* -*- mode: C -*- Time-stamp: "10/01/15 11:30:29 jemarch"
 *
 *       File:         rec-rset.c
 *       Date:         Thu Mar  5 18:12:10 2009
 *
 *       GNU recutils - Record Sets
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
    /* Sentinel */
    ""
  };

static bool
rec_rset_record_equals_fn (const void *elt1, const void *elt2);
static void
rec_rset_record_dispose_fn (const void *elt);

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
      rset->record_list = gl_list_nx_create_empty (GL_ARRAY_LIST,
                                                   rec_rset_record_equals_fn,
                                                   NULL,
                                                   rec_rset_record_dispose_fn,
                                                   true);

      if (rset->record_list == NULL)
        {
          /* Out of memory */
          free (rset);
          rset = NULL;
        }
    }

  return rset;
}

void
rec_rset_destroy (rec_rset_t rset)
{
  if (rset->descriptor)
    {
      rec_record_destroy (rset->descriptor);
    }
  gl_list_free (rset->record_list);
}

int
rec_rset_size (rec_rset_t rset)
{
  return rset->size;
}

rec_record_t
rec_rset_get_record (rec_rset_t rset,
                     int position)
{
  rec_record_t record;

  record = NULL;

  if (rset->size > 0)
    {
      if (position < 0)
        {
          position = 0;
        }
      if (position >= rset->size)
        {
          position = rset->size - 1;
        }

      record = (rec_record_t) gl_list_get_at (rset->record_list,
                                              position);
    }
  
  return record;
}

bool
rec_rset_insert_record (rec_rset_t rset,
                        rec_record_t record,
                        int position)
{
  gl_list_node_t node;

  node = NULL;

  if (position < 0)
    {
      node = gl_list_nx_add_first (rset->record_list,
                                   (void *) record);
    }
  else if (position >= rset->size)
    {
      node = gl_list_nx_add_last (rset->record_list,
                                  (void *) record);
    }
  else
    {
      node = gl_list_nx_add_at (rset->record_list,
                                position,
                                (void *) record);
    }


  if (node != NULL)
    {
      rset->size++;
      return true;
    }

  return false;
}

bool
rec_rset_remove_record (rec_rset_t rset,
                        int position)
{
  bool removed;

  removed = false;

  if (rset->size > 0)
    {
      if (position < 0)
        {
          position = 0;
        }
      if (position >= rset->size)
        {
          position = rset->size - 1;
        }

      if (gl_list_remove_at (rset->record_list,
                             position))
        {
          rset->size--;
          removed = true;
        }
    }

  return removed;
}

rec_record_t
rec_rset_descriptor (rec_rset_t rset)
{
  return rset->descriptor;
}

void
rec_rset_set_descriptor (rec_rset_t rset, rec_record_t record)
{
  if (rset->descriptor)
    {
      rec_record_destroy (rset->descriptor);
    }
  rset->descriptor = record;
}

char *
rec_rset_type (rec_rset_t rset)
{
  char *res;
  rec_record_t descriptor;
  rec_field_t field;
  rec_field_name_t field_name;

  res = NULL;
  if (rset->descriptor)
    {
      field_name = rec_parse_field_name_str ("%rec:");
      field = rec_record_get_field_by_name (rset->descriptor,
                                            field_name,
                                            0);
      if (field)
        {
          res = rec_field_value (field);
        }
    }

  return res;
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

/* End of rec-rset.c */
