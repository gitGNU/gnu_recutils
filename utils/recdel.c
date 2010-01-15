/* -*- mode: C -*- Time-stamp: "10/01/15 18:12:59 jemarch"
 *
 *       File:         recdel.c
 *       Date:         Mon Dec 28 08:54:38 2009
 *
 *       GNU Record Utilities - recdel
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

#include <recdel.h>

/* Forward declarations.  */
bool recdel_parse_db_from_file (FILE *in, char *file_name, rec_db_t db);
void recdel_delete_records (rec_db_t db);
void recdel_parse_args (int argc, char **argv);

/*
 * Global variables
 */

char *program_name; /* Initialized in main() */
char *recdel_type = NULL;
bool recdel_comment = false;
rec_sex_t recdel_sex = NULL;
int recdel_index = -1;
bool recdel_case_insensitive = false;

/*
 * Command line options management
 */

static const struct option GNU_longOptions[] =
  {
    {"help", no_argument, NULL, HELP_ARG},
    {"version", no_argument, NULL, VERSION_ARG},
    {"type", required_argument, NULL, TYPE_ARG},
    {"number", required_argument, NULL, NUMBER_ARG},
    {"expression", required_argument, NULL, EXPRESSION_ARG},
    {"case-insensitive", no_argument, NULL, CASE_INSENSITIVE_ARG},
    {"comment", no_argument, NULL, COMMENT_ARG},
    {NULL, 0, NULL, 0}
  };

/* Messages */

char *recdel_version_msg = "recdel (GNU recutils) 1.0\n\
Copyright (C) 2010 Jose E. Marchesi. \n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>. \n\
This is free software: you are free to change and redistribute it. \n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
Written by Jose E. Marchesi.";

char *recdel_help_msg = "\
Usage: recdel [OPTIONS]... [-t TYPE] [-n NUM | -e EXPR] [FILE]\n\
Remove records from a rec file.\n\
\n\
  -t, --type=TYPE                     specify the type of the new record.\n\
  -n, --number                        select the NUMBERth record.\n\
  -e, --expression=EXPR               select records matching this expression.\n\
  -c, --comment                       comment the matching records instead of\n\
                                         delete them.\n\
  -i, --case-insensitive              make the selection expression operators\n\
                                         case-insensitive.\n\
      --help                          print a help message and exit.\n\
      --usage                         print a usage message and exit.\n\
      --version                       show recdel version and exit.\n\
\n\
If no FILE is specified then the command acts like a filter, getting\n\
the data from the standard input and writing the result in the\n\
standard output.\n\
\n\
Examples:\n\
\n\
        recdel -n 10 contacts.rec\n\
        cat hackers.rec | recdel -e \"Email[0] = 'foo@bar.com'\" > other.rec\n\
\n\
Report recdel bugs to bug-recutils@gnu.org\n\
GNU recutils home page: <http://www.gnu.org/software/recutils/>\n\
General help using GNU software: <http://www.gnu.org/gethelp/>\
";

