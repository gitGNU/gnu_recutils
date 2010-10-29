/* -*- mode: C -*- Time-stamp: "2010-10-29 14:59:14 jco"
 *
 *       File:         rec-field-name-dup.c
 *       Date:         Fri Oct 29 14:08:32 2010
 *
 *       GNU recutils - rec_field_name_dup unit tests
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
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_field_name_dup_empty
 * Unit: rec_field_name_dup
 * Description:
 * + Make a copy of an empty field name.
 * +
 * + 1. A non-null copy shall be returned.
 * + 2. The copy shall be equal to the original.
 */
START_TEST(rec_field_name_dup_empty)
{
  rec_field_name_t fname;
  rec_field_name_t copy;

  fname = rec_field_name_new ();
  fail_if (fname == NULL);

  copy = rec_field_name_dup (fname);
  fail_if (copy == NULL);
  fail_if (copy == fname);

  fail_if (!rec_field_name_eql_p (fname, copy));

  rec_field_name_destroy (fname);
  rec_field_name_destroy (copy);
}
END_TEST

/*-
 * Test: rec_field_name_dup_nominal
 * Unit: rec_field_name_dup
 * Description:
 * + Make a copy of a field name.
 * +
 * + 1. A non-null copy shall be returned.
 * + 2. The copy shall be equal to the original.
 */
START_TEST(rec_field_name_dup_nominal)
{
  rec_field_name_t fname;
  rec_field_name_t copy;

  fname = rec_field_name_new ();
  fail_if (fname == NULL);

  fail_if (!rec_field_name_set (fname, 0, "foo"));
  fail_if (!rec_field_name_set (fname, 1, "bar"));
  fail_if (!rec_field_name_set (fname, 2, "baz"));

  copy = rec_field_name_dup (fname);
  fail_if (copy == NULL);
  fail_if (copy == fname);

  fail_if (!rec_field_name_eql_p (fname, copy));

  rec_field_name_destroy (fname);
  rec_field_name_destroy (copy);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_name_dup (void)
{
  TCase *tc = tcase_create ("rec_field_name_dup");
  tcase_add_test (tc, rec_field_name_dup_empty);
  tcase_add_test (tc, rec_field_name_dup_nominal);

  return tc;
}

/* End of rec-field-name-dup.c */
