/* -*- mode: C -*- Time-stamp: "2010-12-20 17:04:39 jco"
 *
 *       File:         recnav.c
 *       Date:         Mon Dec 20 16:43:01 2010
 *
 *       GNU recutils - recnav
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
#include <xalloc.h>
#include <gettext.h>
#define _(str) gettext (str)

#include <rec.h>
#include <recutl.h>

/* Forward prototypes.  */
void recnav_parse_args (int argc, char **argv);
void recnav_navigate (rec_db_t db);

/*
 * Global variables.
 */

char      *program_name        = NULL;

/*
 * Command line options management.
 */

enum
{
  COMMON_ARGS
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {NULL, 0, NULL, 0}
  };

/*
 * Functions.
 */

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, recnav synopsis.
     no-wrap */
  printf (_("\
Usage: recnav [OPTION]... [FILE]...\n"));

  /* TRANSLATORS: --help output, recnav arguments.
     no-wrap */
  fputs(_("\
Navigate the contents of a recfile.\n"), stdout);

  recutl_print_help_common ();

  puts ("");
  /* TRANSLATORS: --help output, recnav examples.
     no-wrap */
  fputs (_("\
Examples:\n\
\n\
        recnav data.rec\n"),
         stdout);

  puts ("");
  recutl_print_help_footer ();
}

void
recnav_parse_args (int argc,
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
        default:
          {
            exit (EXIT_FAILURE);
          }
        }
    }
}

void
recnav_navigate (rec_db_t db)
{
  
}

int
main (int argc, char *argv[])
{
  int res = 0;
  rec_db_t db;

  recutl_init ("recnav");
  
  /* Parse arguments.  */
  recnav_parse_args (argc, argv);

  /* Get the input data.  */
  db = recutl_build_db (argc, argv);
  if (!db)
    {
      res = 1;
    }

  /* Process the data.  */
  recnav_navigate (db);

  rec_db_destroy (db);
  
  return res;
}

/* End of recnav.c */

