/* -*- mode: C -*- Time-stamp: "2010-12-17 16:39:32 jco"
 *
 *       File:         rec-int.c
 *       Date:         Thu Jul 15 18:23:26 2010
 *
 *       GNU recutils - Data integrity.
 *
 */

/* Copyright (C) 2010 Jose E. Marchesi */

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <libintl.h>
#define _(str) dgettext (PACKAGE, str)
#include <tempname.h>

#if defined REMOTE_DESCRIPTORS
#   include <curl/curl.h>
#endif

#include <rec.h>
#include <rec-utils.h>

/*
 * Forward references.
 */

static int rec_int_check_descriptor (rec_rset_t rset, rec_buf_t errors);
static int rec_int_check_record_key (rec_rset_t rset,
                                     rec_record_t orig_record, rec_record_t record,
                                     rec_buf_t errors);
static int rec_int_check_record_types (rec_db_t db,
                                       rec_rset_t rset,
                                       rec_record_t record,
                                       rec_buf_t errors);
static int rec_int_check_record_mandatory (rec_rset_t rset, rec_record_t record,
                                           rec_buf_t errors);
static int rec_int_check_record_unique (rec_rset_t rset, rec_record_t record,
                                        rec_buf_t errors);
static int rec_int_check_record_prohibit (rec_rset_t rset, rec_record_t record,
                                          rec_buf_t errors);
static void rec_int_merge_remote (rec_rset_t rset);
static bool rec_int_rec_type_p (char *str);

/*
 * Public functions.
 */

int
rec_int_check_db (rec_db_t db,
                  bool check_descriptors_p,
                  bool remote_descriptors_p,
                  char **errors)
{
  int ret;
  size_t db_size;
  size_t n_rset;
  rec_rset_t rset;
  
  ret = 0;

  db_size = rec_db_size (db);
  for (n_rset = 0; n_rset < db_size; n_rset++)
    {
      rset = rec_db_get_rset (db, n_rset);
      ret = ret + rec_int_check_rset (db,
                                      rset,
                                      check_descriptors_p,
                                      remote_descriptors_p,
                                      errors);
    }

  return ret;
}

int
rec_int_check_rset (rec_db_t db,
                    rec_rset_t rset,
                    bool check_descriptor_p,
                    bool remote_descriptor_p,
                    char **errors)
{
  int res;
  rec_rset_elem_t rset_elem;
  rec_record_t record;
  rec_buf_t buf_errors;
  size_t buf_errors_size;

  res = 0;

  if (remote_descriptor_p)
    {
      /* Fetch the remote descriptor, if any, and merge it with the
         local descriptor.  */
      rec_int_merge_remote (rset);
    }

  if (check_descriptor_p)
    {
      if (errors)
        {
          buf_errors = rec_buf_new (errors, &buf_errors_size);
        }
      res += rec_int_check_descriptor (rset, buf_errors);
      if (errors)
        {
          rec_buf_close (buf_errors);
        }
    }

  if (res > 0)
    {
      /* Stop here, since a lot of errors in the records will be
         generated due to errors in the record descriptor.  */
      return res;
    }

  rset_elem = rec_rset_null_elem ();
  while (rec_rset_elem_p (rset_elem = rec_rset_next_record (rset, rset_elem)))
    {
      record = rec_rset_elem_record (rset_elem);

      res += rec_int_check_record (db,
                                   rset,
                                   record, record,
                                   errors);
    }

  return res;
}

int
rec_int_check_record (rec_db_t db,
                      rec_rset_t rset,
                      rec_record_t orig_record,
                      rec_record_t record,
                      char **errors)
{
  int res;
  rec_buf_t errors_buf;
  size_t errors_size;

  errors_buf = rec_buf_new (errors, &errors_size);
  res =
    rec_int_check_record_key (rset, orig_record, record, errors_buf)
    + rec_int_check_record_types     (db, rset, record, errors_buf)
    + rec_int_check_record_mandatory (rset, record, errors_buf)
    + rec_int_check_record_unique    (rset, record, errors_buf)
    + rec_int_check_record_prohibit  (rset, record, errors_buf);

  return res;
}

