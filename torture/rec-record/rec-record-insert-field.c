/* -*- mode: C -*-
 *
 *       File:         rec-record-insert-field.c
 *       Date:         Fri Mar  6 20:08:59 2009
 *
 *       GNU recutils - rec_record_insert_field_001
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
 * Test: rec_record_insert_field_empty
 * Unit: rec_record_insert_field
 * Description:
 * + Insert a field into an empty record.
 * +
 * + 1. The call to rec_record_insert_field should
 * +    return true.
 */
START_TEST(rec_record_insert_field_empty)
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
 * Test: rec_record_insert_field_nonempty
 * Unit: rec_record_insert_field
 * Description:
 * +  Insert a field into a non empy record.
 * +
 * + 1. The call to rec_record_insert_field should
 * +    success.
 */
START_TEST(rec_record_insert_field_nonempty)
{
  rec_record_t record;
  rec_field_t field;
  rec_field_name_t fname;
  rec_field_t field2;
  rec_field_name_t fname2;

  /* Create a new record */
  record = rec_record_new ();
  fail_if(record == NULL);

  /* Create a new field and insert it into the record */
  fname = rec_field_name_new ();
  rec_field_name_set (fname, 0, "name");
  field = rec_field_new (fname, "value");
  fail_if(field == NULL);
  fail_if(!rec_record_insert_field (record, field, rec_record_size (record)));

  /* Create a new field and insert it into the record */
  fname2 = rec_field_name_new ();
  rec_field_name_set (fname, 0, "name2");
  field2 = rec_field_new (fname2, "value");
  fail_if(field2 == NULL);
  fail_if(!rec_record_insert_field (record, field2, rec_record_size (record)));

  /* Check for the existence of the field into the record */
  fail_if(!rec_record_field_p (record, fname2));

  rec_record_destroy (record);
}
END_TEST

/*-
 * Test: rec_record_insert_field_duplicated
 * Unit: rec_record_insert_field
 * Description:
 * + Fields with duplicated names are not allowed.
 * +
 * + 1. The attempt to insert a duplicated field
 * +    should success.
 */
START_TEST(rec_record_insert_field_duplicated)
{
  rec_record_t record;
  rec_field_t field;
  rec_field_name_t fname;
  rec_field_t field2;
  rec_field_name_t fname2;

  /* Create a new record */
  record = rec_record_new ();
  fail_if(record == NULL);

  /* Create a new field and insert it into the record */
  fname = rec_field_name_new ();
  rec_field_name_set (fname, 0, "name");
  field = rec_field_new (fname, "value");
  fail_if(field == NULL);
  fail_if(!rec_record_insert_field (record, field, rec_record_size (record)));

  /* Create a new field and insert it into the record */
  fname2 = rec_field_name_new ();
  rec_field_name_set (fname2, 0, "name");
  field2 = rec_field_new (fname2, "value");
  fail_if(field2 == NULL);
  fail_if(!rec_record_insert_field (record, field2, rec_record_size (record)));

  /* Check for the existence of the field into the record */
  fail_if(rec_record_size (record) != 2);

  rec_record_destroy (record);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_record_insert_field (void)
{
  TCase *tc = tcase_create("rec_record_insert_field");
  tcase_add_test (tc, rec_record_insert_field_empty);
  tcase_add_test (tc, rec_record_insert_field_nonempty);
  tcase_add_test (tc, rec_record_insert_field_duplicated);

  return tc;
}

/* End of rec-record-insert-field.c */
