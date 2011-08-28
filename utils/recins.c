/* -*- mode: C -*-
 *
 *       File:         recins.c
 *       Date:         Mon Dec 28 08:54:38 2009
 *
 *       GNU recutils - recins
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

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <xalloc.h>
#include <gettext.h>
#include <errno.h>
#include <locale.h>
#include <time.h>
#define _(str) gettext (str)
#include <base64.h>

#include <rec.h>
#include <recutl.h>

/* Forward declarations.  */
bool recins_insert_record (rec_db_t db, char *type, rec_record_t record);
void recins_parse_args (int argc, char **argv);

/*
 * Global variables
 */

char         *recutl_type      = NULL;
rec_sex_t     recutl_sex       = NULL;
char         *recutl_sex_str   = NULL;
int           recutl_num       = -1;
bool          recutl_insensitive = false;
rec_record_t  recins_record    = NULL;
char         *recins_file      = NULL;
bool          recins_force     = false;
bool          recins_verbose   = false;
bool          recins_external  = true;
bool          recins_auto      = true;
char         *recins_password  = NULL;

/*
 * Command line options management
 */

enum
{
  COMMON_ARGS,
  RECORD_SELECTION_ARGS,
  NAME_ARG,
  VALUE_ARG,
  FORCE_ARG,
  VERBOSE_ARG,
  NO_EXTERNAL_ARG,
  RECORD_ARG,
  NO_AUTO_ARG,
  PASSWORD_ARG
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    RECORD_SELECTION_LONG_ARGS,
    {"type", required_argument, NULL, TYPE_ARG},
    {"name", required_argument, NULL, NAME_ARG},
    {"value", required_argument, NULL, VALUE_ARG},
    {"force", no_argument, NULL, FORCE_ARG},
    {"verbose", no_argument, NULL, VERBOSE_ARG},
    {"no-external", no_argument, NULL, NO_EXTERNAL_ARG},
    {"record", required_argument, NULL, RECORD_ARG},
    {"no-auto", no_argument, NULL, NO_AUTO_ARG},
    {"password", required_argument, NULL, PASSWORD_ARG},
    {NULL, 0, NULL, 0}
  };

