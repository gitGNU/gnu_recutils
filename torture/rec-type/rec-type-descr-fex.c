/* -*- mode: C -*- Time-stamp: "2010-11-09 11:45:41 jemarch"
 *
 *       File:         rec-type-descr-fex.c
 *       Date:         Fri Oct 29 17:45:57 2010
 *
 *       GNU recutils - rec_type_descr_fex unit tests
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
 * Test: rec_type_descr_fex_nominal
 * Unit: rec_type_descr_fex
 * Description:
 * + Extract a fex from a valid type descriptor.
 */
START_TEST(rec_type_descr_fex_nominal)
{
  rec_field_name_t fname1;
  rec_field_name_t fname2;
  rec_field_name_t fname3;
  rec_fex_elem_t fex_elem;
  size_t i;
  rec_fex_t fex;

  fname1 = rec_field_name_new ();
  fail_if (fname1 == NULL);
  fail_if (!rec_field_name_set (fname1, 0, "foo"));
  fail_if (!rec_field_name_set (fname1, 1, "bar"));
  fail_if (!rec_field_name_set (fname1, 2, "baz"));

  fname2 = rec_field_name_new ();
  fail_if (fname2 == NULL);
  fail_if (!rec_field_name_set (fname2, 0, "one"));

  fname3 = rec_field_name_new ();
  fail_if (fname3 == NULL);
  fail_if (!rec_field_name_set (fname3, 0, "red"));
  fail_if (!rec_field_name_set (fname3, 1, "blue"));

  fex = rec_type_descr_fex ("foo:bar:baz:,one,red:blue enum A B C");
  fail_if (fex == NULL);
  fail_if (rec_fex_size (fex) != 3);
  for (i = 0; i < rec_fex_size (fex); i++)
    {
      fex_elem = rec_fex_get (fex, i);
      switch (i)
        {
        case 0:
          {
            fail_if (!rec_field_name_eql_p (rec_fex_elem_field_name (fex_elem),
                                            fname1));
            break;
          }
        case 1:
          {
            fail_if (!rec_field_name_eql_p (rec_fex_elem_field_name (fex_elem),
                                            fname2));
            break;
          }
        case 2:
          {
            fail_if (!rec_field_name_eql_p (rec_fex_elem_field_name (fex_elem),
                                            fname3));
            break;
          }
        default:
          {
            fail_if (true);
            break;
          }
        }
    }
  
  rec_field_name_destroy (fname1);
  rec_field_name_destroy (fname2);
  rec_field_name_destroy (fname3);
  rec_fex_destroy (fex);
}
END_TEST

/*-
 * Test: rec_type_descr_fex_invalid
 * Unit: rec_type_descr_fex
 * Description:
 * + Try to extract a fex from an
 * + invalid type descriptor.
 */
START_TEST(rec_type_descr_fex_invalid)
{
  rec_fex_t fex;

  fex = rec_type_descr_fex ("foo bar baz");
  fail_if (fex != NULL);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_descr_fex (void)
{
  TCase *tc = tcase_create ("rec_type_descr_fex");
  tcase_add_test (tc, rec_type_descr_fex_nominal);
  tcase_add_test (tc, rec_type_descr_fex_invalid);

  return tc;
}

/* End of rec-type-descr-fex.c */
