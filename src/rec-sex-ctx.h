/* -*- mode: C -*- Time-stamp: "10/01/11 21:48:19 jemarch"
 *
 *       File:         rec-sex-ctx.h
 *       Date:         Sat Jan  9 20:22:52 2010
 *
 *       GNU Records - Select Expressions parse context
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

#ifndef REC_SEX_CTX_H
#define REC_SEX_CTX_H

#include <config.h>

#include <rec.h>

struct rec_sex_ctx_s
{
  char *in;              /* String to be parsed.  */
  size_t index;          /* Index in in_str.  */
  rec_record_t record;   /* Record to apply the expr. on.  */
  void *scanner;         /* Flex scanner.  */

  bool result;
};

enum rec_sex_val_type_e
{
  REC_SEX_INT,
  REC_SEX_STR
};

struct rec_sex_val_s
{
  enum rec_sex_val_type_e type;

  int int_val;
  char *str_val;
};

typedef struct rec_sex_val_s *rec_sex_val_t;

/* Forward reference for the bison parser.  */
int sexparse (struct rec_sex_ctx_s *sex_ctx);

#endif /* rec-sex-ctx.h */

/* End of rec-sex-ctx.h */
