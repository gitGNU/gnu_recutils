/* -*- mode: C -*- Time-stamp: "10/01/11 20:34:34 jemarch"
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

/*
 * Static functions defined in this file
 */
static int rec_parser_getc (rec_parser_t parser);
static int rec_parser_ungetc (rec_parser_t parser, int ci);

static bool rec_expect (rec_parser_t parser, char *str);

static bool rec_parse_field_name_part (rec_parser_t parser, char **str);
static bool rec_parse_field_value (rec_parser_t parser, char **str);

static bool rec_parse_comment (rec_parser_t parser, char **str);

static bool rec_parser_digit_p (char c);
static bool rec_parser_letter_p (char c);

/* Buffer management */

#define REC_PARSER_BUF_STEP 512

struct rec_parser_buf_s
{
  char *data;
  size_t used;
  size_t size;
};

typedef struct rec_parser_buf_s *rec_parser_buf_t;

static rec_parser_buf_t rec_parser_buf_new ();
static bool rec_parser_buf_add (rec_parser_buf_t buf,
                                char c);
static void rec_parser_buf_rewind (rec_parser_buf_t buf, int n);
static char *rec_parser_buf_data (rec_parser_buf_t buf);
static void rec_parser_buf_adjust (rec_parser_buf_t buf);
static void rec_parser_buf_destroy (rec_parser_buf_t buf);

/* Parser Data Structure
 *
 */

enum rec_parser_error_e
{
  REC_PARSER_NOERROR,
  REC_PARSER_ERROR,
  REC_PARSER_EUNGETC,
  REC_PARSER_EFNAME,
  REC_PARSER_ENOMEM,
  REC_PARSER_ETOOMUCHNAMEPARTS,
  REC_PARSER_ECOMMENT,
  REC_PARSER_EFIELD,
  REC_PARSER_ERECORD
};

struct rec_parser_s
{
  FILE *in; /* File stream used by the parser. */
  
