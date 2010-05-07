/* -*- mode: C -*-
 *
 *       File:         rec-utils.h
 *       Date:         Fri Apr  9 19:42:52 2010
 *
 *       GNU recutils - Miscellanea utilities
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

#ifndef REC_UTILS_H
#define REC_UTILS_H

#include <config.h>

#include <stdbool.h>

/* Parse an integer/real in the NULL-terminated string STR and store
   it at NUMBER.  Return true if the conversion was successful.  false
   otherwise. */
bool rec_atoi (char *str, int *number);
bool rec_atof (char *str, float *number);

#endif /* rec-utils.h */

/* End of rec-utils.h.  */
