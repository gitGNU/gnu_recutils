/* -*- mode: C -*-
 *
 *       File:         rec-type-reg-assoc-anon.c
 *       Date:         Fri Oct 29 21:15:17 2010
 *
 *       GNU recutils - rec_type_reg_assoc_anon unit tests
 *
 */

/* Copyright (C) 2010, 2011 Jose E. Marchesi */

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
 * Test: rec_type_reg_assoc_anon_nominal
 * Unit: rec_type_reg_assoc_anon
 * Description:
 * + Registration of a field->type association in
 * + a type registry.
 */
START_TEST (rec_type_reg_assoc_anon_nominal)
{
  rec_type_t type;
  rec_field_name_t fname;
  rec_type_reg_t reg;

  reg = rec_type_reg_new ();
  fail_if (reg == NULL);

  /* Register two types.  */
  fname = rec_parse_field_name_str ("foo");
  type = rec_type_new ("int");
  fail_if (type == NULL);
  fail_if (fname == NULL);
  rec_type_reg_assoc_anon (reg, fname, type);
  
  fname = rec_parse_field_name_str ("bar");
  type = rec_type_new ("range 1 10");
  fail_if (type == NULL);
  fail_if (fname == NULL);
  rec_type_reg_assoc_anon (reg, fname, type);

  /* Register 'foo' again with another type.  */
  fname = rec_parse_field_name_str ("foo");
  type = rec_type_new ("bool");
  fail_if (type == NULL);
  fail_if (fname == NULL);
  rec_type_reg_assoc_anon (reg, fname, type);

  fail_if (rec_type_kind (rec_type_reg_field_type (reg, fname))
           != REC_TYPE_BOOL);

  rec_type_reg_destroy (reg);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_type_reg_assoc_anon (void)
{
  TCase *tc = tcase_create ("rec_type_reg_assoc_anon");
  tcase_add_test (tc, rec_type_reg_assoc_anon_nominal);

  return tc;
}

/* End of rec-type-reg-assoc-anon.c */
