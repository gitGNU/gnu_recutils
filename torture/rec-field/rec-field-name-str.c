/* -*- mode: C -*-
 *
 *       File:         rec-field-name-str.c
 *       Date:         Fri Nov 12 13:27:00 2010
 *
 *       GNU recutils - rec_field_name_str unit tests.
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
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_field_name_str_nominal
 * Unit: rec_field_name_str
 * Description:
 * + Get a string containing the name
 * + of a given field.
 */
START_TEST(rec_field_name_str_nominal)
{
  rec_field_t field;
  char *fname;

  field = rec_field_new_str ("foo:bar:baz", "value");
  fail_if (field == NULL);

  fname = rec_field_name_str (field);
  fail_if (fname == NULL);
  fail_if (strcmp (rec_field_name_str (field), "foo:bar:baz:") != 0);

  rec_field_destroy (field);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_field_name_str (void)
{
  TCase *tc = tcase_create ("rec_field_name_str");
  tcase_add_test (tc, rec_field_name_str_nominal);

  return tc;
}

/* End of rec-field-name-str.c */
