/* -*- mode: C -*-
 *
 *       File:         rec-record-new.c
 *       Date:         Thu Mar  5 23:58:33 2009
 *
 *       GNU recutils - rec_record_new unit tests
 *
 */

/* Copyright (C) 2009, 2010 Jose E. Marchesi */

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
 * Test: rec_record_new_and_destroy
 * Unit: rec_record_new
 * Description:
 * + Create a new record and destroy it.
 * +
 * +  1. The call to rec_record_new should not fail.
 * +  2. The call to rec_record_destroy should not fail.
 */
START_TEST(rec_record_new_and_destroy)
{
  rec_record_t record;

  record = rec_record_new ();
  fail_if(record == NULL);

  rec_record_destroy (record);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_record_new (void)
{
  TCase *tc = tcase_create("rec_record_new");
  tcase_add_test (tc, rec_record_new_and_destroy);

  return tc;
}

/* End of rec-record-new.c */
