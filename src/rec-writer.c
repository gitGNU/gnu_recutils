/* -*- mode: C -*- Time-stamp: "10/01/15 11:42:14 jemarch"
 *
 *       File:         rec-writer.c
 *       Date:         Sat Dec 26 22:47:16 2009
 *
 *       GNU recutils - Writer
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

#include <rec.h>

#include <malloc.h>
#include <string.h>

#include <rec.h>

/*
 * Static functions defined in this file
 */
static bool rec_writer_putc (rec_writer_t writer, char c);
static bool rec_writer_puts (rec_writer_t writer, const char *s);

/* Writer Data Structure
 *
 */
struct rec_writer_s
{
  FILE *out; /* File stream used by the writer. */

  bool eof;
  int line;  /* Current line number. */
};

rec_writer_t
rec_writer_new (FILE *out)
{
  rec_writer_t new;

  new = malloc (sizeof(struct rec_writer_s));
  if (new)
    {
      new->out = out;
      new->line = 1;
      new->eof = false;
    }

  return new;
}

void
rec_writer_destroy (rec_writer_t writer)
{
  fflush (writer->out);
  free (writer);
}

bool
rec_write_field (rec_writer_t writer,
                 rec_field_t field)
{
  int i;
  size_t pos;
  rec_field_name_t fname;
  const char *fvalue;

  /* Write the field name */
  fname = rec_field_name (field);
  if (rec_field_name_size (fname) == 0)
    {
      /* This is a comment */
      if (!rec_writer_putc (writer, '#'))
        {
          return false;
        }
      if (!rec_writer_puts (writer,
                            rec_field_value (field)))
        {
          return false;
        }
    }
  else
    {
      for (i = 0; i < rec_field_name_size (fname); i++)
        {
          if (!rec_writer_puts (writer,
                                rec_field_name_get (fname, i)))
            {
              return false;
            }
          if (!rec_writer_putc (writer, ':'))
            {
              return false;
            }
        }
      
      if (!rec_writer_putc (writer, ' '))
        {
          return false;
        }

      /* Write the field value */
      fvalue = rec_field_value (field);
      for (pos = 0; pos < strlen (fvalue); pos++)
        {
          if (fvalue[pos] == '\n')
            {
              if (!rec_writer_puts (writer, "\n+ "))
                {
                  /* EOF on output */
                  return false;
                }
            }
          else
            {
              if (!rec_writer_putc (writer, fvalue[pos]))
                {
                  /* EOF on output */
                  return false;
                }
            }
        }
    }

  if (!rec_writer_putc (writer, '\n'))
    {
      /* EOF on output */
      return false;
    }
  
  return true;
}

bool
rec_write_record (rec_writer_t writer,
                  rec_record_t record)
{
  bool ret;
  rec_field_t field;
  int i;

  ret = true;

  if (rec_record_comment_p (record))
    {
      if (!rec_writer_putc (writer, '#'))
        {
          ret = false;
        }
      if (!rec_writer_puts (writer,
                            rec_record_comment (record)))
        {
          ret = false;
        }
      if (!rec_writer_putc (writer, '\n'))
        {
          ret = false;
        }
    }
  else
    {
      for (i = 0; i < rec_record_size (record); i++)
        {
          field = rec_record_get_field (record, i);
          if (!rec_write_field (writer, field))
            {
              ret = false;
              break;
            }
        }
    }

  return ret;
}

bool
rec_write_rset (rec_writer_t writer,
                rec_rset_t rset)
{
  int i;
  bool ret;
  rec_record_t descriptor;
  bool wrote_descriptor;

  ret = true;
  wrote_descriptor = false;

  descriptor = rec_rset_descriptor (rset);
  if (descriptor
      && (!(wrote_descriptor = rec_write_record (writer, rec_rset_descriptor (rset)))))
    {
      ret = false;
    }
  else
    {
      if (wrote_descriptor)
        {
          if (!rec_writer_putc (writer, '\n'))

            {
              ret = false;
            }
        }

      for (i = 0; i < rec_rset_size (rset); i++)
        {
          rec_record_t record;
          record = rec_rset_get_record (rset, i);
  
          if ((i != 0)
              && ((rec_record_comment_p (record))
                  && ((i == 0)
                      || (!rec_record_comment_p (rec_rset_get_record (rset,
                                                                      i - 1)))))
              && (!rec_writer_putc (writer, '\n')))

            {
              ret = false;
              break;
            }

          if (!rec_write_record (writer, record))
            {
              ret = false;
              break;
            }
        }
    }

  return ret;
}

bool
rec_write_db (rec_writer_t writer,
              rec_db_t db)
{
  bool ret;
  int i;
  rec_rset_t rset;

  ret = true;
  for (i = 0; i < rec_db_size (db); i++)
    {
      if (i != 0)
        {
          if (!rec_writer_putc (writer, '\n'))
            {
              ret = false;
              break;
            }          
        }
      if (!rec_write_rset (writer,
                           rec_db_get_rset (db, i)))
        {
          ret = false;
          break;
        }
    }

  return ret;
}

char *
rec_write_field_str (rec_field_t field)
{
  rec_writer_t writer;
  char *result;
  size_t result_size;
  FILE *stm;
  
  result = NULL;
  stm = open_memstream (&result, &result_size);
  if (stm)
    {
      writer = rec_writer_new (stm);

      if (writer)
        {
          rec_write_field (writer, field);
          rec_writer_destroy (writer);
        }

      fclose (stm);
    }
  
  return result;
}

/*
 * Private functions
 */

static bool
rec_writer_putc (rec_writer_t writer, char c)
{
  return (fputc (c, writer->out) != EOF);
}

static bool
rec_writer_puts (rec_writer_t writer, const char *s)
{
  return (fputs (s, writer->out) != EOF);
}

/* End of rec-writer.c */
