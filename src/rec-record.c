/* -*- mode: C -*- Time-stamp: "2010-04-01 16:23:41 jemarch"
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

#include <gl_array_list.h>
#include <gl_list.h>

#include <rec.h>

/* Static functions defined in this file */
static bool rec_record_elem_equal_p (pdf_record_elem_t elem1,
                                     pdf_record_elem_t elem2);
static void rec_record_elem_destroy (pdf_record_elem_t elem);

static bool rec_record_field_equals_fn (const void *elt1,
                                        const void *elt2);
static void rec_record_field_dispose_fn (const void *elt);


/* Record data structures.
 *
 * A record is an ordered set of one or more elements.  Elements can
 * be of two types: comments and fields.  A record should have at
 * least one field.
 */

#define REC_RECORD_ELEM_COMMENT 0
#define REC_RECORD_ELEM_FIELD 1

#define ELEM_FIELD_P(elem) ((elem)->type == REC_RECORD_ELEM_FIELD)
#define ELEM_COMMENT_P(elem) ((elem)->type == REC_RECORD_ELEM_COMMENT)

struct rec_record_elem_s
{
  int type;
  union
  {
    rec_field_t field;
    char *comment;
  } value;

  struct rec_record_elem_s *next_field;
  struct rec_record_elem_s *next_comment;
};

typedef struct rec_elem_s *rec_elem_t;

struct rec_record_s
{
  /* Statistics about the contents of the record.  */
  int num_elements;
  int num_fields;
  int num_comments;

  /* List of elements.  */
  gl_list_t elem_list;
};

