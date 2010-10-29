/* -*- mode: C -*- Time-stamp: "2010-10-29 17:46:21 jco"
 *
 *       File:         rec-type-new.c
 *       Date:         Fri Oct 29 16:16:05 2010
 *
 *       GNU recutils - rec_type_new unit tests
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
 * Test: rec_type_new_nominal
 * Unit: rec_type_new
 * Description:
 * + Create types from correct type descriptions.
 */
START_TEST(rec_type_new_nominal)
{
  rec_type_t type;

  type = rec_type_new ("foo int");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo int   \n\n  ");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo bool");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo range 1 10");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo real");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo size 10");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo line");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo regexp /[abc][abc][abc]/");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo regexp |/jo/jo|");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo date");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo enum A B C");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo field");
  fail_if (type == NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo email");
  fail_if (type == NULL);
  rec_type_destroy (type);
}
END_TEST

/*-
 * Test: rec_type_new_invalid
 * Unit: rec_type_new
 * Description:
 * + Try to create types from invalid type descriptions.
 */
START_TEST(rec_type_new_invalid)
{
  rec_type_t type;

  type = rec_type_new ("foo ");
  fail_if (type != NULL);

  type = rec_type_new ("foo  ");
  fail_if (type != NULL);

  type = rec_type_new ("foo int additionalstuff");
  fail_if (type != NULL);
  
  type = rec_type_new ("foo invalidkeyword");
  fail_if (type != NULL);

  type = rec_type_new ("foo range a b");
  fail_if (type != NULL);

  type = rec_type_new ("foo range a 1");
  fail_if (type != NULL);

  type = rec_type_new ("foo range 1 a");
  fail_if (type != NULL);

  type = rec_type_new ("foo size");
  fail_if (type != NULL);

  type = rec_type_new ("foo size xxx");
  fail_if (type != NULL);

  type = rec_type_new ("foo size 10 extra");
  fail_if (type != NULL);

  type = rec_type_new ("foo regexp");
  fail_if (type != NULL);

  type = rec_type_new ("foo regexp foo");
  fail_if (type != NULL);

  type = rec_type_new ("foo regexp /abc/ extra");
  fail_if (type != NULL);

  type = rec_type_new ("foo enum");
  fail_if (type != NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo enum  ");
  fail_if (type != NULL);
  rec_type_destroy (type);

  type = rec_type_new ("foo enum # ! '");
  fail_if (type != NULL);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_new (void)
{
  TCase *tc = tcase_create ("rec_type_new");
  tcase_add_test (tc, rec_type_new_nominal);
  tcase_add_test (tc, rec_type_new_invalid);

  return tc;
}

/* End of rec-type-new.c */
