/* -*- mode: C -*- Time-stamp: "2010-10-29 14:45:23 jco"
 *
 *       File:         rec-field-name-new.c
 *       Date:         Fri Oct 29 13:44:36 2010
 *
 *       GNU recutils - rec_field_name_new unit tests
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
 * Test: rec_field_name_new_nominal
 * Unit: rec_field_name_new
 * Description:
 * + Create a field name.
 * +
 * + 1. rec_field_name_new should return a non-NULL pointer.
 */
START_TEST(rec_field_name_new_nominal)
{
  rec_field_name_t field_name;

  field_name = rec_field_name_new ();
  fail_if (field_name == NULL);

  rec_field_name_destroy (field_name);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_name_new (void)
{
  TCase *tc = tcase_create ("rec_field_name_new");
  tcase_add_test (tc, rec_field_name_new_nominal);

  return tc;
}

/* End of rec-field-name-new.c */
