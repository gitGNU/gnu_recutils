/* -*- mode: C -*- Time-stamp: "2010-10-24 23:13:26 jemarch"
 *
 *       File:         recutl.c
 *       Date:         Thu Apr 22 17:30:48 2010
 *
 *       GNU recutils - Common code for the utilities.
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

#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <closeout.h>
#include <xalloc.h>
#include <libintl.h>
#define _(str) gettext (str)

#include <rec.h>
#include <recutl.h>

extern char *program_name;

void recutl_print_help (void); /* Forward prototype.  */

void
recutl_init (char *util_name)
{
  program_name = xstrdup (util_name);

  /* Initialize librec */
  rec_init ();

  /* Even exiting has subtleties.  On exit, if any writes failed, change
     the exit status.  The /dev/full device on GNU/Linux can be used for
     testing; for instance, hello >/dev/full should exit unsuccessfully.
     This is implemented in the Gnulib module "closeout".  */
  atexit (close_stdout);

  /* i18n */
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
}

void
recutl_print_help_footer (void)
{
  /* TRANSLATORS: --help output 5+ (reports)
     TRANSLATORS: the placeholder indicates the bug-reporting address
     for this application.  Please add _another line_ with the
     address for translation bugs.
     no-wrap */
  printf (_("\
Report bugs to: %s\n"), PACKAGE_BUGREPORT);
#ifdef PACKAGE_PACKAGER_BUG_REPORTS
  printf (_("Report %s bugs to: %s\n"), PACKAGE_PACKAGER,
          PACKAGE_PACKAGER_BUG_REPORTS);
#endif
#ifdef PACKAGE_URL
  printf (_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
#else
  printf (_("%s home page: <http://www.gnu.org/software/recutils/>\n"),
          PACKAGE_NAME, PACKAGE);
#endif
  fputs (_("General help using GNU software: <http://www.gnu.org/gethelp/>\n"),
         stdout);
}

void
recutl_print_help_common (void)
{
  /* TRANSLATORS: --help output, common arguments.
     no-wrap */
  fputs (_("\
      --help                          print a help message and exit.\n\
      --version                       show version and exit.\n"),
         stdout);
}

void
recutl_print_help_record_selection (void)
{
  /* TRANSLATORS: --help output, record selection arguments
     no-wrap */
  fputs (_("\
Record selection options:\n\
  -i, --case-insensitive              make strings case-insensitive in selection\n\
                                        expressions.\n\
  -t, --type=TYPE                     print records of the specified type only.\n\
  -e, --expression=EXPR               selection expression.\n\
  -n, --number=NUM                    select an specific record.\n"),
         stdout);
}

void
recutl_print_version (void)
{
  printf ("%s (GNU %s) %s\n",
          program_name,
          PACKAGE,
          VERSION);
  /* xgettext: no-wrap */
  puts ("");

  /* It is important to separate the year from the rest of the message,
     as done here, to avoid having to retranslate the message when a new
     year comes around.  */  
  printf (_("\
Copyright (C) %s Jose E. Marchesi.\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n"), "2010");

  puts (_("\
\n\
Written by Jose E. Marchesi."));
}

void
recutl_fatal (char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  fputs (program_name, stderr);
  fputs (": error: ", stderr);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

  exit (1);
}

void
recutl_error (char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  fputs (program_name, stderr);
  fputs (": error: ", stderr);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
}

bool
recutl_parse_db_from_file (FILE *in,
                           char *file_name,
                           rec_db_t db)
{
  bool res;
  rec_rset_t rset;
  rec_parser_t parser;

  res = true;
  parser = rec_parser_new (in, file_name);

  while (rec_parse_rset (parser, &rset))
    {
      char *rset_type;
      /* XXX: check for consistency!!!.  */
      rset_type = rec_rset_type (rset);
      if (rec_db_type_p (db, rset_type))
        {
          recutl_fatal (_("duplicated record set '%s' from %s.\n"),
                        rset_type, file_name);
        }

      if (!rec_db_insert_rset (db, rset, rec_db_size (db)))
        {
          /* Error.  */
          res = false;
          break;
        }
    }

  if (rec_parser_error (parser))
    {
      /* Report parsing errors.  */
      rec_parser_perror (parser, "%s", file_name);
    }
  
  return res;
}

rec_db_t
recutl_build_db (int argc, char **argv)
{
  rec_db_t db;
  char *file_name;
  FILE *in;

  db = rec_db_new ();

  /* Process the input files, if any.  Otherwise use the standard
     input to read the rec data.  */
  if (optind < argc)
    {
      while (optind < argc)
        {
          file_name = argv[optind++];
          if (!(in = fopen (file_name, "r")))
            {
              recutl_fatal (_("cannot read file %s\n"), file_name);
            }
          else
            {
              if (!recutl_parse_db_from_file (in, file_name, db))
                {
                  free (db);
                  db = NULL;
                }
              
              fclose (in);
            }
        }
    }
  else
    {
      if (!recutl_parse_db_from_file (stdin, "stdin", db))
        {
          free (db);
          db = NULL;
        }
    }

  return db;
}

char *
recutl_eval_field_expression (rec_fex_t fex,
                              rec_record_t record,
                              rec_writer_mode_t mode,
                              bool print_values_p,
                              bool print_in_a_row_p)
{
  char *res;
  size_t res_size;
  FILE *stm;
  rec_writer_t writer;
  bool wrote_p;

  stm = open_memstream (&res, &res_size);
  writer = rec_writer_new (stm);
  wrote_p = rec_write_record_with_fex (writer, record, fex, mode,
                                       print_values_p, print_in_a_row_p);
  rec_writer_destroy (writer);
  fclose (stm);

  if (!wrote_p)
    {
      free (res);
      res = NULL;
    }

  return res;
}

rec_db_t
recutl_read_db_from_file (char *file_name)
{
  rec_db_t db;
  FILE *in;

  db = rec_db_new ();
  if (file_name)
    {
      in = fopen (file_name, "r");
      if (in == NULL)
        {
          recutl_fatal (_("cannot read file %s\n"), file_name);
          exit (1);
        }
    }
  else
    {
      /* Process the standard input.  */
      file_name = "stdin";
      in = stdin;
    }

  if (!recutl_parse_db_from_file (in,
                                  file_name,
                                  db))
    {
      rec_db_destroy (db);
      db = NULL;
    }

  return db;
}

void
recutl_write_db_to_file (rec_db_t db,
                         char *file_name)
{
  FILE *out;
  char *tmp_file_name;
  rec_writer_t writer;
  int des;

  if (!file_name)
    {
      out = stdout;
    }
  else
    {
      /* Create a temporary file with the results. */
      tmp_file_name = xmalloc (100);
      strcpy (tmp_file_name, "recXXXXXX");
      des = mkstemp (tmp_file_name);
      if (des == -1)
        {
          recutl_fatal (_("cannot create a unique name.\n"));
        }
      out = fdopen (des, "w+");
    }

  writer = rec_writer_new (out);
  rec_write_db (writer, db);
  fclose (out);
  rec_db_destroy (db);

  if (file_name)
    {
      /* Rename the temporary file to file_name.  */
      if (rename (tmp_file_name, file_name) == -1)
        {
          remove (tmp_file_name);
          recutl_fatal (_("renaming file %s to %s\n"), tmp_file_name, file_name);
        }
    }
}

/* End of recutl.c */
