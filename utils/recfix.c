/* -*- mode: C -*- Time-stamp: "2010-08-20 19:56:30 jco"
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

#include <rec.h>
#include <recutl.h>

/* Forward prototypes.  */
void recfix_parse_args (int argc, char **argv);
bool recfix_process_data (rec_db_t db);

/*
 * Global variables.
 */

char *program_name = NULL;

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

RECUTL_COPYRIGHT_DOC ("recfix");

char *recutl_help_msg = "\
Usage: recfix [OPTION]... [FILE]...\n\
Check and fix rec files.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n"
COMMON_ARGS_DOC
"\n\
Examples:\n\
\n\
        recfix data.rec\n\
        recfix data1.rec data2.rec\n\
        cat data1.rec data2.rec | recfix\n\
\n"
  RECUTL_HELP_FOOTER_DOC ("recfix");

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
        default:
          {
            exit (1);
          }
        }
    }
}

bool
recfix_process_data (rec_db_t db)
{
  return (rec_int_check_db (db,
                            true, /* Check descriptors.  */
                            stderr) == 0);
}

int
main (int argc, char *argv[])
{
  int res;
  rec_db_t db;

  recutl_init ("recfix");

  res = 0;

  /* Parse arguments.  */
  recfix_parse_args (argc, argv);

  /* Get the input data.  */
  db = recutl_build_db (argc, argv);
  if (!db)
    {
      res = 1;
    }

  /* Process the data.  */
  if (!recfix_process_data (db))
    {
      res = 1;
    }

  rec_db_destroy (db);

  return res;
}

/* End of recfix.c */
