/* -*- mode: C -*-
 *
 *       File:         rec-write-record-with-fex.c
 *       Date:         Sun Nov 14 14:30:31 2010
 *
 *       GNU recutils - rec_write_record_with_fex unit tests.
 *
 */

/* Copyright (C) 2010, 2011, 2012 Jose E. Marchesi */

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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_write_record_with_fex_nominal
 * Unit: rec_write_record_with_fex
 * Description:
 * + Process a record through a field expression
 * + and write out the result.
 */
START_TEST(rec_write_record_with_fex_nominal)
{
  rec_writer_t writer;
  rec_record_t record;
  rec_field_t field;
  rec_fex_t fex;
  char *str;
  size_t str_size;

  /* Create a record:
   * 
   * a: value a
   * b: value b
   * a: value a 2
   * b: value b 2
   *
   */
  record = rec_record_new ();
  fail_if (record == NULL);
  field = rec_field_new ("a", "value a");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  field = rec_field_new ("b", "value b");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  field = rec_field_new ("a", "value a 2");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  field = rec_field_new ("b", "value b 2");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);

  /*
   * Test several field expressions.
   */
  
  fex = rec_fex_new ("a,b", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       false,   /* print values */
                                       false)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "a: value a\na: value a 2\nb: value b\nb: value b 2\n") != 0);
  free (str);

  fex = rec_fex_new ("b,a", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       false,   /* print values */
                                       false)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "b: value b\nb: value b 2\na: value a\na: value a 2\n") != 0);
  free (str);

  fex = rec_fex_new ("b,a[1]", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       false,   /* print values */
                                       false)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "b: value b\nb: value b 2\na: value a 2\n") != 0);
  free (str);

  fex = rec_fex_new ("b[0],a[1]", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       false,   /* print values */
                                       false)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "b: value b\na: value a 2\n") != 0);
  free (str);

  fex = rec_fex_new ("b[0-1],a[1]", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       false,   /* print values */
                                       false)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "b: value b\nb: value b 2\na: value a 2\n") != 0);
  free (str);
  
}
END_TEST

/*-
 * Test: rec_write_record_with_fex_values
 * Unit: rec_write_record_with_fex
 * Description:
 * + Process a record through a field expression
 * + and write out the result.
 */
START_TEST(rec_write_record_with_fex_values)
{
  rec_writer_t writer;
  rec_record_t record;
  rec_field_t field;
  rec_fex_t fex;
  char *str;
  size_t str_size;

  /* Create a record:
   * 
   * a: value a
   * b: value b
   * a: value a 2
   * b: value b 2
   *
   */
  record = rec_record_new ();
  fail_if (record == NULL);
  field = rec_field_new ("a", "value a");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  field = rec_field_new ("b", "value b");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  field = rec_field_new ("a", "value a 2");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  field = rec_field_new ("b", "value b 2");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);

  /*
   * Test several field expressions.
   */
  
  fex = rec_fex_new ("a,b", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       true,   /* print values */
                                       false)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "value a\nvalue a 2\nvalue b\nvalue b 2\n") != 0);
  free (str);

  fex = rec_fex_new ("b,a", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       true,   /* print values */
                                       false)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "value b\nvalue b 2\nvalue a\nvalue a 2\n") != 0);
  free (str);

  fex = rec_fex_new ("b,a[1]", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       true,   /* print values */
                                       false)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "value b\nvalue b 2\nvalue a 2\n") != 0);
  free (str);

  fex = rec_fex_new ("b[0],a[1]", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       true,   /* print values */
                                       false)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "value b\nvalue a 2\n") != 0);
  free (str);

  fex = rec_fex_new ("b[0-1],a[1]", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       true,   /* print values */
                                       false)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "value b\nvalue b 2\nvalue a 2\n") != 0);
  free (str);
  
}
END_TEST

/*-
 * Test: rec_write_record_with_fex_row
 * Unit: rec_write_record_with_fex
 * Description:
 * + Process a record through a field expression
 * + and write out the result.
 */
START_TEST(rec_write_record_with_fex_row)
{
  rec_writer_t writer;
  rec_record_t record;
  rec_field_t field;
  rec_fex_t fex;
  char *str;
  size_t str_size;

  /* Create a record:
   * 
   * a: value a
   * b: value b
   * a: value a 2
   * b: value b 2
   *
   */
  record = rec_record_new ();
  fail_if (record == NULL);
  field = rec_field_new ("a", "value a");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  field = rec_field_new ("b", "value b");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  field = rec_field_new ("a", "value a 2");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);
  field = rec_field_new ("b", "value b 2");
  fail_if (field == NULL);
  fail_if (rec_mset_append (rec_record_mset (record), MSET_FIELD, (void *) field, MSET_ANY) == NULL);

  /*
   * Test several field expressions.
   */
  
  fex = rec_fex_new ("a,b", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       true,   /* print values */
                                       true)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "value a value a 2 value b value b 2\n") != 0);
  free (str);

  fex = rec_fex_new ("b,a", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       true,   /* print values */
                                       true)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "value b value b 2 value a value a 2\n") != 0);
  free (str);

  fex = rec_fex_new ("b,a[1]", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       true,   /* print values */
                                       true)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "value b value b 2 value a 2\n") != 0);
  free (str);

  fex = rec_fex_new ("b[0],a[1]", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       true,   /* print values */
                                       true)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "value b value a 2\n") != 0);
  free (str);

  fex = rec_fex_new ("b[0-1],a[1]", REC_FEX_SUBSCRIPTS);
  fail_if (fex == NULL);
  writer = rec_writer_new_str (&str, &str_size);
  fail_if (!rec_write_record_with_fex (writer, record, fex,
                                       REC_WRITER_NORMAL,
                                       true,   /* print values */
                                       true)); /* print in a row */
  rec_writer_destroy (writer);
  rec_fex_destroy (fex);
  fail_if (strcmp (str, "value b value b 2 value a 2\n") != 0);
  free (str);
  
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_write_record_with_fex (void)
{
  TCase *tc = tcase_create ("rec_write_record_with_fex");
  tcase_add_test (tc, rec_write_record_with_fex_nominal);
  tcase_add_test (tc, rec_write_record_with_fex_values);
  tcase_add_test (tc, rec_write_record_with_fex_row);

  return tc;
}

/* End of rec-write-record-with-fex.c */
