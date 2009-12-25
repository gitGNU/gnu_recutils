/* -*- mode: C -*- Time-stamp: "09/12/25 18:30:08 jemarch"
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
#include <stdarg.h>

#include <rec.h>

/* Static functions defined in this file */
static int rec_parser_getc (rec_parser_t parser);
static int rec_parser_ungetc (rec_parser_t parser, int ci);

static bool rec_expect (rec_parser_t parser, char *str);

static bool rec_parse_field_name_part (rec_parser_t parser, char **str);
static bool rec_parse_field_name (rec_parser_t parser, rec_field_name_t *fname);
static bool rec_parse_field_value (rec_parser_t parser, char **str);

static bool rec_parser_digit_p (char c);
static bool rec_parser_letter_p (char c);

/* Parser Data Structure
 *
 */

enum rec_parser_error_e
{
  REC_PARSER_NOERROR,
  REC_PARSER_ERROR,
  REC_PARSER_EUNGETC,
  REC_PARSER_EFNAME,
  REC_PARSER_EFNAMETOOLONG,
  REC_PARSER_ENOMEM,
  REC_PARSER_ETOOMUCHNAMEPARTS
};

struct rec_parser_s
{
  FILE *in; /* File stream used by the parser. */
  
  bool eof;
  enum rec_parser_error_e error;
  int line; /* Current line number. */
};

const char *rec_parser_error_strings[] =
{
  "no error (unused)",
  "unknown error",
  "unreading a character",
  "expected a field name",
  "out of memory",
  NULL /* Sentinel */
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
      parser->error = false;
      parser->line = 1;
    }

  return parser;
}

void
rec_parser_destroy (rec_parser_t parser)
{
  free (parser);
}

bool
rec_parser_eof (rec_parser_t parser)
{
  return parser->eof;
}

bool
rec_parser_error (rec_parser_t parser)
{
  return (parser->error != REC_PARSER_NOERROR);
}

void
rec_parser_perror (rec_parser_t parser,
                   char *fmt,
                   ...)
{
  va_list ap;

  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  fputs (": ", stderr);
  fputs (rec_parser_error_strings[parser->error], stderr);
  fputc ('\n', stderr);
  va_end (ap);
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
  else if (ci == '\n')
    {
      parser->line++;
    }

  return ci;
}

int
rec_parser_ungetc (rec_parser_t parser,
                   int ci)
{
  return ungetc (ci, parser->in);
}

static bool
rec_parser_digit_p (char c)
{
  return ((c >= '0') && (c <= '9'));
}

static bool
rec_parser_letter_p (char c)
{
  return (((c >= 'A') && (c <= 'Z'))
          || ((c >= 'a') && (c <= 'z')));
}

static bool
rec_expect (rec_parser_t parser,
            char *str)
{
  size_t str_size;
  size_t counter;
  bool found;
  int ci;
  char c;

  found = true;
  str_size = strlen (str);

  for (counter = 0;
       counter < str_size;
       counter++)
    {
      ci = rec_parser_getc (parser);
      if (ci == EOF)
        {
          /* EOF */
          found = false;
          parser->eof = true;
          break;
        }
      else
        {
          c = (char) ci;
          if (c != str[counter])
            {
              /* Not match */
              if (rec_parser_ungetc (parser, ci)
                  != ci)
                {
                  parser->error = REC_PARSER_EUNGETC;
                }
              found = false;
              break;
            }
        }
    }

  return found;
}

/* Macros used in 'rec_parse_field_name_part' */
#define STR_MAX_SIZE 255

#define ADD_TO_STR(c)                                   \
  do                                                    \
    {                                                   \
     if (index > STR_MAX_SIZE)                          \
        {                                               \
          /* Error: name too long */                    \
          parser->error = REC_PARSER_EFNAMETOOLONG;     \
          ret = false;                                  \
        }                                               \
      else                                              \
        {                                               \
         *str[index++] = c;                             \
         }                                              \
      } while (0)
  
static bool
rec_parse_field_name_part (rec_parser_t parser,
                           char **str)
{
  bool ret;
  int ci;
  size_t index;
  char c;

  ret = true;
  index = 0;
  *str = malloc (sizeof(char) * (STR_MAX_SIZE + 1));

  /* The syntax of a field name is described by the following regexp:
   *
   * [a-zA-Z%][a-zA-Z0-9_]*
   */

  /* [a-zA-Z%] */
  ci = rec_parser_getc (parser);
  if (ci == EOF)
    {
      ret = false;
    }
  else
    {
      c = (char) ci;

      if ((rec_parser_letter_p (c))
          || (c == '%'))
        {
          ADD_TO_STR(c);
        }
      else
        {
          /* Parse error */
          parser->error = REC_PARSER_EFNAME;
          ret = false;
        }
    }

  /* [a-zA-Z0-9_]* */
  if (ret)
    {
      while ((ci = rec_parser_getc (parser)) != EOF)
        {
          c = (char) ci;

          if (rec_parser_letter_p (c)
              || rec_parser_digit_p (c)
              || (c == '_'))
            {
              ADD_TO_STR(c);
            }
          else if (c == ':')
            {
              /* End of token.  Consume the ':' and report success */
              ret = true;
              break;
            }
          else
            {
              /* Parse error */
              parser->error = REC_PARSER_EFNAME;
              ret = false;
              break;
            }
        }
    }

  if (ret)
    {
      /* Resize the token */
      *str = realloc (*str, index);
      *str[index] = '\0';
    }

  return ret;
}

static bool
rec_parse_field_name (rec_parser_t parser,
                      rec_field_name_t *fname)
{
  bool ret;
  char *name_part;

  *fname = rec_field_name_new ();
  if (*fname == NULL)
    {
      /* End of memory */
      parser->error = REC_PARSER_ENOMEM;
      return false;
    }

  ret = true;
  while (rec_parse_field_name_part (parser, &name_part))
    {
      if ((parser->eof)
          || (parser->error > 0))
        {
          ret = false;
          break;
        }
      else
        {
          /* Add this name part to the name */
          if (!rec_field_name_set (*fname,
                                   rec_field_name_size (*fname),
                                   name_part))
            {
              /* Too much parts */
              parser->error = REC_PARSER_ETOOMUCHNAMEPARTS;
              ret = false;
            }
        }
    }

  if (!ret)
    {
      /* Free used memory */
      rec_field_name_destroy (*fname);
    }

  return ret;
}

/* End of rec-parser.c */
