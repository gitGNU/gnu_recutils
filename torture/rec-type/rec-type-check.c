/* -*- mode: C -*- Time-stamp: "2010-10-29 18:55:26 jco"
 *
 *       File:         rec-type-check.c
 *       Date:         Fri Oct 29 18:50:01 2010
 *
 *       GNU recutils - rec_type_check unit tests
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
#include <check.h>

#include <rec.h>

/*-
 * Test: rec_type_check_int
 * Unit: rec_type_check
 * Description:
 * + Check strings of type int.
 */
START_TEST(rec_type_check_int)
{
  rec_type_t type;

  /* XXX */
}
END_TEST

/*-
 * Test: rec_type_check_bool
 * Unit: rec_type_check
 * Description:
 * + Check strings of type bool.
 */
START_TEST(rec_type_check_bool)
{
  rec_type_t type;

  /* XXX */
}
END_TEST

/*-
 * Test: rec_type_check_range
 * Unit: rec_type_check
 * Description:
 * + Check strings of type range.
 */
START_TEST(rec_type_check_range)
{
  rec_type_t type;

  /* XXX */
}
END_TEST

/*-
 * Test: rec_type_check_real
 * Unit: rec_type_check
 * Description:
 * + Check strings of type real.
 */
START_TEST(rec_type_check_real)
{
  rec_type_t type;

  /* XXX */
}
END_TEST

/*-
 * Test: rec_type_check_size
 * Unit: rec_type_check
 * Description:
 * + Check strings of type size.
 */
START_TEST(rec_type_check_size)
{
  rec_type_t type;

  /* XXX */
}
END_TEST

/*-
 * Test: rec_type_check_line
 * Unit: rec_type_check
 * Description:
 * + Check strings of type line.
 */
START_TEST(rec_type_check_line)
{
  rec_type_t type;

  /* XXX */
}
END_TEST

/*-
 * Test: rec_type_check_regexp
 * Unit: rec_type_check
 * Description:
 * + Check strings of type regexp.
 */
START_TEST(rec_type_check_regexp)
{
  rec_type_t type;

  /* XXX */
}
END_TEST

/*-
 * Test: rec_type_check_date
 * Unit: rec_type_check
 * Description:
 * + Check strings of type date.
 */
START_TEST(rec_type_check_date)
{
  rec_type_t type;

  /* XXX */
}
END_TEST

/*-
 * Test: rec_type_check_enum
 * Unit: rec_type_check
 * Description:
 * + Check strings of type enum.
 */
START_TEST(rec_type_check_enum)
{
  rec_type_t type;

  /* XXX */
}
END_TEST

/*-
 * Test: rec_type_check_field
 * Unit: rec_type_check
 * Description:
 * + Check strings of type field.
 */
START_TEST(rec_type_check_field)
{
  rec_type_t type;

  /* XXX */
}
END_TEST

/*-
 * Test: rec_type_check_email
 * Unit: rec_type_check
 * Description:
 * + Check strings of type email.
 */
START_TEST(rec_type_check_email)
{
  rec_type_t type;

  /* XXX */
}
END_TEST

/*
 * Test case creation function.
 */
TCase *
test_rec_type_check (void)
{
  TCase *tc = tcase_create ("rec_type_check");
  tcase_add_test (tc, rec_type_check_int);
  tcase_add_test (tc, rec_type_check_bool);
  tcase_add_test (tc, rec_type_check_range);
  tcase_add_test (tc, rec_type_check_real);
  tcase_add_test (tc, rec_type_check_size);
  tcase_add_test (tc, rec_type_check_line);
  tcase_add_test (tc, rec_type_check_regexp);
  tcase_add_test (tc, rec_type_check_date);
  tcase_add_test (tc, rec_type_check_enum);
  tcase_add_test (tc, rec_type_check_field);
  tcase_add_test (tc, rec_type_check_email);

  return tc;
}

/* End of rec-type-check.c */
