/* -*- mode: C -*-
 *
 *       File:         recdel.c
 *       Date:         Mon Dec 28 08:54:38 2009
 *
 *       GNU recutils - recdel
 *
 */

/* Copyright (C) 2009, 2010 Jose E. Marchesi */

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

/* Forward declarations.  */
void recdel_delete_records (rec_db_t db);
void recdel_parse_args (int argc, char **argv);

/*
 * Global variables
 */

char *recutl_type = NULL;
bool recdel_comment = false;
rec_sex_t recutl_sex = NULL;
char *recutl_sex_str = NULL;
int recutl_num = -1;
bool recutl_insensitive = false;
bool recdel_force = false;
bool recdel_verbose = false;
bool recdel_external = true;
char *recdel_file = NULL;  /* File from where to delete the
                              records.  */

/*
 * Command line options management
 */

enum
{
  COMMON_ARGS,
  RECORD_SELECTION_ARGS,
  COMMENT_ARG,
  FORCE_ARG,
  VERBOSE_ARG,
  NO_EXTERNAL_ARG
};

static const struct option GNU_longOptions[] =
  {
    COMMON_LONG_ARGS,
    RECORD_SELECTION_LONG_ARGS,
    {"comment", no_argument, NULL, COMMENT_ARG},
    {"force", no_argument, NULL, FORCE_ARG},
    {"verbose", no_argument, NULL, VERBOSE_ARG},
    {"no-external", no_argument, NULL, NO_EXTERNAL_ARG},
    {NULL, 0, NULL, 0}
  };

void
recutl_print_help (void)
{
  /* TRANSLATORS: --help output, recdel synopsis.
     no-wrap */
  printf (_("\
Usage: recdel [OPTIONS]... [-t TYPE] [-n NUM | -e EXPR] [FILE]\n"));

  /* TRANSLATORS: --help output, recdel short description.
     no-wrap */
  fputs (_("\
Remove (or comment out) records from a rec file.\n"),
         stdout);

  puts ("");
  /* TRANSLATORS: --help output, recdel arguments.
     no-wrap */
  fputs (_("\
  -c, --comment                       comment out the matching records instead of\n\
                                        deleting them.\n\
      --force                         delete even in potentially dangerous situations,\n\
                                        and if the deletion is violating record restrictions.\n\
      --no-external                   don't use external descriptors.\n\
      --verbose                       give a detailed report if the integrity check\n\
                                        fails.\n"),
         stdout);

  recutl_print_help_common ();
  
  puts ("");
  recutl_print_help_record_selection ();

  puts ("");
  /* TRANSLATORS: --help output, notes on recdel.
     no-wrap */
  fputs (_("\
If no FILE is specified then the command acts like a filter, getting\n\
the data from standard input and writing the result to standard output.\n"),
         stdout);

  puts("");
  /* TRANSLATORS: --help output, recdel examples.
     no-wrap */
  fputs (_("\
Examples:\n\
\n\
        recdel -n 10 contacts.rec\n\
        cat hackers.rec | recdel -e \"Email[0] = 'foo@bar.com'\" > other.rec\n"),
         stdout);
  
  puts ("");
  recutl_print_help_footer ();
}

