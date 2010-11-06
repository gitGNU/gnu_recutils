/* -*- mode: C -*- Time-stamp: "2010-11-06 19:52:49 jemarch"
 *
 *       File:         recfix.c
 *       Date:         Tue Apr 27 12:21:48 2010
 *
 *       GNU recutils - recfix
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
#include <stdlib.h>
#include <libintl.h>
#define _(str) gettext (str)

#include <rec.h>
#include <recutl.h>

/* Forward prototypes.  */
void recfix_parse_args (int argc, char **argv);
bool recfix_process_data (rec_db_t db);

/*
 * Global variables.
 */

char *program_name = NULL;
bool recfix_external = true;

/*
 * Command line options management.
 */

enum
{
  COMMON_ARGS,
  NO_EXTERNAL_ARG
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {"no-external", no_argument, NULL, NO_EXTERNAL_ARG},
    {NULL, 0, NULL, 0}
  };

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, recfix synopsis.
     no-wrap */
  printf (_("\
Usage: recfix [OPTION]... [FILE]...\n"));

  /* TRANSLATORS: --help output, recfix short description.
     no-wrap */
  fputs (_("\
Check and fix rec files.\n"),
         stdout);

  puts ("");
  /* TRANSLATORS: --help output, recfix arguments.
     no-wrap */
  fputs (_("\
      --no-external               don't use external descriptors.\n"),
         stdout);

  recutl_print_help_common ();

  puts("");
  /* TRANSLATORS: --help output, recfix examples.
     no-wrap */
  fputs (_("\
Examples:\n\
\n\
        recfix data.rec\n\
        recfix data1.rec data2.rec\n\
        cat data1.rec data2.rec | recfix\n"),
         stdout);

  puts("");
  recutl_print_help_footer ();
}

void
recfix_parse_args (int argc,
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
          COMMON_ARGS_CASES
        case NO_EXTERNAL_ARG:
          {
            recfix_external = false;
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
recfix_process_data (rec_db_t db)
{
  return (rec_int_check_db (db,
                            true,            /* Check descriptors.  */
                            recfix_external, /* Use external descriptors.  */
                            stderr));
}

int
main (int argc, char *argv[])
{
  int res;
  rec_db_t db;

  recutl_init ("recfix");

  res = EXIT_SUCCESS;

  /* Parse arguments.  */
  recfix_parse_args (argc, argv);

  /* Get the input data.  */
  db = recutl_build_db (argc, argv);
  if (!db)
    {
      res = EXIT_FAILURE;
    }

  /* Process the data.  */
  if (!recfix_process_data (db))
    {
      res = EXIT_FAILURE;
    }

  rec_db_destroy (db);

  return res;
}

/* End of recfix.c */
