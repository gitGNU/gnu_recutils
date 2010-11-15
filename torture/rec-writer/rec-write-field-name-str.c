/* -*- mode: C -*- Time-stamp: "2010-11-15 14:17:56 jemarch"
 *
 *       File:         rec-write-field-name-str.c
 *       Date:         Mon Nov 15 14:14:56 2010
 *
 *       GNU recutils - rec_write_field_name_str unit tests.
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
 * Test: rec_write_field_name_str_nominal
 * Unit: rec_write_field_name_str
 * Description:
 * + Write field names.
 */
START_TEST(rec_write_field_name_str_nominal)
{
  rec_field_name_t fname;
  char *str;

  fname = rec_field_name_new ();
  fail_if (fname == NULL);
  rec_field_name_set (fname, 0, "foo");
  rec_field_name_set (fname, 1, "bar");
  rec_field_name_set (fname, 2, "baz");
  str = rec_write_field_name_str (fname, REC_WRITER_NORMAL);
  fail_if (str == NULL);
  rec_field_name_destroy (fname);
  fail_if (strcmp (str, "foo:bar:baz:") != 0);
}
END_TEST

/*-
 * Test: rec_write_field_name_str_sexp
 * Unit: rec_write_field_name_str
 * Description:
 * + Write field names.
 */
START_TEST(rec_write_field_name_str_sexp)
{
  rec_field_name_t fname;
  char *str;

  fname = rec_field_name_new ();
  fail_if (fname == NULL);
  rec_field_name_set (fname, 0, "foo");
  rec_field_name_set (fname, 1, "bar");
  rec_field_name_set (fname, 2, "baz");
  str = rec_write_field_name_str (fname, REC_WRITER_SEXP);
  rec_field_name_destroy (fname);
  fail_if (strcmp (str, "(\"foo\" \"bar\" \"baz\")") != 0);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_write_field_name_str (void)
{
  TCase *tc = tcase_create ("rec_write_field_name_str");
  tcase_add_test (tc, rec_write_field_name_str_nominal);
  tcase_add_test (tc, rec_write_field_name_str_sexp);

  return tc;
}

/* End of rec-write-field-name-str.c */