bool
rec_int_check_field_type (rec_db_t db,
                          rec_rset_t rset,
                          rec_field_t field,
                          char **errors)
{
  bool res;
  rec_type_reg_t type_reg;
  rec_field_name_t field_name;
  const char *rset_name;
  rec_rset_t referred_rset;
  rec_type_t type;
  rec_type_t referring_type;
  rec_type_t referred_type;
  char *errors_str;
  rec_buf_t buf_errors;
  size_t buf_errors_size;
  char tmp[1024];

  res = true;
  rset_name = NULL;
  referred_rset = NULL;
  referred_type = NULL;
  referring_type = NULL;

  field_name = rec_field_name (field);

  /* Get the proper type to check 'field' with.  The algorithm differs
     depending on the kind of field:
     
     - For normal fields, we check with the type from the type
       registry of 'rset', if any.

     - For compound fields (reference), we check with the type from
       the type registry of the referenced rset, if such an rset is
       found.

     Note that if a type declaration in the referring rset exist for
     the field and a conflict arises then the type descriptor in the
     referred record takes precedence and a warning is emitted.  (XXX:
     maybe a configurable error?).

  */

  /* Get the referred type, if any.  */
  if (rec_field_name_size (field_name) > 1)
    {
      rset_name = rec_field_name_get (field_name, 0);
      if (rset_name)
        {
          referred_rset = rec_db_get_rset_by_type (db, rset_name);
        }

      if (referred_rset)
        {
          type_reg = rec_rset_get_type_reg (referred_rset);
          if (type_reg)
            {
              referred_type = rec_type_reg_get (type_reg, rec_field_name (field));
            }
        }
    }

  /* Get the referring type, if any.  */
  type_reg = rec_rset_get_type_reg (rset);
  if (type_reg)
    {
      referring_type = rec_type_reg_get (type_reg, rec_field_name (field));
    }

  if (errors)
    {
      buf_errors = rec_buf_new (errors, &buf_errors_size);
    }    

  /* The referring type takes precedence.  */
  if (referring_type)
    {
      if (referred_type
          && (!rec_type_equal_p (referred_type, referring_type))
          && errors)
        {
          /* Emit a warning.  */
          sprintf (tmp, _("%s:%s: warning: type %s collides with referred type %s in the rset %s.\n"),
                   rec_field_source (field), rec_field_location_str (field),
                   rec_type_kind_str (referred_type),
                   rec_type_kind_str (referring_type),
                   rset_name);
          rec_buf_puts (tmp, buf_errors);
        }

      type = referring_type;
    }
  else
    {
      type = referred_type;
    }

  /* Check the field with the type.  */

  if (type)
    {
      if (!rec_type_check (type, rec_field_value (field), &errors_str))
        {
          if (errors)
            {
              sprintf (tmp, "%s:%s: error: %s\n",
                       rec_field_source (field), rec_field_location_str (field),
                       errors_str);
              rec_buf_puts (tmp, buf_errors);
            }
          free (errors_str);
          res = false;
        }
    }

  if (errors)
    {
      rec_buf_close (buf_errors);
    }

  return res;
}

static int
rec_int_check_record_types (rec_db_t db,
                            rec_rset_t rset,
                            rec_record_t record,
                            rec_buf_t errors)
{
  int res;
  rec_record_elem_t rec_elem;
  rec_field_t field;

  res = 0;

  rec_elem = rec_record_null_elem ();
  while (rec_record_elem_p (rec_elem = rec_record_next_field (record, rec_elem)))
    {
      field = rec_record_elem_field (rec_elem);

      /* Check for the type.  */
      if (!rec_int_check_field_type (db, rset, field, errors))
        {
          res++;
        }
    }

  return res;
}

