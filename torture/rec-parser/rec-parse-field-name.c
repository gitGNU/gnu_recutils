/* -*- mode: C -*- Time-stamp: "2010-11-13 17:36:39 jemarch"
 *
 *       File:         rec-parse-field-name.c
 *       Date:         Sat Nov 13 16:31:54 2010
 *
 *       GNU recutils - rec_parse_field_name unit tests.
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
#include <string.h>
#include <stdio.h>
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_parse_field_name_nominal
 * Unit: rec_parse_field_name
 * Description:
 * + Parse valid field names.
 */
START_TEST(rec_parse_field_name_nominal)
{
  rec_parser_t parser;
  rec_field_name_t fname;
  FILE *stm;
  char *str;

  str = "foo:";
  stm = fmemopen (str, strlen (str), "r");
  fail_if (stm == NULL);
  parser = rec_parser_new (stm, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field_name (parser, &fname));
  fail_if (rec_field_name_size (fname) != 1);
  fail_if (strcmp (rec_field_name_get (fname, 0), "foo") != 0);
  rec_field_name_destroy (fname);
  rec_parser_destroy (parser);
  fclose (stm);

  str = "%foo:";
  stm = fmemopen (str, strlen (str), "r");
  fail_if (stm == NULL);
  parser = rec_parser_new (stm, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field_name (parser, &fname));
  fail_if (rec_field_name_size (fname) != 1);
  fail_if (strcmp (rec_field_name_get (fname, 0), "%foo") != 0);
  rec_field_name_destroy (fname);
  rec_parser_destroy (parser);
  fclose (stm);

  str = "foo:bar:";
  stm = fmemopen (str, strlen (str), "r");
  fail_if (stm == NULL);
  parser = rec_parser_new (stm, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field_name (parser, &fname));
  fail_if (rec_field_name_size (fname) != 2);
  fail_if (strcmp (rec_field_name_get (fname, 0), "foo") != 0);
  fail_if (strcmp (rec_field_name_get (fname, 1), "bar") != 0);
  rec_field_name_destroy (fname);
  rec_parser_destroy (parser);
  fclose (stm);

  str = "foo:bar:baz:";
  stm = fmemopen (str, strlen (str), "r");
  fail_if (stm == NULL);
  parser = rec_parser_new (stm, "dummy");
  fail_if (parser == NULL);
  fail_if (!rec_parse_field_name (parser, &fname));
  fail_if (rec_field_name_size (fname) != 3);
  fail_if (strcmp (rec_field_name_get (fname, 0), "foo") != 0);
  fail_if (strcmp (rec_field_name_get (fname, 1), "bar") != 0);
  fail_if (strcmp (rec_field_name_get (fname, 2), "baz") != 0);
  rec_field_name_destroy (fname);
  rec_parser_destroy (parser);
  fclose (stm);
}
END_TEST

/*-
 * Test: rec_parse_field_name_invalid
 * Unit: rec_parse_field_name
 * Description:
 * + Try to parse invalid field names.
 */
START_TEST(rec_parse_field_name_invalid)
{
  rec_parser_t parser;
  rec_field_name_t fname;
  FILE *stm;
  char *str;  

  str = " ";
  stm = fmemopen (str, strlen (str), "r");
  fail_if (stm == NULL);
  parser = rec_parser_new (stm, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_field_name (parser, &fname));
  rec_parser_destroy (parser);
  fclose (stm);

  str = "foo";
  stm = fmemopen (str, strlen (str), "r");
  fail_if (stm == NULL);
  parser = rec_parser_new (stm, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_field_name (parser, &fname));
  rec_parser_destroy (parser);
  fclose (stm);

  str = ":foo";
  stm = fmemopen (str, strlen (str), "r");
  fail_if (stm == NULL);
  parser = rec_parser_new (stm, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_field_name (parser, &fname));
  rec_parser_destroy (parser);
  fclose (stm);

  str = "fo!o";
  stm = fmemopen (str, strlen (str), "r");
  fail_if (stm == NULL);
  parser = rec_parser_new (stm, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_field_name (parser, &fname));
  rec_parser_destroy (parser);
  fclose (stm);

  str = "%%foo";
  stm = fmemopen (str, strlen (str), "r");
  fail_if (stm == NULL);
  parser = rec_parser_new (stm, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parse_field_name (parser, &fname));
  rec_parser_destroy (parser);
  fclose (stm);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parse_field_name (void)
{
  TCase *tc = tcase_create ("rec_parse_field_name");
  tcase_add_test (tc, rec_parse_field_name_nominal);
  tcase_add_test (tc, rec_parse_field_name_invalid);

  return tc;
}

/* End of rec-parse-field-name.c */
