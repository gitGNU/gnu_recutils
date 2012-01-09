/* -*- mode: C -*-
 *
 *       File:         recins.c
 *       Date:         Mon Dec 28 08:54:38 2009
 *
 *       GNU recutils - recins
 *
 */

/* Copyright (C) 2009, 2010, 2011, 2012 Jose E. Marchesi */

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
#include <time.h>
#include <locale.h>
#include <getpass.h>
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
char         *recutl_quick_str = NULL;
bool          recutl_insensitive = false;
rec_record_t  recins_record    = NULL;
char         *recins_file      = NULL;
bool          recins_force     = false;
bool          recins_verbose   = false;
bool          recins_external  = true;
bool          recins_auto      = true;
char         *recins_password  = NULL;
size_t        recutl_random    = 0;

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
Usage: recins [OPTION]... [t TYPE] [-n NUM | -e EXPR | -q STR | -m NUM] [(-f STR -v STR]|[-R RECDATA)]... [FILE]\n"));

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
  recutl_print_help_footer ();
}

#if defined REC_CRYPT_SUPPORT

void
recins_encrypt_record (rec_rset_t rset,
                       rec_record_t record)
{
  /* Encrypt the value of fields declared as confidential in this
     record set.  */

  rec_fex_t confidential_fields = rec_rset_confidential (rset);

  if (rec_fex_size (confidential_fields) > 0)
    {
      if (!recins_password && recutl_interactive ())
        {
          recins_password = getpass (_("Password: "));
        }
      
      if (recins_password && (strlen (recins_password) > 0))
        {
          if (!rec_encrypt_record (rset, record, recins_password))
            {
              recutl_fatal ("encrypting a record.  Please report this.\n");
            }
        }
    }
}

#endif /* REC_CRYPT_SUPPORT */

bool
recins_insert_record (rec_db_t db,
                      char *type,
                      rec_record_t record)
{
  bool res;
  rec_rset_t rset;

  if (!record || (rec_record_num_fields (record) == 0))
    {
      /* Do nothing.  */
      return true;
    }

  res = true;
  
  rset = rec_db_get_rset_by_type (db, type);
  if (rset)
    {

      if (recins_auto)
        {
          /* Add auto-set fields required by this record set.  */
          rec_rset_add_auto_fields (rset, record);
        }

#if defined REC_CRYPT_SUPPORT
      recins_encrypt_record (rset, record);
#endif

      /* Set the container in the new record.  */
      rec_record_set_container (record, rset);

      if (rec_rset_num_records (rset) == 0)
        {
          /* The rset is empty => Insert the new record just after the
             relative position of the record descriptor.

             XXX: move this logic into an 'rec_rset_prepend_record' function.
          */

          rec_mset_insert_at (rec_rset_mset (rset),
                              MSET_RECORD,
                              (void *) record,
                              rec_rset_descriptor_pos (rset));
        }
      else
        {
          /* Insert the new record after the last record in the
             set.  */

          rec_mset_t mset = rec_rset_mset (rset);
          rec_record_t last_record =
            (rec_record_t) rec_mset_get_at (mset,
                                            MSET_RECORD,
                                            rec_rset_num_records (rset) - 1);

          rec_mset_insert_after (mset,
                                 MSET_RECORD,
                                 (void *) record,
                                 rec_mset_search (mset, (void *) last_record));
        }
    }
  else
    {
      /* Create a new type and insert the record there.  We don't need
         to check for the type of the field in this case.  */
      rset = rec_rset_new ();
      rec_rset_set_type (rset, type);
      rec_record_set_container (record, rset);
      rec_mset_append (rec_rset_mset (rset), MSET_RECORD, (void *) record, MSET_ANY);
      
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
            rec_mset_append (rec_record_mset (recins_record), MSET_FIELD, (void *) field, MSET_ANY);

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
                
                rec_mset_iterator_t iter = rec_mset_iterator (rec_record_mset (provided_record));
                while (rec_mset_iterator_next (&iter, MSET_FIELD, (const void **) &field, NULL))
                  {
                    rec_mset_append (rec_record_mset (recins_record), MSET_FIELD, (void *) rec_field_dup (field), MSET_ANY);
                    field = NULL;
                  }
                rec_mset_iterator_free (&iter);

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
  rec_mset_iterator_t iter;
  rec_mset_elem_t elem;
  size_t num_rec;
  bool parse_status;

  if ((recutl_num_indexes() != 0)
      || (recutl_sex_str != NULL)
      || (recutl_quick_str != NULL)
      || (recutl_random > 0))
    {
      /* Replace matching records.  */
      rset = rec_db_get_rset_by_type (db, recutl_type);
      if (rset)
        {
          num_rec = -1;


          /* If the user requested to replace random records,
             calculate them now for this record set.  */

          if (recutl_random > 0)
            {
              recutl_reset_indexes ();
              recutl_index_add_random (recutl_random, rec_rset_num_records (rset));
            }
     
          if (recins_auto)
            {
              /* Add auto-set fields required by this record set.  */
              rec_rset_add_auto_fields (rset, recins_record);
            }

#if defined REC_CRYPT_SUPPORT
          recins_encrypt_record (rset, recins_record);
#endif

          iter = rec_mset_iterator (rec_rset_mset (rset));
          while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void **) &record, &elem))
            {
              num_rec++;

              /* Shall we skip this record  */
              if (((recutl_num_indexes() != 0) && (!recutl_index_p (num_rec)))
                  || (recutl_quick_str && !rec_record_contains_value (record,
                                                                      recutl_quick_str,
                                                                      recutl_insensitive))
                  || (recutl_sex_str && !(rec_sex_eval (recutl_sex, record, &parse_status)
                                          && parse_status)))
                {
                  if (recutl_sex_str && (!parse_status))
                    {
                      recutl_error (_("evaluating the selection expression.\n"));
                      exit (EXIT_FAILURE);
                    }
                }
              else
                {
                  rec_record_set_container (recins_record, rset);
                  rec_mset_elem_set_data (elem, (void *) rec_record_dup (recins_record));
                }
            }
        }

      rec_mset_iterator_free (&iter);
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
