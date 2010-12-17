/* -*- mode: C -*-
 *
 *       File:         rec-rset.c
 *       Date:         Thu Mar  5 18:12:10 2009
 *
 *       GNU recutils - Record Sets
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

#include <rec-mset.h>
#include <rec.h>
#include <rec-utils.h>

/* Record Set Data Structure.
 *
 * A record set is a set of zero or more non-special records
 * intermixed with comments, maybe preceded by a record descriptor.
 */

#define REC_RSET_MAX_ORDER 256

struct rec_rset_s
{
  rec_record_t descriptor;
  size_t descriptor_pos; /* Position of the descriptor into the record
                            set.  Some comments can appear before the
                            record descriptor in the rec file, so we
                            need to track it in order to write back
                            the record properly.  */

  /* Type registry.  */
  rec_type_reg_t type_reg;

  /* Storage for records and comments.  */
  int record_type;
  int comment_type;
  rec_mset_t mset;
};

/* Set of names for special fields */

#define REC_NAME_REC "%rec"
#define REC_NAME_KEY "%key"
#define REC_NAME_MANDATORY "%mandatory"
#define REC_NAME_UNIQUE "%unique:"

/* Static functions implemented below.  */

static void rec_rset_update_types (rec_rset_t rset);

static bool rec_rset_record_equal_fn (void *data1, void *data2);
static void rec_rset_record_disp_fn (void *data);
static void *rec_rset_record_dup_fn (void *data);

static bool rec_rset_comment_equal_fn (void *data1, void *data2);
static void rec_rset_comment_disp_fn (void *data);
static void *rec_rset_comment_dup_fn (void *data);

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
          rset->type_reg = NULL;

          /* register the types.  */
          rset->record_type = rec_mset_register_type (rset->mset,
                                                      "record",
                                                      rec_rset_record_disp_fn,
                                                      rec_rset_record_equal_fn,
                                                      rec_rset_record_dup_fn);
          rset->comment_type = rec_mset_register_type (rset->mset,
                                                       "comment",
                                                       rec_rset_comment_disp_fn,
                                                       rec_rset_comment_equal_fn,
                                                       rec_rset_comment_dup_fn);
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
  if (rset->descriptor)
    {
      rec_record_destroy (rset->descriptor);
    }
  if (rset->type_reg)
    {
      rec_type_reg_destroy (rset->type_reg);
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
  rec_mset_insert_at (rset->mset,
                      elem.mset_elem,
                      position);
}

void
rec_rset_append (rec_rset_t rset,
                 rec_rset_elem_t elem)
{
  rec_mset_append (rset->mset, elem.mset_elem);
}

void
rec_rset_append_record (rec_rset_t rset,
                        rec_record_t record)
{
  rec_rset_elem_t elem;
  
  elem = rec_rset_elem_record_new (rset, record);
  rec_mset_append (rset->mset, elem.mset_elem);
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
  rec_mset_insert_after (rset->mset,
                         elem.mset_elem,
                         new_elem.mset_elem);
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

  elem.mset_elem = rec_mset_elem_new (rset->mset, rset->record_type);
  rec_mset_elem_set_data (elem.mset_elem, (void *) record);

  return elem;
}

rec_rset_elem_t
rec_rset_elem_comment_new (rec_rset_t rset,
                           rec_comment_t comment)
{
  rec_rset_elem_t elem;

  elem.mset_elem = rec_mset_elem_new (rset->mset, rset->comment_type);
  rec_mset_elem_set_data (elem.mset_elem, (void *) comment);
  
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
  return (rec_record_t) rec_mset_elem_data (elem.mset_elem);
}

rec_comment_t
rec_rset_elem_comment (rec_rset_elem_t elem)
{
  return (rec_comment_t) rec_mset_elem_data (elem.mset_elem);
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

  /* Update the types registry.  */
  rec_rset_update_types (rset);
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
              if (!rec_type_descr_p (rec_field_value (field)))
                {
                  continue;
                }

              fex = rec_type_descr_fex (rec_field_value (field));
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
                  type_str = rec_type_descr_type (rec_field_value (field));
                  
                  buf = rec_buf_new (&result, &result_size);
                  rec_buf_puts (fex_str, buf);
                  rec_buf_putc (' ', buf);
                  rec_buf_puts (type_str, buf);
                  rec_buf_putc ('\0', buf);
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
  rec_rset_update_types (rset);

  /* Cleanup.  */
  rec_field_name_destroy (type_field_name);
  rec_field_name_destroy (key_field_name);
  rec_field_name_destroy (mandatory_field_name);
  rec_field_name_destroy (unique_field_name);
  rec_field_name_destroy (prohibit_field_name);
}

/*
 * Private functions
 */

static void
rec_rset_record_disp_fn (void *data)
{
  rec_record_destroy ((rec_record_t) data);
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
  return (void *) rec_record_dup ((rec_record_t) data);
}

static void
rec_rset_comment_disp_fn (void *data)
{
  rec_comment_destroy ((rec_comment_t) data);
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
  return (void *) rec_comment_dup ((rec_comment_t) data);
}

static void
rec_rset_update_types (rec_rset_t rset)
{
  rec_field_t descr_field;
  rec_field_name_t type_field_name;
  char *descr_field_value;
  size_t i, num_fields, j;
  rec_type_t type;
  rec_fex_t fex;

  if (rset->descriptor)
    {
      /* Update the types registry.  */
      if (rset->type_reg)
        {
          rec_type_reg_destroy (rset->type_reg);
        }
      rset->type_reg = rec_type_reg_new ();
      
      type_field_name = rec_parse_field_name_str ("%type:");
      num_fields = rec_record_get_num_fields_by_name (rset->descriptor, type_field_name);
      for (i = 0; i < num_fields; i++)
        {
          descr_field = rec_record_get_field_by_name (rset->descriptor, type_field_name, i);
          descr_field_value = rec_field_value (descr_field);
          
          /* Only valid type descriptors are considered.  Invalid
             descriptors are ignored.  */
          if (rec_type_descr_p (descr_field_value))
            {
              fex = rec_type_descr_fex (descr_field_value);
              for (j = 0; j < rec_fex_size (fex); j++)
                {
                  type = rec_type_new (descr_field_value);
                  if (type)
                    {
                      rec_type_reg_register (rset->type_reg,
                                             rec_fex_elem_field_name (rec_fex_get (fex, j)),
                                             type);
                    }
                }
            }
        }

      rec_field_name_destroy (type_field_name);
    }
}

/* End of rec-rset.c */
