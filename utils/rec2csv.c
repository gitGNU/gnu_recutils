/* -*- mode: C -*- Time-stamp: "2011-01-31 22:31:46 jemarch"
 *
 *       File:         rec2csv.c
 *       Date:         Mon Jan 31 22:12:29 2011
 *
 *       GNU recutils - rec to csv converter.
 *
 */

/* Copyright (C) 2011 Jose E. Marchesi */

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

#include <csv.h>
#include <rec.h>
#include <recutl.h>

/* Forward declarations.  */
static void rec2csv_parse_args (int argc, char **argv);
static bool rec2csv_process_data (rec_db_t db);

/*
 * Types
 */

/*
 * Global variables
 */

char *rec2csv_record_type = NULL;

/*
 * Command line options management
 */

enum
  {
    COMMON_ARGS,
    RECORD_TYPE_ARG
  };

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {"type", required_argument, NULL, RECORD_TYPE_ARG},
    {NULL, 0, NULL, 0}
  };


/*
 * Functions.
 */

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, rec2csv synopsis.
     no-wrap */
  printf (_("\
Usage: rec2csv [OPTIONS]... [REC_FILE]\n"));

  /* TRANSLATORS: --help output, rec2csv short description.
     no-wrap */
  fputs (_("\
Convert rec data into csv data.\n"), stdout);

  puts ("");
  /* TRANSLATORS: --help output, rec2csv options.
     no-wrap */
  fputs (_("\
  -t, --type=TYPE                     record set to convert to csv; if this parameter\n\
                                        is ommited then the default record set is used\n"),
         stdout);

  recutl_print_help_common ();

  puts ("");
  /* TRANSLATORS: --help output, rec2csv examples.
     no-wrap */
  fputs (_("\
Examples:\n\
\n\
        rec2csv contacts.rec > contacts.csv\n\
        cat contacts.rec | rec2csv -t Contact > contacts.csv\n"), stdout);

  puts ("");
  recutl_print_help_footer ();
}

static void
rec2csv_parse_args (int argc,
                    char **argv)
{
  int ret;
  char c;

  while ((ret = getopt_long (argc,
                             argv,
                             "t:",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
        case RECORD_TYPE_ARG:
        case 't':
          {
            rec2csv_record_type = xstrdup (optarg);
            break;
          }
        default:
          {
            exit (EXIT_FAILURE);
          }
        }
    }
}

static bool
rec2csv_process_data (rec_db_t db)
{
  bool ret;

  ret = true;

  return ret;
}

int
main (int argc, char *argv[])
{
  int res;
  rec_db_t db;

  res = 0;

  recutl_init ("rec2csv");

  /* Parse arguments.  */
  rec2csv_parse_args (argc, argv);

  /* Get the input data.  */
  db = recutl_build_db (argc, argv);
  if (!db)
    {
      res = 1;
    }

  /* Process the data.  */
  if (!rec2csv_process_data (db))
    {
      res = 1;
    }

  rec_db_destroy (db);
  
  return res;
}

/* End of rec2csv.c */
