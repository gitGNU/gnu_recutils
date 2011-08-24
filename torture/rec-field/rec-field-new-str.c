/* -*- mode: C -*-
 *
 *       File:         rec-field-new-str.c
 *       Date:         Fri Nov 12 13:10:56 2010
 *
 *       GNU recutils - rec_field_new_str unit tests.
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

#include <config.h>
#include <string.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_field_new_str
 * Unit: rec_field_new
 * Description:
 * + Create a field from a valid string.
 */
START_TEST(rec_field_new_str_nominal)
{
  rec_field_t field;

  field = rec_field_new_str ("foo:bar:bar", "value");
  fail_if (field == NULL);

  rec_field_destroy (field);
}
END_TEST

/*-
 * Test: rec_field_new_str_invalid
 * Unit: rec_field_new_str
 * Description:
 * + Try to create a field from a string
 * + not containing a valid field name.
 * +
 * + The creation function shall return NULL.
 */
START_TEST(rec_field_new_str_invalid)
{
  rec_field_t field;

  field = rec_field_new_str ("not a field name", "value");
  fail_if (field != NULL);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_field_new_str (void)
{
  TCase *tc = tcase_create ("rec_field_new_str");
  tcase_add_test (tc, rec_field_new_str_nominal);
  tcase_add_test (tc, rec_field_new_str_invalid);

  return tc;
}

/* End of rec-field-new-str.c */
