/* -*- mode: C -*-
 *
 *       File:         runtests.c
 *       Date:         Sun Mar  1 17:00:14 2009
 *
 *       GNU recutils - Unit testing driver
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

#include <stdio.h>
#include <check.h>

extern Suite *tsuite_rec_mset (void);
extern Suite *tsuite_rec_field (void);
extern Suite *tsuite_rec_record (void);

int
main (int argc, char **argv)
{
  int failures;
  SRunner *sr;

  sr = srunner_create (NULL);

  srunner_add_suite (sr, tsuite_rec_mset ());
  srunner_add_suite (sr, tsuite_rec_field ());
  srunner_add_suite (sr, tsuite_rec_record ());

  srunner_set_log (sr, "tests.log");

  srunner_run_all (sr, CK_ENV);
  failures = srunner_ntests_failed (sr);
  srunner_free (sr);

  return (failures == 0) ? 0 : 1;
}

/* End of runtests.c */
