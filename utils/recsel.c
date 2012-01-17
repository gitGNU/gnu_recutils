/* -*- mode: C -*-
 *
 *       File:         recsel.c
 *       Date:         Fri Jan  1 23:12:38 2010
 *
 *       GNU recutils - recsel
 *
 */

/* Copyright (C) 2010, 2011, 2012 Jose E. Marchesi */

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
#define _(str) gettext (str)
#include <getpass.h>

#include <rec.h>
#include <recutl.h>

/* Forward prototypes.  */
void recsel_parse_args (int argc, char **argv);
bool recsel_process_data (rec_db_t db);

/*
 * Global variables
 */

bool       recsel_print_values = false;
bool       recsel_print_row    = false;
char      *recutl_sex_str      = NULL;
rec_sex_t  recutl_sex          = NULL;
char      *recutl_quick_str    = NULL;
char      *recsel_fex_str      = NULL;
rec_fex_t  recsel_fex          = NULL;
char      *recutl_type         = NULL;
bool       recsel_collapse     = false;
bool       recsel_count        = false;
bool       recutl_insensitive  = false;
bool       recsel_descriptors  = false;
rec_field_name_t recutl_sort_by_field = NULL;
rec_writer_mode_t recsel_write_mode = REC_WRITER_NORMAL;
char      *recsel_password     = NULL;
bool       recsel_uniq         = false;
size_t     recutl_random       = 0;
rec_field_name_t recsel_group_by_field = NULL;

/*
 * Command line options management.
 */

enum
{
  COMMON_ARGS,
  RECORD_SELECTION_ARGS,
  PRINT_ARG,
  PRINT_VALUES_ARG,
  PRINT_IN_A_ROW_ARG,
  COLLAPSE_ARG,
  COUNT_ARG,
  DESCRIPTOR_ARG,
  PRINT_SEXPS_ARG,
  SORT_ARG,
#if defined REC_CRYPT_SUPPORT
  PASSWORD_ARG,
#endif
  UNIQ_ARG,
  GROUP_BY_ARG
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    RECORD_SELECTION_LONG_ARGS,
    {"print", required_argument, NULL, PRINT_ARG},
    {"print-values", required_argument, NULL, PRINT_VALUES_ARG},
    {"print-row", required_argument, NULL, PRINT_IN_A_ROW_ARG},
    {"collapse", no_argument, NULL, COLLAPSE_ARG},
    {"count", no_argument, NULL, COUNT_ARG},
    {"include-descriptors", no_argument, NULL, DESCRIPTOR_ARG},
    {"print-sexps", no_argument, NULL, PRINT_SEXPS_ARG},
    {"sort", required_argument, NULL, SORT_ARG},
#if defined REC_CRYPT_SUPPORT
    {"password", required_argument, NULL, PASSWORD_ARG},
#endif
    {"uniq", no_argument, NULL, UNIQ_ARG},
    {"group-by", required_argument, NULL, GROUP_BY_ARG},
    {NULL, 0, NULL, 0}
  };

