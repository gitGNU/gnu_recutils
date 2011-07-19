/* -*- mode: C -*-
 *
 *       File:         rec-rset.c
 *       Date:         Thu Mar  5 18:12:10 2009
 *
 *       GNU recutils - Record Sets
 *
 */

/* Copyright (C) 2009, 2010, 2011 Jose E. Marchesi */

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
#include <stdint.h>
#include <string.h>

#include <rec-mset.h>
#include <rec.h>
#include <rec-utils.h>

/* Record Set Data Structures.
 *
 * A record set is a set of zero or more non-special records
 * intermixed with comments, maybe preceded by a record descriptor.
 */

/* The fprops structure contains useful properties associated with
   fields.  Those properties are usually extracted from the record
   descriptor of the rset.  */

struct rec_rset_fprops_s
{
  rec_field_name_t fname;

  bool auto_p;     /* Auto-field.  */
  rec_type_t type; /* The field has an anonymous type.  */
  char *type_name; /* The field has a type in the types registry.  */

  struct rec_rset_fprops_s *next;
};

typedef struct rec_rset_fprops_s *rec_rset_fprops_t;

/* The rec_rset_s structure contains the data associated with a record
   set.  */

#define REC_RSET_MAX_ORDER 256

struct rec_rset_s
{
  rec_record_t descriptor;
  size_t descriptor_pos; /* Position of the descriptor into the record
                            set.  Some comments can appear before the
                            record descriptor in the rec file, so we
                            need to track it in order to write back
                            the record properly.  */

  /* Field properties.  */
  rec_rset_fprops_t field_props;

  /* Type registry.  */
  rec_type_reg_t type_reg;

  /* Field to order by.  */
  rec_field_name_t order_by_field;
  bool ordered_p;

  /* Size constraints.  */
  size_t min_size;
  size_t max_size;

  /* Storage for records and comments.  */
  int record_type;
  int comment_type;
  rec_mset_t mset;
};

/* The 'record_type' in the mset contains a list of pointers to
   rec_rset_record_s structures, instead of pointers to rec_record_t.
   This is done in this way to allow the callbacks to access to
   properties of the containing record set by using the 'rset'
   field.  */

struct rec_rset_record_s
{
  rec_rset_t   rset;
  rec_record_t record;
};

typedef struct rec_rset_record_s *rec_rset_record_t;

/* Same for comment_type and rec_rset_comment_s.  */

struct rec_rset_comment_s
{
  rec_rset_t rset;
  rec_comment_t comment;
};

typedef struct rec_rset_comment_s *rec_rset_comment_t;

/* Set of names for special fields */

#define REC_NAME_REC "%rec"
#define REC_NAME_KEY "%key"
#define REC_NAME_MANDATORY "%mandatory"
#define REC_NAME_UNIQUE "%unique:"

/* Static functions implemented below.  */

static void rec_rset_update_types (rec_rset_t rset);
static void rec_rset_update_field_props (rec_rset_t rset);
static void rec_rset_update_size_constraints (rec_rset_t rset);

static bool rec_rset_record_equal_fn (void *data1, void *data2);
static void rec_rset_record_disp_fn (void *data);
static void *rec_rset_record_dup_fn (void *data);
static int  rec_rset_record_compare_fn (void *data1, void *data2, int type1);

static bool rec_rset_comment_equal_fn (void *data1, void *data2);
static void rec_rset_comment_disp_fn (void *data);
static void *rec_rset_comment_dup_fn (void *data);
static int  rec_rset_comment_compare_fn (void *data1, void *data2, int type2);

static bool rec_rset_type_field_p (const char *str);
static rec_fex_t rec_rset_type_field_fex (const char *str);
static char *rec_rset_type_field_type (const char *str);

static rec_rset_fprops_t rec_rset_get_props (rec_rset_t rset,
                                             rec_field_name_t fname,
                                             bool create_p);

/*
 * Public functions.
 */

rec_rset_t
rec_rset_new (void)
{
  rec_rset_t rset;
  
  rset = malloc (sizeof (struct rec_rset_s));
  if (rset)
    {
      /* Create the mset.  */
      rset->mset = rec_mset_new ();
      if (rset->mset)
        {
          /* No descriptor, initially.  */
          rset->descriptor = NULL;
          rset->descriptor_pos = 0;
          rset->min_size = 0;
          rset->max_size = SIZE_MAX;

          /* Create an empty type registry.  */
          rset->type_reg = rec_type_reg_new ();

          /* No field properties, initially.  */
          rset->field_props = NULL;

          /* No order by field, initially.  */
          rset->order_by_field = NULL;
          rset->ordered_p = false;

          /* register the types.  */
          rset->record_type = rec_mset_register_type (rset->mset,
                                                      "record",
                                                      rec_rset_record_disp_fn,
                                                      rec_rset_record_equal_fn,
                                                      rec_rset_record_dup_fn,
                                                      rec_rset_record_compare_fn);
          rset->comment_type = rec_mset_register_type (rset->mset,
                                                       "comment",
                                                       rec_rset_comment_disp_fn,
                                                       rec_rset_comment_equal_fn,
                                                       rec_rset_comment_dup_fn,
                                                       rec_rset_comment_compare_fn);
        }
      else
        {
          /* Error.  */
          free (rset);
          rset = NULL;
        }
    }

  return rset;
}

