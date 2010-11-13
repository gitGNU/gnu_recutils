/* -*- mode: C -*- Time-stamp: "2010-11-13 22:35:51 jemarch"
 *
 *       File:         rec-parser-reset.c
 *       Date:         Sat Nov 13 22:34:36 2010
 *
 *       GNU recutils - rec_parser_reset unit tests.
 *
 */

#include <check.h>

#include <rec.h>

/*-
 * Test: rec_parser_reset_nominal
 * Unit: rec_parser_reset
 * Description:
 * + Reset a rec parser.
 */
START_TEST(rec_parser_reset_nominal)
{
  rec_parser_t parser;
  rec_field_t field;
  FILE *stm;
  char *str;

  str = "invalid field";
  stm = fmemopen (str, strlen (str), "r");
  fail_if (stm == NULL);
  parser = rec_parser_new (stm, "dummy");
  fail_if (parser == NULL);
  fail_if (rec_parser_error (parser));
  fail_if (rec_parse_field (parser, &field));
  fail_if (!rec_parser_error (parser));
  rec_parser_reset (parser);
  fail_if (rec_parser_error (parser));
  rec_parser_destroy (parser);
  fclose (stm);
}
END_TEST

/*
 * Test creation function
 */
TCase *
test_rec_parser_reset (void)
{
  TCase *tc = tcase_create ("rec_parser_reset");
  tcase_add_test (tc, rec_parser_reset_nominal);

  return tc;
}

/* End of rec-parser-reset.c */
