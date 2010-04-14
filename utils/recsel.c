/* -*- mode: C -*-
 *
 *       File:         recsel.c
 *       Date:         Fri Jan  1 23:12:38 2010
 *
 *       GNU recutils - recsel
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
    {"print-values", required_argument, NULL, PRINT_VALUES_ARG},
    {"type", required_argument, NULL, TYPE_ARG},
    {"collapse", no_argument, NULL, COLLAPSE_ARG},
    {"count", no_argument, NULL, COUNT_ARG},
    {"num", required_argument, NULL, NUM_ARG},
    {"case-insensitive", no_argument, NULL, INSENSITIVE_ARG},
    {"include-descriptors", no_argument, NULL, DESCRIPTOR_ARG},
    {NULL, 0, NULL, 0}
  };

/* Messages */

char *recsel_version_msg = "recsel (GNU recutils) 1.0\n\
Copyright (C) 2010 Jose E. Marchesi.\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
Written by Jose E. Marchesi.";

char *recsel_help_msg = "\
Usage: recsel [OPTION]... [-t TYPE] [-n NUM | -e RECORD_EXPR] [-c | (-p|-P) FIELD_EXPR] [FILE]...\n\
Select and print rec data.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -i, --case-insensitive              make strings case-insensitive in selection\n\
                                        expressions.\n\
  -d, --include-descriptors           print record descriptors along with the matched\n\
                                        records.\n\
  -C, --collapse                      do not section the result in records with newlines.\n\
      --help                          print a help message and exit.\n\
      --version                       show recsel version and exit.\n\
\n\
Record selection options:\n\
  -t, --type=TYPE                     print records of the specified type only.\n\
  -e, --expression=EXPR               selection expression.\n\
  -n, --number=NUM                    select an specific record.\n\
\n\
Output options:\n\
  -p, --print=FIELDS                  comma-separated list of fields to print for each\n\
                                        matching record.\n\
  -P, --print-values=FIELDS           same than -p, but print the values of the selected\n\
                                        fields.\n\
  -c, --count                         provide a count of the matching records instead of\n\
                                        the records themselves.\n\
\n\
Examples:\n\
\n\
        recsel -e \"Name ~ 'Smith'\" friends.rec\n\
        recsel -C -e \"#Email && Wiki = 'no'\" -P Email[0] gnupdf-hackers.rec\n\
\n\
Report recsel bugs to bug-recutils@gnu.org\n\
GNU recutils home page: <http://www.gnu.org/software/recutils/>\n\
General help using GNU software: <http://www.gnu.org/gethelp/>\
";

bool recsel_print_values = false;

/* String containing the selection expression.  */
char *recsel_sex_str = NULL;
rec_sex_t recsel_sex = NULL;

/* Field list.  */
char *recsel_fex_str = NULL;
rec_fex_t recsel_fex = NULL;

/* Record type.  */
char *recsel_type = NULL;

/* Whether to collapse the output.  */
bool recsel_collapse = false;

/* Whether to provide a count of the matching records.  */
bool recsel_count = false;

/* Whether to be case-insensitive while evaluating
   selection expressions.  */
bool recsel_insensitive = false;