void
rec_rset_destroy (rec_rset_t rset)
{
  rec_rset_fprops_t props, aux = NULL;

  if (rset->descriptor)
    {
      rec_record_destroy (rset->descriptor);
    }

  rec_type_reg_destroy (rset->type_reg);

  props = rset->field_props;
  while (props)
    {
      aux = props;

      if (aux->type)
        {
          rec_type_destroy (aux->type);
        }
      free (aux->type_name);
      props = props->next;
      free (aux);
    }

  if (rset->order_by_field)
    {
      rec_field_name_destroy (rset->order_by_field);
    }

  rec_mset_destroy (rset->mset);
  free (rset);
}

rec_rset_t
rec_rset_dup (rec_rset_t rset)
{
  rec_rset_t new;

  new = malloc (sizeof (struct rec_rset_s));
  if (new)
    {
      new->record_type = rset->record_type;
      new->comment_type = rset->comment_type;
      new->mset = rec_mset_dup (rset->mset);
      new->min_size = rset->min_size;
      new->max_size = rset->max_size;
      /* XXX: make copies of the following structures.  */
      new->type_reg = NULL;
      new->field_props = NULL;

      if (rset->order_by_field)
        {
          new->order_by_field =
            rec_field_name_dup (rset->order_by_field);
        }
      new->ordered_p = rset->ordered_p;
    }

  return new;
}

int
rec_rset_num_elems (rec_rset_t rset)
{
  return rec_mset_count (rset->mset,
                         MSET_ANY);
}

int
rec_rset_num_records (rec_rset_t rset)
{
  return rec_mset_count (rset->mset,
                         rset->record_type);
}

int
rec_rset_num_comments (rec_rset_t rset)
{
  return rec_mset_count (rset->mset,
                         rset->comment_type);
}

rec_rset_elem_t
rec_rset_null_elem (void)
{
  rec_rset_elem_t elem;

  elem.mset_elem = NULL;

  return elem;
}

rec_rset_elem_t
rec_rset_get_elem (rec_rset_t rset,
                   int position)
{
  rec_rset_elem_t elem;

  elem.mset_elem = rec_mset_get (rset->mset,
                                 MSET_ANY,
                                 position);

  return elem;
}

rec_rset_elem_t
rec_rset_get_record (rec_rset_t rset,
                     int position)
{
  rec_rset_elem_t elem;

  elem.mset_elem = rec_mset_get (rset->mset,
                                 rset->record_type,
                                 position);

  return elem;
}

rec_rset_elem_t
rec_rset_get_comment (rec_rset_t rset,
                      int position)
{
  rec_rset_elem_t elem;

  elem.mset_elem = rec_mset_get (rset->mset,
                                 rset->comment_type,
                                 position);

  return elem;
}

bool
rec_rset_remove_at (rec_rset_t rset,
                    int position)
{
  return rec_mset_remove_at (rset->mset, position);
}

void
rec_rset_insert_at (rec_rset_t rset,
                    rec_rset_elem_t elem,
                    int position)
{
  if (rset->ordered_p
      && rset->order_by_field
      && rec_rset_elem_record_p (rset, elem))
    {
      /* Don't insert the record at the requested position: use a
         sorting criteria instead.  */
      rec_mset_add_sorted (rset->mset, elem.mset_elem);
    }
  else
    {
      /* Insert at the requested position.  */
      rec_mset_insert_at (rset->mset,
                          elem.mset_elem,
                          position);
    }
}

void
rec_rset_append (rec_rset_t rset,
                 rec_rset_elem_t elem)
{
  if (rset->ordered_p
      && rset->order_by_field
      && rec_rset_elem_record_p (rset, elem))
    {
      /* Don't append the record: use a sorting criteria instead.  */
      rec_mset_add_sorted (rset->mset, elem.mset_elem);
    }
  else
    {
      rec_mset_append (rset->mset, elem.mset_elem);
    }
}

void
rec_rset_append_record (rec_rset_t rset,
                        rec_record_t record)
{
  rec_rset_elem_t elem;
  
  elem = rec_rset_elem_record_new (rset, record);

  if (rset->ordered_p && rset->order_by_field)
    {
      rec_mset_add_sorted (rset->mset, elem.mset_elem);
    }
  else
    {
      rec_mset_append (rset->mset, elem.mset_elem);
    }
}