/*
 * Functions.
 */

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, recsel synopsis.
     no-wrap */
  printf (_("\
Usage: recsel [OPTION]... [-t TYPE] [-n INDEXES | -e RECORD_EXPR | -q STR | -m NUM] [-c | (-p|-P) FIELD_EXPR] [FILE]...\n"));

  /* TRANSLATORS: --help output, recsel arguments.
     no-wrap */
  fputs(_("\
Select and print rec data.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, recsel arguments.
     no-wrap */
  fputs (_("\
  -d, --include-descriptors           print record descriptors along with the matched\n\
                                        records.\n\
  -C, --collapse                      do not section the result in records with newlines.\n\
  -S, --sort=FIELD                    sort the output by the specified field.\n\
  -G, --group-by=FIELD                group records by the specified field.\n\
  -U, --uniq                          remove duplicated fields in the output records.\n"),
         stdout);

#if defined REC_CRYPT_SUPPORT
  /* TRANSLATORS: --help output, encryption related options.
     no-wrap */
  fputs (_("\
  -s, --password=STR                  decrypt confidential fields with the given password.\n"),
         stdout);
#endif
  
  recutl_print_help_common ();

  puts ("");
  recutl_print_help_record_selection ();

  puts ("");
  /* TRANSLATORS: --help output, recsel output options.
     no-wrap */
  fputs (_("\
Output options:\n\
  -p, --print=FIELDS                  comma-separated list of fields to print for each\n\
                                        matching record.\n\
  -P, --print-values=FIELDS           as -p, but print only the values of the selected\n\
                                        fields.\n\
  -R, --print-row=FIELDS              as -P, but separate the values with spaces instead\n\
                                        of newlines.\n\
  -c, --count                         print a count of the matching records instead of\n\
                                        the records themselves.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, recsel special options.
     no-wrap */
  fputs (_("\
Special options:\n\
      --print-sexps                   print the data in sexps instead of rec format.\n"),
         stdout);

  puts ("");
  recutl_print_help_footer ();
}

void
recsel_parse_args (int argc,
                   char **argv)
{
  char c;
  int ret;

  while ((ret = getopt_long (argc,
                             argv,
                             RECORD_SELECTION_SHORT_ARGS
                             ENCRYPTION_SHORT_ARGS
                             "S:Cdcp:P:R:UG:",
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
        case PRINT_SEXPS_ARG:
          {
            recsel_write_mode = REC_WRITER_SEXP;
            break;
          }
        case UNIQ_ARG:
        case 'U':
          {
            recsel_uniq = true;
            break;
          }
#if defined REC_CRYPT_SUPPORT
        case PASSWORD_ARG:
        case 's':
          {
            if (recsel_password != NULL)
              {
                recutl_fatal (_("more than one password was specified\n"));
              }

            recsel_password = xstrdup (optarg);
            break;
          }
#endif
        case SORT_ARG:
        case 'S':
          {
            if (recutl_sort_by_field)
              {
                recutl_fatal (_("only one field can be specified as a sorting criteria.\n"));
              }

            /* Parse the field name.  */
            recutl_sort_by_field = rec_parse_field_name_str (optarg);
            if (!recutl_sort_by_field)
              {
                recutl_fatal (_("invalid field name in -S.\n"));
              }

            break;
          }
        case GROUP_BY_ARG:
        case 'G':
          {
            if (recsel_group_by_field)
              {
                recutl_fatal (_("only one field can be specified as a grouping criteria.\n"));
              }

            /* Parse the field name.  */

            recsel_group_by_field = rec_parse_field_name_str (optarg);
            if (!recsel_group_by_field)
              {
                recutl_fatal (_("invalid field name in -G.\n"));
              }

            break;
          }
        case PRINT_ARG:
        case 'p':
        case 'P':
        case 'R':
          {
            if (recsel_count)
              {
                recutl_fatal (_("cannot specify -[pPR] and also -c.\n"));
              }

            if (c == 'P')
              {
                recsel_print_values = true;
              }

            if (c == 'R')
              {
                recsel_print_values = true;
                recsel_print_row = true;
              }

            recsel_fex_str = xstrdup (optarg);

            if (!rec_fex_check (recsel_fex_str, REC_FEX_SUBSCRIPTS))
              {
                recutl_fatal (_("invalid list of fields in -%c\n"), c);
              }

            /* Create the field expresion.  */
            recsel_fex = rec_fex_new (recsel_fex_str,
                                      REC_FEX_SUBSCRIPTS);
            if (!recsel_fex)
              {
                recutl_fatal (_("internal error creating the field expression.\n"));
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
                recutl_fatal (_("cannot specify -c and also -p.\n"));
                exit (EXIT_FAILURE);
              }

            recsel_count = true;
            break;
          }
        default:
          {
            exit (EXIT_FAILURE);
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
  int n_rset, written, num_rec;
  rec_writer_t writer;
  bool parse_status;
  bool wrote_descriptor;
  rec_mset_iterator_t iter;

  ret = true;

  writer = rec_writer_new (stdout);

#if defined REC_CRYPT_SUPPORT

  /* Set the password in the writer.  If recsel was called
     interactively and with an empty -s, was not used then prompt the
     user for it.  Otherwise use the password specified in the command
     line if any.  */

  if (!recsel_password
      && (recutl_type || (rec_db_size (db) == 1))
      && recutl_interactive ())
    {
      rec_rset_t rset;
      rec_fex_t confidential_fields;

      if (recutl_type)
        {
          rset = rec_db_get_rset_by_type (db, recutl_type);
        }
      else
        {
          rset = rec_db_get_rset (db, 0);
        }

      confidential_fields = rec_rset_confidential (rset);
      if (rec_fex_size (confidential_fields) > 0)
        {
          recsel_password = getpass (_("Password: "));
        }

      rec_fex_destroy (confidential_fields);
    }

  /* Note that the password must be at least one character long.  */

  if (recsel_password && (strlen (recsel_password) > 0))
    {
      rec_writer_set_password (writer,
                               recsel_password);
    }

#endif /* REC_CRYPT_SUPPORT */

  /* If the database contains more than one type of records and the
     user did'nt specify the recutl_type then ask the user to clarify
     the request.  */
  if (!recutl_type && (rec_db_size (db) > 1))
    {
      recutl_fatal (_("several record types found.  Please use -t to specify one.\n"));
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
       * is of the given type.  */
      if (recutl_type
          && (!rec_rset_type (rset)
              || (strcmp (recutl_type, rec_rset_type (rset)) != 0)))
        {
          continue;
        }

      /* If the user didn't specify a type, print a record set if and
       * only if:
       *
       * - It is the default record set.
       * - The file contains just one record set.
       */

      if (!recutl_type
          && rec_rset_type (rset)
          && (rec_db_size (db) > 1))
        {
          continue;
        }
          
      /* Process this record set.  */

      /* If the user requested to print random records, calculate them
         now for this record set.  */

      if (recutl_random > 0)
        {
          recutl_reset_indexes ();
          recutl_index_add_random (recutl_random, rec_rset_num_records (rset));
        }

      if (recsel_group_by_field)
        {
          rec_rset_sort  (rset, recsel_group_by_field);
          rec_rset_group (rset, recsel_group_by_field);
        }

      rec_rset_sort (rset, recutl_sort_by_field);

      num_rec = -1;
      iter = rec_mset_iterator (rec_rset_mset (rset));
      while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void **) &record, NULL))
        {
          num_rec++;

          /* Shall we skip this record?  */

          if (recutl_quick_str)
            {
              if (!rec_record_contains_value (record,
                                              recutl_quick_str,
                                              recutl_insensitive))
                {
                  continue;
                }
            }
          else
            {
              if (((recutl_num_indexes() != 0) && (!recutl_index_p (num_rec)))
                  || (recutl_sex_str && !(rec_sex_eval (recutl_sex, record, &parse_status)
                                          && parse_status)))
                {
                  if (recutl_sex_str && (!parse_status))
                    {
                      recutl_error (_("evaluating the selection expression.\n"));
                      return false;
                    }
      
                  continue;
                }
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

              /* Remove duplicated fields in the record if requested
                 by the user.  */

              if (recsel_uniq)
                {
                  rec_record_uniq (record);
                }

              if (recsel_fex_str)
                {
                  output = recutl_eval_field_expression (recsel_fex,
                                                         record,
                                                         recsel_write_mode,
                                                         recsel_print_values,
                                                         recsel_print_row,
                                                         recsel_password);
                }

              /* Insert a newline?  */
              if ((written != 0)
                  && (!recsel_collapse)
                  && (!recsel_fex_str || output))
                {
                  fprintf (stdout, "\n");
                }

              /* Write the record descriptor if required.  */
              if (recsel_descriptors
                  && !wrote_descriptor
                  && rec_rset_descriptor (rset))
                {
                  rec_write_record (writer, rec_rset_descriptor (rset), recsel_write_mode);
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
                  rec_write_record_with_rset (writer,
                                              rset,
                                              record,
                                              recsel_write_mode);
                }

              written++;
            }
        }

      rec_mset_iterator_free (&iter);
    }

  if (recsel_count)
    {
      fprintf (stdout, "%d\n", written);
    }

  rec_writer_destroy (writer);

  return ret;
}

int
main (int argc, char *argv[])
{
  int res;
  rec_db_t db;

  res = 0;

  recutl_init ("recsel");

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

  rec_db_destroy (db);

  return res;
}

/* End of recsel.c */
