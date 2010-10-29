/* -*- mode: C -*- Time-stamp: "2010-10-29 17:30:02 jco"
 *
 *       File:         rec-type-descr-p.c
 *       Date:         Fri Oct 29 17:15:50 2010
 *
 *       GNU recutils - rec_type_descr_p unit tests
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
 * Test: rec_type_descr_p_nominal
 * Unit: rec_type_descr_p
 * Description:
 * + Check valid type descriptions.
 */
START_TEST(rec_type_descr_p_nominal)
{
  fail_if (!rec_type_descr_p ("foo int"));
  fail_if (!rec_type_descr_p ("foo int  \n\n  "));
  fail_if (!rec_type_descr_p ("foo bool"));
  fail_if (!rec_type_descr_p ("foo range 1 10"));
  fail_if (!rec_type_descr_p ("foo real"));
  fail_if (!rec_type_descr_p ("foo size 10"));
  fail_if (!rec_type_descr_p ("foo line"));
  fail_if (!rec_type_descr_p ("foo regexp /[abc][abc][abc]/"));
  fail_if (!rec_type_descr_p ("foo regexp |/jo/jo|"));
  fail_if (!rec_type_descr_p ("foo date"));
  fail_if (!rec_type_descr_p ("foo enum A B C"));
  fail_if (!rec_type_descr_p ("foo field"));
  fail_if (!rec_type_descr_p ("foo email"));
}
END_TEST

/*-
 * Test: rec_type_descr_p_invalid
 * Unit: rec_type_descr_p
 * Description:
 * + Check for invalid type descriptions.
 */
START_TEST(rec_type_descr_p_invalid)
{
  fail_if (rec_type_descr_p ("foo "));
  fail_if (rec_type_descr_p ("foo  "));
  fail_if (rec_type_descr_p ("foo int additionalstuff"));
  fail_if (rec_type_descr_p ("foo invalidkeyword"));
  fail_if (rec_type_descr_p ("foo range a b"));
  fail_if (rec_type_descr_p ("foo range a 1"));
  fail_if (rec_type_descr_p ("foo range 1 a"));
  fail_if (rec_type_descr_p ("foo size"));
  fail_if (rec_type_descr_p ("foo size xxx"));
  fail_if (rec_type_descr_p ("foo size 10 extra"));
  fail_if (rec_type_descr_p ("foo regexp"));
  fail_if (rec_type_descr_p ("foo regexp foo"));
  fail_if (rec_type_descr_p ("foo regexp /abc/ extra"));
  fail_if (rec_type_descr_p ("foo enum"));
  fail_if (rec_type_descr_p ("foo enum # ! '"));
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_descr_p (void)
{
  TCase *tc = tcase_create ("rec_type_descr_p");
  tcase_add_test (tc, rec_type_descr_p_nominal);
  tcase_add_test (tc, rec_type_descr_p_invalid);

  return tc;
}

/* End of rec-type-descr-p.c */