static int
rec_int_check_record_mandatory (rec_rset_t rset,
                                rec_record_t record,
                                rec_buf_t errors)
{
  int res;
  rec_record_t descriptor;
  rec_fex_t mandatory_fex;
  rec_field_name_t field_name;
  rec_field_name_t mandatory_field_name;
  char *mandatory_field_str;
  rec_field_t field;
  size_t i, j, num_fields;
  char tmp[1024];
  
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
          mandatory_fex = rec_fex_new (rec_field_value (field), REC_FEX_SIMPLE);
          if (!mandatory_fex)
            {
              /* Invalid value in %mandatory:.  Ignore it.  */
              break;
            }

          for (j = 0; j < rec_fex_size (mandatory_fex); j++)
            {
              mandatory_field_name = rec_fex_elem_field_name (rec_fex_get (mandatory_fex, j));
              mandatory_field_str = rec_fex_elem_field_name_str (rec_fex_get (mandatory_fex, j));

              if (rec_record_get_num_fields_by_name (record, mandatory_field_name)
                  == 0)
                {
                  sprintf (tmp,
                           _("%s:%s: error: mandatory field '%s' not found in record\n"),
                           rec_record_source (record),
                           rec_record_location_str (record),
                           mandatory_field_str);
                  rec_buf_puts (tmp, errors);
                  res++;
                }
            }
        }
      
      rec_field_name_destroy (field_name);
    }

  return res;
}

static int
rec_int_check_record_unique (rec_rset_t rset,
                             rec_record_t record,
                             rec_buf_t errors)
{
  int res;
  rec_record_t descriptor;
  rec_fex_t unique_fex;
  rec_field_name_t field_name;
  rec_field_name_t unique_field_name;
  char *unique_field_str;
  rec_field_t field;
  size_t i, j, num_fields;
  char tmp[1024];
  
  res = 0;

  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      field_name = rec_parse_field_name_str ("%unique:");
      num_fields = rec_record_get_num_fields_by_name (descriptor, field_name);
      for (i = 0; i < num_fields; i++)
        {
          field = rec_record_get_field_by_name (descriptor, field_name, i);

          /* Parse the field name from the value of %unique:  */
          unique_fex = rec_fex_new (rec_field_value (field), REC_FEX_SIMPLE);
          if (!unique_fex)
            {
              /* Invalid value in %unique:.  Ignore it.  */
              break;
            }

          for (j = 0; j < rec_fex_size (unique_fex); j++)
            {
              unique_field_name = rec_fex_elem_field_name (rec_fex_get (unique_fex, j));
              unique_field_str = rec_fex_elem_field_name_str (rec_fex_get (unique_fex, j));

              if (rec_record_get_num_fields_by_name (record, unique_field_name)
                  > 1)
                {
                  sprintf (tmp,
                           _("%s:%s: error: field '%s' shall be unique in this record\n"),
                           rec_record_source (record),
                           rec_record_location_str (record),
                           unique_field_str);
                  rec_buf_puts (tmp, errors);
                  res++;
                }
            }
        }
      
      rec_field_name_destroy (field_name);
    }

  return res;
}

static int
rec_int_check_record_prohibit (rec_rset_t rset,
                               rec_record_t record,
                               rec_buf_t errors)
{
  int res;
  rec_record_t descriptor;
  rec_fex_t prohibit_fex;
  rec_field_name_t field_name;
  rec_field_name_t prohibit_field_name;
  char *prohibit_field_str;
  rec_field_t field;
  size_t i, j, num_fields;
  char tmp[1024];
  
  res = 0;

  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      field_name = rec_parse_field_name_str ("%prohibit:");
      num_fields = rec_record_get_num_fields_by_name (descriptor, field_name);
      for (i = 0; i < num_fields; i++)
        {
          field = rec_record_get_field_by_name (descriptor, field_name, i);

          /* Parse the field name from the value of %prohibit:  */
          prohibit_fex = rec_fex_new (rec_field_value (field), REC_FEX_SIMPLE);
          if (!prohibit_fex)
            {
              /* Invalid value in %prohibit:.  Ignore it.  */
              break;
            }

          for (j = 0; j < rec_fex_size (prohibit_fex); j++)
            {
              prohibit_field_name = rec_fex_elem_field_name (rec_fex_get (prohibit_fex, j));
              prohibit_field_str = rec_fex_elem_field_name_str (rec_fex_get (prohibit_fex, j));

              if (rec_record_get_num_fields_by_name (record, prohibit_field_name)
                  > 0)
                {
                  sprintf (tmp,
                           _("%s:%s: error: prohibited field '%s' found in record\n"),
                           rec_record_source (record),
                           rec_record_location_str (record),
                           prohibit_field_str);
                  rec_buf_puts (tmp, errors);
                  res++;
                }
            }
        }
      
      rec_field_name_destroy (field_name);
    }

  return res;
}

