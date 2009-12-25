/* -*- mode: C -*- Time-stamp: "09/12/25 18:28:16 jemarch"
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
  tcase_add_test (tc, rec_record_remove_field_001);

  return tc;
}


/* End of rec-record-remove-field.c */
