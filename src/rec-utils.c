/* -*- mode: C -*-
 *
 *       File:         rec-utils.c
 *       Date:         Fri Apr  9 19:45:00 2010
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

#include <config.h>

#include <stdlib.h>

#include <rec-utils.h>

bool
rec_atoi (char *str,
          int *number)
{
  bool res;
  long int li;
  char *end;

  res = false;
 
  li = strtol (str, &end, 10);
  if ((*str != '\0') && (*end == '\0'))
    {
      /* The entire string is valid.  */
      res = true;
      *number = (int) li;
    }

  return res;
}

bool
rec_atof (char *str,
          double *number)
{
  bool res;
  char *end;

  res = false;
  
  *number = strtod (str, &end);
  if ((*str != '\0') && (*end == '\0'))
    {
      /* The entire string is valid.  */
      res = true;
    }
  
  return res;
}

/* End of rec-utils.c */