static int
rec_int_check_record_key (rec_rset_t rset,
                          rec_record_t orig_record,
                          rec_record_t record,
                          rec_buf_t errors)
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
  size_t num_fields;
  char tmp[1024];
  
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
              num_fields = rec_record_get_num_fields_by_name (record, key_field_name);

              if (num_fields == 0)
                {
                  sprintf (tmp,
                           _("%s:%s: error: key field '%s' not found in record\n"),
                           rec_record_source (record),
                           rec_record_location_str (record),
                           rec_field_value (field));
                  rec_buf_puts (tmp, errors);
                  res++;
                }
              else if (num_fields > 1)
                {
                  sprintf (tmp,
                           _("%s:%s: error: multiple key fields '%s' in record\n"),
                           rec_record_source (record),
                           rec_record_location_str (record),
                           rec_field_value (field));
                  rec_buf_puts (tmp, errors);
                  res++;
                }
              else  /* num_fields == 1 */
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
                      sprintf (tmp,
                               _("%s:%s: error: duplicated key value in field '%s' in record\n"),
                               rec_record_source (orig_record),
                               rec_record_location_str (orig_record),
                               rec_field_name_str (key));
                      rec_buf_puts (tmp, errors);
                      res++;
                      break;
                    }
                }

              rec_field_name_destroy (key_field_name);
            }
        }                                          
      
      rec_field_name_destroy (field_name);
    }

  return res;
}

