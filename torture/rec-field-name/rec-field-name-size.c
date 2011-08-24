/* -*- mode: C -*-
 *
 *       File:         rec-field-name-size.c
 *       Date:         Fri Oct 29 14:58:14 2010
 *
 *       GNU recutils - rec_field_name_size unit tests
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
 * Test: rec_field_name_size_empty
 * Unit: rec_field_name_size
 * Description:
 * + Get the size of an empty field name.
 * +
 * + 1. The returned size shall be 0.
 */
START_TEST(rec_field_name_size_empty)
{
  rec_field_name_t fname;

  fname = rec_field_name_new ();
  fail_if (fname == NULL);
  fail_if (rec_field_name_size (fname) != 0);

  rec_field_name_destroy (fname);
}
END_TEST

/*-
 * Test: rec_field_name_size_nonempty
 * Unit: rec_field_name_size
 * Description:
 * + Check the size of a non empty field name.
 */
START_TEST(rec_field_name_size_nonempty)
{
  rec_field_name_t fname;

  fname = rec_field_name_new ();
  fail_if (fname == NULL);
  fail_if (!rec_field_name_set (fname, 0, "a"));
  fail_if (rec_field_name_size (fname) != 1);
  fail_if (!rec_field_name_set (fname, 1, "b"));
  fail_if (rec_field_name_size (fname) != 2);
  fail_if (!rec_field_name_set (fname, 2, "c"));
  fail_if (rec_field_name_size (fname) != 3);

  rec_field_name_destroy (fname);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_name_size (void)
{
  TCase *tc = tcase_create ("rec_field_name_size");
  tcase_add_test (tc, rec_field_name_size_empty);
  tcase_add_test (tc, rec_field_name_size_nonempty);

  return tc;
}

/* End of rec-field-name-size.c */
