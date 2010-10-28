/* -*- mode: C -*- Time-stamp: "2010-10-28 16:47:20 jco"
 *
 *       File:         tsuite-rec-mset.c
 *       Date:         Thu Oct 28 16:45:34 2010
 *
 *       GNU recutils - rec_mset test suite
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

extern TCase *test_rec_mset_new (void);

Suite *
tsuite_rec_mset ()
{
  Suite *s;
  
  s = suite_create ("rec-mset");
  suite_add_tcase (s, test_rec_mset_new ());

  return s;
}

/* End of tsuite-rec-mset.c */
