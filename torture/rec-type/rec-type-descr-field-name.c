/* -*- mode: C -*- Time-stamp: "2010-10-29 18:01:51 jco"
 *
 *       File:         rec-type-descr-field-name.c
 *       Date:         Fri Oct 29 17:45:57 2010
 *
 *       GNU recutils - rec_type_descr_field_name unit tests
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
 * Test: rec_type_descr_field_name_nominal
 * Unit: rec_type_descr_field_name
 * Description:
 * + Extract a field name from a valid type descriptor.
 */
START_TEST(rec_type_descr_field_name_nominal)
{
  rec_field_name_t fname1;
  rec_field_name_t fname2;

  fname1 = rec_field_name_new ();
  fail_if (fname1 == NULL);
  fail_if (!rec_field_name_set (fname1, 0, "foo"));
  fail_if (!rec_field_name_set (fname1, 1, "bar"));
  fail_if (!rec_field_name_set (fname1, 2, "baz"));

  fname2 = rec_type_descr_field_name ("foo:bar:baz: enum A B C");
  fail_if (fname2 == NULL);
  
  fail_if (!rec_field_name_eql_p (fname1, fname2));

  rec_field_name_destroy (fname1);
  rec_field_name_destroy (fname2);
}
END_TEST

/*-
 * Test: rec_type_descr_field_name_invalid
 * Unit: rec_type_descr_field_name
 * Description:
 * + Try to extract a field name from an
 * + invalid type descriptor.
 */
START_TEST(rec_type_descr_field_name_invalid)
{
  rec_field_name_t fname;

  fname = rec_type_descr_field_name ("foo bar baz");
  fail_if (fname != NULL);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_descr_field_name (void)
{
  TCase *tc = tcase_create ("rec_type_descr_field_name");
  tcase_add_test (tc, rec_type_descr_field_name_nominal);
  tcase_add_test (tc, rec_type_descr_field_name_invalid);

  return tc;
}

/* End of rec-type-descr-field-name.c */
