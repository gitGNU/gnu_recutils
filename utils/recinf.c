/* -*- mode: C -*- Time-stamp: "09/12/28 09:38:13 jemarch"
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
#include <rec.h>
#include <stdlib.h>

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
Usage: recinf [OPTION]... [FILE]\n\
Print information about the specified rec files.\n\
\n\
available options\n\
  --help                              print a help message and exit.\n\
  --usage                             print a usage message and exit.\n\
  --version                           show recinf version and exit.\n\
";

char *recinf_help_msg = "";

int
main (int argc, char *argv[])
{
  char c;
  bool finish;
  bool printall;
  int ret;

  finish = false;
  printall = false;

  while (!finish &&
         (ret = getopt_long (argc,
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

  program_name = strdup (argv[0]);


  return 0;
}

/* End of recinf.c */
