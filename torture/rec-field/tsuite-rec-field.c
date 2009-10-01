/* -*- mode: C -*- Time-stamp: "09/10/01 13:48:18 jemarch"
 *
 *       File:         tsuite-rec-field.c
 *       Date:         Sun Mar  1 17:06:28 2009
 *
 *       Fairly Light Files - rec_field test suite
 *
 */

/* Copyright (C) 2009 Jose E. Marchesi */

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

extern TCase *test_rec_field_get_name (void);
extern TCase *test_rec_field_set_name (void);
extern TCase *test_rec_field_get_type (void);
extern TCase *test_rec_field_set_type (void);
extern TCase *test_rec_field_get_value (void);
extern TCase *test_rec_field_set_value (void);
extern TCase *test_rec_field_dup (void);

Suite *
tsuite_rec_field ()
{
  Suite *s;

  s = suite_create ("rec-field");
  suite_add_tcase (s, test_rec_field_get_name ());
  suite_add_tcase (s, test_rec_field_set_name ());
  suite_add_tcase (s, test_rec_field_get_type ());
  suite_add_tcase (s, test_rec_field_set_type ());
  suite_add_tcase (s, test_rec_field_get_value ());
  suite_add_tcase (s, test_rec_field_set_value ());
  suite_add_tcase (s, test_rec_field_dup ());

  return s;
}

/* End of tsuite-rec-field.c */
