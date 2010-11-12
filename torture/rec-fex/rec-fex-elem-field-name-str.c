/* -*- mode: C -*- Time-stamp: "2010-11-12 12:37:13 jemarch"
 *
 *       File:         rec-fex-elem-field-name-str.c
 *       Date:         Fri Nov 12 12:28:57 2010
 *
 *       GNU recutils - rec_fex_elem_field_name_str unit tests
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
#include <string.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_fex_elem_field_name_str_nominal
 * Unit: rec_fex_elem_field_name_str
 * Description:
 * + Get the string containing the field name of
 * + a field expression element.
 */
START_TEST(rec_fex_elem_field_name_str_nominal)
{
  rec_fex_t fex;
  rec_fex_elem_t elem;
  char *str;

  fex = rec_fex_new ("foo bar baz", REC_FEX_SIMPLE);
  fail_if (fex == NULL);

  elem = rec_fex_get (fex, 1);
  fail_if (elem == NULL);

  str = rec_fex_elem_field_name_str (elem);
  fail_if (str == NULL);
  fail_if (strcmp (str, "bar") != 0);

  rec_fex_destroy (fex);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_fex_elem_field_name_str (void)
{
  TCase *tc = tcase_create ("rec_fex_elem_field_name_str");
  tcase_add_test (tc, rec_fex_elem_field_name_str_nominal);

  return tc;
}

/* End of rec-fex-elem-field-name-str.c */
