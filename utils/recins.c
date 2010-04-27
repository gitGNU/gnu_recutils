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

#include <rec.h>
#include <recutl.h>

/* Forward declarations.  */
bool recins_insert_record (rec_db_t db, char *type, rec_record_t record);
void recins_parse_args (int argc, char **argv);

/*
 * Global variables
 */

char         *program_name   = NULL;
char         *recins_type    = NULL;
rec_record_t  recins_record  = NULL;
char         *recins_file    = NULL;
bool          recins_force   = false;
bool          recins_verbose = false;

/*
 * Command line options management
 */

enum
{
  COMMON_ARGS,
  TYPE_ARG,
  NAME_ARG,
  VALUE_ARG,
  FORCE_ARG,
  VERBOSE_ARG
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {"type", required_argument, NULL, TYPE_ARG},
    {"name", required_argument, NULL, NAME_ARG},
    {"value", required_argument, NULL, VALUE_ARG},
    {"force", no_argument, NULL, FORCE_ARG},
    {"verbose", no_argument, NULL, VERBOSE_ARG},
    {NULL, 0, NULL, 0}
  };

/* Messages */

RECUTL_COPYRIGHT_DOC ("recins");

char *recutl_help_msg = "\
Usage: recins [OPTION]... [-f STR -v STR]... [FILE]\n\
Insert a record in a rec file.\n\
\n\
  -t, --type=TYPE                     specify the type of the new record.\n\
  -f, --field=STR                     field name.  Should be followed by a -v.\n\
  -v, --value=STR                     field value.  Should be preceded by a -f.\n\
      --force                         insert the record even if it is violating\n\
                                        record restrictions.\n\
      --verbose                       get a detailed report if the integrity check\n\
                                        fails.\n"
COMMON_ARGS_DOC
"\n\
If no FILE is specified then the command acts like a filter, getting\n\
the data from the standard input and writing the result in the\n\
standard output.\n\
\n\
Examples:\n\
\n\
        recins -f Name -v \"Mr Foo\" -f Email -v foo@foo.org contacts.rec\n\
        cat hackers.rec | recins -t Hacker -f Email -v foo@foo.org > other.rec\n\
\n"
  RECUTL_HELP_FOOTER_DOC ("recins");

bool
recins_insert_record (rec_db_t db,
                      char *type,
                      rec_record_t record)
{
  bool res;
  rec_rset_t rset;
  rec_record_t rec;
  rec_field_t field;
  rec_rset_elem_t last_elem, new_elem;
  rec_record_elem_t rec_elem;
  char *errors;
  FILE *errors_stm;
  char *errors_str;
  size_t errors_str_size;

  if (!record || (rec_record_num_fields (record) == 0))
    {
      /* Do nothing.  */
      return true;
    }

  res = true;
  
  rset = rec_db_get_rset_by_type (db, type);
  if (rset)
    {
      new_elem = rec_rset_elem_record_new (rset, record);

      if (rec_rset_num_records (rset) == 0)
        {
          /* The rset is empty => prepend to it.  */
          rec_rset_insert_at (rset, new_elem, -1);
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

  if (!recins_force && rset)
    {
      errors_stm = open_memstream (&errors_str, &errors_str_size);
      if (rec_rset_check (rset, errors_stm) > 0)
        {
          fclose (errors_stm);
          if (!recins_verbose)
            {
              fprintf (stderr, "%s: operation aborted due to integrity failures\n",
                       program_name);
              fprintf (stderr, "%s: use --verbose to get a detailed report\n",
                       program_name);
            }
          else
            {
              fprintf (stderr, "%s", errors_str);
            }

          fprintf (stderr, "%s: use --force to skip the integrity check\n",
                   program_name);
          exit (1);
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

  field = NULL;

  while ((ret = getopt_long (argc,
                             argv,
                             "t:f:v:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
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
        case TYPE_ARG:
        case 't':
          {
            recins_type = strdup (optarg);
            break;
          }
        case NAME_ARG:
        case 'f':
          {
            if (field != NULL)
              {
                fprintf (stderr, "recins: error: a -f should be followed by a -v\n");
                exit (1);
              }

            if (recins_record == NULL)
              {
                recins_record = rec_record_new ();
                rec_record_set_source (recins_record, "cmdli");
                rec_record_set_location (recins_record, 0);
              }

            /* Make sure that the field name ends with a colon ':'.  */
            field_name_str = malloc (strlen (optarg) + 2);
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
                fprintf (stderr, "%s: error: invalid field name %s\n",
                         program_name, optarg);
                exit (1);
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
                fprintf (stderr, "%s: error: a -v should be preceded by a -f\n",
                         program_name);
                exit (1);
              }

            rec_field_set_value (field, optarg);
            rec_record_append_field (recins_record, field);

            field = NULL;
            break;
          }
        default:
          {
            exit (1);
          }
        }
    }

  if (field != NULL)
    {
      fprintf (stderr,
               "recins: error: you should specify a value for the field %s\n",
               field_name_str);
      exit (1);
    }

  /* Read the name of the file where to make the insertions.  */
  if (optind < argc)
    {

      if ((argc - optind) != 1)
        {
          fprintf (stdout, "%s\n", recutl_help_msg);
          exit (1);
        }

      recins_file = argv[optind++];
    }
}

int
main (int argc, char *argv[])
{
  rec_db_t db;


  program_name = strdup (argv[0]);

  recins_parse_args (argc, argv);

  db = recutl_read_db_from_file (recins_file);
  recins_insert_record (db, recins_type, recins_record);
  recutl_write_db_to_file (db, recins_file);

  return 0;
}

/* End of recins.c */
