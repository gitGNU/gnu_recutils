/* -*- mode: C -*- Time-stamp: "2010-10-28 20:28:12 jemarch"
 *
 *       File:         rec-mset-count.c
 *       Date:         Thu Oct 28 20:17:55 2010
 *
 *       GNU recutils - unit tests for rec_mset_count
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
#include <check.h>

#include <rec-mset.h>
#include "elem-types.h"

/*-
 * Test: rec_mset_count_empty
 * Unit: rec_mset_count
 * Description:
 * + Count the number of total elements of an
 * + empty mset.
 * +
 * + 1. The function shall return 0.
 */
START_TEST(rec_mset_count_empty)
{
  rec_mset_t mset;
  
  mset = rec_mset_new ();
  fail_if (mset == NULL);
  fail_if (rec_mset_count (mset, MSET_ANY) != 0);
  
  rec_mset_destroy (mset);
}
END_TEST

/*-
 * Test: rec_mset_count_existing
 * Unit: rec_mset_count
 * Description:
 * + Count the number of elements of a non empty
 * + mset.
 * +
 * + 1. The function shall return the number of
 * +    elements.
 */
START_TEST(rec_mset_count_existing)
{
  int type;
  struct type1_t *elem1;
  struct type1_t *elem2;
  rec_mset_elem_t e1;
  rec_mset_elem_t e2;
  rec_mset_t mset1;

  /* Create a mset, register a type and insert two elements of that
     type.  */
  mset1 = rec_mset_new ();
  fail_if (mset1 == NULL);
  type = rec_mset_register_type (mset1,
                                 TYPE1,
                                 type1_disp,
                                 type1_equal,
                                 type1_dup);
  elem1 = malloc (sizeof (struct type1_t));
  fail_if (elem1 == NULL);
  elem1->i = 1;
  e1 = rec_mset_elem_new (mset1, type);
  fail_if (e1 == NULL);
  rec_mset_elem_set_data (e1, (void *) elem1);
  rec_mset_append (mset1, e1);

  elem2 = malloc (sizeof (struct type1_t));
  fail_if (elem2 == NULL);
  elem2->i = 2;
  e2 = rec_mset_elem_new (mset1, type);
  fail_if (e2 == NULL);
  rec_mset_elem_set_data (e2, (void *) elem2);
  rec_mset_append (mset1, e2);

  /* Count elements.  */
  fail_if (rec_mset_count (mset1, MSET_ANY) != 2);
  fail_if (rec_mset_count (mset1, type) != 2);

  rec_mset_destroy (mset1);
}
END_TEST

/*-
 * Test: rec_mset_count_nonexisting
 * Unit: rec_mset_count
 * Description:
 * + Count the number of total elements pertaining to
 * + an unexisting type in a mset.
 * +
 * + 1. The function shall return 0.
 */
START_TEST(rec_mset_count_nonexisting)
{
  rec_mset_t mset;
  
  mset = rec_mset_new ();
  fail_if (mset == NULL);
  fail_if (rec_mset_count (mset, MSET_ANY + 1) != 0);
  
  rec_mset_destroy (mset);
}
END_TEST

/*
 * Test case creation function
 */
TCase *
test_rec_mset_count (void)
{
  TCase *tc = tcase_create ("rec_mset_count");
  tcase_add_test (tc, rec_mset_count_empty);
  tcase_add_test (tc, rec_mset_count_existing);
  tcase_add_test (tc, rec_mset_count_nonexisting);

  return tc;
}

/* End of rec-mset-count.c */