void
rec_rset_append_comment (rec_rset_t rset,
                         rec_comment_t comment)
{
  rec_rset_elem_t elem;

  elem = rec_rset_elem_comment_new (rset, comment);
  rec_mset_append (rset->mset, elem.mset_elem);
}

rec_rset_elem_t
rec_rset_remove (rec_rset_t rset,
                 rec_rset_elem_t elem)
{
  elem.mset_elem = rec_mset_remove (rset->mset, elem.mset_elem);
  return elem;
}

rec_rset_elem_t
rec_rset_remove_record (rec_rset_t rset,
                        rec_rset_elem_t elem)
{
  elem = rec_rset_remove (rset, elem);
  if (rec_rset_elem_p (elem)
      && !rec_rset_elem_record_p (rset, elem))
    {
      elem = rec_rset_next_record (rset, elem);
    }

  return elem;
}

rec_rset_elem_t
rec_rset_remove_comment (rec_rset_t rset,
                         rec_rset_elem_t elem)
{
  elem = rec_rset_remove (rset, elem);
  if (rec_rset_elem_p (elem)
      && !rec_rset_elem_comment_p (rset, elem))
    {
      elem = rec_rset_next_comment (rset, elem);
    }

  return elem;
}

void
rec_rset_insert_after (rec_rset_t rset,
                       rec_rset_elem_t elem,
                       rec_rset_elem_t new_elem)
{
  if (rset->ordered_p
      && rset->order_by_field
      && rec_rset_elem_record_p (rset, new_elem))
    {
      /* Don't insert the record at the specified location: use a
         sorting criteria instead.  */
      rec_mset_add_sorted (rset->mset, new_elem.mset_elem);
    }
  else
    {
      rec_mset_insert_after (rset->mset,
                             elem.mset_elem,
                             new_elem.mset_elem);
    }
}

rec_rset_elem_t
rec_rset_first (rec_rset_t rset)
{
  rec_rset_elem_t elem;

  elem.mset_elem = rec_mset_first (rset->mset, MSET_ANY);
  return elem;
}

rec_rset_elem_t
rec_rset_first_record (rec_rset_t rset)
{
  rec_rset_elem_t elem;

  elem.mset_elem = rec_mset_first (rset->mset, rset->record_type);
  return elem;
}

rec_rset_elem_t
rec_rset_first_comment (rec_rset_t rset)
{
  rec_rset_elem_t elem;

  elem.mset_elem = rec_mset_first (rset->mset, rset->comment_type);
  return elem;
}

rec_rset_elem_t
rec_rset_next (rec_rset_t rset,
               rec_rset_elem_t elem)
{
  elem.mset_elem = rec_mset_next (rset->mset,
                                  elem.mset_elem,
                                  MSET_ANY);

  return elem;
}

rec_rset_elem_t
rec_rset_next_record (rec_rset_t rset,
                      rec_rset_elem_t elem)
{
  elem.mset_elem = rec_mset_next (rset->mset,
                                  elem.mset_elem,
                                  rset->record_type);

  return elem;
}

rec_rset_elem_t
rec_rset_next_comment (rec_rset_t rset,
                       rec_rset_elem_t elem)
{
  elem.mset_elem = rec_mset_next (rset->mset,
                                  elem.mset_elem,
                                  rset->comment_type);

  return elem;
}

rec_rset_elem_t
rec_rset_elem_record_new (rec_rset_t rset,
                          rec_record_t record)
{
  rec_rset_elem_t elem;
  rec_rset_record_t rset_record = NULL;

  /* Create the rset_record to insert in the mset.  */

  rset_record = malloc (sizeof (struct rec_rset_record_s));
  rset_record->rset = rset;
  rset_record->record = record;

  /* Insert it in the mset.  */

  elem.mset_elem = rec_mset_elem_new (rset->mset, rset->record_type);
  rec_mset_elem_set_data (elem.mset_elem, (void *) rset_record);

  return elem;
}

rec_rset_elem_t
rec_rset_elem_comment_new (rec_rset_t rset,
                           rec_comment_t comment)
{
  rec_rset_elem_t elem;
  rec_rset_comment_t rset_comment = NULL;

  /* Create the rset_comment to insert in the mset.  */

  rset_comment = malloc (sizeof (struct rec_rset_comment_s));
  rset_comment->rset = rset;
  rset_comment->comment = comment;

  /* Insert it in the mset.  */

  elem.mset_elem = rec_mset_elem_new (rset->mset, rset->comment_type);
  rec_mset_elem_set_data (elem.mset_elem, (void *) rset_comment);
  
  return elem;
}

bool
rec_rset_elem_p (rec_rset_elem_t elem)
{
  return (elem.mset_elem != NULL);
}

