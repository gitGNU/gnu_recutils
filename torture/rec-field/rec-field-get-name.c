/* -*- mode: C -*- Time-stamp: "09/10/01 13:46:35 jemarch"
 *
 *       File:         rec-field-get-name.c
 *       Date:         Sun Mar  1 17:04:00 2009
 *
 *       GNU rec library - rec_field_get_name unit tests
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
 * Test: rec_field_get_name_001
 * Description:
 *   Get the name of a field with an empy name
 * Success conditions:
 *   1. The call should not produce an error.
 *   2. The name of the field should be properly
 *      returned.
 */
START_TEST(rec_field_get_name_001)
{
  rec_field_t field;
  const char *field_name;
  
  field = rec_field_new ("", "");
  fail_if(field == NULL);

  field_name = rec_field_get_name (field);
  fail_if(strcmp (field_name, "") != 0);
}
END_TEST

/*
 * Test: rec_field_get_name_002
 * Description:
 *   Get the name of a field with a non-empty name
 * Success conditions:
 *   1. The call should not produce an error.
 *   2. The name of the field should be properly
 *      returned.
 */
START_TEST(rec_field_get_name_002)
{
  rec_field_t field;
  const char *field_name;
  
  field = rec_field_new ("foo", "");
  fail_if(field == NULL);

  field_name = rec_field_get_name (field);
  fail_if(strcmp (field_name, "foo") != 0);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_field_get_name (void)
{
  TCase *tc = tcase_create("rec_field_get_name");
  tcase_add_test (tc, rec_field_get_name_001);
  tcase_add_test (tc, rec_field_get_name_002);

  return tc;
}

/* End of rec-field-get-name.c */
