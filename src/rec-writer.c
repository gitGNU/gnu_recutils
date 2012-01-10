/* -*- mode: C -*-
 *
 *       File:         rec-writer.c
 *       Date:         Sat Dec 26 22:47:16 2009
 *
 *       GNU recutils - Writer
 *
 */

/* Copyright (C) 2009, 2010, 2011, 2012 Jose E. Marchesi */

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

#include <rec.h>

#include <stdlib.h>
#include <string.h>

#include <rec.h>
#include <rec-utils.h>

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
  FILE *file_out;    /* File stream used by the writer. */
  rec_buf_t buf_out; /* Growable buffer used by the writer. */

  bool eof;
  int line;  /* Current line number. */

  char *password;    /* Password to use to decrypt encrypted
                        fields.  */
};

static void
rec_writer_new_common (rec_writer_t writer)
{
  writer->file_out = NULL;
  writer->buf_out = NULL;
  writer->line = 1;
  writer->eof = false;
  writer->password = NULL;
}

rec_writer_t
rec_writer_new (FILE *file_out)
{
  rec_writer_t new;

  new = malloc (sizeof(struct rec_writer_s));
  if (new)
    {
      rec_writer_new_common (new);
      new->file_out = file_out;
    }

  return new;
}

rec_writer_t
rec_writer_new_str (char **str, size_t *str_size)
{
  rec_writer_t new;

  new = malloc (sizeof(struct rec_writer_s));
  if (new)
    {
      rec_writer_new_common (new);
      new->buf_out = rec_buf_new (str, str_size);
    }

  return new;
}

void
rec_writer_destroy (rec_writer_t writer)
{
  if (writer->file_out)
    {
      fflush (writer->file_out);
    }
  if (writer->buf_out)
    {
      rec_buf_close (writer->buf_out);
    }

/*  free (writer->password); */
  free (writer);
}

bool
rec_write_comment (rec_writer_t writer,
                   rec_comment_t comment,
                   rec_writer_mode_t mode)
{
  char *line;
  char *str;
  char *orig_str;
  size_t i;
  
  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_puts (writer, "(comment "))
        {
          return false;
        }
      if (!rec_writer_putc (writer, '"'))
        {
          return false;
        }

      str = rec_comment_text (comment);
      for (i = 0; i < strlen (str); i++)
        {
          if (str[i] == '\n')
            {
              if (!rec_writer_puts (writer, "\\n"))
                {
                  return false;
                }
            }
          else
            {
              if (!rec_writer_putc (writer, str[i]))
                {
                  return false;
                }
            }
        }

      if (!rec_writer_puts (writer, "\")"))
        {
          return false;
        }
    }
  else
    {
      /* Every line in the comment is written preceded by a '#' character
         and postceded by a newline character.  */
      
      str = strdup (rec_comment_text (comment));
      orig_str = str; /* Save a pointer to str to deallocate it later,
                         since strsep will modify the str
                         variable.  */
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

      free (orig_str);
    }

  return true;
}

bool
rec_write_field (rec_writer_t writer,
                 rec_field_t field,
                 rec_writer_mode_t mode)
{
  return rec_write_field_with_rset (writer,
                                    NULL,
                                    field,
                                    mode);
}

bool
rec_write_field_with_rset (rec_writer_t writer,
                           rec_rset_t rset,
                           rec_field_t field,
                           rec_writer_mode_t mode)
{
  size_t pos;
  rec_field_name_t fname;
  const char *fvalue;

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_puts (writer, "(field "))
        {
          return false;
        }
      if (!rec_writer_puts (writer, rec_field_char_location_str (field)))
        {
          return false;
        }
      if (!rec_writer_putc (writer, ' '))
        {
          return false;
        }
    }

  /* Write the field name */
  fname = rec_field_name (field);

  if (!rec_write_field_name (writer, fname, mode))
    {
      return false;
    }
  
  /* Write the field value */
  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_putc (writer, ' '))
        {
          return false;
        }
  
      if (!rec_writer_putc (writer, '"'))
        {
          return false;
        }
    }

#if defined REC_CRYPT_SUPPORT
  if (writer->password)
    {
      rec_decrypt_field (field, writer->password);
    }
#endif /* REC_CRYPT_SUPPORT */

  fvalue = rec_field_value (field);

  if ((strlen (fvalue) > 0) && (mode != REC_WRITER_SEXP))
    {
      if (!rec_writer_putc (writer, ' '))
        {
          return false;
        }
    }

  for (pos = 0; pos < strlen (fvalue); pos++)
    {
      if (fvalue[pos] == '\n')
        {
          if (mode == REC_WRITER_SEXP)
            {
              if (!rec_writer_puts (writer, "\\n"))
                {
                  return false;
                }
            }
          else
            {
              if (!rec_writer_puts (writer, "\n+ "))
                {
                  /* EOF on output */
                  return false;
                }
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

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_putc (writer, '"'))
        {
          return false;
        }
    }

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_puts (writer, ")\n"))
        {
          return false;
        }
    }
  else
    {
      if (!rec_writer_putc (writer, '\n'))
        {
          /* EOF on output */
          return false;
        }
    }

  return true;
}