bool
rec_rset_elem_record_p (rec_rset_t rset,
                        rec_rset_elem_t elem)
{
  return (rec_mset_elem_type (elem.mset_elem) == rset->record_type);
}

bool
rec_rset_elem_comment_p (rec_rset_t rset,
                         rec_rset_elem_t elem)
{
  return (rec_mset_elem_type (elem.mset_elem) == rset->comment_type);
}

rec_record_t
rec_rset_elem_record (rec_rset_elem_t elem)
{
  rec_rset_record_t rset_record =
    (rec_rset_record_t) rec_mset_elem_data (elem.mset_elem);
  return rset_record->record;
}

rec_comment_t
rec_rset_elem_comment (rec_rset_elem_t elem)
{
  rec_rset_comment_t rset_comment =
    (rec_rset_comment_t) rec_mset_elem_data (elem.mset_elem);
  return rset_comment->comment;
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
      rset->descriptor = NULL;
    }
  rset->descriptor = record;

  /* Update the types registry and the auto fields.  */
  rec_rset_update_types (rset);
  rec_rset_update_field_props (rset);
  rec_rset_update_size_constraints (rset);
}

size_t
rec_rset_descriptor_pos (rec_rset_t rset)
{
  return rset->descriptor_pos;
}

void
rec_rset_set_descriptor_pos (rec_rset_t rset,
                             size_t position)
{
  rset->descriptor_pos = position;
}

void
rec_rset_set_type (rec_rset_t rset,
                   char *type)
{
  rec_field_t rec_field;
  rec_field_name_t rec_field_name;

  if (!type)
    {
      /* This is a no-op for the default record set.  */
      return;
    }

  if (!rset->descriptor)
    {
      /* Create a record descriptor.  */
      rset->descriptor = rec_record_new ();
      
    }

  rec_field_name = rec_parse_field_name_str ("%rec:");
  rec_field = rec_record_get_field_by_name (rset->descriptor,
                                            rec_field_name,
                                            0);

  if (rec_field)
    {
      rec_field_set_value (rec_field, type);
    }
  else
    {
      rec_field = rec_field_new (rec_field_name,
                                 type);
      rec_record_append_field (rset->descriptor, rec_field);
    }
}

char *
rec_rset_type (rec_rset_t rset)
{
  char *res;
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
          res = rec_extract_type (rec_field_value (field));
        }
    }

  return res;
}

char *
rec_rset_url (rec_rset_t rset)
{
  char *res;
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
          res = rec_extract_url (rec_field_value (field));
        }
    }

  return res;
}

rec_type_reg_t
rec_rset_get_type_reg (rec_rset_t rset)
{
  return rset->type_reg;
}

void
rec_rset_rename_field (rec_rset_t rset,
                       rec_field_name_t field_name,
                       rec_field_name_t new_field_name)
{
  size_t i, j;
  rec_field_t field;
  rec_record_t descriptor;
  rec_fex_t fex;
  char *fex_str;
  char *type_str;
  rec_buf_t buf;
  char *result;
  size_t result_size;
  rec_fex_elem_t fex_elem;
  rec_field_name_t fex_fname;
  rec_field_name_t type_field_name;
  rec_field_name_t key_field_name;
  rec_field_name_t mandatory_field_name;
  rec_field_name_t unique_field_name;
  rec_field_name_t prohibit_field_name;

  type_field_name = rec_parse_field_name_str ("%type:");
  key_field_name = rec_parse_field_name_str ("%key:");
  mandatory_field_name = rec_parse_field_name_str ("%mandatory:");
  unique_field_name = rec_parse_field_name_str ("%unique:");
  prohibit_field_name = rec_parse_field_name_str ("%prohibit:");
  
  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      for (i = 0; i < rec_record_num_fields (descriptor); i++)
        {
          field = rec_record_elem_field (rec_record_get_field (descriptor, i));
          
          if (rec_field_name_eql_p (rec_field_name (field), type_field_name))
            {
              /* Process a %type entry.  Invalid entries are
                 skipped.  */
              if (!rec_rset_type_field_p (rec_field_value (field)))
                {
                  continue;
                }

              fex = rec_rset_type_field_fex (rec_field_value (field));
              if (fex)
                {
                  for (j = 0; j < rec_fex_size (fex); j++)
                    {
                      fex_elem = rec_fex_get (fex, j);
                      fex_fname = rec_fex_elem_field_name (fex_elem);
                      if (rec_field_name_eql_p (field_name, fex_fname))
                        {
                          /* Replace it with new_field_name.  */
                          rec_fex_elem_set_field_name (fex_elem, new_field_name);
                        }
                    }

                  fex_str = rec_fex_str (fex, REC_FEX_CSV);
                  type_str = rec_rset_type_field_type (rec_field_value (field));
                  
                  buf = rec_buf_new (&result, &result_size);
                  rec_buf_puts (fex_str, buf);
                  rec_buf_putc (' ', buf);
                  rec_buf_puts (type_str, buf);
                  rec_buf_close (buf);

                  rec_field_set_value (field, result);

                  free (fex_str);
                  free (type_str);
                  rec_fex_destroy (fex);
                }
            }
          else if (rec_field_name_eql_p (rec_field_name (field), key_field_name)
                   || rec_field_name_eql_p (rec_field_name (field), mandatory_field_name)
                   || rec_field_name_eql_p (rec_field_name (field), unique_field_name)
                   || rec_field_name_eql_p (rec_field_name (field), prohibit_field_name))
            {
              /* Rename the field in the fex expression that is the
                 value of the field.  Skip invalid entries.  */
              fex = rec_fex_new (rec_field_value (field), REC_FEX_SIMPLE);
              if (fex)
                {
                  for (j = 0; j < rec_fex_size (fex); j++)
                    {
                      fex_elem = rec_fex_get (fex, j);

                      fex_fname = rec_fex_elem_field_name (fex_elem);
                      if (rec_field_name_eql_p (field_name, fex_fname))
                        {
                          /* Replace it with new_field_name.  */
                          rec_fex_elem_set_field_name (fex_elem, new_field_name);
                        }
                    }
                  
                  fex_str = rec_fex_str (fex, REC_FEX_SIMPLE);
                  rec_field_set_value (field, fex_str);
                  free (fex_str);
                }
            }
        }
    }

  /* Update the types registry.  */
  rec_rset_update_field_props (rset);

  /* Cleanup.  */
  rec_field_name_destroy (type_field_name);
  rec_field_name_destroy (key_field_name);
  rec_field_name_destroy (mandatory_field_name);
  rec_field_name_destroy (unique_field_name);
  rec_field_name_destroy (prohibit_field_name);
}

