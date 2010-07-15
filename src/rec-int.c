/* -*- mode: C -*- Time-stamp: "2010-07-15 18:49:12 jemarch"
 *
 *       File:         rec-int.c
 *       Date:         Thu Jul 15 18:23:26 2010
 *
 *       GNU recutils - Data integrity.
 *
 */

#include <config.h>

#include <rec.h>

/*
 * Forward references.
 */

static int rec_int_check_descriptor (rec_rset_t rset, FILE *errors);
static int rec_int_check_record_key (rec_rset_t rset,
                                     rec_record_t orig_record, rec_record_t record,
                                     FILE *errors);
static int rec_int_check_record_types (rec_rset_t rset, rec_record_t record,
                                       FILE *errors);
static int rec_int_check_record_mandatory (rec_rset_t rset, rec_record_t record,
                                           FILE *errors);
static int rec_int_check_record_unique (rec_rset_t rset, rec_record_t record,
                                        FILE *errors);
static int rec_int_check_record_prohibit (rec_rset_t rset, rec_record_t record,
                                          FILE *errors);

/*
 * Public functions.
 */

int
rec_int_check_rset (rec_rset_t rset,
                    bool check_descriptor_p,
                    FILE *errors)
{
  int res;
  rec_rset_elem_t rset_elem;
  rec_record_t record;
  rec_record_t descriptor;

  res = 0;

  if (check_descriptor_p)
    {
      res += rec_int_check_descriptor (rset, errors);
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

      res += rec_int_check_record (rset,
                                   record, record,
                                   errors);
    }

  return res;
}

int
rec_int_check_record (rec_rset_t rset,
                      rec_record_t orig_record,
                      rec_record_t record,
                      FILE *errors)
{
  int res;

  res =
    rec_int_check_record_key (rset, orig_record, record, errors)
    + rec_int_check_record_types     (rset, record, errors)
    + rec_int_check_record_mandatory (rset, record, errors)
    + rec_int_check_record_unique    (rset, record, errors)
    + rec_int_check_record_prohibit  (rset, record, errors);

  return res;
}

bool
rec_int_check_field_type (rec_rset_t rset,
                          rec_field_t field,
                          char **type_str)
{
  bool res;
  rec_type_t type;
  rec_type_reg_t type_reg;

  res = true;
  type_reg = rec_rset_get_type_reg (rset);

  if (type_reg)
    {
      type = rec_type_reg_get (type_reg, rec_field_name (field));
      if (type)
        {
          if (!rec_type_check (type, rec_field_value (field)))
            {
              *type_str = rec_type_kind_str (type);
              res = false;
            }
        }
    }

  return res;
}

static int
rec_int_check_record_types (rec_rset_t rset,
                            rec_record_t record,
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
      if (!rec_int_check_field_type (rset, field, &type_str))
        {
          fprintf (errors,
                   "%s:%s: error: expected '%s' value in %s[%d]\n",
                   rec_record_source (record),
                   rec_record_location_str (record),
                   type_str,
                   rec_field_name_str (field),
                   rec_record_get_field_index_by_name (record, field));

          res++;
        }
    }

  return res;
}

static int
rec_int_check_record_mandatory (rec_rset_t rset,
                                rec_record_t record,
                                FILE *errors)
{
  int res;
  rec_record_t descriptor;
  rec_fex_t mandatory_fex;
  rec_field_name_t field_name;
  rec_field_name_t mandatory_field_name;
  char *mandatory_field_str;
  rec_field_t field;
  size_t i, j, num_fields;
  
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
                  fprintf (errors,
                           "%s:%s: error: mandatory field '%s' not found in record\n",
                           rec_record_source (record),
                           rec_record_location_str (record),
                           mandatory_field_str);
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
                             FILE *errors)
{
  int res;
  rec_record_t descriptor;
  rec_fex_t unique_fex;
  rec_field_name_t field_name;
  rec_field_name_t unique_field_name;
  char *unique_field_str;
  rec_field_t field;
  size_t i, j, num_fields;
  
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
                  fprintf (errors,
                           "%s:%s: error: field '%s' shall be unique in this record\n",
                           rec_record_source (record),
                           rec_record_location_str (record),
                           unique_field_str);
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
                               FILE *errors)
{
  int res;
  rec_record_t descriptor;
  rec_fex_t prohibit_fex;
  rec_field_name_t field_name;
  rec_field_name_t prohibit_field_name;
  char *prohibit_field_str;
  rec_field_t field;
  size_t i, j, num_fields;
  
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
                  fprintf (errors,
                           "%s:%s: error: prohibited field '%s' found in record\n",
                           rec_record_source (record),
                           rec_record_location_str (record),
                           prohibit_field_str);
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
  size_t num_fields;
  
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
                  fprintf (errors,
                           "%s:%s: error: key field '%s' not found in record\n",
                           rec_record_source (record),
                           rec_record_location_str (record),
                           rec_field_value (field));
                  res++;
                }
              else if (num_fields > 1)
                {
                  fprintf (errors,
                           "%s:%s: error: multiple key fields '%s' in record\n",
                           rec_record_source (record),
                           rec_record_location_str (record),
                           rec_field_value (field));
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
                      fprintf (errors,
                               "%s:%s: error: duplicated key value in field '%s' in record\n",
                               rec_record_source (orig_record),
                               rec_record_location_str (orig_record),
                               rec_field_name_str (key));
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
                          FILE *errors)
{
  int res;
  rec_record_t descriptor;
  rec_record_elem_t rec_elem;
  rec_field_t field;
  rec_field_name_t field_name;
  rec_field_name_t key_fname;
  rec_field_name_t type_fname;
  rec_field_name_t mandatory_fname;
  rec_field_name_t unique_fname;
  rec_field_name_t prohibit_fname;
  char *field_value;
  rec_fex_t fex;

  res = 0;
  descriptor = rec_rset_descriptor (rset);
  if (descriptor)
    {
      /* Prepare fnames.  */
      key_fname = rec_parse_field_name_str ("%key:");
      type_fname = rec_parse_field_name_str ("%type:");
      mandatory_fname = rec_parse_field_name_str ("%mandatory:");
      unique_fname = rec_parse_field_name_str ("%unique:");
      prohibit_fname = rec_parse_field_name_str ("%prohibit:");

      /* Only one 'key:' entry is allowed, if any.  */
      if (rec_record_get_num_fields_by_name (descriptor, key_fname) > 1)
        {
          fprintf (errors,
                   "%s:%s: error: only one %%key field is allowed in a record descriptor\n",
                   rec_record_source (descriptor),
                   rec_record_location_str (descriptor));
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
                  fprintf (errors,
                           "%s:%s: error: invalid type specification in %%type[%d]\n",
                           rec_record_source (descriptor),
                           rec_record_location_str (descriptor),
                           rec_record_get_field_index_by_name (descriptor, field));
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
                  fprintf (errors,
                           "%s:%s: error: value for %s[%d] is not a list of field names\n",
                           rec_record_source (descriptor),
                           rec_record_location_str (descriptor),
                           rec_field_name_str (field),
                           rec_record_get_field_index_by_name (descriptor, field));
                  res++;
                }
            }
        }

      /* Destroy names.  */
      rec_field_name_destroy (key_fname);
      rec_field_name_destroy (type_fname);
      rec_field_name_destroy (mandatory_fname);
      rec_field_name_destroy (unique_fname);
      rec_field_name_destroy (prohibit_fname);
    }

  return res;
}

/* End of rec-int.c */
