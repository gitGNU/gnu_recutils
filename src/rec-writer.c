/* -*- mode: C -*-
 *
 *       File:         rec-writer.c
 *       Date:         Sat Dec 26 22:47:16 2009
 *
 *       GNU recutils - Writer
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
rec_write_comment (rec_writer_t writer,
                   rec_comment_t comment)
{
  char *line;
  char *str;
  
  /* Every line in the comment is written preceded by a '#' character
     and postceded by a newline character.  */

  str = strdupa (rec_comment_text (comment));
  line = strsep (&str, "\n");
  do
    {
      if (!rec_writer_putc (writer, '#') 
          || !rec_writer_puts (writer, line)
          || !rec_writer_putc (writer, '\n'))
        {
          return false;
        }
    }
  while ((line = strsep (&str, "\n")));

  free (str);

  return true;
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

  if (!rec_write_field_name (writer, fname))
    {
      return false;
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

  if (!rec_writer_putc (writer, '\n'))
    {
      /* EOF on output */
      return false;
    }
  
  return true;
}

bool
rec_write_field_name (rec_writer_t writer,
                      rec_field_name_t field_name)
{
  int i;

  for (i = 0; i < rec_field_name_size (field_name); i++)
    {
      if (!rec_writer_puts (writer,
                            rec_field_name_get (field_name, i)))
        {
          return false;
        }
      if (!rec_writer_putc (writer, ':'))
        {
          return false;
        }
    }

  return true;
}

bool
rec_write_record (rec_writer_t writer,
                  rec_record_t record)
{
  bool ret;
  rec_field_t field;
  rec_comment_t comment;
  rec_record_elem_t elem;

  ret = true;

  elem = rec_record_null_elem ();
  while (rec_record_elem_p (elem = rec_record_next (record, elem)))
    {
      if (rec_record_elem_field_p (record, elem))
        {
          /* Write a field.  */
          field = rec_record_elem_field (elem);
          if (!rec_write_field (writer, field))
            {
              ret = false;
              break;
            }
        }
      else
        {
          /* Write a comment.  */
          comment = rec_record_elem_comment (elem);
          if (!rec_write_comment (writer, comment))
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
  bool ret;
  rec_record_t descriptor;
  bool wrote_descriptor;
  rec_rset_elem_t elem;
  rec_record_t record;
  size_t position;
  size_t descriptor_pos;

  ret = true;
  wrote_descriptor = false;
  position = 0;
  descriptor_pos = rec_rset_descriptor_pos (rset);

  elem = rec_rset_null_elem ();
  while (rec_rset_elem_p (elem = rec_rset_next (rset, elem)))
    {
      if (position != 0)
        {
          if (!rec_writer_putc (writer, '\n'))
            {
              ret = false;
            }
        }

      if (position == descriptor_pos)
        {
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
            }
        }
      
      if (rec_rset_elem_record_p (rset, elem))
        {
          ret = rec_write_record (writer, rec_rset_elem_record (elem));
        }
      else
        {
          ret = rec_write_comment (writer, rec_rset_elem_comment (elem));
        }
      
      if (!ret)
        {
          break;
        }
      
      position++;
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

char *
rec_write_field_name_str (rec_field_name_t field)
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
          rec_write_field_name (writer, field);
          rec_writer_destroy (writer);
        }

      fclose (stm);
    }
  
  return result;
}

char *
rec_write_comment_str (rec_comment_t comment)
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
          rec_write_comment (writer, comment);
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