rec_fex_t
rec_rset_auto (rec_rset_t rset)
{
  rec_fex_t fex;
  rec_rset_fprops_t props;

  fex = rec_fex_new (NULL, REC_FEX_SIMPLE);

  props = rset->field_props;
  while (props)
    {
      if (props->auto_p)
        {
          rec_fex_append (fex,
                          props->fname,
                          -1, -1);
        }
      props = props->next;
    }

  return fex;
}

rec_type_t
rec_rset_get_field_type (rec_rset_t rset,
                         rec_field_name_t field_name)
{
  rec_type_t type = NULL;
  rec_rset_fprops_t props = NULL;

  props = rec_rset_get_props (rset, field_name, false);
  if (props)
    {
      type = props->type;
      if (!type)
        {
          type = rec_type_reg_get (rset->type_reg, props->type_name);
        }
    }
  
  return type;
}

size_t
rec_rset_min_records (rec_rset_t rset)
{
  return rset->min_size;
}

size_t
rec_rset_max_records (rec_rset_t rset)
{
  return rset->max_size;
}

char *
rec_rset_source (rec_rset_t rset)
{
  rec_record_t record;
  
  record = rec_rset_descriptor (rset);
  if (!record)
    {
      record = rec_rset_elem_record (rec_rset_get_record (rset, 0));
    }

  return rec_record_source (record);
}

void
rec_rset_set_ordered (rec_rset_t rset,
                      bool ordered_p)
{
  rset->ordered_p = ordered_p;
}

bool
rec_rset_ordered (rec_rset_t rset)
{
  return rset->ordered_p;
}


/*
 * Private functions
 */

static void
rec_rset_record_disp_fn (void *data)
{
  rec_rset_record_t rset_record = (rec_rset_record_t) data;

  rec_record_destroy (rset_record->record);
  free (rset_record);
}

static bool
rec_rset_record_equal_fn (void *data1,
                          void *data2)
{
  return (data1 == data2);
  /*  return rec_record_equal_p ((rec_record_t) data1,
      (rec_record_t) data2); */
}

static void *
rec_rset_record_dup_fn (void *data)
{
  rec_rset_record_t rset_record = (rec_rset_record_t) data;
  rec_rset_record_t new = NULL;

  new = malloc (sizeof (struct rec_rset_record_s));
  new->rset = rset_record->rset;
  new->record = rec_record_dup (rset_record->record);

  return (void *) new;
}

