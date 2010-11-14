/* -*- mode: C -*- Time-stamp: "2010-11-14 15:05:41 jemarch"
 *
 *       File:         rec-write-record-with-fex.c
 *       Date:         Sun Nov 14 14:30:31 2010
 *
 *       GNU recutils - rec_write_record_with_fex unit tests.
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
#include <stdio.h>
#include <check.h>

#include <rec.h>

/*
 * Test creation function
 */
TCase *
test_rec_write_record_with_fex (void)
{
  TCase *tc = tcase_create ("rec_write_record_with_fex");
  tcase_add_test (tc, rec_write_record_with_fex_nominal);
  tcase_add_test (tc, rec_write_record_with_fex_sexp);
  tcase_add_test (tc, rec_write_record_with_fex_values);
  tcase_add_test (tc, rec_write_record_with_fex_row);

  return tc;
}

/* End of rec-write-record-with-fex.c */
