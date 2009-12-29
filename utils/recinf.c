/* -*- mode: C -*- Time-stamp: "09/12/29 16:02:23 jemarch"
 *
 *       File:         recinf.c
 *       Date:         Mon Dec 28 08:54:38 2009
 *
 *       GNU Rec - recinf
 *
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
    {NULL, 0, NULL, 0}
  };

/* Messages */

char *recinf_version_msg = "recinf 1.0";

char *recinf_usage_msg = "\
Usage: recinf [OPTION]... [FILE]...\n\
Print information about the contents of the specified rec files.\n\
\n\
available options\n\
  --help                              print a help message and exit.\n\
  --usage                             print a usage message and exit.\n\
  --version                           show recinf version and exit.\n\
";

char *recinf_help_msg = "";

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
      printf("%d ", rec_rset_size (rset));
      if (descriptor)
        {
          printf ("%s", rec_field_value (rec_record_get_field_name (descriptor,
                                                                    "%rec")));
        }
      else
        {
          printf ("unknown");
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
                             "",
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
