/* -*- mode: C -*-
 *
 *       File:         recfix.c
 *       Date:         Tue Apr 27 12:21:48 2010
 *
 *       GNU recutils - recfix
 *
 */

/* Copyright (C) 2010,2011 Jose E. Marchesi */

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
#include <gettext.h>
#define _(str) gettext (str)

#include <rec.h>
#include <recutl.h>

/* Forward prototypes.  */
void recfix_parse_args (int argc, char **argv);
bool recfix_process_data (rec_db_t db);

/*
 * Data types.
 */

/* recfix supports several operations, enumerated in the following
   type.  */

enum recfix_op
{
  RECFIX_OP_CHECK,
  RECFIX_OP_SORT,
  RECFIX_OP_ENCRYPT,
  RECFIX_OP_DECRYPT
};

/*
 * Global variables.
 */

bool  recfix_external = true;
char *recfix_file     = NULL;
int   recfix_op       = RECFIX_OP_CHECK;

/*
 * Command line options management.
 */

enum
{
  COMMON_ARGS,
  NO_EXTERNAL_ARG,
  OP_SORT_ARG
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    {"no-external", no_argument, NULL, NO_EXTERNAL_ARG},
    {"sort", no_argument, NULL, OP_SORT_ARG},
    {NULL, 0, NULL, 0}
  };

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, recfix synopsis.
     no-wrap */
  printf (_("\
Usage: recfix [OPTION]... [OPERATION] [OP_OPTION]... [FILE]\n"));

  /* TRANSLATORS: --help output, recfix short description.
     no-wrap */
  fputs (_("\
Check and fix rec files.\n"),
         stdout);

  puts ("");
  /* TRANSLATORS: --help output, recfix global arguments.
     no-wrap */
  fputs (_("\
  -t, --type=TYPE                     process only records of the given type.\n\
      --no-external                   don't use external descriptors.\n"),
         stdout);

  recutl_print_help_common ();

  puts("");
  /* TRANSLATORS: --help output, recfix operations.
     no-wrap */
  fputs (_("\
Operations:\n\
      --check                         check integrity of the specified file.  This\n\
                                        is the default operation.\n\
      --sort                          sort the records in the specified file.\n\
      --encrypt                       encrypt confidential fields in the specified file.\n\
      --decrypt                       decrypt confidential fields in the specified file.\n"),
         stdout);

  puts("");
  /* TRANSLATORS: --help output, recfix encryption and decryption
     options.
     no-wrap */
  fputs (_("\
De/Encryption options:\n\
  -s, --password=PASSWORD             encrypt/decrypt with this password.\n"),
         stdout);

  puts("");
  /* TRANSLATORS: --help output, notes on recfix.
     no-wrap */
  fputs (_("\
If no FILE is specified then the command acts like a filter, getting\n\
the data from standard input and writing the result to standard output.\n"), stdout);

  puts("");
  recutl_print_help_footer ();
}

void
recfix_parse_args (int argc,
                   char **argv)
{
  char c;
  int ret;

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
        case OP_SORT_ARG:
          {
            /* Sort all records when parsing.  */
            recutl_sorting_parser (true, NULL, NULL);

            /* Mark the operation.  */
            recfix_op = RECFIX_OP_SORT;

            break;
          }
        default:
          {
            exit (EXIT_FAILURE);
          }
        }
    }

  /* Read the name of the file to work on.  */
  if (optind < argc)
    {
      if ((argc - optind) != 1)
        {
          recutl_print_help ();
          exit (EXIT_FAILURE);
        }

      recfix_file = argv[optind++];
    }
}

bool
recfix_process_data (rec_db_t db)
{
  bool ret;
  char *errors;
  size_t errors_size;
  rec_buf_t buf;

  buf = rec_buf_new (&errors, &errors_size);
  ret = (rec_int_check_db (db,
                           true,            /* Check descriptors.  */
                           recfix_external, /* Use external descriptors.  */
                           buf) == 0);
  rec_buf_close (buf);
  fprintf (stderr, "%s", errors);

  return ret;
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

  db = recutl_read_db_from_file (recfix_file);
  if (!db)
    {
      res = EXIT_FAILURE;
    }

  /* Process the data.  */
  if (!recfix_process_data (db))
    {
      res = EXIT_FAILURE;
    }

  if ((res == EXIT_SUCCESS)
      && (recfix_op == RECFIX_OP_SORT))
    {
      recutl_write_db_to_file (db, recfix_file);
    }

  return res;
}

/* End of recfix.c */
