/* -*- mode: C -*- Time-stamp: "2010-10-29 18:40:45 jco"
 *
 *       File:         rec-type-equal-p.c
 *       Date:         Fri Oct 29 18:33:21 2010
 *
 *       GNU recutils - rec_type_equal_p unit tests
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
 * Test: rec_type_equal_p_nominal
 * Unit: rec_type_equal_p
 * Description:
 * + Check equality for equal types.
 */
START_TEST(rec_type_equal_p_nominal)
{
  rec_type_t type1;
  rec_type_t type2;

  type1 = rec_type_new ("foo int");
  type2 = rec_type_new ("bar int");
  fail_if (!rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo bool");
  type2 = rec_type_new ("bar bool");
  fail_if (!rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo range 1 10");
  type2 = rec_type_new ("bar range 1 10");
  fail_if (!rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo range 1 10");
  type2 = rec_type_new ("bar range -10 0");
  fail_if (rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo real");
  type2 = rec_type_new ("bar real");
  fail_if (!rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo size 10");
  type2 = rec_type_new ("bar size 10");
  fail_if (!rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo size 10");
  type2 = rec_type_new ("bar size 20");
  fail_if (rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo line");
  type2 = rec_type_new ("bar line");
  fail_if (!rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo date");
  type2 = rec_type_new ("bar date");
  fail_if (!rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo enum A B C");
  type2 = rec_type_new ("bar enum A B C \n\n  ");
  fail_if (!rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo enum A B C");
  type2 = rec_type_new ("bar enum B C A");
  fail_if (rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo enum A B C");
  type2 = rec_type_new ("bar enum B C A D");
  fail_if (rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo field");
  type2 = rec_type_new ("bar field");
  fail_if (!rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);

  type1 = rec_type_new ("foo email");
  type2 = rec_type_new ("bar email");
  fail_if (!rec_type_equal_p (type1, type2));
  rec_type_destroy (type1);
  rec_type_destroy (type2);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_equal_p (void)
{
  TCase *tc = tcase_create ("rec_type_equal_p");
  tcase_add_test (tc, rec_type_equal_p_nominal);

  return tc;
}

/* End of rec-type-equal-p.c */
