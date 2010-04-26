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

#include <malloc.h>
#include <stdlib.h>

#include <rec-mset.h>
#include <rec.h>

/* Record Set Data Structure.
 *
 * A record set is a set of zero or more non-special records
 * intermixed with comments, maybe preceded by a record descriptor.
 */

#define REC_RSET_MAX_ORDER 256

struct rec_rset_s
{
  rec_record_t descriptor;

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

static char *special_fields[] =
  {
    REC_NAME_REC,
    REC_NAME_KEY,
    REC_NAME_MANDATORY,
    REC_NAME_UNIQUE,
    /* Sentinel */
    ""
  };

/* Static functions implemented below.  */

static bool rec_rset_record_equal_fn (void *data1, void *data2);
static void rec_rset_record_disp_fn (void *data);
static void *rec_rset_record_dup_fn (void *data);

static bool rec_rset_comment_equal_fn (void *data1, void *data2);
static void rec_rset_comment_disp_fn (void *data);
static void *rec_rset_comment_dup_fn (void *data);

static int rec_rset_check_record_key (rec_rset_t rset,
                                      rec_record_t orig_record, rec_record_t record,
                                      char *program_name, FILE *errors);
static int rec_rset_check_record_types (rec_rset_t rset, rec_record_t record,
                                        char *program_name, FILE *errors);
static int rec_rset_check_record_mandatory (rec_rset_t rset, rec_record_t record,
                                            char *program_name, FILE *errors);
static int rec_rset_check_record_unique (rec_rset_t rset, rec_record_t record,
                                         char *program_name, FILE *errors);
static int rec_rset_check_record_prohibit (rec_rset_t rset, rec_record_t record,
                                           char *program_name, FILE *errors);

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

int
rec_rset_check_record (rec_rset_t rset,
                       rec_record_t orig_record,
                       rec_record_t record,
                       char *program_name,
                       FILE *errors)
{
  int res;

  res =
    rec_rset_check_record_key (rset, orig_record, record, program_name, errors)
    + rec_rset_check_record_types     (rset, record, program_name, errors)
    + rec_rset_check_record_mandatory (rset, record, program_name, errors)
    + rec_rset_check_record_unique    (rset, record, program_name, errors)
    + rec_rset_check_record_prohibit  (rset, record, program_name, errors);

  return res;
}

bool
rec_rset_check_field_type (rec_rset_t rset,
                           rec_field_t field,
                           char **type_str)
{
  bool res;
  rec_record_t descriptor;
  rec_field_t descr_field;
  char *descr_field_value;
  char *descr_field_name_str;
  rec_field_name_t type_field_name;
  rec_field_name_t field_name;
  rec_type_t type;
  size_t i, num_fields;

  res = true;

  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      type_field_name = rec_parse_field_name_str ("%type:");
      num_fields = rec_record_get_num_fields_by_name (descriptor, type_field_name);
      for (i = 0; i < num_fields; i++)
        {
          descr_field = rec_record_get_field_by_name (descriptor, type_field_name, i);
          descr_field_name_str = rec_field_name_str (descr_field);
          descr_field_value = rec_field_value (descr_field);

          /* Only valid type descriptors are considered.  Invalid
             descriptors are ignored.  */
          if (rec_type_descr_p (descr_field_value))
            {
              type = rec_type_new (descr_field_value);
              
              if (type)
                {
                  field_name = rec_type_descr_field_name (descr_field_value);
                  if (rec_field_name_equal_p (field_name,
                                              rec_field_name (field)))
                    {
                      if (!rec_type_check (type, rec_field_value (field)))
                        {
                          *type_str = rec_type_kind_str (type);
                          res = false;
                          break;
                        }
                    }
                  
                  rec_type_destroy (type);
                }
              
            }
        }

      rec_field_name_destroy (type_field_name);
    }

  return res;
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

static int
rec_rset_check_record_types (rec_rset_t rset,
                             rec_record_t record,
                             char *program_name,
                             FILE *errors)
{
  int res;
  rec_record_elem_t rec_elem;
  rec_field_t field;
  char *type_str;

  res = 0;

  rec_elem = rec_record_null_elem ();
  while (rec_record_elem_p (rec_elem = rec_record_next_field (record, rec_elem)))
    {
      field = rec_record_elem_field (rec_elem);

      /* Check for the type.  */
      if (!rec_rset_check_field_type (rset, field, &type_str))
        {
          fprintf (errors,
                   "%s: error: invalid value for field %s[%d] of type '%s'\n",
                   program_name,
                   rec_field_name_str (field),
                   rec_record_get_field_index_by_name (record, field),
                   type_str);

          res++;
        }
    }

  return res;
}

static int
rec_rset_check_record_mandatory (rec_rset_t rset,
                                 rec_record_t record,
                                 char *program_name,
                                 FILE *errors)
{
  int res;
  rec_record_t descriptor;
  rec_field_name_t field_name;
  rec_field_name_t mandatory_field_name;
  rec_field_t field;
  size_t i, num_fields;
  
  res = 0;

  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      field_name = rec_parse_field_name_str ("%mandatory:");
      num_fields = rec_record_get_num_fields_by_name (descriptor, field_name);
      for (i = 0; i < num_fields; i++)
        {
          field = rec_record_get_field_by_name (descriptor, field_name, i);

          /* Parse the field name from the value of %mandatory:  */
          mandatory_field_name = rec_parse_field_name_str (rec_field_value (field));
          if (mandatory_field_name)
            {
              if (rec_record_get_num_fields_by_name (record, mandatory_field_name)
                  == 0)
                {
                  fprintf (errors,
                           "%s: error: mandatory field '%s' not found in record\n",
                           program_name, rec_field_value (field));
                  res++;
                }

              rec_field_name_destroy (mandatory_field_name);
            }
        }                                          
      
      rec_field_name_destroy (field_name);
    }

