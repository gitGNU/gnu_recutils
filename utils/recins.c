/* -*- mode: C -*- Time-stamp: "10/01/15 12:04:20 jemarch"
 *
 *       File:         recins.c
 *       Date:         Mon Dec 28 08:54:38 2009
 *
 *       GNU Record Utilities - recins
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

#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include <rec.h>

#include <recins.h>

/*
 * Global variables
 */

char *program_name; /* Initialized in main() */

/*
 * Command line options management
 */

static const struct option GNU_longOptions[] =
  {
    {"help", no_argument, NULL, HELP_ARG},
    {"version", no_argument, NULL, VERSION_ARG},
    {"type", required_argument, NULL, TYPE_ARG},
    {"name", required_argument, NULL, NAME_ARG},
    {"value", required_argument, NULL, VALUE_ARG},
    {NULL, 0, NULL, 0}
  };

/* Messages */

char *recins_version_msg = "recins (GNU recutils) 1.0\n\
Copyright (C) 2010 Jose E. Marchesi. \n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>. \n\
This is free software: you are free to change and redistribute it. \n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
Written by Jose E. Marchesi.";

char *recins_help_msg = "\
Usage: recins [OPTION]... [FILE]\n\
Create rec data.\n\
\n\
  -v, --verbose                       include the full record descriptors.\n\
      --help                          print a help message and exit.\n\
      --usage                         print a usage message and exit.\n\
      --version                       show recins version and exit.\n\
\n\
Examples:\n\
\n\
        recins -f Name -v \"Mr Foo\" -f Email -v foo@foo.org contacts.rec\n\
\n\
Report recins bugs to bug-recutils@gnu.org\n\
GNU recutils home page: <http://www.gnu.org/software/recutils/>\n\
General help using GNU software: <http://www.gnu.org/gethelp/>\
";

bool
recins_parse_db_from_file (FILE *in,
                           char *file_name,
                           rec_db_t db)
{
  bool res;
  rec_rset_t rset;
  rec_parser_t parser;

  res = true;
  parser = rec_parser_new (in);

  while (rec_parse_rset (parser, &rset))
    {
      char *rset_type;
      rset_type = rec_rset_type (rset);
      if (rec_db_type_p (db, rset_type))
        {
          fprintf (stderr, "recins: error: duplicated record set '%s' from %s.\n",
                   rset_type, file_name);
          exit (1);
        }

      if (!rec_db_insert_rset (db, rset, rec_db_size (db)))
        {
          /* Error.  */
          res = false;
          break;
        }
    }

  if (rec_parser_error (parser))
    {
      res = false;
      rec_parser_perror (parser, "stdin");
    }
  
  return res;
}

int
main (int argc, char *argv[])
{
  char c;
  char ret;
  char *file_name;
  FILE *in;
  rec_db_t db;
  rec_writer_t writer;
  char *type;
  rec_record_t record;
  rec_field_name_t field_name;
  char *field_name_str;
  rec_field_t field;

  program_name = strdup (argv[0]);
  

  record = rec_record_new ();
  type = NULL;
  field = NULL;

  while ((ret = getopt_long (argc,
                             argv,
                             "t:n:v:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          /* COMMON ARGUMENTS */
        case HELP_ARG:
          {
            fprintf (stdout, "%s\n", recins_help_msg);
            exit (0);
            break;
          }
        case VERSION_ARG:
          {
            fprintf (stdout, "%s\n", recins_version_msg);
            exit (0);
            break;
          }
        case TYPE_ARG:
        case 't':
          {
            type = strdup (optarg);
            break;
          }
        case NAME_ARG:
        case 'n':
          {
            if (field != NULL)
              {
                fprintf (stderr, "recins: error: a -n should be followed by a -v.\n");
                exit (1);
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
                fprintf (stderr, "recins: error: invalid field name %s\n", optarg);
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
                fprintf (stderr, "recins: error: a -v should be preceded by a -n.\n");
                exit (1);
              }

            rec_field_set_value (field, optarg);
            rec_record_insert_field (record,
                                     field,
                                     rec_record_size (record));

            field = NULL;
            break;
          }
        default:
          {
            return 1;
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

  db = rec_db_new ();
  if (!recins_parse_db_from_file (stdin,
                                  "stdin",
                                  db))
    {
      exit (1);
    }

  /* Insert the record in the specified tpe, if it is not empty.  */
  if (rec_record_size (record) > 0)
    {
      rec_rset_t rset;

      rset = rec_db_get_rset_by_type (db, type);
      if (rset)
        {
          int i;

          for (i = (rec_rset_size (rset) - 1); i >= 0; i--)
            {
              rec_record_t rec;
              rec = rec_rset_get_record (rset, i);
              if (rec_record_p (rec))
                {
                  /* Insert the new record just after rec.  */
                  if (!rec_rset_insert_record (rset,
                                               record,
                                               i + 1))
                  {
                    fprintf (stderr, "recins: error: inserting the new record.\n");
                    exit (1);
                  }     

                  break;
                }
            }

          if (i == -1)
            {
              /* The rset was empty => prepend to it.  */
              if (!rec_rset_insert_record (rset,
                                           record,
                                           -1))
                {
                  fprintf (stderr, "recins: error: inserting the new record.\n");
                  exit (1);
                }
            }
        }
      else
        {
          /* Create a new type and insert the record there.  */
          rset = rec_rset_new ();
          rec_rset_set_type (rset, type);
          rec_rset_insert_record (rset, record, rec_rset_size (rset));
           
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
    } 

  writer = rec_writer_new (stdout);
  rec_write_db (writer, db);
  rec_db_destroy (db);

  return 0;
}

/* End of recins.c */
