/* -*- mode: C -*- Time-stamp: "2013-08-24 16:01:44 jemarch"
 *
 *       File:         readrec.c
 *       Date:         Fri Aug 23 18:38:08 2013
 *
 *       GNU recutils - readrec bash loadable builtin.
 *
 */

/* Copyright (C) 2013 Jose E. Marchesi */

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

#include <bash/config.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>
#include <rec.h>

#include "builtins.h"
#include "shell.h"
#include "builtins/bashgetopt.h"

/* The function implementing the builtin.  It uses internal_getopt to
   parse options.  It is the same as getopt(3), but it takes a pointer
   to a WORD_LIST.

   If the builtin takes no options, call no_options(list) before doing
   anything else.  If it returns a non-zero value, your builtin should
   immediately return EX_USAGE.

   A builtin command returns EXECUTION_SUCCESS for success and
   EXECUTION_FAILURE to indicate failure.  */
int
readrec_builtin (WORD_LIST *list)
{
  SHELL_VAR *var;
  rec_parser_t parser;
  rec_record_t record;

  no_options (list);

  /* Create a librec parser to operate on the standard input and try
     to read a record.  If there is a parse error then report it and
     fail.  */

  parser = rec_parser_new (stdin, "stdin");
  if (!parser)
    return EXECUTION_FAILURE;

  if (!rec_parse_record (parser, &record))
    {
      if (rec_parser_eof (parser))
        {
          return EXIT_FAILURE;
        }

      builtin_error (_("parse error reading a record"));
      return EXECUTION_FAILURE;
    }
  
  {
    size_t record_str_size = 0;
    char *record_str = NULL;
    char *record_str_dequoted = NULL;
    rec_writer_t writer = rec_writer_new_str (&record_str, &record_str_size);

    if (!writer || !rec_write_record (writer, record, REC_WRITER_NORMAL))
      return EXIT_FAILURE;
    rec_writer_destroy (writer);

    /* Set the REPLY_REC environment variable to the read record.  */
    record_str_dequoted = dequote_string (record_str);
    var = bind_variable ("REPLY_REC", record_str_dequoted, 0);
    VUNSETATTR (var, att_invisible);
    xfree (record_str_dequoted);
  }

  return EXECUTION_SUCCESS;
}

/* An array of strings forming the `long' documentation for the builtin,
   which is printed by `help xxx'.  It must end with a NULL.  By convention,
   the first line is a short description. */
char *readrec_doc[] = {
  "Read a recutils record from the standard input.",
  "",
  "Reads a recutils record from the standard input.  The record is stored\n\
  in the REPLY_REC variable.\n\
\n\
Exit Status:\n\
The return code is zero, unless end-of-file is encountered.\n",
  (char *) NULL
};

/* The standard structure describing a builtin command.  bash keeps an
   array of these structures.  The flags must include BUILTIN_ENABLED
   so the builtin can be used. */
struct builtin readrec_struct = {
	"readrec",		/* builtin name */
	readrec_builtin,	/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	readrec_doc,		/* array of long documentation strings. */
	"readrec",		/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
	