rec_record_t
rec_record_new (void)
{
  rec_record_t record;

  record = malloc (sizeof (struct rec_record_s));

  if (record != NULL)
    {
      record->num_elements = 0;
      record->num_fields = 0;
      record->num_comments = 0;

      record->elem_list = gl_list_nx_create_empty (GL_ARRAY_LIST,
                                                   rec_record_elem_equals_fn,
                                                   NULL,
                                                   rec_record_elem_dispose_fn,
                                                   true);
      
      if (record->elem_list == NULL)
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
  gl_list_free (record->elem_list);
  free (record);
}

int
rec_record_num_elems (rec_record_t record)
{
  return record->num_elements;
}

int
rec_record_num_fields (rec_record_t record)
{
  return record->num_fields;
}

int
rec_record_num_comments (rec_record_t record)
{
  return record->num_comments;
}

bool
rec_record_field_p (rec_record_t record,
                    rec_field_name_t field_name)
{
  bool found;
  rec_elem_t elem;
  gl_list_iterator_t iter;

  found = false;
  iter = gl_list_iterator (record->elem_list);
  
  while (gl_list_iterator_next (&iter, (const void **) &elem, NULL))
    {            
      if (ELEM_FIELD_P(elem)
          && (rec_field_name_equal_p (field_name,
                                      rec_field_name (elem->value.field))))
        {
          found = true;
          break;
        }
    }

  gl_list_iterator_free (&iter);

  return found;
}

bool
rec_record_remove_field (rec_record_t record,
                         int position)
{
  bool removed;

  removed = false;

  if (record->size > 0)
    {
      if (position < 0)
        {
          position = 0;
        }
      if (position >= record->size)
        {
          position = record->size - 1;
        }
      
      if (gl_list_remove_at (record->field_list,
                             position))
        {
          record->size--;
          removed = true;
        }
    }
     
  return removed;
}

rec_field_t
rec_record_get_field (rec_record_t record,
                      int position)
{
  rec_field_t field;

  field = NULL;

  if (record->size > 0)
    {
      if (position < 0)
        {
          position = 0;
        }
      if (position >= record->size)
        {
          position = record->size - 1;
        }

      field = (rec_field_t) gl_list_get_at (record->field_list,
                                            position);
    }

  return field;
}

rec_field_t
rec_record_get_field_by_name (rec_record_t record,
                              rec_field_name_t fname,
                              int n)
{
  rec_field_t ret;
  rec_field_t field;
  rec_field_name_t field_name;
  const char *field_name_str;
  int i;
  int num_found;

  ret = NULL;
  num_found = 0;
  
  for (i = 0; i < rec_record_size (record); i++)
    {
      field = rec_record_get_field (record, i);
      field_name = rec_field_name (field);
      if (rec_field_name_equal_p (field_name,
                                  fname))
        {
          if (n == num_found)
            {
              ret = field;
              break;
            }
          else
            {
              num_found++;
            }
        }
    }

  return ret;
}

bool
rec_record_insert_field (rec_record_t record,
                         rec_field_t field,
                         int position)
{
  gl_list_node_t node;
    
  node = NULL;

  if (position < 0)
    {
      node = gl_list_nx_add_first (record->field_list,
                                   (void *) field);
    }
  else if (position >= record->size)
    {
      node = gl_list_nx_add_last (record->field_list,
                                  (void *) field);
    }
  else
    {
      node = gl_list_nx_add_at (record->field_list,
                                position,
                                (void *) field);
    }

  if (node != NULL)
    {
      record->size++;
      return true;
    }

  return false;
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
      field1 = rec_record_get_field (record1, index1);

      for (index2 = 0; index2 < rec_record_size (record2); index2++)
        {
          field2 = rec_record_get_field (record2, index2);

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
      field = rec_record_get_field (record, index);

      new_field = rec_field_dup (field);
      rec_record_insert_field (new_record, new_field, index);
    }

  return new_record;
}

int
rec_record_get_num_fields (rec_record_t record,
                           rec_field_name_t field_name)
{
  rec_field_t field;
  int i;
  int n;

  n = 0;
  for (i = 0; i < rec_record_size (record); i++)
    {
      field = rec_record_get_field (record, i);

      if (rec_field_name_equal_p (field_name,
                                  rec_field_name (field)))
        {
          n++;
        }
    }

  return n;
}

void
rec_record_remove_field_by_name (rec_record_t record,
                                 rec_field_name_t field_name,
                                 int index)
{
  rec_field_t field;
  gl_list_node_t node;
  gl_list_iterator_t iter;
  int i;


  i = 0;
  iter = gl_list_iterator (record->field_list);
  while (gl_list_iterator_next (&iter, (const void **) &field, &node))
    {
      if (((index == -1) || (index == i))
          && (rec_field_name_equal_p (field_name,
                                      rec_field_name (field))))
        {
          /* Delete and advance.  */
          gl_list_remove_node (record->field_list,
                               node);
          record->size--;
          i++;
        }
    }
}

bool
rec_record_comment_p (rec_record_t record)
{
  return ((record->size == 0)
          && (record->comment != NULL));
}

bool
rec_record_p (rec_record_t record)
{
  return (!rec_record_comment_p (record));
}

char *
rec_record_comment (rec_record_t record)
{
  return record->comment;
}

void
rec_record_set_comment (rec_record_t record,
                        char *comment)
{
  if (record->comment != NULL)
    {
      free (record->comment);
    }

  record->comment = strdup (comment);
}

/*
 * Private functions
 */

static bool
rec_record_elem_equal_p (pdf_record_elem_t elem1,
                         pdf_record_elem_t elem2)
{
  bool result;

  result = elem1->type = elem2->type;
  if (result)
    {
      switch (elem1->type)
        {
        case REC_RECORD_ELEM_COMMENT:
          {
            result = (strcmp (elem1->value.comment,
                              elem2->value.comment) == 0);
            break;
          }
        case REC_RECORD_ELEM_FIELD:
          {
            result = pdf_field_equal_p (elem1->value.field,
                                        elem2->value.field);
            break;
          }
        }
    }

  return result;
}

static void
rec_record_elem_destroy (pdf_record_elem_t elem)
{
  switch (elem->type)
    {
    case REC_RECORD_ELEM_COMMENT:
      {
        free (elem->value.comment);
        break;
      }
    case REC_RECORD_ELEM_FIELD:
      {
        rec_field_destroy (elem->value.field);
        break;
      }
    }

  free (elem);
}

static bool
rec_record_elem_equals_fn (const void *e1,
                           const void *e2)
{
  return rec_record_elem_equal_p ((rec_record_elem_t) e1,
                                  (rec_record_elem_t) e2);
}

static void
rec_record_elem_dispose_fn (const void *e)
{
  rec_record_elem_destroy ((rec_record_elem_t) elt);
}

/* End of rec-record.c */
