/* -*- mode: C -*- Time-stamp: "2011-02-08 21:24:28 jemarch"
 *
 *       File:         rec-type-descr-type.c
 *       Date:         Thu Nov 25 16:58:19 2010
 *
 *       GNU recutils - rec_type_descr_type unit tests.
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

#include <stdlib.h>
#include <string.h>

#include <rec.h>

/*-
 * Test: rec_type_descr_type_nominal
 * Unit: rec_type_descr_type
 * Description:
 * + Extract the type part from a type
 * + description.
 */
START_TEST(rec_type_descr_type_nominal)
{
  char *str;

  str = rec_type_descr_type ("foo:bar:baz:,one,red:blue enum A B C");
  fail_if (str == NULL);
  fail_if (strcmp (str, "enum A B C") != 0);
  free (str);

  str = rec_type_descr_type ("foo:bar:baz:,one,red:blue\n \tenum A B C");
  fail_if (str == NULL);
  fail_if (strcmp (str, "enum A B C") != 0);
  free (str);

  str = rec_type_descr_type ("foo int");
  fail_if (str == NULL);
  fail_if (strcmp (str, "int") != 0);
  free (str);
}
END_TEST

/*-
 * Test: rec_type_descr_type_invalid
 * Unit: rec_type_descr_type
 * Description:
 * + Try to extract the type part from an
 * + invalid type description.
 */
START_TEST(rec_type_descr_type_invalid)
{
  char *str;

  str = rec_type_descr_type ("");
  fail_if (str != NULL);

  str = rec_type_descr_type ("foo");
  fail_if (str != NULL);
  
  str = rec_type_descr_type ("foo  \n\t");
  fail_if (str != NULL);

  str = rec_type_descr_type ("invalid type description");
  fail_if (str != NULL);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_descr_type (void)
{
  TCase *tc = tcase_create ("rec_type_descr_type");
  tcase_add_test (tc, rec_type_descr_type_nominal);
  tcase_add_test (tc, rec_type_descr_type_invalid);

  return tc;
}

/* End of rec-type-descr-type.c */
