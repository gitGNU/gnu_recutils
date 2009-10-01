/* -*- mode: C -*- Time-stamp: "09/10/01 13:46:55 jemarch"
 *
 *       File:         rec-field-get-type.c
 *       Date:         Thu Mar  5 23:14:31 2009
 *
 *       GNU rec library - rec_field_get_type unit tests
 *
 */

/* Copyright (C) 2009 Jose E. Marchesi */

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
#include <stdlib.h>
#include <check.h>

#include <rec.h>

/*
 * Test: rec_field_get_type_001
 * Description:
 *   Get the default type of a newly created field.
 * Success conditions:
 *   1. The returned type should be TEXT.
 */
START_TEST(rec_field_get_type_001)
{
  rec_field_t field;

  field = rec_field_new ("", "");
  fail_if(field == NULL);
  fail_if(rec_field_get_type (field) != REC_FIELD_TYPE_TEXT);

  rec_field_destroy (field);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_get_type (void)
{
  TCase *tc = tcase_create("rec_field_get_type");
  tcase_add_test (tc, rec_field_get_type_001);

  return tc;
}

/* End of rec-field-get-type.c */
