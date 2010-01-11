/* -*- mode: C -*- Time-stamp: "10/01/11 13:30:13 jemarch"
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
char *recsel_sex = NULL;

/* Field names.  */
rec_field_name_t recsel_fields[256];
int recsel_num_fields = 0;

/* Record type.  */
char *recsel_type = NULL;

/* Whether to collapse the output.  */
bool recsel_collapse = false;

/* Whether to provide a count of the matching records.  */
bool recsel_count = false;

/* Whether to provide an specific record.  */
long recsel_num = -1;

bool
mount_recsel_fields (char *str)
{
  rec_field_name_t field_name;
  char *field_name_str;
  char *p, *c;
  bool parse_error;

  parse_error = false;

  p = str;
  c = str;
  while (*c != 0)
    {
      if ((*c == ',')
          || (*(c + 1) == 0))
        {
          if (*(c + 1) == 0)
            {
              c++;
            }

          /* Parse a field name from *p .. *(c - 1).  */
          field_name_str = malloc ((c - p) + 2);
          field_name_str[(c - p) + 1] = 0;
          field_name_str[(c - p)] = ':';
          strncpy (field_name_str, p, (c - p));

          if ((field_name = rec_parse_field_name_str (field_name_str)))
            {
              /* Add the field name to recsel_fields.  */
              recsel_fields[recsel_num_fields++] = field_name;
            }
          else
            {
              /* Error.  */
              parse_error = true;
            }

          free (field_name_str);
          p = c + 1;

          if (parse_error)
            {
              return false;
            }
        }

      c++;
    }

  return true;
}

void
write_fields (rec_writer_t writer,
              rec_record_t record)
{
  int i, j;
  rec_field_t field;
  bool found;

  /* Scan the fields.  */
  for (j = 0; j < recsel_num_fields; j++)
    {
      for (i = 0; i < rec_record_size (record); i++)
        {
          field = rec_record_get_field (record, i);
          
          found = false;
          if (rec_field_name_equal_p (recsel_fields[j],
                                      rec_field_name (field)))
            {
              found = true;
              break;
            }
        }
      
      if (found)
        {
          rec_write_field (writer, field);
        }
    }
}

bool
recsel_file (FILE *in)
{
  bool ret;
  rec_rset_t rset;
  rec_record_t record;
  rec_record_t descriptor;
  rec_field_t type;
  int i, written;
  rec_parser_t parser;
  rec_writer_t writer;
  rec_sex_t sex;
  bool parse_status;

  ret = true;

  sex = rec_sex_new ();
  parser = rec_parser_new (in);
  writer = rec_writer_new (stdout);

  written = 0;
  while (rec_parse_rset (parser, &rset))
    {
      if (recsel_type != NULL)
        {
          descriptor = rec_rset_descriptor (rset);
          type = rec_record_get_field_name (descriptor, "%rec");
          if (strcmp (rec_field_value (type), recsel_type) != 0)
            {
              continue;
            }
        }

      for (i = 0; i < rec_rset_size (rset); i++)
        {
          record = rec_rset_get_record (rset, i);

          if (((recsel_num == -1) &&
               ((!recsel_sex) ||
                (rec_sex_apply (sex, recsel_sex, record, &parse_status))))
              || (recsel_num == i))
            {
              if ((written != 0)
                  && (!recsel_collapse)
                  && (!recsel_count))
                {
                  fprintf (stdout, "\n");
                }

              if (!recsel_count)
                {
                  if (recsel_num_fields > 0)
                    {
                      write_fields (writer, record);
                    }
                  else
                    {
                      rec_write_record (writer, record);
                    }
                }

              written++;
            }

          if (!parse_status)
            {
              return false;
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
  char c;
  char ret;
  char *file_name;
  FILE *in;

  program_name = strdup (argv[0]);

  while ((ret = getopt_long (argc,
                             argv,
                             "Cct:e:n:p:",
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
                fprintf (stderr, "Cannot specify -e and also -n.\n");
                return 1;
              }

            recsel_sex = strdup (optarg);
            break;
          }
        case NUM_ARG:
        case 'n':
          {
            if (recsel_sex)
              {
                fprintf (stderr, "Cannot specify -n and also -e.\n");
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
                fprintf (stderr, "Cannot specify -p and also -c.\n");
                return 1;
              }

            if (!mount_recsel_fields (strdup (optarg)))
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
            if (recsel_num_fields > 0)
              {
                fprintf (stderr, "Cannot specify -c and also -p.\n");
                return 1;
              }

            recsel_count = true;
            break;
          }
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
              printf("error: cannot read file %s\n", file_name);
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