  return res;
}

static int
rec_rset_check_record_unique (rec_rset_t rset,
                              rec_record_t record,
                              char *program_name,
                              FILE *errors)
{
  int res;
  rec_record_t descriptor;
  rec_field_name_t field_name;
  rec_field_name_t unique_field_name;
  rec_field_t field;
  size_t i;
  
  res = 0;

  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      field_name = rec_parse_field_name_str ("%unique:");
      for (i = 0; i < rec_record_get_num_fields_by_name (descriptor,
                                                         field_name);
           i++)
        {
          field = rec_record_get_field_by_name (descriptor, field_name, i);

          /* Parse the field name from the value of %unique:  */
          unique_field_name = rec_parse_field_name_str (rec_field_value (field));
          if (unique_field_name)
            {
              if (rec_record_get_num_fields_by_name (record, unique_field_name)
                  > 1)
                {
                  fprintf (errors,
                           "%s: error: field '%s' shall be unique in this record\n",
                           program_name, rec_field_value (field));
                  res++;
                }

              rec_field_name_destroy (unique_field_name);
            }
        }                                          
      
      rec_field_name_destroy (field_name);
    }

  return res;
}

static int
rec_rset_check_record_prohibit (rec_rset_t rset,
                                rec_record_t record,
                                char *program_name,
                                FILE *errors)
{
  int res;
  rec_record_t descriptor;
  rec_field_name_t field_name;
  rec_field_name_t prohibit_field_name;
  rec_field_t field;
  size_t i;
  
  res = 0;

  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      field_name = rec_parse_field_name_str ("%prohibit:");
      for (i = 0; i < rec_record_get_num_fields_by_name (descriptor,
                                                         field_name);
           i++)
        {
          field = rec_record_get_field_by_name (descriptor, field_name, i);

          /* Parse the field name from the value of %prohibit:  */
          prohibit_field_name = rec_parse_field_name_str (rec_field_value (field));
          if (prohibit_field_name)
            {
              if (rec_record_get_num_fields_by_name (record, prohibit_field_name)
                  > 0)
                {
                  fprintf (errors,
                           "%s: error: prohibited field '%s' found in record\n",
                           program_name, rec_field_value (field));
                  res++;
                }

              rec_field_name_destroy (prohibit_field_name);
            }
        }                                          
      
      rec_field_name_destroy (field_name);
    }

  return res;
}

static int
rec_rset_check_record_key (rec_rset_t rset,
                           rec_record_t orig_record,
                           rec_record_t record,
                           char *program_name,
                           FILE *errors)
{
  int res;
  rec_record_t descriptor;
  rec_record_t other_record;
  rec_rset_elem_t rset_elem;
  rec_field_name_t field_name;
  rec_field_name_t key_field_name;
  rec_field_t field;
  rec_field_t key;
  rec_field_t other_key;
  bool duplicated_key;
  size_t i;
  
  res = 0;

  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      field_name = rec_parse_field_name_str ("%key:");
      for (i = 0; i < rec_record_get_num_fields_by_name (descriptor,
                                                         field_name);
           i++)
        {
          field = rec_record_get_field_by_name (descriptor, field_name, i);

          /* Parse the field name from the value of %key:  */
          key_field_name = rec_parse_field_name_str (rec_field_value (field));
          if (key_field_name)
            {
              if (rec_record_get_num_fields_by_name (record, key_field_name)
                  == 1)
                {
                  /* Check that the value specified as the key is
                     unique in the whole record set.  */
                  key = rec_record_get_field_by_name (record,
                                                      key_field_name,
                                                      0);
                  duplicated_key = false;
                  
                  rset_elem = rec_rset_null_elem ();
                  while (rec_rset_elem_p (rset_elem = rec_rset_next_record (rset, rset_elem)))
                    {
                      other_record = rec_rset_elem_record (rset_elem);

                      if (other_record != orig_record)
                        {
                          /* XXX: Only the first key field is considered.  */
                          other_key = rec_record_get_field_by_name (other_record,
                                                                    key_field_name,
                                                                    0);
                          if (other_key)
                            {
                              if (strcmp (rec_field_value (other_key),
                                          rec_field_value (key)) == 0)
                                {
                                  /* Found a key field with the same
                                     value in other record.  */
                                  duplicated_key = true;
                                  break;
                                }
                            }
                        }
                    }

                  if (duplicated_key)
                    {
                      fprintf (errors,
                               "%s: error: duplicated key value in field '%s' in record\n",
                               program_name,
                               rec_field_name_str (key));
                      res++;
                      break;
                    }
                }
              else
                {
                  fprintf (errors,
                           "%s: error: key field '%s' not found in record\n",
                           program_name, rec_field_value (field));
                  res++;
                }

              rec_field_name_destroy (key_field_name);
            }
        }                                          
      
      rec_field_name_destroy (field_name);
    }

  return res;
}

/* End of rec-rset.c */
