/* -*- mode: C -*-
 *
 *       File:         rec-record-field-p.c
 *       Date:         Fri Mar  6 20:01:09 2009
 *
 *       GNU recutils - rec_record_field_p unit tests
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
 * Test: rec_record_field_p_existing
 * Unit: rec_record_field_p
 * Description:
 * + Check for the existence of a field included
 * + in the record.
 * +
 * + 1. The call to rec_record_field_p should
 * +    return true
 */
START_TEST(rec_record_field_p_existing)
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

  /* Check for the existence of the field into the record */
  fail_if(!rec_record_field_p (record, fname));

  rec_record_destroy (record);
}
END_TEST

/*-
 * Test: rec_record_field_p_nonexisting
 * Unit: rec_record_field_p
 * Description:
 * + Check for the existence of a field not included
 * + in the record.
 * +
 * + 1. The call to rec_record_field_p should
 * +    return false.
 */
START_TEST(rec_record_field_p_nonexisting)
{
  rec_record_t record;
  rec_field_t field;
  rec_field_name_t fname;
  rec_field_name_t fname2;

  /* Create a new record */
  record = rec_record_new ();
  fail_if(record == NULL);

  /* Create a new field and insert it into the record*/
  fname = rec_field_name_new ();
  rec_field_name_set (fname, 0, "name");
  field = rec_field_new (fname, "value");
  fail_if(field == NULL);
  fail_if(!rec_record_insert_field (record, field, rec_record_size (record)));

  /* Check for the existence of the field into the record */
  fname2 = rec_field_name_dup (fname);
  rec_field_name_set (fname2, 1, "nonexistant");
  fail_if(rec_record_field_p (record, fname2));

  rec_record_destroy (record);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_record_field_p (void)
{
  TCase *tc = tcase_create("rec_record_field_p");
  tcase_add_test (tc, rec_record_field_p_existing);
  tcase_add_test (tc, rec_record_field_p_nonexisting);
  
  return tc;
}

/* End of rec-record-field-p.c */
