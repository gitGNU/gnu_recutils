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

#include <regex.h>
#include <stdlib.h>
#include <libintl.h>
#define _(str) dgettext (PACKAGE, str)
#include <string.h>

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
rec_atod (char *str,
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

char *
rec_extract_url (char *str)
{
  regex_t regexp;
  regmatch_t matches;
  char *rec_url = NULL;
  size_t rec_url_length = 0;

  if (regcomp (&regexp, REC_URL_REGEXP, REG_EXTENDED) != 0)
    {
      fprintf (stderr, _("internal error: rec_int_rec_extract_url: error compiling regexp.\n"));
      return NULL;
    }

  if ((regexec (&regexp, str, 1, &matches, 0) == 0)
      && (matches.rm_so != -1))
    {
      /* Get the match.  */
      rec_url_length = matches.rm_eo - matches.rm_so;
      rec_url = malloc (rec_url_length + 1);
      strncpy (rec_url, str + matches.rm_so, rec_url_length);
      rec_url[rec_url_length] = '\0';
    }

  regfree (&regexp);
  return rec_url;
}

char *
rec_extract_type (char *str)
{
  regex_t regexp;
  regmatch_t matches;
  char *rec_type = NULL;
  size_t rec_type_length = 0;

  if (regcomp (&regexp, REC_FNAME_PART_RE, REG_EXTENDED) != 0)
    {
      fprintf (stderr, _("internal error: rec_int_rec_extract_url: error compiling regexp.\n"));
      return NULL;
    }

  if ((regexec (&regexp, str, 1, &matches, 0) == 0)
      && (matches.rm_so != -1))
    {
      /* Get the match.  */
      rec_type_length = matches.rm_eo - matches.rm_so;
      rec_type = malloc (rec_type_length + 1);
      strncpy (rec_type, str + matches.rm_so, rec_type_length);
      rec_type[rec_type_length] = '\0';
    }

  regfree (&regexp);
  return rec_type;
}

bool
rec_parse_int (char **str, int *num)
{
  bool ret;
  char *p, *b;
  char number[30];
  bool first = true;

  ret = true;
  p = *str;

  b = p;
  while (rec_digit_p (*p) || (first && (*p == '-')))
    {
      number[p - b] = *p;
      p++;

      first = false;
    }
  number[p - b] = '\0';

  if (!rec_atoi (number, num))
    {
      ret = false;
    }

  *str = p;
  return ret;
}

bool
rec_parse_regexp (char **str, char *re, char **result)
{
  bool ret;
  char *p;
  regex_t regexp;
  regmatch_t pm;

  ret = true;
  p = *str;

  /* Compile the regexp.  */
  if (regcomp (&regexp, re, REG_EXTENDED) != 0)
    {
      ret = false;
    }

  if (ret)
    {
      /* Try to match the regexp.  */
      if (regexec (&regexp, p, 1, &pm, 0) == 0)
        {
          /* Get the match into 'result'.  Note that
             since the pattern starts with a ^ rm_so shall be 0 and we
             can use rm_eo relative to *p.  */
          *result = malloc (pm.rm_eo + 1);
          strncpy (*result, p, pm.rm_eo);
          (*result)[pm.rm_eo] = '\0';

          /* Advance 'p'.  */
          p = p + pm.rm_eo;
        }
      else
        {
          ret = false;
        }

      regfree (&regexp);
    }

  *str = p;
  return ret;
}

void
rec_skip_blanks (char **str)
{
  char *p;

  p = *str;
  while (rec_blank_p (*p))
    {
      p++;
    }

  *str = p;
}

bool
rec_blank_p (char c)
{
  return ((c == ' ')
          || (c == '\n')
          || (c == '\t'));
}

bool
rec_digit_p (char c)
{
  return ((c >= '0') && (c <= '9'));
}

bool
rec_letter_p (char c)
{
  return (((c >= 'a') && (c <= 'z'))
          || ((c >= 'A') && (c <= 'Z')));
}

/* End of rec-utils.c */
