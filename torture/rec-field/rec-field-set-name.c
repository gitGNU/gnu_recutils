/* -*- mode: C -*- Time-stamp: "09/12/25 18:13:53 jemarch"
 *
 *       File:         rec-field-set-name.c
 *       Date:         Sun Mar  1 17:04:00 2009
 *
 *       GNU rec library - rec_field_set_name unit tests
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
#include <string.h>
#include <check.h>

#include <rec.h>

/*
 * Test: rec_field_set_name_001
 * Description:
 *   Set the name of a field to the empty string
 * Success conditions:
 *   1. The call should not produce an error.
 *   2. The name of the field should be properly
 *      set.
 */
START_TEST(rec_field_set_name_001)
{
  rec_field_t field;
  rec_field_name_t field_name;
  rec_field_name_t field_name_2;
  
  field_name = rec_field_name_new ();
  rec_field_name_set (field_name, 0, "");

  field = rec_field_new (field_name, "");
  fail_if(field == NULL);

  field_name_2 = rec_field_name_new ();
  rec_field_name_set (field_name_2, 0, "");

  rec_field_set_name (field, field_name_2);
  field_name_2 = rec_field_name (field);
  fail_if(strcmp (rec_field_name_get (field_name_2, 0), "")
          != 0);
}
END_TEST

/*
 * Test: rec_field_set_name_002
 * Description:
 *   Set the name of a field to a non-empty name
 * Success conditions:
 *   1. The call should not produce an error.
 *   2. The name of the field should be properly
 *      set.
 */
START_TEST(rec_field_set_name_002)
{
  rec_field_t field;
  rec_field_name_t field_name;
  rec_field_name_t field_name_2;
  
  field_name = rec_field_name_new ();
  rec_field_name_set (field_name, 0, "");

  field = rec_field_new (field_name, "");
  fail_if(field == NULL);

  field_name_2 = rec_field_name_new ();
  rec_field_name_set (field_name_2, 0, "foo");

  rec_field_set_name (field, field_name_2);
  field_name_2 = rec_field_name (field);
  fail_if(strcmp (rec_field_name_get (field_name_2, 0), "foo")
          != 0);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_set_name (void)
{
  TCase *tc = tcase_create("rec_field_set_name");
  tcase_add_test (tc, rec_field_set_name_001);
  tcase_add_test (tc, rec_field_set_name_002);

  return tc;
}

/* End of rec-field-set-name.c */