  rec_record_t prev_descriptor;

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
  "too much parts in field name",
  "expected a comment",
  "expected a field",
  "expected a record",
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
      parser->prev_descriptor = NULL;
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
  char *number_str;

  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  fputs (":", stderr);
  /* XXX: replace the following line with a calculation of the number
     of digits of a given number.  Sorry, I am a bit drunk while
     writing this and cannot think clearly :D -jemarch */
  number_str = malloc (10); /* 10 is a big arbitrary number */
  sprintf(number_str, "%d", parser->line - 1);
  fputs (number_str, stderr);
  fputs (": error: ", stderr);
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
                  break;
                }
              else
                {
                  rec_parser_ungetc (parser, c);
                }
            }
          else
            {
              break;
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

bool
rec_parse_record (rec_parser_t parser,
                  rec_record_t *record)
{
  rec_record_t new;
  rec_field_t field;
  rec_field_name_t field_name;
  bool ret, field_p;
  int ci;
  char c;
  char *comment_value;

  /* Sanity check */
  if (rec_parser_eof (parser)
      || rec_parser_error (parser))
    {
      return false;
    }

  new = rec_record_new ();
  if (!new)
    {
      parser->error = REC_PARSER_ENOMEM;
      return false;
    }

  /* A record is a list of mixed fields and comments, containing at
   * least one field starting it:
   *
   *  FIELD (FIELD|COMMENT)*
   */
  if (rec_parse_field (parser, &field))
    {
      /* Add the field to the record */
      if (!rec_record_insert_field (new,
                                    field,
                                    rec_record_size (new)))
        {
          parser->error = REC_PARSER_ENOMEM;
          return false;
        }
    }
  else
    {
      /* Expected a field */
      parser->error = REC_PARSER_EFIELD;
      return false;
    }

  ret = true;
  while ((ci = rec_parser_getc (parser)) != EOF)
    {
      c = (char) ci;

      if (c == '#')
        {
          rec_parser_ungetc (parser, ci);
          if (rec_parse_comment (parser, &comment_value))
            {
              /* Insert the comment fake field:
               *
               *  empty name -> comment value
               */
              field_name = rec_field_name_new ();
              field = rec_field_new (field_name,
                                     comment_value);
              if (!rec_record_insert_field (new,
                                            field,
                                            rec_record_size (new)))
                {
                  parser->error = REC_PARSER_ENOMEM;
                  ret = false;
                  break;
                }
            }
        }
      else if (c == '\n')
        {
          /* End of record */
          break;
        }
      else
        {
          /* Try to parse a field */
          rec_parser_ungetc (parser, ci);
          if (rec_parse_field (parser, &field))
            {
              /* Add the field to the record */
              if (!rec_record_insert_field (new,
                                            field,
                                            rec_record_size (new)))
                {
                  parser->error = REC_PARSER_ENOMEM;
                  ret = false;
                  break;
                }
            }
          else
            {
              /* Parse error: field expected */
              parser->error = REC_PARSER_EFIELD;
              ret = false;
              break;
            }
        }
    }

  if (ret)
    {
      *record = new;
    }
  else
    {
      rec_record_destroy (new);
      *record = NULL;
    }

  return ret;
}

bool
rec_parse_rset (rec_parser_t parser,
                rec_rset_t *rset)
{
  bool ret;
  int ci;
  char c;
  rec_rset_t new;
  rec_record_t record;
  rec_field_name_t rec_fname;
  char *comment_value;

  ret = false;

  if ((rec_fname = rec_field_name_new ()) == NULL)
    {
      /* Out of memory */
      parser->error = REC_PARSER_ENOMEM;
      return false;
    }
  
  rec_field_name_set (rec_fname, 0, "%rec");

  if ((new = rec_rset_new ()) == NULL)
    {
      /* Out of memory */
      parser->error = REC_PARSER_ENOMEM;
      rec_field_name_destroy (rec_fname);
      return false;
    }

  /* Set the descriptor for this record set */
  rec_rset_set_descriptor (new, parser->prev_descriptor);
  parser->prev_descriptor = NULL;

  while ((ci = rec_parser_getc (parser)) != EOF)
    {
      c = (char) ci;

      /* Skip newline characters and blanks */
      if ((c == '\n') || (c == ' '))
        {
        }
      /* Skip comments */
      else if (c == '#')
        {
          rec_parser_ungetc (parser, c);
          rec_parse_comment (parser, &comment_value);
        }
      else
        {
          /* Try to parse a record */
          rec_parser_ungetc (parser, c);
          if (rec_parse_record (parser, &record))
            {
              /* Check if the parsed record is a descriptor.  In that
                 case, set it as the previous descriptor in the parser
                 state and stop parsing.  Otherwise, add the record to
                 the current record set. */
              if (rec_record_field_p (record,
                                      rec_fname))
                {
                  if ((rec_rset_size (new) == 0) &&
                      (!rec_rset_descriptor (new)))
                    {
                      /* Special case: the first record found in the
                         input stream is a descriptor. */
                      rec_rset_set_descriptor (new, record);
                    }
                  else
                    {
                      parser->prev_descriptor = record;
                      ret = true;
                      break;
                    }
                }
              else
                {
                  if (!rec_rset_insert_record (new,
                                               record,
                                               rec_rset_size (new)))
                    {
                      /* Parse error: out of memory */
                      parser->error = REC_PARSER_ENOMEM;
                      break;
                    }
                }
            }
          else
            {
              /* Parse error */
              break;
            }
        }
    }

  if (rec_rset_descriptor (new)
      || (rec_rset_size (new) > 0))
    {
      ret = true;
    }

  if (ret)
    {
      *rset = new;
    }
  else
    {
      rec_rset_destroy (new);
      *rset = NULL;
    }

  return ret;
}

rec_field_name_t
rec_parse_field_name_str (char *str)
{
  rec_parser_t parser;
  rec_field_name_t field_name;
  FILE *stm;

  field_name = NULL;
  stm = fmemopen (str, strlen (str), "r");
  if (stm)
    {
      parser = rec_parser_new (stm);
      if (parser)
        {
          rec_parse_field_name (parser, &field_name);
          rec_parser_destroy (parser);
        }

      fclose (stm);
    }
  
  return field_name;
}

/*
 * Private functions
 */

static int
rec_parser_getc (rec_parser_t parser)
{
  int ci;

  ci = getc (parser->in);
  if (ci == EOF)
    {
      parser->eof = true;
    }
  else if (((char) ci) == '\n')
    {
      parser->line++;
    }

  return ci;
}

int
rec_parser_ungetc (rec_parser_t parser,
                   int ci)
{
  int res;
  
  if (((char) ci) == '\n')
    {
      parser->line--;
    }

  res = ungetc (ci, parser->in);
  if (res != ci)
    {
      parser->error = REC_PARSER_EUNGETC;
    }

  return res;
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
              rec_parser_ungetc (parser, ci);
              found = false;
              break;
            }
        }
    }

  return found;
}

static bool
rec_parse_field_name_part (rec_parser_t parser,
                           char **str)
{
  bool ret;
  int ci;
  size_t index;
  char c;
  rec_parser_buf_t buf;

  ret = true;
  index = 0;
  buf = rec_parser_buf_new ();
  if (!buf)
    {
      /* Out of memory */
      parser->error = REC_PARSER_ENOMEM;
      return false;
    }

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
          if (!rec_parser_buf_add (buf, c))
            {
              /* Out of memory */
              parser->error = REC_PARSER_ENOMEM;
              return false;
            }
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
              if (!rec_parser_buf_add (buf, c))
                {
                  /* Out of memory */
                  parser->error = REC_PARSER_ENOMEM;
                  return false;
                }
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
      rec_parser_buf_adjust (buf);
      *str = rec_parser_buf_data (buf);
    }

  rec_parser_buf_destroy (buf);

  return ret;
}