bool
recdel_parse_db_from_file (FILE *in,
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
          fprintf (stderr, "recdel: error: duplicated record set '%s' from %s.\n",
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

void
recdel_delete_records (rec_db_t db)
{
  int n_rset, n_rec, i, j;
  int numrec;
  int deleted;
  int rset_size;
  rec_rset_t rset;
  rec_record_t record;
  rec_record_t crec;
  rec_field_t cfield;
  bool *delmask;
  bool parse_status = true;
  rec_record_t drec;
  char *comment_field_str;
  char *comment_str;

  if (!rec_db_type_p (db, recdel_type))
    {
      fprintf (stderr, "recdel: error: no records of type %s found.\n",
               recdel_type ? recdel_type : "default");
      exit (1);
    }

  for (n_rset = 0; n_rset < rec_db_size (db); n_rset++)
    {
      rset = rec_db_get_rset (db, n_rset);
      rset_size = rec_rset_size (rset);
      delmask = malloc (sizeof(bool) * rset_size);

      if ((rset_size > 0)
          && (recdel_type && rec_rset_type (rset) && (strcmp (recdel_type, rec_rset_type (rset)) == 0)
              || (!recdel_type && !rec_rset_type (rset))))
        {
          /* Initialize the deletion mask.  */
          for (i = 0; i < rset_size; i++) delmask[i] = false;

          /* Fill in the mask.  */
          numrec = 0;
          for (n_rec = 0; n_rec < rset_size; n_rec++)
            {
              record = rec_rset_get_record (rset, n_rec);
              if (!rec_record_p (record))
                {
                  continue;
                }

              if ((recdel_index == -1) &&
                  ((recdel_sex &&
                    (rec_sex_eval (recdel_sex, record, &parse_status))))
                  || (recdel_index == numrec))
                {
                  delmask[n_rec] = true;
                }

              if (!parse_status)
                {
                  fprintf (stderr, "recdel: error: evaluating selection expression.\n");
                  exit (1);
                }

              numrec++;
            }

          /* Delete the masked record.  */
          deleted = 0;
          for (i = 0; i < rset_size; i++)
            {
              if (delmask[i])
                {
                  drec = rec_record_dup (rec_rset_get_record (rset, i - deleted));
                  rec_rset_remove_record (rset, i - deleted);

                  if (recdel_comment)
                    {
                      for (j = 0; j < rec_record_size (drec); j++)
                        {
                          /* Insert comments.  */
                          crec = rec_record_new ();
                          cfield = rec_record_get_field (drec, j);
                          if (rec_field_comment_p (cfield))
                            {
                              rec_record_set_comment (crec,
                                                      rec_field_value (cfield));
                            }
                          else
                            {
                              comment_field_str = rec_write_field_str (cfield);
                              comment_str = malloc (strlen (comment_field_str) + 2);
                              comment_str[0] = ' ';
                              strncpy (comment_str + 1, comment_field_str, strlen (comment_field_str));
                              comment_str[strlen (comment_field_str)] = 0;
                              rec_record_set_comment (crec, comment_str);
                              free (comment_str);
                            }

                          rec_rset_insert_record (rset, crec, i - deleted);
                          deleted--;
                        }
                    }

                  rec_record_destroy (drec);
                  deleted++;
                }
            }
        }
    }

}

void
recdel_parse_args (int argc,
                   char **argv)
{
  int ret;
  char c;
  char *sex_str = NULL;

  while ((ret = getopt_long (argc,
                             argv,
                             "in:ct:e:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          /* COMMON ARGUMENTS */
        case HELP_ARG:
          {
            fprintf (stdout, "%s\n", recdel_help_msg);
            exit (0);
            break;
          }
        case VERSION_ARG:
          {
            fprintf (stdout, "%s\n", recdel_version_msg);
            exit (0);
            break;
          }
        case TYPE_ARG:
        case 't':
          {
            recdel_type = strdup (optarg);
            break;
          }
        case CASE_INSENSITIVE_ARG:
        case 'i':
          {
            recdel_case_insensitive = true;
            break;
          }
        case COMMENT_ARG:
        case 'c':
          {
            recdel_comment = true;
            break;
          }
        case NUMBER_ARG:
        case 'n':
          {
            if (sex_str)
              {
                fprintf (stderr, "recdel: error: you cannot use both -e and -n.\n");
                exit (1);
              }

            /* XXX: check atoi.  */
            recdel_index = atoi (optarg);
            break;
          }
        case EXPRESSION_ARG:
        case 'e':
          {
            /* Get sex, but incompatible with -n. */
            if (recdel_index != -1)
              {
                fprintf (stderr, "recdel: error: you cannot use both -n and -e.\n");
                exit (1);
              }

            sex_str = strdup (optarg);
            break;
          }
        default:
          {
            exit (1);
          }
        }
    }

  if (sex_str)
    {
      recdel_sex = rec_sex_new (recdel_case_insensitive);
      if (!rec_sex_compile (recdel_sex, sex_str))
        {
          fprintf (stderr, "recdel: error: invalid selection expression.\n");
          exit (1);
        }
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
  recdel_parse_args (argc, argv);

  db = rec_db_new ();

  /* Read the name of the file where to delete the records.  */
  if (optind < argc)
    {
      if ((argc - optind) != 1)
        {
          fprintf (stdout, "%s\n", recdel_help_msg);
          exit (1);
        }

      file_name = argv[optind++];
    }


  if (file_name)
    {
      in = fopen (file_name, "r");
      if (in == NULL)
        {
          fprintf (stderr, "recdel: error: cannot read %s.\n", file_name);
          exit (1);
        }
    }
  else
    {
      /* Process the standard input.  */
      in = stdin;
    }

  if (!recdel_parse_db_from_file (in,
                                  file_name,
                                  db))
    {
      exit (1);
    }
  
  if ((recdel_index != -1)
      || recdel_sex)
    {
      recdel_delete_records (db);
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
          fprintf(stderr, "recdel: error: cannot create a unique name.\n");
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
          fprintf (stderr, "recdel: error: moving %s to %s\n",
                   tmp_file_name, file_name);
          remove (tmp_file_name);
          exit (1);
        }
    }

  return 0;
}

/* End of recdel.c */