static int
rec_rset_record_compare_fn (void *data1,
                            void *data2,
                            int type2)
{
  rec_rset_t rset                  = NULL;
  rec_rset_record_t rset_record_1  = NULL; 
  rec_rset_record_t rset_record_2  = NULL;
  rec_field_name_t order_by_field  = NULL;
  rec_record_t record1             = NULL;
  rec_record_t record2             = NULL;
  rec_field_t field1               = NULL;
  rec_field_t field2               = NULL;
  rec_type_t field_type            = NULL;
  int type_comparison              = 0;
  int int1, int2                   = 0;

  /* data1 and data2 are both records.

     order_by_field can't be NULL, because this callback is invoked
     only if rec_mset_add_sorted is used to add an element to the
     list.

     The following rules apply here:
     
     1. If group_by is not in both record1 and record2,
        data1 < data2.
 
     2. Else,

     2.1 If type(order_by_field) == int, range, or real:
         compare by numerical order.

     2.2. If type(order_by_field) == bool:
          true < false.

     2.2.1. If type(order_by_field) == date:
            compare by date chronology order.

     2.2.2. Otherwise: compare by lexicographical order.

    Note that record1 will always be a regular record.  Never a
    descriptor.
  */

  /* Get the records and the containing rset.  */
  rset_record_1 = (rec_rset_record_t) data1;
  rset_record_2 = (rec_rset_record_t) data2;
  record1 = rset_record_1->record;
  record2 = rset_record_2->record;
  rset = rset_record_1->rset;

  /* Get the order by field and check if it is present in both
     registers.  */
  order_by_field = rset_record_1->rset->order_by_field;
  field1 = rec_record_get_field_by_name (record1, order_by_field, 0);
  field2 = rec_record_get_field_by_name (record2, order_by_field, 0);
  
  if (field1 && !field2)
    {
      return 1; /* field1 > field2 */
    }
  else if (!field1 && field2)
    {
      return -1;  /* field1 < field2 */

    }
  else if (!field1 && !field2)
    {
      return 0; /* field1 == field2 */
    }

  /* Discriminate by field type.  */
  field_type = rec_rset_get_field_type (rset,
                                        order_by_field);
  if (field_type)
    {
      switch (rec_type_kind (field_type))
        {
        case REC_TYPE_INT:
        case REC_TYPE_RANGE:
          {
            if (!rec_atoi (rec_field_value(field1), &int1)
                || !rec_atoi (rec_field_value(field2), &int2))
              {
                goto lexi;
              }

            if (int1 < int2)
              {
                type_comparison = -1;
              }
            else if (int1 > int2)
              {
                type_comparison = 1;
              }
            else
              {
                type_comparison = 0;
              }

            break;
          }
        case REC_TYPE_REAL:
        case REC_TYPE_BOOL:
        case REC_TYPE_DATE:
        default:
          {
          lexi:
            /* Lexicographic order.  */
            type_comparison =
              strcmp (rec_field_value (field1),
                      rec_field_value (field2)); /* 3.2.2.  */
            break;
          }
        }
    }
  else
    {
      /* Non typed fields contain free text, so apply a lexicographic
         order as well.  */
      type_comparison =
        strcmp (rec_field_value (field1),
                rec_field_value (field2)); /* 3.2.2.  */
    }

  return type_comparison;
}

static void
rec_rset_comment_disp_fn (void *data)
{
  rec_rset_comment_t rset_comment = (rec_rset_comment_t) data;

  rec_comment_destroy (rset_comment->comment);
  free (rset_comment);
}

static bool
rec_rset_comment_equal_fn (void *data1,
                           void *data2)
{
  return (data1 == data2);
  /*  return rec_comment_equal_p ((rec_comment_t) data1,
      (rec_comment_t) data2);*/
}

static void *
rec_rset_comment_dup_fn (void *data)
{
  rec_rset_comment_t rset_comment = (rec_rset_comment_t) data;
  rec_rset_comment_t new = NULL;

  new = malloc (sizeof (struct rec_rset_comment_s));
  new->rset = rset_comment->rset;
  new->comment = rec_comment_dup (rset_comment->comment);

  return (void *) new;
}

static int
rec_rset_comment_compare_fn (void *data1,
                             void *data2,
                             int   type2)
{
  /* data1 is a comment, and data2 is a record.  The comment is always
     < than the record.  */
  return -1;
}

static void
rec_rset_update_size_constraints (rec_rset_t rset)
{
  rec_field_t field;
  rec_field_name_t size_fname;
  enum rec_size_condition_e condition;
  size_t size = 0;

  /* Reset the constraints. */
  rset->min_size = 0;
  rset->max_size = SIZE_MAX;

  /* Scan the record descriptor for %size: directives, and build the
     new list.  */
  if (rset->descriptor)
    {
      size_fname = rec_parse_field_name_str ("%size:");

      field = rec_record_get_field_by_name (rset->descriptor,
                                            size_fname,
                                            0);

      if (field && rec_match (rec_field_value (field), REC_INT_SIZE_RE))
        {
          /* Extract 'min' and 'max' and update the constraints in the
             rset.  */
          condition = rec_extract_size_condition (rec_field_value (field));
          size = rec_extract_size (rec_field_value (field));
          
          /* Set min_size and max_size depending on the
             condition.  */
          switch (condition)
            {
            case SIZE_COND_E:
              {
                rset->min_size = size;
                rset->max_size = size;
                break;
              }
            case SIZE_COND_L:
              {
                rset->max_size = size - 1;
                break;
              }
            case SIZE_COND_LE:
              {
                rset->max_size = size;
                break;
              }
            case SIZE_COND_G:
              {
                rset->min_size = size + 1;
                break;
              }
            case SIZE_COND_GE:
              {
                rset->min_size = size;
                break;
              }
            }
        }
    }
}

