/* -*- mode: C -*- Time-stamp: "2010-11-09 11:42:02 jemarch"
 *
 *       File:         rec-type-reg-get.c
 *       Date:         Fri Oct 29 21:30:27 2010
 *
 *       GNU recutils - rec_type_reg_get unit tests
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
 * Test: rec_type_reg_get_nominal
 * Unit: rec_type_reg_get
 * Description:
 * + Get an existing association from a type registry.
 */
START_TEST(rec_type_reg_get_nominal)
{
  rec_type_t type;
  rec_type_t type2;
  rec_field_name_t fname;
  rec_type_reg_t reg;

  reg = rec_type_reg_new ();
  fail_if (reg == NULL);

  /* Register a type.  */
  type = rec_type_new ("foo int");
  fail_if (type == NULL);
  fname = 
    rec_fex_elem_field_name (rec_fex_get (rec_type_descr_fex ("foo int"), 0));
  fail_if (fname == NULL);
  rec_type_reg_register (reg, fname, type);

  /* Get the type and compare.  */
  type2 = rec_type_reg_get (reg, fname);
  fail_if (type2 == NULL);
  fail_if (type2 != type);

  rec_type_reg_destroy (reg);
}
END_TEST

/*-
 * Test: rec_type_reg_get_nonexisting
 * Unit: rec_type_reg_get
 * Description:
 * + Try to get a nonexisting association
 * + from a type registry.
 */
START_TEST(rec_type_reg_get_nonexisting)
{
  rec_type_t type;
  rec_field_name_t fname;
  rec_type_reg_t reg;

  reg = rec_type_reg_new ();
  fail_if (reg == NULL);
  fname = rec_fex_elem_field_name (rec_fex_get (rec_type_descr_fex ("foo int"), 0));
  fail_if (fname == NULL);

  fail_if (rec_type_reg_get (reg, fname) != NULL);
  
  rec_field_name_destroy (fname);
  rec_type_reg_destroy (reg);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_reg_get (void)
{
  TCase *tc = tcase_create ("rec_type_reg_get");
  tcase_add_test (tc, rec_type_reg_get_nominal);
  tcase_add_test (tc, rec_type_reg_get_nonexisting);

  return tc;
}

/* End of rec-type-reg-get.c */
