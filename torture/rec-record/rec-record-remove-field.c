/* -*- mode: C -*- Time-stamp: "09/10/01 13:50:17 jemarch"
 *
 *       File:         rec-record-remove-field.c
 *       Date:         Fri Mar  6 21:11:18 2009
 *
 *       GNU rec library - rec_record_remove_field unit tests
 *
 */

#include <config.h>
#include <check.h>

#include <rec.h>

/*
 * Test: rec_record_remove_field_001
 * Description:
 *   Remove a field from a record.
 * Success condition:
 *   1. The call to rec_record_remove_field should
 *      return true.
 *   2. The field should be removed from the record.
 */
START_TEST(rec_record_remove_field_001)
{
  rec_record_t record;
  rec_field_t field;

  /* Create a new record */
  record = rec_record_new ();
  fail_if(record == NULL);

  /* Create a new field and insert it into the record */
  field = rec_field_new ("name", "value");
  fail_if(field == NULL);
  fail_if(!rec_record_insert_field (record, field));

  /* Remove the field from the record */
  fail_if(!rec_record_remove_field (record, "name"));
  fail_if(rec_record_size (record) != 0);

  rec_record_destroy (record);
}
END_TEST

/*
 * Test: rec_record_remove_field_002
 * Description:
 *   Remove a nonexistent field from a record.
 * Success condition:
 *   1. The call to rec_record_remove_field should
 *      return false.
 *   2. The contents of the record should keep
 *      untouched.
 */
START_TEST(rec_record_remove_field_002)
{
  rec_record_t record;
  rec_field_t field;

  /* Create a new record */
  record = rec_record_new ();
  fail_if(record == NULL);

  /* Create a new field and insert it into the record */
  field = rec_field_new ("name", "value");
  fail_if(field == NULL);
  fail_if(!rec_record_insert_field (record, field));

  /* Remove the field from the record */
  fail_if(rec_record_remove_field (record, "name2"));
  fail_if(rec_record_size (record) != 1);

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
  tcase_add_test (tc, rec_record_remove_field_001);
  tcase_add_test (tc, rec_record_remove_field_002);

  return tc;
}


/* End of rec-record-remove-field.c */
