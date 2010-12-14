/* -*- mode: C -*- Time-stamp: "2010-12-14 22:07:15 jemarch"
 *
 *       File:         rec-parse-record.c
 *       Date:         Sat Nov 13 19:17:40 2010
 *
 *       GNU recutils - rec_parse_record unit tests.
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

/*-
 * Test: rec_parse_record_nominal
 * Unit: rec_parse_record
 * Description:
 * + Parse valid records.
 */
START_TEST(rec_parse_record_nominal)
{
  rec_parser_t parser;
  rec_record_t record;
  rec_field_t field;
  rec_field_name_t fname;
  char *str;

  str = "foo: bar";
  fname = rec_parse_field_name_str ("foo");
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_record (parser, &record));
  field = rec_record_elem_field (rec_record_get_field (record, 0));
  fail_if (strcmp (rec_field_value (field), "bar") != 0);
  fail_if (!rec_field_name_eql_p (fname,
                                  rec_field_name (field)));
  rec_field_name_destroy (fname);
  rec_record_destroy (record);
  rec_parser_destroy (parser);

  str = "foo: bar\nfoo2: bar2";
  fname = rec_parse_field_name_str ("foo");
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_record (parser, &record));
  field = rec_record_elem_field (rec_record_get_field (record, 0));
  fail_if (strcmp (rec_field_value (field), "bar") != 0);
  fail_if (!rec_field_name_eql_p (fname,
                                  rec_field_name (field)));
  rec_field_name_destroy (fname);
  fname = rec_parse_field_name_str ("foo2");
  field = rec_record_elem_field (rec_record_get_field (record, 1));
  fail_if (strcmp (rec_field_value (field), "bar2") != 0);
  fail_if (!rec_field_name_eql_p (fname,
                                  rec_field_name (field)));
  rec_record_destroy (record);
  rec_parser_destroy (parser);

  str = "foo: bar\nfoo2:\nfoo3: bar3";
  fname = rec_parse_field_name_str ("foo");
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_record (parser, &record));
  field = rec_record_elem_field (rec_record_get_field (record, 0));
  fail_if (strcmp (rec_field_value (field), "bar") != 0);
  fail_if (!rec_field_name_eql_p (fname,
                                  rec_field_name (field)));
  rec_field_name_destroy (fname);
  fname = rec_parse_field_name_str ("foo2");
  field = rec_record_elem_field (rec_record_get_field (record, 1));
  fail_if (strcmp (rec_field_value (field), "") != 0);
  fail_if (!rec_field_name_eql_p (fname,
                                  rec_field_name (field)));
  rec_field_name_destroy (fname);
  fname = rec_parse_field_name_str ("foo3");
  field = rec_record_elem_field (rec_record_get_field (record, 2));
  fail_if (strcmp (rec_field_value (field), "bar3") != 0);
  fail_if (!rec_field_name_eql_p (fname,
                                  rec_field_name (field)));
  rec_field_name_destroy (fname);
  rec_record_destroy (record);
  rec_parser_destroy (parser);
}
END_TEST

/*-
 * Test: rec_parse_record_invalid
 * Unit: rec_parse_record
 * Description:
 * + Try to parse invalid records.
 */
START_TEST(rec_parse_record_invalid)
{
  rec_parser_t parser;
  rec_record_t record;
  char *str;

  str = " ";
  parser = rec_parser_new_str (str, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_record (parser, &record));
  rec_parser_destroy (parser);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parse_record (void)
{
  TCase *tc = tcase_create ("rec_parse_record");
  tcase_add_test (tc, rec_parse_record_nominal);
  tcase_add_test (tc, rec_parse_record_invalid);

  return tc;
}

/* End of rec-parse-record.c */
