/* -*- mode: C -*- Time-stamp: "2010-10-29 21:24:25 jemarch"
 *
 *       File:         rec-type-reg-register.c
 *       Date:         Fri Oct 29 21:15:17 2010
 *
 *       GNU recutils - rec_type_reg_register unit tests
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
 * Test: rec_type_reg_register_nominal
 * Unit: rec_type_reg_register
 * Description:
 * + Registration of a field->type association in
 * + a type registry.
 */
START_TEST (rec_type_reg_register_nominal)
{
  rec_type_t type;
  rec_field_name_t fname;
  rec_type_reg_t reg;

  reg = rec_type_reg_new ();
  fail_if (reg == NULL);

  /* Register two types.  */
  type = rec_type_new ("foo int");
  fail_if (type == NULL);
  fname = rec_type_descr_field_name ("foo int");
  fail_if (fname == NULL);
  rec_type_reg_register (reg, fname, type);

  type = rec_type_new ("bar range 1 10");
  fail_if (type == NULL);
  fname = rec_type_descr_field_name ("bar range 1 10");
  fail_if (fname == NULL);
  rec_type_reg_register (reg, fname, type);

  /* Register 'foo' again with another type.  */
  type = rec_type_new ("foo bool");
  fail_if (type == NULL);
  fname = rec_type_descr_field_name ("foo bool");
  fail_if (fname == NULL);
  rec_type_reg_register (reg, fname, type);

  fail_if (rec_type_kind (rec_type_reg_get (reg, fname))
           != REC_TYPE_BOOL);

  rec_type_reg_destroy (reg);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_reg_register (void)
{
  TCase *tc = tcase_create ("rec_type_reg_register");
  tcase_add_test (tc, rec_type_reg_register_nominal);

  return tc;
}

/* End of rec-type-reg-register.c */
