/* -*- mode: C -*-
 *
 *       File:         rec-record-size.c
 *       Date:         Fri Mar  6 00:02:20 2009
 *
 *       GNU recutils - rec_record_size unit tests
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
 * Test: rec_record_size_empty
 * Unit: rec_record_size
 * Description:
 * + Get the size of a newly created record.
 * +
 * + 1. The call to rec_record_size should return 0.
 */
START_TEST(rec_record_size_empty)
{
  rec_record_t record;

  record = rec_record_new ();
  fail_if(record == NULL);
  fail_if(rec_record_size (record) != 0);

  rec_record_destroy (record);
}
END_TEST

/*-
 * Test: rec_record_size_nonempty
 * Unit: rec_record_size
 * Description:
 * + Get the size of a record containing 3 fields.
 * +
 * + 1. The call to rec_record_size should return 3.
 */
START_TEST(rec_record_size_nonempty)
{
  rec_record_t record;
  rec_field_t field1;
  rec_field_name_t fname1;
  rec_field_t field2;
  rec_field_name_t fname2;
  rec_field_t field3;
  rec_field_name_t fname3;

  fname1 = rec_field_name_new ();
  rec_field_name_set (fname1, 0, "foo1");
  field1 = rec_field_new (fname1, "bar");
  fail_if(field1 == NULL);

  fname2 = rec_field_name_new ();
  rec_field_name_set (fname2, 0, "foo2");
  field2 = rec_field_new (fname2, "bar");
  fail_if(field2 == NULL);

  fname3 = rec_field_name_new ();
  rec_field_name_set (fname3, 0, "foo3");
  field3 = rec_field_new (fname3, "bar");
  fail_if(field3 == NULL);

  record = rec_record_new ();
  fail_if(record == NULL);

  fail_if(!rec_record_insert_field (record, field1, rec_record_size (record)));
  fail_if(!rec_record_insert_field (record, field2, rec_record_size (record)));
  fail_if(!rec_record_insert_field (record, field3, rec_record_size (record)));

  fail_if(rec_record_size (record) != 3);
  
  rec_record_destroy (record);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_record_size (void)
{
  TCase *tc = tcase_create("rec_record_size");
  tcase_add_test (tc, rec_record_size_empty);
  tcase_add_test (tc, rec_record_size_nonempty);

  return tc;
}

/* End of rec-record-size.c */