bool
rec_write_field_name (rec_writer_t writer,
                      rec_field_name_t field_name,
                      rec_writer_mode_t mode)
{
  /* Field names can be written in several formats, according to the
   * desired mode:
   *
   * REC_WRITER_NORMAL
   *    The field name is written in rec format. i.e. NP:NP:...NP:
   * REC_WRITER_SEXP
   *    The field name is a list of strings: ("NP" "NP" ... "NP")
  */

  int i;

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_putc (writer, '('))
        {
          return false;
        }
    }
 
 for (i = 0; i < rec_field_name_size (field_name); i++)
    {
      if (mode == REC_WRITER_SEXP)
        {
          if ((i != 0) && (!rec_writer_putc (writer, ' ')))
            {
              return false;
            }
          if (!rec_writer_putc (writer, '"'))
            {
              return false;
            }
        }

      if (!rec_writer_puts (writer,
                            rec_field_name_get (field_name, i)))
        {
          return false;
        }

      if (mode == REC_WRITER_SEXP)
        {
          if (!rec_writer_putc (writer, '"'))
            {
              return false;
            }
        }
      else
        {
          if (!rec_writer_putc (writer, ':'))
            {
              return false;
            }
        }
    }

 if (mode == REC_WRITER_SEXP)
   {
     if (!rec_writer_putc (writer, ')'))
       {
         return false;
       }
   }

  return true;
}

bool
rec_write_record (rec_writer_t writer,
                  rec_record_t record,
                  rec_writer_mode_t mode)
{
  return rec_write_record_with_rset (writer,
                                     NULL,
                                     record,
                                     mode);
}

bool
rec_write_record_with_rset (rec_writer_t writer,
                            rec_rset_t rset,
                            rec_record_t record,
                            rec_writer_mode_t mode)
{
  bool ret;
  rec_field_t field;
  rec_comment_t comment;
  rec_mset_iterator_t iter;
  rec_mset_elem_t elem;
  char *data;

  ret = true;

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_puts (writer, "(record "))
        {
          return false;
        }
      if (!rec_writer_puts (writer, rec_record_char_location_str (record)))
        {
          return false;
        }
      if (!rec_writer_puts (writer, " (\n"))
        {
          return false;
        }
    }

  iter = rec_mset_iterator (rec_record_mset (record));
  while (rec_mset_iterator_next (&iter, MSET_ANY, (const void **) &data, &elem))
    {
      if (rec_mset_elem_type (elem) == MSET_FIELD)
        {
          /* Write a field.  */
          field = (rec_field_t) data;
          if (!rec_write_field_with_rset (writer,
                                          rset,
                                          field,
                                          mode))
            {
              ret = false;
              break;
            }
        }
      else
        {
          /* Write a comment.  */
          comment = (rec_comment_t) data;
          if (!rec_write_comment (writer, comment, mode))
            {
              ret = false;
              break;
            }
        }
    }

  rec_mset_iterator_free (&iter);

  if (mode == REC_WRITER_SEXP)
    {
      if (!rec_writer_puts (writer, "))\n"))
        {
          return false;
        }
    }

  return ret;
}

bool
rec_write_record_with_fex (rec_writer_t writer,
                           rec_record_t record,
                           rec_fex_t fex,
                           rec_writer_mode_t mode,
                           bool print_values_p,
                           bool print_in_a_row_p)
{
  rec_fex_elem_t elem;
  rec_field_t field;
  rec_field_name_t field_name;
  int i, j, min, max;
  size_t fex_size;
  size_t written_fields = 0;

  fex_size = rec_fex_size (fex);
  for (i = 0; i < fex_size; i++)
    {
      elem = rec_fex_get (fex, i);

      field_name = rec_fex_elem_field_name (elem);
      min = rec_fex_elem_min (elem);
      max = rec_fex_elem_max (elem);

      if ((min == -1) && (max == -1))
        {
          /* Print all the fields with that name.  */
          min = 0;
          max = rec_record_get_num_fields_by_name (record, field_name);
        }
      else if (max == -1)
        {
          /* Print just one field: Field[min].  */
          max = min + 1;
        }
      else
        {
          /* Print the interval min..max, max inclusive.  */
          max++;
        }

      for (j = min; j < max; j++)
        {
          if (!(field = rec_record_get_field_by_name (record, field_name, j)))
            {
              continue;
            }

          written_fields++;

#if defined REC_CRYPT_SUPPORT
          if (writer->password)
            {
              rec_decrypt_field (field, writer->password);
            }
#endif /* REC_CRYPT_SUPPORT */

          if (print_values_p)
            {
              /* Write just the value of the field.  */
              rec_writer_puts (writer, rec_field_value (field));
              if (print_in_a_row_p)
                {
                  if ((j < (max - 1)) || (i < (fex_size - 1)))
                    {
                      rec_writer_putc (writer, ' ');
                    }
                }
              else
                {
                  if ((j < (max - 1)) || (i < (fex_size - 1)))
                    {
                      rec_writer_putc (writer, '\n');
                    }
                }
            }
          else
            {
              /* Print the field according to the requested mode. */
              rec_write_field (writer, field, mode);
            }
        }
    }

  if (print_values_p && (written_fields > 0))
    {
      rec_writer_putc (writer, '\n');
    }

  return true;
}

