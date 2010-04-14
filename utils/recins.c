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

#include <recins.h>

/* Forward declarations.  */
bool recins_parse_db_from_file (FILE *in, char *file_name, rec_db_t db);
bool recins_insert_record (rec_db_t db, char *type, rec_record_t record);
void recins_parse_args (int argc, char **argv, char **type, rec_record_t *rec);

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
Usage: recins [OPTION]... [-f STR -v STR]... [FILE]\n\
Insert a record in a rec file.\n\
\n\
  -t, --type=TYPE                     specify the type of the new record.\n\
  -f, --field=STR                     field name.  Should be followed by a -v.\n\
  -v, --value=STR                     field value.  Should be preceded by a -f.\n\
      --help                          print a help message and exit.\n\
      --version                       show recins version and exit.\n\
\n\
If no FILE is specified then the command acts like a filter, getting\n\
the data from the standard input and writing the result in the\n\
standard output.\n\
\n\
Examples:\n\
\n\
        recins -f Name -v \"Mr Foo\" -f Email -v foo@foo.org contacts.rec\n\
        cat hackers.rec | recins -t Hacker -f Email -v foo@foo.org > other.rec\n\
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
                   rset_type, file_name ? file_name : "stdin");
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

bool
recins_insert_record (rec_db_t db,
                      char *type,
                      rec_record_t record)
{
  bool res;
  rec_rset_t rset;
  rec_record_t rec;
  rec_rset_elem_t last_elem, new_elem;

  if (rec_record_num_fields (record) == 0)
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
      /* Create a new type and insert the record there.  */
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
                        char **argv,
                        char **type,
                        rec_record_t *rec)
{
  int ret;
  char c;
  rec_field_t field;
  rec_field_name_t field_name;
  char *field_name_str;
  rec_record_t record = NULL;

  record = *rec;
  field = NULL;
  *type = NULL;

  while ((ret = getopt_long (argc,
                             argv,
                             "t:f:v:",
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
            *type = strdup (optarg);
            break;
          }
        case NAME_ARG:
        case 'f':
          {
            if (field != NULL)
              {
                fprintf (stderr, "recins: error: a -f should be followed by a -v.\n");
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
                fprintf (stderr, "recins: error: a -v should be preceded by a -f.\n");
                exit (1);
              }

            rec_field_set_value (field, optarg);
            rec_record_append_field (record, field);

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
}

int
main (int argc, char *argv[])
{
  char c;
  char *file_name = NULL;
  char *tmp_file_name = NULL;
  FILE *in;
  FILE *out;
  rec_db_t db;
  rec_writer_t writer;
  char *type;
  rec_record_t record;


  program_name = strdup (argv[0]);

  record = rec_record_new ();
  recins_parse_args (argc,
                     argv,
                     &type,
                     &record);

  db = rec_db_new ();

  /* Read the name of the file where to make the insertions.  */
  if (optind < argc)
    {

      if ((argc - optind) != 1)
        {
          fprintf (stdout, "%s\n", recins_help_msg);
          exit (1);
        }

      file_name = argv[optind++];
    }


  if (file_name)
    {
      in = fopen (file_name, "r");
      if (in == NULL)
        {
          fprintf (stderr, "recins: error: cannot read %s.\n", file_name);
          exit (1);
        }
    }
  else
    {
      /* Process the standard input.  */
      in = stdin;
    }

  if (!recins_parse_db_from_file (in,
                                  file_name,
                                  db))
    {
      exit (1);
    }

  if (!recins_insert_record (db,
                             type,
                             record))
    {
      exit (1);
    }
           
  /* Output.  */
             
  if (!file_name)
    {
      out = stdout;
    }
  else
    {
      int des;

      /* Create a temporary file with the results. */
      tmp_file_name = malloc (100);
      strcpy (tmp_file_name, "recXXXXXX");
      des = mkstemp (tmp_file_name);
      if (des == -1)
        {
          fprintf(stderr, "recins: error: cannot create a unique name.\n");
          exit (1);
        }
      out = fdopen (des, "w+");
    }

  writer = rec_writer_new (out);
  rec_write_db (writer, db);
  fclose (out);
  rec_db_destroy (db);

  if (file_name)
    {
      /* Rename the temporary file to file_name.  */
      if (rename (tmp_file_name, file_name) == -1)
        {
          fprintf (stderr, "recins: error: moving %s to %s\n",
                   tmp_file_name, file_name);
          remove (tmp_file_name);
          exit (1);
        }
    }

  return 0;
}

/* End of recins.c */