void
recdel_delete_records (rec_db_t db)
{
  int n_rset;
  int numrec;
  int rset_size;
  rec_rset_t rset;
  rec_record_t record;
  rec_comment_t comment;
  bool parse_status = true;
  rec_rset_elem_t rec_elem;
  rec_rset_elem_t new_elem;

  if (!rec_db_type_p (db, recutl_type))
    {
      recutl_fatal (_("no records of type %s found.\n"),
                    recutl_type ? recutl_type : "<default>");
    }

  for (n_rset = 0; n_rset < rec_db_size (db); n_rset++)
    {
      rset = rec_db_get_rset (db, n_rset);
      rset_size = rec_rset_num_records (rset);

      /* Don't process empty record sets.  */
      if (rset_size == 0)
        {
          continue;
        }

      /* If the user specified a type, print the record set only if it
         is of the given type.  */
      if (recutl_type
          && (!rec_rset_type (rset)
              || (strcmp (recutl_type, rec_rset_type (rset)) != 0)))
        {
          continue;
        }

      /* If the user didn't specified a type, process the record set if and only if:
       *
       * - It is the default record set.
       * - The file contains just one record set.
       */
      if (!recutl_type
          && rec_rset_type (rset)
          && (rec_db_size (db) > 1))
        {
          continue;
        }

      /* Process this record set.  */
      numrec = 0;
      rec_elem = rec_rset_first_record (rset);
      while (rec_rset_elem_p (rec_elem))
        {
          record = rec_rset_elem_record (rec_elem);
          
          if (((recutl_num == -1) && !recutl_sex)
              || ((recutl_num == -1) &&
                  ((recutl_sex &&
                    (rec_sex_eval (recutl_sex, record, &parse_status))))
                  || (recutl_num == numrec)))
            {
              /* Delete this record.  */
              if (recdel_comment)
                {
                  comment = rec_record_to_comment (record);
                  new_elem = rec_rset_elem_comment_new (rset, comment);
                  rec_rset_insert_after (rset, rec_elem, new_elem);
                }

              rec_elem = rec_rset_remove_record (rset, rec_elem);
            }
          else
            {
              /* Process the next record. */
              rec_elem = rec_rset_next_record (rset, rec_elem);
            }
          
          if (!parse_status)
            {
              recutl_fatal (_("evaluating the selection expression.\n"));
            }
          
          numrec++;
        }
    }

  /* Integrity check.  */
  if (!recdel_force && db)
    {
      recutl_check_integrity (db, recdel_verbose, recdel_external);
    }
}

void
recdel_parse_args (int argc,
                   char **argv)
{
  int ret;
  char c;

  while ((ret = getopt_long (argc,
                             argv,
                             RECORD_SELECTION_SHORT_ARGS
                             "c",
                             GNU_longOptions,
                             NULL)) != -1)
    {
      c = ret;
      switch (c)
        {
          COMMON_ARGS_CASES
          RECORD_SELECTION_ARGS_CASES
        case FORCE_ARG:
          {
            recdel_force = true;
            break;
          }
        case VERBOSE_ARG:
          {
            recdel_verbose = true;
            break;
          }
        case NO_EXTERNAL_ARG:
          {
            recdel_external = false;
            break;
          }
        case COMMENT_ARG:
        case 'c':
          {
            recdel_comment = true;
            break;
          }
        default:
          {
            exit (EXIT_FAILURE);
          }
        }
    }
  
  if ((recutl_num == -1) && !recutl_sex_str && !recdel_force)
    {
      recutl_error (_("ignoring a request to delete all records of type %s.\n"),
                    recutl_type ? recutl_type : "unknown");
      recutl_fatal (_("use --force if you really want to proceed, or use either -n or -e.\n"));
    }

  if (recutl_sex_str)
    {
      recutl_sex = rec_sex_new (recutl_insensitive);
      if (!rec_sex_compile (recutl_sex, recutl_sex_str))
        {
          recutl_fatal (_("invalid selection expression.\n"));
        }
    }

  /* Read the name of the file where to delete the records.  */
  if (optind < argc)
    {
      if ((argc - optind) != 1)
        {
          recutl_print_help ();
          exit (EXIT_FAILURE);
        }

      recdel_file = argv[optind++];
    }
}

int
main (int argc, char *argv[])
{
  rec_db_t db;

  recutl_init ("recdel");

  recdel_parse_args (argc, argv);

  db = recutl_read_db_from_file (recdel_file);
  if (((recutl_num != -1) || recutl_sex) || recdel_force)
    {
      recdel_delete_records (db);
    }
  recutl_write_db_to_file (db, recdel_file);

  return 0;
}

/* End of recdel.c */
