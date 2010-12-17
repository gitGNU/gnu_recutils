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

#include <rec.h>
#include <stdbool.h>

/* Parse an integer/real in the NULL-terminated string STR and store
   it at NUMBER.  Return true if the conversion was successful.  false
   otherwise. */
bool rec_atoi (char *str, int *number);
bool rec_atod (char *str, double *number);

/* Extract type and url from a %rec: field value.  */
char *rec_extract_url (char *str);
char *rec_extract_file (char *str);
char *rec_extract_type (char *str);

/* Generic parsing routines.  */
bool rec_blank_p (char c);
bool rec_digit_p (char c);
bool rec_letter_p (char c);
bool rec_parse_int (char **str, int *num);
void rec_skip_blanks (char **str);
bool rec_parse_regexp (char **str, char *re, char **result);

/* Auto-growing buffer implementation.  */
typedef struct rec_buf_s *rec_buf_t;


rec_buf_t rec_buf_new (char **data, size_t *size);
void rec_buf_close (rec_buf_t buffer);

/* rec_buf_putc returns the character written as an unsigned char cast
   to an int, or EOF on error.  */
int rec_buf_putc (int c, rec_buf_t buffer);
/* rec_buf_puts returns a non-negative number on success (number of
   characters written), or EOF on error.  */
int rec_buf_puts (const char *s, rec_buf_t buffer);

void rec_buf_rewind (rec_buf_t buf, int n);

#endif /* rec-utils.h */

/* End of rec-utils.h.  */
