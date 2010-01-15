/* -*- mode: C -*- Time-stamp: "10/01/15 17:31:12 jemarch"
 *
 *       File:         recsel.c
 *       Date:         Fri Jan  1 23:12:38 2010
 *
 *       GNU Rec - recsel
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

#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include <rec.h>

#include <recsel.h>

/* Forward prototypes.  */
void recsel_parse_args (int argc, char **argv);
rec_db_t recsel_build_db (int argc, char **argv);
bool recsel_process_data (rec_db_t db);
bool recsel_parse_db_from_file (FILE *in, char *file_name, rec_db_t db);

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
    {"expression", required_argument, NULL, EXPRESSION_ARG},
    {"print", required_argument, NULL, PRINT_ARG},
    {"type", required_argument, NULL, TYPE_ARG},
    {"collapse", no_argument, NULL, COLLAPSE_ARG},
    {"count", no_argument, NULL, COUNT_ARG},
    {"num", required_argument, NULL, NUM_ARG},
    {"case-insensitive", no_argument, NULL, INSENSITIVE_ARG},
    {NULL, 0, NULL, 0}
  };

/* Messages */

char *recsel_version_msg = "recsel (GNU recutils) 1.0\n\
Copyright (C) 2010 Jose E. Marchesi. \n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>. \n\
This is free software: you are free to change and redistribute it. \n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
Written by Jose E. Marchesi.";

char *recsel_help_msg = "\
Usage: recsel [OPTION]... [FILE]...\n\
Select and print rec data.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -t, --type=TYPE                     print records of the specified type only.\n\
  -i, --case-insensitive              make strings case-insensitive in selection\n\
                                        expressions.\n\
  -e, --expression=EXPR               selection expression.\n\
  -n, --number=NUM                    select an specific record.\n\
  -p, --print=FIELDS                  comma-separated list of fields to print for each\n\
                                        matching record.\n\
  -c, --count                         provide a count of the matching records instead of\n\
                                        the records themselves.\n\
  -C, --collapse                      do not section the result in records with newlines.\n\
      --help                          print a help message and exit.\n\
      --version                       show recsel version and exit.\n\
\n\
Examples:\n\
\n\
        recsel -e \"Name ~ 'Smith'\" friends.rec\n\
        recsel -C -e \"#Email && Wiki = 'no'\" -p /Email[0] gnupdf-hackers.rec\n\
\n\
Report recsel bugs to bug-recutils@gnu.org\n\
GNU recutils home page: <http://www.gnu.org/software/recutils/>\n\
General help using GNU software: <http://www.gnu.org/gethelp/>\
";

/* String containing the selection expression.  */
char *recsel_sex_str = NULL;
rec_sex_t recsel_sex = NULL;

/* Field list.  */
char *recsel_expr = NULL;

/* Record type.  */
char *recsel_type = NULL;

/* Whether to collapse the output.  */
bool recsel_collapse = false;

/* Whether to provide a count of the matching records.  */
bool recsel_count = false;

/* Whether to be case-insensitive while evaluating
   selection expressions.  */
bool recsel_insensitive = false;

/* Whether to provide an specific record.  */
long recsel_num = -1;