/* XXX: the following function fails in the case:
 
   foo:
   EOF
*/
static bool
rec_parse_field_value (rec_parser_t parser,
                       char **str)
{
  bool ret;
  int ci, ci2;
  char c, c2;
  size_t index;
  bool prev_newline;
  rec_parser_buf_t buf;

  /* Sanity check */
  if (rec_parser_eof (parser)
      || rec_parser_error (parser))
    {
      return false;
    }

  prev_newline = false;
  ret = true;
  index = 0;
  buf = rec_parser_buf_new ();
  if (!buf)
    {
      /* Out of memory */
      parser->error = REC_PARSER_ENOMEM;
      return false;
    }
  
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
          rec_parser_ungetc (parser, ci);
          rec_parser_buf_rewind (buf, 1);
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
                  if (!rec_parser_buf_add (buf, c))
                    {
                      /* Out of memory */
                      parser->error = REC_PARSER_ENOMEM;
                      return false;
                    }

                  if (parser->error > 0)
                    {
                      break;
                    }

                  if (rec_parser_ungetc (parser, ci2)
                      != ci2)
                    {
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
                     Otherwise put it back in the stream so it will be
                     processed in the next iteration. */
                  if (c2 != ' ')
                    {
                      if (rec_parser_ungetc (parser, ci2) != ci2)
                        {
                          ret = false;
                          break;
                        }
                    }
                }
            }
          else
            {
              if (!rec_parser_buf_add (buf, c))
                {
                  /* Out of memory */
                  parser->error = REC_PARSER_ENOMEM;
                  return false;
                }

              if (parser->error > 0)
                {
                  break;
                }
            }

          prev_newline = false;
        }
      else if (c == '\n')
        {
          if (!rec_parser_buf_add (buf, c))
            {
              /* Out of memory */
              parser->error = REC_PARSER_ENOMEM;
              return false;
            }

          if (parser->error > 0)
            {
              break;
            }
          prev_newline = true;
        }
      else
        {
          if (!rec_parser_buf_add (buf, c))
            {
              /* Out of memory */
              parser->error = REC_PARSER_ENOMEM;
              return false;
            }

          if (parser->error > 0)
            {
              break;
            }
          prev_newline = false;
        }
    }

  if (ret)
    {
      if (rec_parser_eof (parser)
          && (c == '\n'))
        {
          /* Special case: field just before EOF */
          rec_parser_buf_rewind (buf, 1);
        }

      /* Resize the token */
     rec_parser_buf_adjust (buf);
      *str = rec_parser_buf_data (buf);
    }

  rec_parser_buf_destroy (buf);

  return ret;
}

static rec_parser_buf_t
rec_parser_buf_new ()
{
  rec_parser_buf_t new;

  new = malloc (sizeof (struct rec_parser_buf_s));
  if (new)
    {
      new->data = malloc (REC_PARSER_BUF_STEP);
      new->size = REC_PARSER_BUF_STEP;
      new->used = 0;

      if (!new->data)
        {
          free (new);
          new = NULL;
        }
    }

  return new;
}

static void
rec_parser_buf_destroy (rec_parser_buf_t buf)
{
  /* Don't deallocate buf->data */
  free (buf);
}

static void
rec_parser_buf_rewind (rec_parser_buf_t buf,
                       int n)
{
  if ((buf->used - n) >= 0)
    {
      buf->used = buf->used - n;
    }
}

static bool
rec_parser_buf_add (rec_parser_buf_t buf,
                    char c)
{
  bool ret;

  ret = true;

  if ((buf->used + 1) > buf->size)
    {
      /* Allocate a new block */
      buf->size = buf->size + REC_PARSER_BUF_STEP;
      buf->data = realloc (buf->data, buf->size);

      if (!buf->data)
        {
          /* Not enough memory.
           * REC_PARSER_BUF_STEP should not be 0. */
          ret = false;
        }
    }

  if (ret)
    {
      /* Add the character */
      buf->data[buf->used++] = c;
    }

  return ret;
}

static char *
rec_parser_buf_data (rec_parser_buf_t buf)
{
  return buf->data;
}

static void
rec_parser_buf_adjust (rec_parser_buf_t buf)
{
  /* The following realloc fails and I don't know why -jemarch */
  /* buf->data = realloc (buf->data, buf->used); */
  buf->data[buf->used] = '\0';
}

static bool
rec_parse_comment (rec_parser_t parser, char **str)
{
  bool ret;
  rec_parser_buf_t buf;
  int ci;
  char c;

  ret = false;
  buf = rec_parser_buf_new ();

  /* Comments start at the beginning of line and span until the first
   * \n character or EOF.
   */
  if (rec_expect (parser, "#"))
    {
      while ((ci = rec_parser_getc (parser)) != EOF)
        {
          c = (char) ci;

          if (c == '\n')
            {
              break;
            }
          else
            {
              if (!rec_parser_buf_add (buf, c))
                {
                  /* Out of memory */
                  parser->error = REC_PARSER_ENOMEM;
                  return false;
                }
            }
        }
      
      ret = true;
    }

  if (ret)
    {
      /* Resize the token */
      rec_parser_buf_adjust (buf);
      *str = rec_parser_buf_data (buf);
    }
  else
    {
      *str = NULL;
    }

  rec_parser_buf_destroy (buf);
  return ret;
}

/* End of rec-parser.c */
