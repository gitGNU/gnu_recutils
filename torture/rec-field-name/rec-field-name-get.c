/* -*- mode: C -*- Time-stamp: "2010-10-29 15:26:57 jco"
 *
 *       File:         rec-field-name-get.c
 *       Date:         Fri Oct 29 15:18:02 2010
 *
 *       GNU recutils - rec_field_name_get unit tests
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
 * Test: rec_field_name_get_nominal
 * Unit: rec_field_name_get
 * Description:
 * + Get the name parts of a field name.
 */
START_TEST(rec_field_name_get_nominal)
{
  rec_field_name_t fname;

  fname = rec_field_name_new ();
  fail_if (fname == NULL);
  fail_if (!rec_field_name_set (fname, 0, "a"));
  fail_if (!rec_field_name_set (fname, 1, "b"));
  fail_if (!rec_field_name_set (fname, 2, "c"));

  fail_if (rec_field_name_get (fname, -10) != NULL);
  fail_if (rec_field_name_get (fname, 3) != NULL);

  fail_if (strcmp (rec_field_name_get (fname, 0), "a") != 0);
  fail_if (strcmp (rec_field_name_get (fname, 1), "b") != 0);
  fail_if (strcmp (rec_field_name_get (fname, 2), "c") != 0);

  rec_field_name_destroy (fname);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_name_get (void)
{
  TCase *tc = tcase_create ("rec_field_name_get");
  tcase_add_test (tc, rec_field_name_get_nominal);

  return tc;
}

/* End of rec-field-name-get.c */