static int
rec_int_check_descriptor (rec_rset_t rset,
                          rec_buf_t errors)
{
  int res;
  rec_record_t descriptor;
  rec_record_elem_t rec_elem;
  rec_field_t field;
  rec_field_name_t field_name;
  rec_field_name_t rec_fname;
  rec_field_name_t key_fname;
  rec_field_name_t type_fname;
  rec_field_name_t mandatory_fname;
  rec_field_name_t unique_fname;
  rec_field_name_t prohibit_fname;
  char *field_value;
  rec_fex_t fex;
  char tmp[1024];

  res = 0;
  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      /* Prepare fnames.  */
      rec_fname = rec_parse_field_name_str ("%rec:");
      key_fname = rec_parse_field_name_str ("%key:");
      type_fname = rec_parse_field_name_str ("%type:");
      mandatory_fname = rec_parse_field_name_str ("%mandatory:");
      unique_fname = rec_parse_field_name_str ("%unique:");
      prohibit_fname = rec_parse_field_name_str ("%prohibit:");

      /* Check the type of the record set:

         1. There should be one (and only one) %rec: field in the
            record.
         2. The value of the %rec: field shall be well-formed.
      */
      if (rec_record_get_num_fields_by_name (descriptor, rec_fname) == 0)
        {
          sprintf (tmp,
                   _("%s:%s: error: missing %%rec field in record descriptor\n"),
                   rec_record_source (descriptor),
                   rec_record_location_str (descriptor));
          rec_buf_puts (tmp, errors);
          res++;
        }
      else if (rec_record_get_num_fields_by_name (descriptor, rec_fname) > 1)
        {
          sprintf (tmp,
                   _("%s:%s: error: too many %%rec fields in record descriptor\n"),
                   rec_record_source (descriptor),
                   rec_record_location_str (descriptor));
          rec_buf_puts (tmp, errors);
          res++;
        }

      field = rec_record_get_field_by_name (descriptor, rec_fname, 0);
      if (!rec_int_rec_type_p (rec_field_value (field)))
        {
          sprintf (tmp,
                   _("%s:%s: error: invalid record type %s\n"),
                   rec_field_source (field),
                   rec_field_location_str (field),
                   rec_field_value (field));
          rec_buf_puts (tmp, errors);
          res++;
        }

      /* Only one 'key:' entry is allowed, if any.  */
      if (rec_record_get_num_fields_by_name (descriptor, key_fname) > 1)
        {
          sprintf (tmp,
                   _("%s:%s: error: only one %%key field is allowed in a record descriptor\n"),
                   rec_record_source (descriptor),
                   rec_record_location_str (descriptor));
          rec_buf_puts (tmp, errors);
          res++;
        }

      /* Iterate on fields.  */
      rec_elem = rec_record_null_elem ();
      while (rec_record_elem_p (rec_elem = rec_record_next_field (descriptor, rec_elem)))
        {
          field = rec_record_elem_field (rec_elem);
          field_name = rec_field_name (field);
          field_value = rec_field_value (field);

          if (rec_field_name_equal_p (field_name, type_fname))
            {
              /* Check the type descriptor.  */
              if (!rec_type_descr_p (field_value))
                {
                  /* XXX: make rec_type_descr_p to report more details.  */
                  fprintf (tmp,
                           _("%s:%s: error: invalid type specification\n"),
                           rec_field_source (field),
                           rec_field_location_str (field));
                  rec_buf_puts (tmp, errors);
                  res++;
                }
            }
          else if (rec_field_name_equal_p (field_name, mandatory_fname)
                   || rec_field_name_equal_p (field_name, unique_fname)
                   || rec_field_name_equal_p (field_name, prohibit_fname))
            {
              /* Check that the value of this field is a parseable
                 list of field names.  */
              fex = rec_fex_new (field_value, REC_FEX_SIMPLE);
              if (fex)
                {
                  rec_fex_destroy (fex);
                }
              else
                {
                  sprintf (tmp,
                           _("%s:%s: error: value for %s[%d] is not a list of field names\n"),
                           rec_record_source (descriptor),
                           rec_record_location_str (descriptor),
                           rec_field_name_str (field),
                           rec_record_get_field_index_by_name (descriptor, field));
                  rec_buf_puts (tmp, errors);
                  res++;
                }
            }
        }

      /* Destroy names.  */
      rec_field_name_destroy (rec_fname);
      rec_field_name_destroy (key_fname);
      rec_field_name_destroy (type_fname);
      rec_field_name_destroy (mandatory_fname);
      rec_field_name_destroy (unique_fname);
      rec_field_name_destroy (prohibit_fname);
    }

  return res;
}

