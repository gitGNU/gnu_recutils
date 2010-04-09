/* -*- mode: C -*-
 *
 *       File:         recset.c
 *       Date:         Fri Apr  9 17:06:39 2010
 *
 *       GNU recutils - recset
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
#include <recset.h>

/*
 * Forward prototypes.
 */

void recset_parse_args (int argc, char **argv);

/*
 * Global variables.
 */

char *program_name; /* Initialized in main().  */

/*
 * Command line options management
 */

static const struct option GNU_longOptions[] =
  {
    {"help", no_argument, NULL, HELP_ARG},
    {"version", no_argument, NULL, VERSION_ARG},
    {NULL, 0, NULL, 0}
  };

/* Messages */

char *recset_version_msg = "recset (GNU recutils) 1.0\n\
Copyright (C) 2010 Jose E. Marchesi.\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
Written by Jose E. Marchesi.";

char *recset_help_msg = "\
Usage: recset [OPTION]... [FILE]...\n\
Alter or delete fields in rec data.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
      --help                          print a help message and exit.\n\
      --version                       show recset version and exit.\n\
\n\
Examples:\n\
\n\
        recset -f TmpName -d data.rec\n\
        recset -f Email -v invalid@email.com friends.rec\n\
        recset -e \"Name ~ 'Smith'\" -f Email -a new@email.com friends.rec\n\
\n\
Report recset bugs to bug-recutils@gnu.org\n\
GNU recutils home page: <http://www.gnu.org/software/recutils/>\n\
General help using GNU software: <http://www.gnu.org/gethelp/>\
";

void
recset_parse_args (int argc,
                   char **argv)
{
  char c;
  char ret;

  while ((ret = getopt_long (argc,
                             argv,
                             "",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
        case HELP_ARG:
          {
            fprintf (stdout, "%s\n", recset_help_msg);
            exit (0);
            break;
          }
        case VERSION_ARG:
          {
            fprintf (stdout, "%s\n", recset_version_msg);
            exit (0);
            break;
          }
        default:
          {
            exit (1);
          }
        }
    }
}

int
main (int argc, char *argv[])
{
  int res;
  
  res = 0;
  program_name = strdup (argv[0]);

  /* Parse arguments.  */
  recset_parse_args (argc, argv);

  return res;
}

/* End of recset.c */
