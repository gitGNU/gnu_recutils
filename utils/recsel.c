/* -*- mode: C -*- Time-stamp: "10/01/12 23:20:36 jemarch"
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
    {"usage", no_argument, NULL, USAGE_ARG},
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

char *recsel_usage_msg = "\
Usage: recsel [OPTION]... [FILE]...\n\
Print the contents of the specified rec files.\n\
\n\
available options\n\
  -t TYPE, --type                     print records of the specified type only.\n\
  -i, --case-insensitive              make strings case-insensitive in selection\n\
                                        expressions.\n\
  -e EXPR, --expression               selection expression.\n\
  -n NUM, --number                    select an specific record.\n\
  -p FIELDS, --print                  comma-separated list of fields to print for each\n\
                                        matching record.\n\
  -c, --count                         provide a count of the matching records instead of\n\
                                        the records themselves.\n\
  -C, --collapse                      do not section the result in records with newlines.\n\
  --help                              print a help message and exit.\n\
  --usage                             print a usage message and exit.\n\
  --version                           show recsel version and exit.\n\
";

char *recsel_help_msg = "";

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

bool
recsel_file (FILE *in)
{
  bool ret;
  int rset_size;
  rec_rset_t rset;
  rec_record_t record;
  rec_record_t descriptor;
  rec_field_t type;
  int i, written;
  rec_parser_t parser;
  rec_writer_t writer;
  bool parse_status;

  ret = true;

  parser = rec_parser_new (in);
  writer = rec_writer_new (stdout);

  written = 0;
  while (rec_parse_rset (parser, &rset))
    {
      if (recsel_type != NULL)
        {
          rec_field_name_t fname;

          descriptor = rec_rset_descriptor (rset);
          fname = rec_parse_field_name_str ("%rec");
          type = rec_record_get_field_by_name (descriptor, fname, 0);
          if (strcmp (rec_field_value (type), recsel_type) != 0)
            {
              continue;
            }
        }

      rset_size = rec_rset_size (rset);
      for (i = 0; i < rset_size; i++)
        {
          record = rec_rset_get_record (rset, i);

          if (((recsel_num == -1) &&
               ((!recsel_sex) ||
                (rec_sex_eval (recsel_sex, record, &parse_status))))
              || (recsel_num == i))
            {
              char *resolver_result = NULL;

              if (recsel_expr)
                {
                  resolver_result = rec_resolve_str (record, recsel_expr);
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

          if (recsel_sex
              && (!parse_status))
            {
              return false;
            }
        }

    }

  if (rec_parser_error (parser))
    {
      rec_parser_perror (parser, "recsel:");
      exit(1);
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
  char c;
  char ret;
  char *file_name;
  FILE *in;

  program_name = strdup (argv[0]);

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
            fprintf (stdout, "%s\n", recsel_usage_msg);
            exit (0);
            break;
          }
        case VERSION_ARG:
          {
            fprintf (stdout, "%s\n", recsel_version_msg);
            exit (0);
            break;
          }
        case USAGE_ARG:
          {
            fprintf (stdout, "%s\n", recsel_usage_msg);
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
                return 1;
              }
            
            recsel_sex_str = strdup (optarg);
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
                return 1;
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
                return 1;
              }

            recsel_expr = strdup (optarg);

            if (!rec_resolver_check (recsel_expr))
              {
                fprintf (stderr, "Invalid field list.\n");
                return 1;
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
                return 1;
              }

            recsel_count = true;
            break;
          }
        default:
          {
            return 1;
          }
        }
    }

  /* Compile the search expression.  */
  if (recsel_sex_str)
    {
      recsel_sex = rec_sex_new (recsel_insensitive);
      if (!rec_sex_compile (recsel_sex, recsel_sex_str))
        {
          return 1;
        }
    }

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
              return 1;
            }
          else
            {
              if (!recsel_file (in))
                {
                  /* Parse error.  */
                  return 1;
                }
              
              fclose (in);
            }
        }
    }
  else
    {
      recsel_file (stdin);
    }

  return 0;
}

/* End of recsel.c */