bool
rec_write_rset (rec_writer_t writer,
                rec_rset_t rset)
{
  bool ret;
  rec_record_t descriptor;
  bool wrote_descriptor;
  size_t position;
  size_t descriptor_pos;
  rec_mset_iterator_t iter;
  rec_mset_elem_t elem;
  void *data;
  
  ret = true;
  wrote_descriptor = false;
  position = 0;
  descriptor_pos = rec_rset_descriptor_pos (rset);
  descriptor = rec_rset_descriptor (rset);

  /* Special case: record set containing just the record
     descriptor.  */
  if ((rec_rset_num_elems (rset) == 0) && descriptor)
    {
      rec_write_record (writer,
                        rec_rset_descriptor (rset),
                        REC_WRITER_NORMAL);
      return true;
    }

  iter = rec_mset_iterator (rec_rset_mset (rset));
  while (rec_mset_iterator_next (&iter, MSET_ANY, (const void **) &data, &elem))
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
          if (descriptor 
              && (!(wrote_descriptor = rec_write_record (writer,
                                                         rec_rset_descriptor (rset),
                                                         REC_WRITER_NORMAL))))
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
      
      if (rec_mset_elem_type (elem) == MSET_RECORD)
        {
          ret = rec_write_record (writer,
                                  (rec_record_t) data,
                                  REC_WRITER_NORMAL);
        }
      else
        {
          ret = rec_write_comment (writer,
                                   (rec_comment_t) data,
                                   REC_WRITER_NORMAL);
        }
      
      if (!ret)
        {
          break;
        }
      
      position++;
    }

  rec_mset_iterator_free (&iter);

  /* Special case:
   *
   * # comment 1
   * 
   * # comment 2
   * ...
   * %rec: foo
   */
  if (!wrote_descriptor
      && (descriptor_pos >= rec_rset_num_elems (rset))
      && rec_rset_descriptor (rset))
    {
      if (!rec_writer_putc (writer, '\n'))
        {
          ret = false;
        }
      if (!rec_write_record (writer,
                             rec_rset_descriptor (rset),
                             REC_WRITER_NORMAL))
        {
          ret = false;
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
rec_write_field_str (rec_field_t field,
                     rec_writer_mode_t mode)
{
  rec_writer_t writer;
  char *result;
  size_t result_size;
  
  result = NULL;
  writer = rec_writer_new_str (&result, &result_size);
  if (writer)
    {
      rec_write_field (writer, field, mode);
      rec_writer_destroy (writer);
    }
  
  return result;
}

char *
rec_write_field_name_str (rec_field_name_t field,
                          rec_writer_mode_t mode)
{
  rec_writer_t writer;
  char *result;
  size_t result_size;
  
  result = NULL;
  writer = rec_writer_new_str (&result, &result_size);
  if (writer)
    {
      rec_write_field_name (writer, field, mode);
      rec_writer_destroy (writer);
    }
  
  return result;
}

char *
rec_write_comment_str (rec_comment_t comment,
                       rec_writer_mode_t mode)
{
  rec_writer_t writer;
  char *result;
  size_t result_size;
  
  result = NULL;
  writer = rec_writer_new_str (&result, &result_size);
  if (writer)
    {
      rec_write_comment (writer, comment, mode);
      rec_writer_destroy (writer);
    }
  
  return result;
}

void
rec_writer_set_password (rec_writer_t writer,
                         char *password)
{
  free (writer->password);
  writer->password = strdup (password);
}

/*
 * Private functions
 */

static bool
rec_writer_putc (rec_writer_t writer, char c)
{
  bool ret;

  ret = false;
  if (writer->file_out)
    {
      ret = (fputc (c, writer->file_out) != EOF);
    }
  if (writer->buf_out)
    {
      ret = (rec_buf_putc (c, writer->buf_out) != EOF);
    }

  return ret;
}

static bool
rec_writer_puts (rec_writer_t writer, const char *s)
{
  bool ret;

  ret = false;
  if (writer->file_out)
    {
      ret = (fputs (s, writer->file_out) != EOF);
    }
  if (writer->buf_out)
    {
      ret = (rec_buf_puts (s, writer->buf_out) != EOF);
    }

  return ret;
}

/* End of rec-writer.c */