static void
rec_rset_update_field_props (rec_rset_t rset)
{
  rec_rset_fprops_t props = NULL;
  rec_record_elem_t record_elem;
  rec_field_t field;
  rec_field_name_t field_name;
  rec_field_name_t auto_fname;
  rec_field_name_t auto_field_name;
  rec_field_name_t type_fname;
  rec_field_name_t sort_fname;
  char *field_value;
  rec_fex_t fex;
  size_t i;
  rec_type_t type;
  char *p, *q = NULL;
  char *type_name = NULL;

  /* Reset the field properties.  */
  props = rset->field_props;
  while (props)
    {
      props->auto_p = false;
      if (props->type)
        {
          rec_type_destroy (props->type);
          props->type = NULL;
        }

      props = props->next;
    }

  /* Scan the record descriptor for % directives, and update the
     fields properties accordingly.  */
  if (rset->descriptor)
    {
      auto_fname = rec_parse_field_name_str ("%auto:");
      type_fname = rec_parse_field_name_str ("%type:");
      sort_fname = rec_parse_field_name_str ("%sort:");

      record_elem = rec_record_first_field (rset->descriptor);
      while (rec_record_elem_p (record_elem))
        {
          field = rec_record_elem_field (record_elem);
          field_name = rec_field_name (field);
          field_value = rec_field_value (field);

          /* Update field types.  Only valid %type: descriptors are
             considered.  Invalid descriptors are ignored.  */
          if (rec_field_name_equal_p (field_name, type_fname)
              && rec_rset_type_field_p (field_value))
            {
              fex = rec_rset_type_field_fex (field_value);
              for (i = 0; i < rec_fex_size (fex); i++)
                {
                  p = rec_rset_type_field_type (field_value);
                  type = rec_type_new (p);
                  if (!type)
                    {
                      /* p is the name of the type.  Set it as a field
                         property.  Note that if the field is already
                         associated with an anonymous type, or a type
                         name, they are replaced.  */

                      q = p;
                      rec_parse_regexp (&q, "^" REC_TYPE_NAME_RE, &type_name);
                      props = rec_rset_get_props (rset,
                                                  rec_fex_elem_field_name (rec_fex_get (fex, i)),
                                                  true);
                      if (props->type)
                        {
                          rec_type_destroy (props->type);
                          props->type = NULL;
                        }
                      free (props->type_name);
                      props->type_name = type_name;
                    }
                  else
                    {
                      /* Set the type as a field property.  Note that
                         if the field is already associated with an
                         anonymous type, or a type name, they are
                         replaced.  */

                      props = rec_rset_get_props (rset,
                                                  rec_fex_elem_field_name (rec_fex_get (fex, i)),
                                                  true);
                      if (props->type)
                        {
                          rec_type_destroy (props->type);
                        }
                      free (props->type_name);
                      props->type_name = NULL;
                      props->type = type;
                    }
                  
                  free (p);
                }
            }

          /* Update auto fields.  */
          if (rec_field_name_equal_p (field_name, auto_fname))
            {
              /* %auto: fields containing incorrect data are
                  ignored.  */
              fex = rec_fex_new (rec_field_value (field), REC_FEX_SIMPLE);
              if (fex)
                {
                  for (i = 0; i < rec_fex_size (fex); i++)
                    {
                      auto_field_name = rec_fex_elem_field_name (rec_fex_get (fex, i));
                      props = rec_rset_get_props (rset, auto_field_name, true);
                      props->auto_p = true;
                    }
                }
            }

          /* Update sort fields.  Since only one field can be set as
             the sorting criteria, the last field takes
             precedence.  */
          if (rec_field_name_equal_p (field_name, sort_fname))
            {
              /* Parse the field name in the field value.  Invalid
                 entries are just ignored.  */
              p = rec_field_value (field);
              rec_skip_blanks (&p);
              q = p;
              if (rec_parse_regexp (&q, "^" REC_TYPE_NAME_RE "[ \n\t]*", NULL))
                {
                  rec_parse_regexp (&p, "^" REC_TYPE_NAME_RE, &q);
                  if (rset->order_by_field)
                    {
                      rec_field_name_destroy (rset->order_by_field);
                    }
                  rset->order_by_field = rec_parse_field_name_str (q);
                }
            }

          record_elem = rec_record_next_field (rset->descriptor, record_elem);
        }

      rec_field_name_destroy (auto_fname);
      rec_field_name_destroy (type_fname);
      rec_field_name_destroy (sort_fname);
    } 
}

