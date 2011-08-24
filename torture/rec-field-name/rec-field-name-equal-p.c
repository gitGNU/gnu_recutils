/* -*- mode: C -*-
 *
 *       File:         rec-field-name-equal-p.c
 *       Date:         Fri Oct 29 14:35:27 2010
 *
 *       GNU recutils - rec_field_name_equal_p unit tests
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
 * Test: rec_field_name_equal_p_empty
 * Unit: rec_field_name_equal_p
 * Description:
 * + Compare two empty field names with
 * + rec_field_name_equal_p.
 * +
 * + 1. The function call shall return true.
 */
START_TEST(rec_field_name_equal_p_empty)
{
  rec_field_name_t fname1;
  rec_field_name_t fname2;

  fname1 = rec_field_name_new ();
  fail_if (fname1 == NULL);

  fname2 = rec_field_name_new ();
  fail_if (fname2 == NULL);

  fail_if (!rec_field_name_equal_p (fname1, fname2));

  rec_field_name_destroy (fname1);
  rec_field_name_destroy (fname2);
}
END_TEST

/*-
 * Test: rec_field_name_equal_p_nonempty
 * Unit: rec_field_name_equal_p
 * Description:
 * + Compare two field names with
 * + rec_field_name_equal_p.
 */
START_TEST(rec_field_name_equal_p_nonempty)
{
  rec_field_name_t fname1;
  rec_field_name_t fname2;
  rec_field_name_t fname3;

  fname1 = rec_field_name_new ();
  fail_if (fname1 == NULL);
  fail_if (!rec_field_name_set (fname1, 0, "a"));
  fail_if (!rec_field_name_set (fname1, 1, "b"));
  fail_if (!rec_field_name_set (fname1, 2, "c"));

  fname2 = rec_field_name_new ();
  fail_if (fname1 == NULL);
  fail_if (!rec_field_name_set (fname2, 0, "x"));
  fail_if (!rec_field_name_set (fname2, 1, "y"));
  fail_if (!rec_field_name_set (fname2, 2, "c"));

  fname3 = rec_field_name_new ();
  fail_if (fname1 == NULL);
  fail_if (!rec_field_name_set (fname3, 0, "a"));
  fail_if (!rec_field_name_set (fname3, 1, "b"));
  fail_if (!rec_field_name_set (fname3, 2, "z"));

  fail_if (!rec_field_name_equal_p (fname1, fname1));
  fail_if (!rec_field_name_equal_p (fname1, fname2));
  fail_if (rec_field_name_equal_p (fname1, fname3));

  rec_field_name_destroy (fname1);
  rec_field_name_destroy (fname2);
  rec_field_name_destroy (fname3);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_name_equal_p (void)
{
  TCase *tc = tcase_create ("rec-field-name-equal-p");
  tcase_add_test (tc, rec_field_name_equal_p_empty);
  tcase_add_test (tc, rec_field_name_equal_p_nonempty);

  return tc;
}

/* End of rec-field-name-equal-p.c */
