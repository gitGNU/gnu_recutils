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
#include <recutl.h>

/* Forward prototypes.  */
void recsel_parse_args (int argc, char **argv);
bool recsel_process_data (rec_db_t db);

/*
 * Global variables
 */

char *program_name; /* Initialized in main() */

/*
 * Command line options management
 */

enum
{
  COMMON_ARGS,
  RECORD_SELECTION_ARGS,
  PRINT_ARG,
  PRINT_VALUES_ARG,
  COLLAPSE_ARG,
  COUNT_ARG,
  DESCRIPTOR_ARG
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    RECORD_SELECTION_LONG_ARGS,
    {"print", required_argument, NULL, PRINT_ARG},
    {"print-values", required_argument, NULL, PRINT_VALUES_ARG},
    {"collapse", no_argument, NULL, COLLAPSE_ARG},
    {"count", no_argument, NULL, COUNT_ARG},
    {"include-descriptors", no_argument, NULL, DESCRIPTOR_ARG},
    {NULL, 0, NULL, 0}
  };

/* Messages */

RECUTL_COPYRIGHT_DOC ("recsel");

char *recutl_help_msg = "\
Usage: recsel [OPTION]... [-t TYPE] [-n NUM | -e RECORD_EXPR] [-c | (-p|-P) FIELD_EXPR] [FILE]...\n\
Select and print rec data.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -d, --include-descriptors           print record descriptors along with the matched\n\
                                        records.\n\
  -C, --collapse                      do not section the result in records with newlines.\n"
COMMON_ARGS_DOC
"\n"
RECORD_SELECTION_ARGS_DOC
"\n\
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
        recsel -t Friend -e \"Name ~ 'Smith'\" friends.rec\n\
        recsel -C -e \"#Email && Wiki = 'no'\" -P Email[0] gnupdf-hackers.rec\n\
\n"
  RECUTL_HELP_FOOTER_DOC ("recsel");

bool recsel_print_values = false;

/* String containing the selection expression.  */
char *recutl_sex_str = NULL;
rec_sex_t recutl_sex = NULL;

/* Field list.  */
char *recsel_fex_str = NULL;
rec_fex_t recsel_fex = NULL;

/* Record type.  */
char *recutl_type = NULL;

/* Whether to collapse the output.  */
bool recsel_collapse = false;

/* Whether to provide a count of the matching records.  */
bool recsel_count = false;

/* Whether to be case-insensitive while evaluating
   selection expressions.  */
bool recutl_insensitive = false;

/* Whether to include record descriptors in the selection results.  */
bool recsel_descriptors = false;

/* Whether to provide an specific record.  */
long recutl_num = -1;

void
recsel_parse_args (int argc,
                   char **argv)
{
  char c;
  char ret;

  while ((ret = getopt_long (argc,
                             argv,
                             RECORD_SELECTION_SHORT_ARGS
                             "Cdcp:P:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
        COMMON_ARGS_CASES
        RECORD_SELECTION_ARGS_CASES
        case DESCRIPTOR_ARG:
        case 'd':
          {
            recsel_descriptors = true;
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
     user did'nt specify the recutl_type then ask the user to clear
     the request.  */
  if (!recutl_type && (rec_db_size (db) > 1))
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
      if (recutl_type
          && (!rec_rset_type (rset)
              || (strcmp (recutl_type, rec_rset_type (rset)) != 0)))
        {
          continue;
        }

      /* If the user didn't specify a type, print a record set if and
       * only if:
       *
       * -  It is the default record set.
       * -  The file contains just one record set.
       */

      if (!recutl_type
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
          if (((recutl_num != -1) && (num_rec != num_rec))
              || (recutl_sex_str && !(rec_sex_eval (recutl_sex, record, &parse_status)
                                      && parse_status)))
            {
              if (recutl_sex_str && (!parse_status))
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
                  output = recutl_eval_field_expression (recsel_fex,
                                                         record,
                                                         recsel_print_values);
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
  db = recutl_build_db (argc, argv);
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