static void
rec_rset_update_types (rec_rset_t rset)
{
  rec_field_t field;
  rec_record_elem_t record_elem;
  rec_field_name_t field_name;
  char *field_value;
  rec_field_name_t typedef_fname;
  char *p, *q = NULL;
  rec_type_t type;
  char *type_name, *to_type = NULL;
  

  /* Scan the record descriptor for %typedef directives and update the
     types registry accordingly.  */
  if (rset->descriptor)
    {
      /* Create some field names.  */
      typedef_fname = rec_parse_field_name_str ("%typedef:");

      /* Purge the registry.  */
      rec_type_reg_destroy (rset->type_reg);
      rset->type_reg = rec_type_reg_new ();

      /* Iterate on the fields of the descriptor.  */
      record_elem = rec_record_first_field (rset->descriptor);
      while (rec_record_elem_p (record_elem))
        {
          field = rec_record_elem_field (record_elem);
          field_name = rec_field_name (field);
          field_value = rec_field_value (field);

          if (rec_field_name_equal_p (field_name, typedef_fname))
            {
              p = field_value;
              rec_skip_blanks (&p);

              /* Get the name of the type.  */
              if (rec_parse_regexp (&p, "^" REC_TYPE_NAME_RE, &type_name))
                {
                  /* Get the type.  */
                  type = rec_type_new (p);
                  if (type)
                    {
                      /* Set the name of the type.  */
                      rec_type_set_name (type, type_name);

                      /* Create and insert the type in the type
                         registry.  */
                      rec_type_reg_add (rset->type_reg, type);
                    }
                  else
                    {
                      /* This could be a synonym.  Try to parse a type
                         name and, if the operation succeeds, insert
                         the synonym in the registry.  */
                      rec_skip_blanks (&p);
                      q = p;
                      if (rec_parse_regexp (&q,
                                            "^" REC_TYPE_NAME_RE "[ \t\n]*",
                                            NULL))
                        {
                          rec_parse_regexp (&p, "^" REC_TYPE_NAME_RE, &to_type);
                          rec_type_reg_add_synonym (rset->type_reg,
                                                    type_name,
                                                    to_type);
                        }
                    }
                  
                  free (type_name);
                }
            }

          record_elem = rec_record_next_field (rset->descriptor,
                                               record_elem);
        }

      rec_field_name_destroy (typedef_fname);
    }
}

static bool
rec_rset_type_field_p (const char *str)
{
  char *p;

  p = str;

  /* Check the fex */

  rec_skip_blanks (&p);
  if (!rec_parse_regexp (&p,
                         "^" REC_FNAME_RE "(," REC_FNAME_RE ")*",
                         NULL))
    {
      return false;
    }
  rec_skip_blanks (&p);

  /* Check the type description, or the name of a type.  */

  return (rec_type_descr_p (p)
          || rec_parse_regexp (&p, "^" REC_TYPE_NAME_RE "[ \t\n]*$", NULL));
}

static rec_fex_t
rec_rset_type_field_fex (const char *str)
{
  rec_fex_t fex = NULL;
  char *p;
  char *name;

  p = str;

  if (rec_parse_regexp (&p,
                        "^" REC_FNAME_RE "(," REC_FNAME_RE ")*",
                        &name))
    {
      fex = rec_fex_new (name, REC_FEX_CSV);
      free (name);
    }

  return fex;
}

static char*
rec_rset_type_field_type (const char *str)
{
  char *result = NULL;
  char *p;

  if (rec_rset_type_field_p (str))
    {
      p = str;

      rec_skip_blanks (&p);
      rec_parse_regexp (&p, "^" REC_FNAME_RE "(," REC_FNAME_RE ")*", NULL);
      rec_skip_blanks (&p);

      /* Return the rest of the string.  */
      result = strdup (p);
    }

  return result;
}

static rec_rset_fprops_t
rec_rset_get_props (rec_rset_t rset,
                    rec_field_name_t fname,
                    bool create_p)
{
  rec_rset_fprops_t props = NULL;
  
  props = rset->field_props;
  while (props)
    {
      if (rec_field_name_equal_p (fname, props->fname))
        {
          break;
        }

      props = props->next;
    }

  if (!props && create_p)
    {
      /* Create a new properties structure for this field name and
         initialize it.  */
      props = malloc (sizeof (struct rec_rset_fprops_s));
      if (props)
        {
          props->fname = rec_field_name_dup (fname);
          props->auto_p = false;
          props->type = NULL;
          props->type_name = NULL;
          
          /* Prepend it to the field properties list.  */
          props->next = rset->field_props;
          rset->field_props = props;
        }
    }

  return props;
}


/* End of rec-rset.c */