/*
 * Functions.
 */

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, recins synopsis.
     no-wrap */
  printf (_("\
Usage: recins [OPTION]... [t TYPE] [-n NUM | -e EXPR] [(-f STR -v STR]|[-r RECDATA)]... [FILE]\n"));

  /* TRANSLATORS: --help output, recins short description.
     no-wrap */
  fputs (_("\
Insert new records in a rec database.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, recins arguments.
     no-wrap */
  fputs (_("\
  -f, --field=STR                     field name; should be followed by a -v.\n\
  -v, --value=STR                     field value; should be preceded by an -f.\n\
  -r, --record=STR                    record that will be inserted in the file.\n\
      --force                         insert the record even if it is violating\n\
                                        record restrictions.\n\
      --no-external                   don't use external descriptors.\n\
      --no-auto                       don't insert auto generated fields.\n\
      --verbose                       give a detailed report if the integrity check\n\
                                        fails.\n"), stdout);

#if defined REC_CRYPT_SUPPORT
  /* TRANSLATORS: --help output, encryption related options.
     no-wrap */
  fputs (_("\
  -s, --password=STR                  encrypt confidential fields with the given password.\n"),
         stdout);
#endif

  recutl_print_help_common ();

  puts ("");
  recutl_print_help_record_selection ();

  puts ("");
  /* TRANSLATORS: --help output, notes on recins.
     no-wrap */
  fputs (_("\
If no FILE is specified then the command acts like a filter, getting\n\
the data from standard input and writing the result to standard output.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, recins examples.
     no-wrap */
  fputs (_("\
Examples:\n\
\n\
        recins -f Name -v \"Mr Foo\" -f Email -v foo@foo.org contacts.rec\n\
        cat hackers.rec | recins -t Hacker -f Email -v foo@foo.org > other.rec\n"),
         stdout);

  puts ("");
  recutl_print_help_footer ();
}

void
recins_add_auto_field_int (rec_rset_t rset,
                           rec_field_name_t field_name,
                           rec_record_t record)
{
  rec_rset_elem_t rset_elem;
  rec_record_t rec;
  rec_field_t field;
  size_t num_fields, i;
  int auto_value, field_value;
  char *end;
  char *auto_value_str;
  rec_record_elem_t rec_elem;

  /* Find the auto value.  */

  auto_value = 0;

  rset_elem = rec_rset_first_record (rset);
  while (rec_rset_elem_p (rset_elem))
    {
      rec = rec_rset_elem_record (rset_elem);

      num_fields = rec_record_get_num_fields_by_name (rec, field_name);
      for (i = 0; i < num_fields; i++)
        {
          field = rec_record_get_field_by_name (rec, field_name, i);
          
          /* Ignore fields that can't be converted to integer
             values.  */
          errno = 0;
          field_value = strtol (rec_field_value (field), &end, 10);
          if ((errno == 0) && (*end == '\0'))
            {
              if (auto_value <= field_value)
                {
                  auto_value = field_value + 1;
                }
            }
        }

      rset_elem = rec_rset_next_record (rset, rset_elem);
    }
       
  /* Insert the auto field.  */
  asprintf (&auto_value_str, "%d", auto_value);
  field = rec_field_new (rec_field_name_dup (field_name),
                         auto_value_str);
  rec_elem = rec_record_elem_field_new (record, field);
  rec_record_insert_at (record, rec_elem, 0);
  free (auto_value_str);
}

void
recins_add_auto_field_date (rec_rset_t rset,
                            rec_field_name_t field_name,
                            rec_record_t record)
{
  rec_field_t auto_field;
  rec_record_elem_t rec_elem;
  time_t t;
  char outstr[200];
  struct tm *tmp;

  t = time (NULL);
  tmp = localtime (&t);

  setlocale (LC_TIME, "C"); /* We want english dates that can be
                                 parsed with parse_datetime */
  strftime (outstr, sizeof(outstr), "%a, %d %b %Y %T %z", tmp);
  setlocale (LC_TIME, ""); /* And restore the locale from the
                              environment. */

  auto_field = rec_field_new (rec_field_name_dup (field_name),
                              outstr);
  rec_elem = rec_record_elem_field_new (record, auto_field);
  rec_record_insert_at (record, rec_elem, 0);
}

#if defined REC_CRYPT_SUPPORT

rec_record_t
recins_encrypt_fields (rec_rset_t rset,
                       rec_record_t record)
{
  rec_field_t field;
  rec_record_t res = NULL;
  rec_field_name_t field_name;
  rec_fex_t confidential_fields;
  size_t i, k, num_fields;
  
  res = rec_record_dup (record);

  if (rset)
    {
      confidential_fields = rec_rset_confidential (rset);
      if (!recins_password && (rec_fex_size (confidential_fields) > 0))
        {
          recutl_warning ("the record set contains confidential fields but no password was provided\n");
          recutl_warning ("the resulting record will have those fields unencrypted!\n");
          return res;
        }

      for (i = 0; i < rec_fex_size (confidential_fields); i++)
        {
          field_name = rec_fex_elem_field_name (rec_fex_get (confidential_fields, i));

          num_fields = rec_record_get_num_fields_by_name (res, field_name);
          for (k = 0; k < num_fields; k++)
            {
              field = rec_record_get_field_by_name (res, field_name, k);
              if (field)
                {
                  /* Encrypt the value of this field.  */
                  char *field_value = xstrdup (rec_field_value (field));
                  char *field_value_encrypted;
                  char *field_value_base64;
                  size_t out_size, base64_size;

                  if (!rec_encrypt (field_value,
                                    strlen (field_value),
                                    recins_password,
                                    &field_value_encrypted,
                                    &out_size))
                    {
                      recutl_fatal (_("rec_encrypt failed.  Please report this\n"));
                    }

                  /* Encode the encrypted into base64.  */
                  base64_size = base64_encode_alloc (field_value_encrypted,
                                                     out_size,
                                                     &field_value_base64);
                  base64_encode (field_value_encrypted,
                                 out_size,
                                 field_value_base64,
                                 base64_size);

                  /* Replace the value of the field.  */
                  rec_field_set_value (field, field_value_base64);

                  /* Free resources.  */
                  free (field_value);
                  free (field_value_encrypted);
                  free (field_value_base64);
                }
            }
        }
    }  

  return res;
}

#endif /* REC_CRYPT_SUPPORT */

rec_record_t
recins_add_auto_fields (rec_rset_t rset,
                        rec_record_t record)
{
  rec_record_t res = NULL;
  rec_field_name_t auto_field_name;
  rec_fex_t auto_fields;
  rec_type_t type;
  int i;

  res = rec_record_dup (record);

  if (!recins_auto)
    {
      return res;
    }

  if (rset)
    {
      if ((auto_fields = rec_rset_auto (rset)))
        {
          for (i = 0; i < rec_fex_size (auto_fields); i++)
            {
              auto_field_name = rec_fex_elem_field_name (rec_fex_get (auto_fields, i));
              
              if (!rec_record_field_p (record, auto_field_name))
                {
                  /* The auto field is not already present in record, so add
                     one automatically.  Depending on its type the value is
                     calculated differently. If the record does not have a
                     type, or the type is incorrect, ignore it.  */
                  
                  type = rec_rset_get_field_type (rset, auto_field_name);
                  if (type)
                    {
                      switch (rec_type_kind (type))
                        {
                        case REC_TYPE_INT:
                        case REC_TYPE_RANGE:
                          {
                            recins_add_auto_field_int (rset,
                                                       auto_field_name,
                                                       res);
                            break;
                          }
                        case REC_TYPE_DATE:
                          {
                            recins_add_auto_field_date (rset,
                                                        auto_field_name,
                                                        res);
                            break;
                          }
                        default:
                          {
                            /* Do nothing for other types.  */
                            break;
                          }
                        }
                    }
                }
            }
        }
    }

  return res;
}

bool
recins_insert_record (rec_db_t db,
                      char *type,
                      rec_record_t record)
{
  bool res;
  rec_rset_t rset;
  rec_rset_elem_t last_elem, new_elem;
  rec_record_t record_to_insert;

  if (!record || (rec_record_num_fields (record) == 0))
    {
      /* Do nothing.  */
      return true;
    }

  res = true;
  
  rset = rec_db_get_rset_by_type (db, type);
  if (rset)
    {
      /* Add auto-set fields required by this record set.  */
      record_to_insert = recins_add_auto_fields (rset, record);

#if defined REC_CRYPT_SUPPORT
          /* Encrypt the value of fields declared as confidential in
             this record set.  */

          record_to_insert = recins_encrypt_fields (rset, recins_record);
#endif

      new_elem = rec_rset_elem_record_new (rset, record_to_insert);

      if (rec_rset_num_records (rset) == 0)
        {
          /* The rset is empty => Insert the new record just after the
             relative position of the record descriptor.

             XXX: move this logic into an 'rec_rset_prepend_record' function.
          */
          rec_rset_insert_at (rset, new_elem, rec_rset_descriptor_pos (rset));
        }
      else
        {
          /* Insert the new record after the last record in the
             set.  */
          last_elem = rec_rset_get_record (rset,
                                           rec_rset_num_records (rset) - 1);
          rec_rset_insert_after (rset, last_elem, new_elem);
        }
    }
  else
    {
      /* Create a new type and insert the record there.  We don't need
         to check for the type of the field in this case.  */
      rset = rec_rset_new ();
      rec_rset_set_type (rset, type);
      rec_rset_append_record (rset, record);
      
      if (type)
        {
          rec_db_insert_rset (db, rset, rec_db_size (db));
        }
      else
        {
          /* The default rset should always be in the beginning of
             the db.  */
          rec_db_insert_rset (db, rset, -1);
        }
    }

  return res;
}

void recins_parse_args (int argc,
                        char **argv)
{
  int ret;
  char c;
  rec_field_t field;
  rec_field_name_t field_name;
  char *field_name_str;
  rec_record_t provided_record;
  rec_record_elem_t rec_elem;

  field = NULL;
  field_name_str = NULL;

  while ((ret = getopt_long (argc,
                             argv,
                             RECORD_SELECTION_SHORT_ARGS
                             "f:v:r:s:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
          RECORD_SELECTION_ARGS_CASES
        case FORCE_ARG:
          {
            recins_force = true;
            break;
          }
        case VERBOSE_ARG:
          {
            recins_verbose = true;
            break;
          }
        case NAME_ARG:
        case 'f':
          {
            if (field != NULL)
              {
                recutl_fatal (_("a -f should be followed by a -v\n"));
                exit (EXIT_FAILURE);
              }

            if (recins_record == NULL)
              {
                recins_record = rec_record_new ();
                rec_record_set_source (recins_record, "cmdli");
                rec_record_set_location (recins_record, 0);
              }

            /* Make sure that the field name ends with a colon ':'.  */
            field_name_str = xmalloc (strlen (optarg) + 2);
            field_name_str = strncpy (field_name_str, optarg, strlen (optarg));
            if (field_name_str[strlen (optarg) - 1] != ':')
              {
                field_name_str[strlen (optarg)] = ':';
                field_name_str[strlen (optarg) + 1] = 0;
              }
            else
              {
                field_name_str[strlen (optarg)] = 0;
              }

            if (!(field_name = rec_parse_field_name_str (field_name_str)))
              {
                recutl_fatal (_("invalid field name %s.\n"), optarg);
              }
            
            field = rec_field_new (field_name,
                                   "foo");
            break;
          }
        case VALUE_ARG:
        case 'v':
          {
            if (field == NULL)
              {
                recutl_fatal (_("a -v should be preceded by a -f\n"));
              }

            rec_field_set_value (field, optarg);
            rec_record_append_field (recins_record, field);

            field = NULL;
            break;
          }
        case NO_EXTERNAL_ARG:
          {
            recins_external = false;
            break;
          }
        case NO_AUTO_ARG:
          {
            recins_auto = false;
            break;
          }
        case PASSWORD_ARG:
        case 's':
          {
            if (recins_password != NULL)
              {
                recutl_fatal (_("more than one password was specified\n"));
              }

            recins_password = xstrdup (optarg);
            break;
          }
        case RECORD_ARG:
        case 'r':
          {
            /* Parse the provided record and put it in recins_record.  */
            provided_record = rec_parse_record_str (optarg);
            if (!provided_record)
              {
                recutl_fatal (_("error while parsing the record provided by -r\n"));
              }

            if (recins_record)
              {
                /* Append the fields in provided_record into
                   recins_record.  */
                rec_elem = rec_record_first_field (provided_record);
                while (rec_record_elem_p (rec_elem))
                  {
                    field = rec_record_elem_field (rec_elem);
                    rec_record_append_field (recins_record, rec_field_dup (field));
                    field = NULL;
                    rec_elem = rec_record_next_field (provided_record, rec_elem);
                  }

                rec_record_destroy (provided_record);
                provided_record = NULL;
              }
            else
              {
                recins_record = provided_record;
              }

            break;
          }
        default:
          {
            exit (EXIT_FAILURE);
          }
        }
    }

  if (field != NULL)
    {
      recutl_fatal (_("please provide a value for the field %s\n"), field_name_str);
    }

  /* Read the name of the file where to make the insertions.  */
  if (optind < argc)
    {

      if ((argc - optind) != 1)
        {
          recutl_print_help ();
          exit (EXIT_FAILURE);
        }

      recins_file = argv[optind++];
    }
}

void
recins_add_new_record (rec_db_t db)
{
  rec_rset_t rset;
  rec_record_t record;
  rec_rset_elem_t rset_elem;
  rec_rset_elem_t new_rset_elem;
  size_t num_rec;
  bool parse_status;
  rec_record_t record_to_insert;

  if ((recutl_num != -1)
      || (recutl_sex_str != NULL))
    {
      /* Replace matching records.  */
      rset = rec_db_get_rset_by_type (db, recutl_type);
      if (rset)
        {
          num_rec = -1;
     
          /* Add auto-set fields required by this record set.  */
          record_to_insert = recins_add_auto_fields (rset, recins_record);

#if defined REC_CRYPT_SUPPORT
          /* Encrypt the value of fields declared as confidential in
             this record set.  */


          record_to_insert = recins_encrypt_fields (rset, recins_record);
#endif

          rset_elem = rec_rset_first_record (rset);
          while (rec_rset_elem_p (rset_elem))
            {
              num_rec++;
              record = rec_rset_elem_record (rset_elem);

              /* Shall we skip this record?  */
              if (((recutl_num != -1) && (recutl_num != num_rec))
                  || (recutl_sex_str && !(rec_sex_eval (recutl_sex, record, &parse_status)
                                          && parse_status)))
                {
                  if (recutl_sex_str && (!parse_status))
                    {
                      recutl_error (_("evaluating the selection expression.\n"));
                      exit (EXIT_FAILURE);
                    }
                  rset_elem = rec_rset_next_record (rset, rset_elem);
                }
              else
                {
                  new_rset_elem = rec_rset_elem_record_new (rset, record_to_insert);
                  rec_rset_insert_after (rset, rset_elem, new_rset_elem);
                  rec_rset_remove (rset, rset_elem);
                  rset_elem = rec_rset_next_record (rset, new_rset_elem);
                }
            }
        }
    }
  else
    {
      /* Append the record in the proper rset.  */
      recins_insert_record (db, recutl_type, recins_record);
    }

  /* Integrity check.  */
  if (!recins_force && db)
    {
      recutl_check_integrity (db, recins_verbose, recins_external);
    }
}

int
main (int argc, char *argv[])
{
  rec_db_t db;

  recutl_init ("recins");

  recins_parse_args (argc, argv);

  db = recutl_read_db_from_file (recins_file);
  if (!db)
    {
      /* Create an empty database.  */
      db = rec_db_new ();
    }
  recins_add_new_record (db);
  recutl_write_db_to_file (db, recins_file);

  return EXIT_SUCCESS;
}

/* End of recins.c */
