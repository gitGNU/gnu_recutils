/* -*- mode: C -*- Time-stamp: "2010-11-01 16:35:54 jco"
 *
 *       File:         rec-mset-get.c
 *       Date:         Thu Oct 28 20:35:33 2010
 *
 *       GNU recutils - Unit tests for rec_mset_get
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
#include <rec-mset/elem-types.h>

/*-
 * Test: rec_mset_get_empty
 * Unit: rec_mset_get
 * Description:
 * + Try to get an element from an empty mset.
 * +
 * + 1. The function shall return NULL.
 */
START_TEST(rec_mset_get_empty)
{
  rec_mset_t mset;

  mset = rec_mset_new ();
  fail_if (mset == NULL);
  fail_if (rec_mset_get (mset, MSET_ANY, 0) != NULL);

  rec_mset_destroy (mset);
}
END_TEST

/*-
 * Test: rec_mset_get_existing
 * Unit: rec_mset_get
 * Description:
 * + Get an existing element from a mset.
 * +
 * + 1. The function shall return a pointer to
 * +    the element.
 */
START_TEST(rec_mset_get_existing)
{
  int type;
  struct type1_t *elem1;
  rec_mset_elem_t e1;
  rec_mset_elem_t e1aux;
  rec_mset_t mset;

  /* Create a mset, register a type and insert two elements of that
     type.  */
  mset = rec_mset_new ();
  fail_if (mset == NULL);
  type = rec_mset_register_type (mset,
                                 TYPE1,
                                 type1_disp,
                                 type1_equal,
                                 type1_dup);
  elem1 = malloc (sizeof (struct type1_t));
  fail_if (elem1 == NULL);
  elem1->i = 1;
  e1 = rec_mset_elem_new (mset, type);
  fail_if (e1 == NULL);
  rec_mset_elem_set_data (e1, (void *) elem1);
  rec_mset_append (mset, e1);

  /* Get the element and compare.  */
  e1aux = rec_mset_get (mset, type, 0);
  fail_if (e1aux == NULL);
  fail_if (e1aux != e1);

  rec_mset_destroy (mset);
}
END_TEST

/*-
 * Test: rec_mset_get_any
 * Unit: rec_mset_get
 * Description:
 * + Get an existing element from a mset using
 * + the ANY index.
 * +
 * + 1. The function shall return a pointer to
 * +    the element.
 */
START_TEST(rec_mset_get_any)
{
  int type;
  struct type1_t *elem1;
  rec_mset_elem_t e1;
  rec_mset_elem_t e1aux;
  rec_mset_t mset;

  /* Create a mset, register a type and insert two elements of that
     type.  */
  mset = rec_mset_new ();
  fail_if (mset == NULL);
  type = rec_mset_register_type (mset,
                                 TYPE1,
                                 type1_disp,
                                 type1_equal,
                                 type1_dup);
  elem1 = malloc (sizeof (struct type1_t));
  fail_if (elem1 == NULL);
  elem1->i = 1;
  e1 = rec_mset_elem_new (mset, type);
  fail_if (e1 == NULL);
  rec_mset_elem_set_data (e1, (void *) elem1);
  rec_mset_append (mset, e1);

  /* Get the element and compare.  */
  e1aux = rec_mset_get (mset, MSET_ANY, 0);
  fail_if (e1aux == NULL);
  fail_if (e1aux != e1);

  rec_mset_destroy (mset);
}
END_TEST

/*-
 * Test: rec_mset_get_invalid
 * Unit: rec_mset_get
 * Description:
 * + Get the first element of a mset by using invalid
 * + indexes.
 * +
 * + 1. The function shall return NULL.
 */
START_TEST(rec_mset_get_invalid)
{
  int type;
  struct type1_t *elem1;
  rec_mset_elem_t e1;
  rec_mset_elem_t e1aux;
  rec_mset_t mset;

  /* Create a mset, register a type and insert two elements of that
     type.  */
  mset = rec_mset_new ();
  fail_if (mset == NULL);
  type = rec_mset_register_type (mset,
                                 TYPE1,
                                 type1_disp,
                                 type1_equal,
                                 type1_dup);
  elem1 = malloc (sizeof (struct type1_t));
  fail_if (elem1 == NULL);
  elem1->i = 1;
  e1 = rec_mset_elem_new (mset, type);
  fail_if (e1 == NULL);
  rec_mset_elem_set_data (e1, (void *) elem1);
  rec_mset_append (mset, e1);

  /* Try to get invalid elements.  */
  e1aux = rec_mset_get (mset, MSET_ANY, -10);
  fail_if (e1aux != NULL);
  e1aux = rec_mset_get (mset, MSET_ANY, 1000);
  fail_if (e1aux != NULL);

  rec_mset_destroy (mset);
}
END_TEST


/*
 * Test case creation function
 */
TCase *
test_rec_mset_get (void)
{
  TCase *tc = tcase_create ("rec_mset_get");
  tcase_add_test (tc, rec_mset_get_empty);
  tcase_add_test (tc, rec_mset_get_existing);
  tcase_add_test (tc, rec_mset_get_any);
  tcase_add_test (tc, rec_mset_get_invalid);

  return tc;
}

/* End of rec-mset-get.c */
