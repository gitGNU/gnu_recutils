/* -*- mode: C -*- Time-stamp: "10/01/11 20:15:55 jemarch"
 *
 *       File:         recinf.c
 *       Date:         Mon Dec 28 08:54:38 2009
 *
 *       GNU Rec - recinf
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

#include <recinf.h>

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
    {"verbose", no_argument, NULL, VERBOSE_ARG},
    {NULL, 0, NULL, 0}
  };

/* Messages */

char *recinf_version_msg = "recinf (GNU recutils) 1.0\n\
Copyright (C) 2010 Jose E. Marchesi. \n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>. \n\
This is free software: you are free to change and redistribute it. \n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
Written by Jose E. Marchesi.";

char *recinf_usage_msg = "\
Usage: recinf [OPTION]... [FILE]...\n\
Print information about the contents of the specified rec files.\n\
\n\
available options\n\
  -V, --verbose                       include the full record descriptors.\n\
  --help                              print a help message and exit.\n\
  --usage                             print a usage message and exit.\n\
  --version                           show recinf version and exit.\n\
";

char *recinf_help_msg = "";

bool recinf_verbose = false;

bool
print_info_file (FILE *in)
{
  bool ret;
  rec_rset_t rset;
  rec_record_t descriptor;
  rec_field_t field;
  rec_parser_t parser;
  rec_field_name_t fname;
  char *fvalue;

  ret = true;
  parser = rec_parser_new (in);
  while (rec_parse_rset (parser, &rset))
    {
      descriptor = rec_rset_descriptor (rset);

      if (recinf_verbose)
        {
          rec_writer_t writer;

          writer = rec_writer_new (stdout);
          rec_write_record (writer, descriptor);
          rec_writer_destroy (writer);
        }
      else
        {
          if (descriptor)
            {
              printf ("%s", rec_field_value (rec_record_get_field_name (descriptor,
                                                                        "%rec")));
            }
          else
            {
              printf ("unknown");
            }
        }

      printf ("\n");
    }
  
  if (rec_parser_error (parser))
    {
      rec_parser_perror (parser, "stdin");
    }

  rec_parser_destroy (parser);

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
                             "V",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          /* COMMON ARGUMENTS */
        case HELP_ARG:
          {
            fprintf (stdout, "%s\n", recinf_usage_msg);
            exit (0);
            break;
          }
        case VERSION_ARG:
          {
            fprintf (stdout, "%s\n", recinf_version_msg);
            exit (0);
            break;
          }
        case USAGE_ARG:
          {
            fprintf (stdout, "%s\n", recinf_usage_msg);
            exit (0);
            break;
          }
        case VERBOSE_ARG:
          {
            recinf_verbose = true;
          }
        default:
          {
            return 1;
          }
        }
    }

  /* Process the input files, if any.  Otherwise use the standard
     input to read the rec data. */
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
              if (!print_info_file (in))
                {
                  /* Parse error */
                  return 1;
                }

              fclose (in);
            }
        }
    }
  else
    {
      print_info_file (stdin);
    }

  return 0;
}

/* End of recinf.c */
