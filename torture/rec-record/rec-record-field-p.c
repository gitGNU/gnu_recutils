/* -*- mode: C -*- Time-stamp: "09/12/23 20:31:20 jemarch"
 *
 *       File:         rec-record-field-p.c
 *       Date:         Fri Mar  6 20:01:09 2009
 *
 *       GNU rec library - rec_record_field_p unit tests
 *
 */

#include <config.h>
#include <check.h>

#include <rec.h>

/*
 * Test: rec_record_field_p_001
 * Description:
 *   Check for the existence of a field included
 *   in the record.
 * Success conditions:
 *   1. The call to rec_record_field_p should 
 *      return true
 */
START_TEST(rec_record_field_p_001)
{
  rec_record_t record;
  rec_field_t field;

  /* Create a new record */
  record = rec_record_new ();
  fail_if(record == NULL);

  /* Create a new field and insert it into the record */
  field = rec_field_new ("name", "value");
  fail_if(field == NULL);
  fail_if(!rec_record_insert_field (record, field, rec_record_size (record)));

  /* Check for the existence of the field into the record */
  fail_if(!rec_record_field_p (record, "name"));

  rec_record_destroy (record);
}
END_TEST

/*
 * Test: rec_record_field_p_002
 * Description:
 *   Check for the existence of a field not included
 *   in the record.
 * Success conditions:
 *   1. The call to rec_record_field_p should 
 *      return false.
 */
START_TEST(rec_record_field_p_002)
{
  rec_record_t record;
  rec_field_t field;

  /* Create a new record */
  record = rec_record_new ();
  fail_if(record == NULL);

  /* Create a new field and insert it into the record*/
  field = rec_field_new ("name", "value");
  fail_if(field == NULL);
  fail_if(!rec_record_insert_field (record, field, rec_record_size (record)));

  /* Check for the existence of the field into the record */
  fail_if(rec_record_field_p (record, "nonexistant"));

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
  tcase_add_test (tc, rec_record_field_p_001);
  tcase_add_test (tc, rec_record_field_p_002);
  
  return tc;
}

/* End of rec-record-field-p.c */
