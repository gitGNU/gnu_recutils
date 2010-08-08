/* -*- mode: C -*-
 *
 *       File:         rec-record-remove-field.c
 *       Date:         Fri Mar  6 21:11:18 2009
 *
 *       GNU recutils - rec_record_remove_field unit tests
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
 * Test: rec_record_remove_field_existing
 * Unit: rec_record_remove_field
 * Description:
 * +  Remove a field from a record.
 * +
 * + 1. The call to rec_record_remove_field should
 * +    return true.
 * + 2. The field should be removed from the record.
 */
START_TEST(rec_record_remove_field_existing)
{
  rec_record_t record;
  rec_field_t field;
  rec_field_name_t fname;

  /* Create a new record */
  record = rec_record_new ();
  fail_if(record == NULL);

  /* Create a new field and insert it into the record */
  fname = rec_field_name_new ();
  rec_field_name_set (fname, 0, "name");
  field = rec_field_new (fname, "value");
  fail_if(field == NULL);
  fail_if(!rec_record_insert_field (record, field, rec_record_size (record)));

  /* Remove the field from the record */
  fail_if(!rec_record_remove_field (record, 0));
  fail_if(rec_record_size (record) != 0);

  rec_record_destroy (record);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_record_remove_field (void)
{
  TCase *tc = tcase_create("rec_record_remove_field");
  tcase_add_test (tc, rec_record_remove_field_existing);

  return tc;
}


/* End of rec-record-remove-field.c */
