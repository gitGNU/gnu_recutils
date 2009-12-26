/* -*- mode: C -*- Time-stamp: "09/12/26 01:10:16 jemarch"
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
  "field name is too long",
  "out of memory",
  "too much parts in field name",
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

bool
rec_parse_field_name (rec_parser_t parser,
                      rec_field_name_t *fname)
{
  bool ret;
  char *name_part;
  int ci;
  char c;

  /* Sanity check */
  if (rec_parser_eof (parser)
      || rec_parser_error (parser))
    {
      return false;
    }

  *fname = rec_field_name_new ();
  if (*fname == NULL)
    {
      /* End of memory */
      parser->error = REC_PARSER_ENOMEM;
      return false;
    }

  ret = false;
  while (rec_parse_field_name_part (parser, &name_part))
    {
      if ((parser->eof)
          || (parser->error > 0))
        {
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
            }
          else
            {
              ret = true;
            }
        }
    }

  if (ret && (parser->error > 0))
    {
      /* Field names ends with:
       *
       * - A blank character or
       * - A newline or
       * - The end of the file
       */
      ci = rec_parser_getc (parser);
      if (ci != EOF)
        {
          c = (char) ci;
          if ((c == '\n') || (c == ' '))
            {
              parser->error = REC_PARSER_NOERROR;
            }
          else
            {
              if (!rec_parser_ungetc (parser, c))
                {
                  parser->error = REC_PARSER_EUNGETC;
                }
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

void
rec_parser_reset (rec_parser_t parser)
{
  parser->eof = false;
  parser->error = REC_PARSER_NOERROR;
}

bool
rec_parse_field (rec_parser_t parser,
                 rec_field_t *field)
{
  bool ret;
  rec_field_t new;
  rec_field_name_t field_name;
  char *field_value;

  /* Sanity check */
  if (rec_parser_eof (parser)
      || rec_parser_error (parser))
    {
      return false;
    }

  ret = rec_parse_field_name (parser, &field_name);
  if (ret)
    {
      ret = rec_parse_field_value (parser, &field_value);

      if (ret)
        {
          new = rec_field_new (field_name,
                               field_value);
          *field = new;
        }
      else
        {
          rec_field_name_destroy (field_name);
        }
    }

  return ret;
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
         (*str)[index++] = c;                           \
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
          ADD_TO_STR (c);
        }
      else
        {
          /* Parse error */
          parser->error = REC_PARSER_EFNAME;
          if (rec_parser_ungetc (parser, ci) != ci)
            {
              parser->error = REC_PARSER_EUNGETC;
            }
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
              ADD_TO_STR (c);
              if (parser->error > 0)
                {
                  /* error was set by ADD_TO_STR */
                  break;
                }
            }
          else if (c == ':')
            {
              /* End of token.  Consume the ':' and report success */
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

      if (parser->eof)
        {
          parser->error = REC_PARSER_EFNAME;
          ret = false;
        }
    }

  if (ret)
    {
      /* Resize the token */
      *str = realloc (*str, index);
      (*str)[index] = '\0';
    }

  return ret;
}

#define FIELD_VALUE_MAX 2048
#if defined(ADD_TO_STR)
#   undef ADD_TO_STR
#endif
#define ADD_TO_STR(c)                                   \
  do                                                    \
    {                                                   \
      if (index > FIELD_VALUE_MAX)                      \
        {                                               \
          /* Error: name too long */                    \
          parser->error = REC_PARSER_EFNAMETOOLONG;     \
          ret = false;                                  \
        }                                               \
      else                                              \
        {                                               \
         (*str)[index++] = c;                           \
        }                                               \
      } while (0)


static bool
rec_parse_field_value (rec_parser_t parser,
                       char **str)
{
  bool ret;
  int ci;
  int ci2;
  size_t index;
  char c;
  char c2;
  bool prev_newline;

  /* Sanity check */
  if (rec_parser_eof (parser)
      || rec_parser_error (parser))
    {
      return false;
    }

  prev_newline = false;
  ret = true;
  index = 0;
  *str = malloc (sizeof(char) * (FIELD_VALUE_MAX + 1));
  
  /* A field value is a sequence of zero or more ascii codes finished
   * with a newline character.
   *
   *  \$ is translated to nothing.
   *  $+ ? is translated to $.
   */
  while ((ci = rec_parser_getc (parser)) != EOF)
    {
      c = (char) ci;

      if ((prev_newline) && (c != '+'))
        {
          /* End of value */
          break;
        }

      if (c == '\\')
        {
          ci2 = rec_parser_getc (parser);
          if (ci2 == EOF)
            {
              parser->eof = true;
              ret = false;
              break;
            }
          else
            {
              c2 = (char) ci2;
              if (c2 == '\n')
                {
                  /* Consume both $\n chars not adding them to str =>
                     do nothing here. */
                }
              else
                {
                  /* Add \ and put back c2 */
                  ADD_TO_STR (c);
                  if (parser->error > 0)
                    {
                      break;
                    }

                  if (rec_parser_ungetc (parser, ci2)
                      != ci2)
                    {
                      parser->error = REC_PARSER_EUNGETC;
                      ret = false;
                      break;
                    }
                }
            }

          prev_newline = false;
        }
      else if (c == '+')
        {
          if (prev_newline)
            {
              /* Reduce \n+ ? to \n by ignoring the + ? */
              ci2 = rec_parser_getc (parser);
              
              if (ci2 == EOF)
                {
                  parser->eof = true;
                  ret = false;
                  break;
                }
              else
                {
                  c2 = (char) ci2;
                  /* If the look ahead character is a blank, skip it.
                     Otherwise put it back in the stream to be
                     processed in the next iteration. */
                  if (c2 != ' ')
                    {
                      if (rec_parser_ungetc (parser, ci2) != ci2)
                        {
                          parser->error = REC_PARSER_EUNGETC;
                          ret = false;
                          break;
                        }
                    }
                }
            }
          else
            {
              ADD_TO_STR (c);
              if (parser->error > 0)
                {
                  break;
                }
            }

          prev_newline = false;
        }
      else if (c == '\n')
        {
          ADD_TO_STR (c);
          if (parser->error > 0)
            {
              break;
            }
          prev_newline = true;
        }
      else
        {
          ADD_TO_STR (c);
          if (parser->error > 0)
            {
              break;
            }
          prev_newline = false;
        }
    }

  if (ret)
    {
      /* Resize the token */
      *str = realloc (*str, index);
      (*str)[index] = '\0';
    }

  return ret;
}

/* End of rec-parser.c */
