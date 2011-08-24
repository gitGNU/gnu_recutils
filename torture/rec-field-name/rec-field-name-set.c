/* -*- mode: C -*-
 *
 *       File:         rec-field-name-set.c
 *       Date:         Fri Oct 29 15:12:22 2010
 *
 *       GNU recutils - rec_field_name_set unit tests
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
 * Test: rec_field_name_set_nominal
 * Unit: rec_field_name_set
 * Description:
 * + Check the setting of name parts.
 */
START_TEST(rec_field_name_set_nominal)
{
  rec_field_name_t fname;

  fname = rec_field_name_new ();
  fail_if (fname == NULL);

  fail_if (rec_field_name_set (fname, -10, "x"));
  fail_if (rec_field_name_set (fname, 100, "y")); /* The number shall
                                                     be bigger than
                                                     the internal
                                                     symbol
                                                     NAME_MAX_PARTS in
                                                     rec-field-name.c */
  fail_if (!rec_field_name_set (fname, 0, "z"));

  rec_field_name_destroy (fname);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_name_set (void)
{
  TCase *tc = tcase_create ("rec_field_name_set");
  tcase_add_test (tc, rec_field_name_set_nominal);

  return tc;
}

/* End of rec-field-name-set.c */
