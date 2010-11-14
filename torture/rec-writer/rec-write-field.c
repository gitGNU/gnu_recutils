/* -*- mode: C -*- Time-stamp: "2010-11-14 15:06:21 jemarch"
 *
 *       File:         rec-write-field.c
 *       Date:         Sun Nov 14 13:11:01 2010
 *
 *       GNU recutils - rec_write_field unit tests.
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
#include <string.h>
#include <stdio.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_write_field_nominal
 * Unit: rec_write_field
 * Description:
 * + Write fields.
 */
START_TEST(rec_write_field_nominal)
{
  rec_writer_t writer;
  rec_field_t field;
  FILE *stm;
  char *str;
  size_t str_size;

  field = rec_field_new (rec_parse_field_name_str ("foo"), "value");
  fail_if (field == NULL);
  stm = open_memstream (&str, &str_size);
  fail_if (stm == NULL);
  writer = rec_writer_new (stm);
  fail_if (!rec_write_field (writer, field, REC_WRITER_NORMAL));
  rec_field_destroy (field);
  rec_writer_destroy (writer);
  fclose (stm);
  fail_if (strcmp (str, "foo: value\n") != 0);
}
END_TEST

/*-
 * Test: rec_write_field_sexp
 * Unit: rec_write_field
 * Description:
 * + Write fields.
 */
START_TEST(rec_write_field_sexp)
{
  rec_writer_t writer;
  rec_field_t field;
  FILE *stm;
  char *str;
  size_t str_size;

  field = rec_field_new (rec_parse_field_name_str ("foo"), "value");
  fail_if (field == NULL);
  stm = open_memstream (&str, &str_size);
  writer = rec_writer_new (stm);
  fail_if (stm == NULL);
  fail_if (!rec_write_field (writer, field, REC_WRITER_SEXP));
  rec_field_destroy (field);
  rec_writer_destroy (writer);
  fclose (stm);
  fail_if (strcmp (str, "(field  (\"foo\") \"value\")\n") != 0);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_write_field (void)
{
  TCase *tc = tcase_create ("rec_write_field");
  tcase_add_test (tc, rec_write_field_nominal);
  tcase_add_test (tc, rec_write_field_sexp);

  return tc;
}

/* End of rec-write-field.c */
