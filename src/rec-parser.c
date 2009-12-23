/* -*- mode: C -*- Time-stamp: "09/12/23 21:46:45 jemarch"
 *
 *       File:         rec-parser.c
 *       Date:         Wed Dec 23 20:55:15 2009
 *
 *       GNU rec - Parsing routines
 *
 */

/* Copyright (C) 2009 Jose E. Marchesi */

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
#include <malloc.h>
#include <string.h>

#include <rec.h>

/* Static functions defined in this file */
static char rec_parser_getc (rec_parser_t parser);
static bool rec_parser_number_p (char c);
static bool rec_parser_letter_p (char c);

/* Parser Data Structure
 *
 */
struct rec_parser_s
{
  FILE *in; /* File stream used by the parser. */
  
  bool eof;
  int line; /* Current line. */
};

rec_parser_t
rec_parser_new (FILE *in)
{
  rec_parser_t parser;

  parser = malloc (sizeof (struct rec_parser_s));
  if (parser != NULL)
    {
      parser->in = in;
      parser->eof = false;
      parser->line = 1;
    }

  return parser;
}

void
rec_parser_destroy (rec_parser_t parser)
{
  free (parser);
}

char *
rec_parse_field_name (rec_parser_t parser)
{
  char *name;
  char c;
  int ci;
  int index;
  
  name = malloc (256);
  index = 0;
  
  /* The name begins with: [a-zA-Z] */
  ci = rec_parser_getc (parser);
  if (ci == EOF)
    {
      /* XXX return */
    }
  c = (char) ci;
  if (rec_parser_letter_p (c))
    {
      name[index++] = c;

      /* Rest of the name: [a-zA-Z0-9_]+ */
      while ((ci = rec_parser_getc (parser)) != EOF)
        {
          c = (char) ci;
          
          if (rec_parser_number_p (c)
              || rec_parser_letter_p (c)
              || (c == '_'))
            {
              name[index++] = c;
            }
          else
            {
              name[index++] = 0;
              break;
            }
        }
    }

  if (index != 0)
    {
       realloc (name, index);
      return name;
    }
  else
    {
      free (name);
      return NULL;
    }
}

/*
 * Private functions
 */

static int
rec_parser_getc (rec_parser_t parser)
{
  int ci;

  ci = fgetc (parser->in);
  if (ci == EOF)
    {
      parser->eof = true;
    }

  return ci;
}

static int
rec_parser_number_p (char *c)
{
  return (((c >= 'A') && (c <= 'Z'))
          || ((c >= 'a') && (c <= 'z')));
}

/* End of rec-parser.c */