/* Whether to include record descriptors in the selection results.  */
bool recsel_descriptors = false;

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
                             "Cdict:e:n:p:P:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
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
        case DESCRIPTOR_ARG:
        case 'd':
          {
            recsel_descriptors = true;
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
        case 'P':
          {
            if (recsel_count)
              {
                fprintf (stderr, "%s: cannot specify -[pP] and also -c.\n",
                         argv[0]);
                exit (1);
              }

            if (c == 'P')
              {
                recsel_print_values = true;
              }

            recsel_fex_str = strdup (optarg);

            if (!rec_fex_check (recsel_fex_str))
              {
                exit (1);
              }

            /* Create the field expresion.  */
            recsel_fex = rec_fex_new (recsel_fex_str);
            if (!recsel_fex)
              {
                fprintf (stderr, "internal error: creating the field expression.\n");
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
            if (recsel_fex_str)
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

char *
recsel_eval_field_expression (rec_fex_t fex,
                              rec_record_t record)
{
  char *res;
  size_t res_size;
  FILE *stm;
  rec_writer_t writer;
  rec_fex_elem_t elem;
  rec_field_t field;
  rec_field_name_t field_name;
  int i, j, min, max;

  stm = open_memstream (&res, &res_size);

  for (i = 0; i < rec_fex_size (fex); i++)
    {
      elem = rec_fex_get (fex, i);
      
      field_name = rec_fex_elem_field_name (elem);
      min = rec_fex_elem_min (elem);
      max = rec_fex_elem_max (elem);

      if ((min == -1) && (max == -1))
        {
          /* Print all the fields with that name.  */
          min = 0;
          max = rec_record_get_num_fields_by_name (record, field_name);
        }
      else if (max == -1)
        {
          /* Print just one field: Field[min].  */
          max = min + 1;
        }
      else
        {
          /* Print the interval min..max, max inclusive.  */
          max++;
        }

      for (j = min; j < max; j++)
        {
          if (!(field = rec_record_get_field_by_name (record, field_name, j)))
            {
              continue;
            }

          if (recsel_print_values)
            {
              /* Write just the value of the field.  */
              fprintf (stm, rec_field_value (field));
              fprintf (stm, "\n");
            }
          else
            {
              /* Write the whole field.  */
              writer = rec_writer_new (stm);
              rec_write_field (writer, field);
              rec_writer_destroy (writer);
            }
        }
      
    }

  fclose (stm);

  if (res_size == 0)
    {
      free (res);
      res = NULL;
    }

  return res;
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
  int n_rset, written, num_rec;
  rec_writer_t writer;
  bool parse_status;
  bool wrote_descriptor;
  rec_rset_elem_t elem_rset;
  rec_fex_t fex;

  ret = true;

  writer = rec_writer_new (stdout);

  /* If the database contains more than one type of records and the
     user did'nt specify the recsel_type then ask the user to clear
     the request.  */
  if (!recsel_type && (rec_db_size (db) > 1))
    {
      fprintf (stderr, "Several record types found.  Please use -t to specify one.\n");
      exit (1);
    }

  written = 0;
  for (n_rset = 0; n_rset < rec_db_size (db); n_rset++)
    {
      rset = rec_db_get_rset (db, n_rset);
      rset_size = rec_rset_num_records (rset);

      wrote_descriptor = false;

      /* Don't process empty record sets.  */
      if (rset_size == 0)
        {
          continue;
        }

      /* If the user specified a type, print the record set only if it
       * is of the given size.  */
      if (recsel_type
          && (!rec_rset_type (rset)
              || (strcmp (recsel_type, rec_rset_type (rset)) != 0)))
        {
          continue;
        }

      /* If the user didn't specify a type, print a record set if and
       * only if:
       *
       * -  It is the default record set.
       * -  The file contains just one record set.
       */

      if (!recsel_type
          && rec_rset_type (rset)
          && (rec_db_size (db) > 1))
        {
          continue;
        }
          
      /*  Process this record set.  */
      num_rec = -1;
      elem_rset = rec_rset_null_elem ();
      while (rec_rset_elem_p (elem_rset = rec_rset_next_record (rset, elem_rset)))
        {
          record = rec_rset_elem_record (elem_rset);
          num_rec++;

          /* Shall we skip this record?  */
          if (((recsel_num != -1) && (num_rec != num_rec))
              || (recsel_sex_str && !(rec_sex_eval (recsel_sex, record, &parse_status)
                                      && parse_status)))
            {
              if (recsel_sex_str && (!parse_status))
                {
                  fprintf (stderr, "recsel: error: evaluating the selection expression.\n");
                  return false;
                }
      
              continue;
            }

          /* Process this record.  */
          if (recsel_count)
            {
              /* We just count this record and continue.  */
              written++;
            }
          else
            {
              char *output = NULL;

              if (recsel_fex_str)
                {
                  output = recsel_eval_field_expression (recsel_fex, record);
                }

              /* Insert a newline?  */
              if ((written != 0)
                  && (!recsel_collapse)
                  && (!recsel_fex_str || output))
                {
                  fprintf (stdout, "\n");
                }

              /* Write the record descriptor if required.  */
              if (recsel_descriptors && !wrote_descriptor)
                {
                  rec_write_record (writer, rec_rset_descriptor (rset));
                  fprintf (stdout, "\n");
                  wrote_descriptor = true;
                }

              if (recsel_fex_str)
                {
                  /* Print the field expression.  */
                  if (output)
                    {
                      fprintf (stdout, "%s", output);
                    }
                }
              else
                {
                  rec_write_record (writer, record);
                }

              written++;
            }
        }
    }

  if (recsel_count)
    {
      fprintf (stdout, "%d\n", written);
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