void
recsel_parse_args (int argc,
                   char **argv)
{
  char c;
  char ret;

  while ((ret = getopt_long (argc,
                             argv,
                             "Cict:e:n:p:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          /* COMMON ARGUMENTS */
        case HELP_ARG:
          {
            fprintf (stdout, "%s\n", recsel_help_msg);
            exit (0);
            break;
          }
        case VERSION_ARG:
          {
            fprintf (stdout, "%s\n", recsel_version_msg);
            exit (0);
            break;
          }
        case EXPRESSION_ARG:
        case 'e':
          {
            if (recsel_num != -1)
              {
                fprintf (stderr, "%s: cannot specify -e and also -n.\n",
                         argv[0]);
                exit (1);
              }
            
            recsel_sex_str = strdup (optarg);

            /* Compile the search expression.  */
            if (recsel_sex_str)
              {
                recsel_sex = rec_sex_new (recsel_insensitive);
                if (!rec_sex_compile (recsel_sex, recsel_sex_str))
                  {
                    fprintf (stderr, "recsel: error: invalid selection expression.\n");
                    exit (1);
                  }
              }
            
            break;
          }
        case INSENSITIVE_ARG:
        case 'i':
          {
            recsel_insensitive = true;
            break;
          }
        case NUM_ARG:
        case 'n':
          {
            if (recsel_sex)
              {
                fprintf (stderr, "%s: cannot specify -n and also -e.\n",
                         argv[0]);
                exit (1);
              }

            /* XXX: check for conversion errors.  */
            recsel_num = atoi (optarg);
            break;
          }
        case PRINT_ARG:
        case 'p':
          {
            if (recsel_count)
              {
                fprintf (stderr, "%s: cannot specify -p and also -c.\n",
                         argv[0]);
                exit (1);
              }

            recsel_expr = strdup (optarg);

            if (!rec_resolver_check (recsel_expr))
              {
                fprintf (stderr, "Invalid field list.\n");
                exit (1);
              }

            break;
          }
        case TYPE_ARG:
        case 't':
          {
            recsel_type = strdup (optarg);
            break;
          }
        case COLLAPSE_ARG:
        case 'C':
          {
            recsel_collapse = true;
            break;
          }
        case COUNT_ARG:
        case 'c':
          {
            if (recsel_expr)
              {
                fprintf (stderr, "%s: cannot specify -c and also -p.\n",
                         argv[0]);
                exit (1);
              }

            recsel_count = true;
            break;
          }
        default:
          {
            exit (1);
          }

        }
    }
}

bool
recsel_parse_db_from_file (FILE *in,
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
      /* XXX: check for consistency!!!.  */
      rset_type = rec_rset_type (rset);
      if (rec_db_type_p (db, rset_type))
        {
          fprintf (stderr, "recsel: error: duplicated record set '%s' from %s.\n",
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
  
  return res;
}

rec_db_t
recsel_build_db (int argc,
                 char **argv)
{
  rec_db_t db;
  char *file_name;
  FILE *in;

  db = rec_db_new ();

  /* Process the input files, if any.  Otherwise use the standard
     input to read the rec data.  */
  if (optind < argc)
    {
      while (optind < argc)
        {
          file_name = argv[optind++];
          if (!(in = fopen (file_name, "r")))
            {
              printf("%s: cannot read file %s\n", argv[0], file_name);
              exit (1);
            }
          else
            {
              if (!recsel_parse_db_from_file (in, file_name, db))
                {
                  free (db);
                  db = NULL;
                }
              
              fclose (in);
            }
        }
    }
  else
    {
      if (!recsel_parse_db_from_file (stdin, "stdin", db))
        {
          free (db);
          db = NULL;
        }
    }

  return db;
}

bool
recsel_process_data (rec_db_t db)
{
  bool ret;
  int rset_size;
  rec_rset_t rset;
  rec_record_t record;
  rec_record_t descriptor;
  rec_field_t type;
  int n_rset, i, written, num_rec;
  rec_writer_t writer;
  bool parse_status;

  ret = true;

  writer = rec_writer_new (stdout);

  written = 0;
  for (n_rset = 0; n_rset < rec_db_size (db); n_rset++)
    {
      rset = rec_db_get_rset (db, n_rset);
      rset_size = rec_rset_size (rset);

      if ((rset_size > 0)
          && ((recsel_type && rec_rset_type (rset) && (strcmp (recsel_type, rec_rset_type (rset)) == 0)
               || (!recsel_type && !rec_rset_type (rset)))))
        {
          /*  Print out the records of this rset, if appropriate.  */
          num_rec = 0;
          rset_size = rec_rset_size (rset);

          for (i = 0; i < rset_size; i++)
            {
              record = rec_rset_get_record (rset, i);
              if (!rec_record_p (record))
                {
                  continue;
                }
          
              if (((recsel_num == -1) &&
                   ((!recsel_sex_str) ||
                    (rec_sex_eval (recsel_sex, record, &parse_status))))
                  || (recsel_num == num_rec))
                {
                  char *resolver_result = NULL;
              
                  if (recsel_expr)
                    {
                      resolver_result = rec_resolve_str (db,
                                                         rec_rset_type (rset),
                                                         record,
                                                         recsel_expr);
                    }
              
                  if ((written != 0)
                      && (!recsel_collapse)
                      && (!recsel_count)
                      && (!resolver_result || strcmp(resolver_result, "") != 0))
                    {
                      fprintf (stdout, "\n");
                    }

                  if (!recsel_count)
                    {
                      if (recsel_expr)
                        {
                          if (strcmp (resolver_result, "") != 0)
                            {
                              fprintf (stdout, "%s", resolver_result);
                              written++;
                            }
                        }
                      else
                        {
                          rec_write_record (writer, record);
                        }
                    }

                  if (!recsel_expr)
                    {
                      written++;
                    }
                }

              if (recsel_sex_str
                  && (!parse_status))
                {
                  fprintf (stderr, "recsel: error: evaluating the selection expression.\n");
                  return false;
                }

              num_rec++;
            }
        }
    }

  if (recsel_count)
    {
      printf ("%d\n", written);
    }

  return ret;
}

int
main (int argc, char *argv[])
{
  int res;
  rec_db_t db;

  res = 0;
  program_name = strdup (argv[0]);

  /* Parse arguments.  */
  recsel_parse_args (argc, argv);

  /* Get the input data.  */
  db = recsel_build_db (argc, argv);
  if (!db)
    {
      res = 1;
    }

  /* Process the data.  */
  if (!recsel_process_data (db))
    {
      res = 1;
    }

  return res;
}

/* End of recsel.c */