void
rec_int_merge_remote (rec_rset_t rset)
{
  rec_parser_t parser;
  rec_field_name_t rec_fname;
  rec_record_t descriptor;
  rec_db_t remote_db;
  rec_rset_t remote_rset;
  rec_field_t remote_field;
  rec_record_elem_t rec_elem;
  rec_record_t remote_descriptor;
  rec_field_t rec_field;
  char *rec_type;
  char *rec_url = NULL;
  char *rec_file = NULL;
  char *rec_source = NULL;
  FILE *external_file;
  int tmpfile_des;
  char tmpfile_name[14];

  rec_fname = rec_parse_field_name_str ("%rec:");
  tmpfile_name[0] = '\0';

  /* If a remote descriptor is defined in the record descriptor of
     RSET, fetch it and merge it with the local descriptor.  */

  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      /* Check if there is an URL in the %rec: field.  */
      rec_field = rec_record_get_field_by_name (descriptor, rec_fname, 0);

      if (!rec_int_rec_type_p (rec_field_value (rec_field)))
        {
          return;
        }

      rec_type = rec_extract_type (rec_field_value (rec_field));
      rec_file = rec_extract_file (rec_field_value (rec_field));
      rec_url  = rec_extract_url  (rec_field_value (rec_field));

      if (rec_file || rec_url)
        {
          if (rec_url)
            {
#if defined REMOTE_DESCRIPTORS
              CURL *curl;

              /* Fetch the remote descriptor.  */
              curl = curl_easy_init ();

              /* Create a temporary file.  */
              strncpy (tmpfile_name, "recint-XXXXXX", 13);
              tmpfile_name[13] = '\0';
              tmpfile_des = gen_tempname (tmpfile_name, 0, 0, GT_FILE);
              external_file = fdopen (tmpfile_des, "r+");
              
              /* Fetch the remote file.  */
              curl_easy_setopt (curl, CURLOPT_URL, rec_url);
              curl_easy_setopt (curl, CURLOPT_WRITEDATA, external_file);
              curl_easy_setopt (curl, CURLOPT_FAILONERROR, 1);
              if (curl_easy_perform (curl) != 0)
                {
                  /* Print a warning and return.  */
                  fprintf (stderr,
                           _("warning: could not fetch remote descriptor from url %s.\n"),
                           rec_url);
                  goto exit;
                }
              curl_easy_cleanup (curl);
              rec_source = rec_url;
#else
              goto exit;
#endif /* REMOTE_DESCRIPTORS */
            }
          else
            {
              /* Try to open the file.  */
              external_file = fopen (rec_file, "r");
              if (!external_file)
                {
                  fprintf (stderr,
                           _("warning: could not read external descriptor from file %s.\n"),
                           rec_file);
                  goto exit;
                }
              rec_source = rec_file;
            }              

          /* Parse the contents of the external file.  */
          fseek (external_file, 0, SEEK_SET);
          parser = rec_parser_new (external_file, rec_source);
          if (!rec_parse_db (parser, &remote_db))
            {
              fprintf (stderr,
                       _("warning: %s does not contain valid rec data.\n"),
                       rec_url);
              goto exit;
            }
          rec_parser_destroy (parser);
          
          /* Get the proper external descriptor and merge it with
             the local one.  */
          remote_rset = rec_db_get_rset_by_type (remote_db, rec_type);
          if (!remote_rset)
            {
              fprintf (stderr,
                       _("warning: %s does not contain information for %s.\n"),
                       rec_source, rec_type);
              goto exit;
            }
          remote_descriptor = rec_rset_descriptor (remote_rset);
          if (!remote_descriptor)
            {
              /* Do nothing.  */
              goto exit;
            }
          
          rec_elem = rec_record_first_field (remote_descriptor);
          while (rec_record_elem_p (rec_elem))
            {
              remote_field = rec_record_elem_field (rec_elem);
              
              /* Merge the descriptors, but take care to not add a
                 new %rec: field.  */
              if (!rec_field_name_equal_p (rec_field_name (remote_field), rec_fname))
                {
                  rec_record_append_field (descriptor, rec_field_dup (remote_field));
                }
              rec_elem = rec_record_next_field (remote_descriptor, rec_elem);
            }
          
          /* Update the record descriptor (triggering the creation
             of a new type registry).  */
          rec_rset_set_descriptor (rset, rec_record_dup (descriptor));
          
          rec_db_destroy (remote_db);
          fclose (external_file);
        }
    }

 exit:

  if (rec_url && (tmpfile_name[0] != '\0'))
    {
      remove (tmpfile_name);
    }

  free (rec_url);
  free (rec_file);
}

static bool
rec_int_rec_type_p (char *str)
{
  regex_t regexp;
  bool ret;

  if (regcomp (&regexp, "^[ \t]*"
               REC_FNAME_PART_RE
               "[ \n\t]*"
               "("
               "(" REC_URL_REGEXP ")"
               "|"
               "(" REC_FILE_REGEXP ")"
               "[ \t]*)?"
               "$",
               REG_EXTENDED) != 0)
    {
      fprintf (stderr, _("internal error: rec_int_rec_type_p: error compiling regexp.\n"));
      return false;
    }

  ret = (regexec (&regexp, str, 0, NULL, 0) == 0);
  regfree (&regexp);

  return ret;
}

/* End of rec-int.c */
