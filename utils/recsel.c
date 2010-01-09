/* -*- mode: C -*- Time-stamp: "10/01/09 22:43:17 jemarch"
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
  --help                              print a help message and exit.\n\
  --usage                             print a usage message and exit.\n\
  --version                           show recsel version and exit.\n\
";

char *recsel_help_msg = "";

/* String containing the selection expression.  */
char *recsel_sex = NULL;

bool
recsel_file (FILE *in)
{
  bool ret;
  rec_rset_t rset;
  rec_record_t record;
  int i, written;
  rec_parser_t parser;
  rec_writer_t writer;
  rec_sex_t sex;

  ret = true;

  sex = rec_sex_new ();
  parser = rec_parser_new (in);
  writer = rec_writer_new (stdout);

  written = 0;
  while (rec_parse_rset (parser, &rset))
    {
      for (i = 0; i < rec_rset_size (rset); i++)
        {
          record = rec_rset_get_record (rset, i);

          if ((!recsel_sex) ||
              (rec_sex_apply (sex, recsel_sex, record)))
            {
              if (written != 0)
                {
                  fprintf (stdout, "\n");
                }
              rec_write_record (writer, record);
              written++;
            }
        }

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
                             "e:",
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
            recsel_sex = strdup (optarg);
          }
        }
    }

  /* optarg9 the input files, if any.  Otherwise use the standard
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
