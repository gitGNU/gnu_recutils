/* -*- mode: C -*- Time-stamp: "09/10/01 23:06:39 jemarch"
 *
 *       File:         rec-record-insert-field.c
 *       Date:         Fri Mar  6 20:08:59 2009
 *
 *       GNU rec library - rec_record_insert_field_001
 *
 */

#include <config.h>
#include <check.h>

#include <rec.h>

/*
 * Test: rec_record_insert_field_001
 * Description:
 *   Insert a field into an empty record.
 * Success conditions:
 *   1. The call to rec_record_insert_field should
 *      return true.
 */
START_TEST(rec_record_insert_field_001)
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

  /* Check for the existence of the field into the record */
  fail_if(!rec_record_field_p (record, "name"));

  rec_record_destroy (record);
}
END_TEST

/*
 * Test: rec_record_insert_field_002
 * Description:
 *   Insert a field into a non empy record.
 * Success conditions:
 *   1. The call to rec_record_insert_field should
 *      success.
 */
START_TEST(rec_record_insert_field_002)
{
  rec_record_t record;
  rec_field_t field;
  rec_field_t field2;

  /* Create a new record */
  record = rec_record_new ();
  fail_if(record == NULL);

  /* Create a new field and insert it into the record */
  field = rec_field_new ("name", "value");
  fail_if(field == NULL);
  fail_if(!rec_record_insert_field (record, field));

  /* Create a new field and insert it into the record */
  field2 = rec_field_new ("name2", "value");
  fail_if(field2 == NULL);
  fail_if(!rec_record_insert_field (record, field2));

  /* Check for the existence of the field into the record */
  fail_if(!rec_record_field_p (record, "name2"));

  rec_record_destroy (record);
}
END_TEST

/*
 * Test: rec_record_insert_field_003
 * Description:
 *   Fields with duplicated names are not allowed.
 * Success conditions:
 *   1. The attempt to insert a duplicated field
 *      should success.
 */
START_TEST(rec_record_insert_field_003)
{
  rec_record_t record;
  rec_field_t field;
  rec_field_t field2;

  /* Create a new record */
  record = rec_record_new ();
  fail_if(record == NULL);

  /* Create a new field and insert it into the record */
  field = rec_field_new ("name", "value");
  fail_if(field == NULL);
  fail_if(!rec_record_insert_field (record, field));

  /* Create a new field and insert it into the record */
  field2 = rec_field_new ("name", "value");
  fail_if(field2 == NULL);
  fail_if(!rec_record_insert_field (record, field2));

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
  tcase_add_test (tc, rec_record_insert_field_001);
  tcase_add_test (tc, rec_record_insert_field_002);
  tcase_add_test (tc, rec_record_insert_field_003);

  return tc;
}

/* End of rec-record-insert-field.c */
