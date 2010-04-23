/* -*- mode: C -*- Time-stamp: "2010-04-23 16:40:26 jco"
 *
 *       File:         rec-types.c
 *       Date:         Fri Apr 23 14:10:05 2010
 *
 *       GNU recutils - Field types.
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
#include <stdio.h>
#include <regex.h>

#include <rec.h>

/*
 * Constants.
 */

/* Textual name of the types expected in the type description
   strings.  */
#define REC_TYPE_INT_NAME    "int"
#define REC_TYPE_RANGE_NAME  "range"
#define REC_TYPE_REAL_NAME   "real"
#define REC_TYPE_SIZE_NAME   "size"
#define REC_TYPE_LINE_NAME   "line"
#define REC_TYPE_REGEXP_NAME "regexp"
#define REC_TYPE_DATE_NAME   "date"
#define REC_TYPE_ENUM_NAME   "enum"
#define REC_TYPE_FIELD_NAME  "field"

/* Regular expression denoting a blank character in a type
   description.  */
#define REC_TYPE_BLANK_RE "[ \t\n]"
#define REC_TYPE_BLANKS_RE REC_TYPE_BLANK_RE "+"
#define REC_TYPE_ZBLANKS_RE REC_TYPE_BLANK_RE "*"

/* Regular expression denoting a type name.  */
#define REC_TYPE_NAME_RE                              \
  "(" REC_TYPE_INT_NAME  "|" REC_TYPE_RANGE_NAME  "|" \
      REC_TYPE_REAL_NAME "|" REC_TYPE_SIZE_NAME   "|" \
      REC_TYPE_LINE_NAME "|" REC_TYPE_REGEXP_NAME "|" \
      REC_TYPE_DATE_NAME "|" REC_TYPE_ENUM_NAME   "|" \
      REC_TYPE_FIELD_NAME                             \
  ")"

/* Regular expressions for the type descriptions.  */

/* int  */
#define REC_TYPE_INT_DESCR_RE                      \
  REC_TYPE_INT_NAME

/* range MIN..MAX  */
#define REC_TYPE_RANGE_DESCR_RE                    \
  REC_TYPE_RANGE_NAME                              \
  REC_TYPE_BLANKS_RE                               \
  "[0-9]+"                                         \
  REC_TYPE_ZBLANKS_RE "\\.\\." REC_TYPE_ZBLANKS_RE   \
  "[0-9]+"

/* real  */
#define REC_TYPE_REAL_DESCR_RE                     \
  REC_TYPE_REAL_NAME

/* size NUM  */
#define REC_TYPE_SIZE_DESCR_RE                     \
  REC_TYPE_SIZE_NAME                               \
  REC_TYPE_BLANKS_RE                               \
  "[0-9]+"

/* line  */
#define REC_TYPE_LINE_DESCR_RE                  \
  REC_TYPE_LINE_NAME

/* regexp /RE/  */
#define REC_TYPE_REGEXP_DESCR_RE                \
  REC_TYPE_REGEXP_NAME                          \
  REC_TYPE_BLANKS_RE                            \
  "/([^/]|\\/)*/"

/* date  */
#define REC_TYPE_DATE_DESCR_RE                  \
  REC_TYPE_DATE_NAME

/* enum NAME NAME NAME*/
#define REC_TYPE_ENUM_NAME_RE                   \
  "[a-zA-Z0-9][a-zA-Z0-9_-]*"
#define REC_TYPE_ENUM_DESCR_RE                  \
  REC_TYPE_ENUM_NAME                            \
  REC_TYPE_BLANKS_RE                            \
  REC_TYPE_ENUM_NAME_RE                         \
  "(" REC_TYPE_BLANKS_RE REC_TYPE_ENUM_NAME_RE ")*"

/* field FIELD_NAME  */
#define REC_TYPE_FIELD_PART_RE                  \
  "[a-zA-Z%][a-zA-Z0-9_]"
#define REC_TYPE_FIELD_DESCR_RE                 \
  REC_TYPE_FIELD_NAME                           \
  REC_TYPE_BLANKS_RE                            \
  REC_TYPE_FIELD_PART_RE                        \
  "(" ":" REC_TYPE_FIELD_PART_RE ")*" ":?"

/* Regexp denoting any type description.  */
#define REC_TYPE_DESCR_RE                       \
  REC_TYPE_ZBLANKS_RE                           \
  "("                                           \
         REC_TYPE_INT_DESCR_RE                  \
     "|" REC_TYPE_RANGE_DESCR_RE                \
     "|" REC_TYPE_REAL_DESCR_RE                 \
     "|" REC_TYPE_SIZE_DESCR_RE                 \
     "|" REC_TYPE_LINE_DESCR_RE                 \
     "|" REC_TYPE_REGEXP_DESCR_RE               \
     "|" REC_TYPE_DATE_DESCR_RE                 \
     "|" REC_TYPE_ENUM_DESCR_RE                 \
     "|" REC_TYPE_FIELD_DESCR_RE                \
  "("                                           \
  REC_TYPE_ZBLANKS_RE
      
/*
 * Data types.
 */

struct rec_type_s
{
  enum rec_type_kind_e kind;  /* Kind of the type.  */
  char *expr;                 /* Copy of the type descriptor used to
                                 create the type.  */
  union
  {
    size_t max_size;          /* Size of string.  */
    int min;                  /* Range.  */
    int max;
    char *regexp;             /* Regular expression.  */
    char **names;             /* Names in enumeration.  */
  } data;
};

/*
 * Forward declarations.
 */

static bool rec_type_check_re (char *regexp_str, char *str);
static enum rec_type_kind_e rec_type_parse_type_kind (char *str);
static bool rec_type_blank_p (char c);
static bool rec_type_digit_p (char c);
static bool rec_type_letter_p (char c);

/*
 * Public functions.
 */

bool
rec_type_descr_p (char *str)
{
  return rec_type_check_re (REC_TYPE_DESCR_RE, str);
}

rec_type_t
rec_type_new (char *str)
{
  rec_type_t new;
  char *p, *b;
  char name[100];
  char number[30];

  if (!rec_type_descr_p (str))
    {
      return NULL;
    }

  new = malloc (sizeof (struct rec_type_s));
  if (new)
    {
      /* Skip blanks.  */
      p = str;
      while (p && rec_type_blank_p (*p))
        {
          p++;
        }

      /* Get the type name.  */
      b = p;
      while (p && rec_type_letter_p (*p))
        {
          name[p - b] = *p;
          p++;
        }
      name[p - b] = '\0';
      
      new->kind = rec_type_parse_type_kind (name);
      switch (new->kind)
        {
        case REC_TYPE_NONE:
          {
            /* This point should not be reached.  */
            fprintf (stderr,
                     "internal error: got REC_TYPE_NONE from rec_type_parse_type kind in rec_type_new.\n");
            exit (1);
            break;
          }
        case REC_TYPE_INT:
        case REC_TYPE_REAL:
        case REC_TYPE_LINE:
          {
            /* We are done.  */
            break;
          }
        case REC_TYPE_SIZE:
          {
            /* Skip blanks.  */
            while (p && rec_type_blank_p (*p))
              {
                p++;
              }

            /* Get the number.  */
            b = p;
            while (p && rec_type_digit_p (*p))
              {
                number[p - b] = *p;
                p++;
              }
            rec_atoi (number, &(new->data.max_size));

            break;
          }
        case REC_TYPE_RANGE:
        case REC_TYPE_REGEXP:
        case REC_TYPE_ENUM:
        case REC_TYPE_FIELD:
          {
            /* Not implemented yet.  */
            fprintf (stderr,
                     "internal error: type not implemented.\n");
            exit (1);
            break;
          }
        }
    }

  return new;
}

/*
 * Private functions.
 */

static bool
rec_type_check_re (char *regexp_str,
                   char *str)
{
  int status;
  regex_t regexp;

  /* Compile the regexp.  */
  if ((status = regcomp (&regexp, regexp_str, REG_EXTENDED)) != 0)
    {
      fprintf (stderr, "internal error: rec-types: error compiling regexp.\n");
      return false;
    }

  /* Check.  */
  status = regexec (&regexp, str, 0, NULL, 0);

  return (status == 0);
}

static enum rec_type_kind_e
rec_type_parse_type_kind (char *str)
{
  enum rec_type_kind_e res;

  res = REC_TYPE_NONE;

  if (strcmp (str, REC_TYPE_INT_NAME) == 0)
    {
      res = REC_TYPE_INT;
    }
  if (strcmp (str, REC_TYPE_RANGE_NAME) == 0)
    {
      res = REC_TYPE_RANGE;
    }
  if (strcmp (str, REC_TYPE_REAL_NAME) == 0)
    {
      res = REC_TYPE_REAL;
    }
  if (strcmp (str, REC_TYPE_SIZE_NAME) == 0)
    {
      res = REC_TYPE_SIZE;
    }
  if (strcmp (str, REC_TYPE_LINE_NAME) == 0)
    {
      res = REC_TYPE_LINE;
    }
  if (strcmp (str, REC_TYPE_REGEXP_NAME) == 0)
    {
      res = REC_TYPE_REGEXP;
    }
  if (strcmp (str, REC_TYPE_DATE_NAME) == 0)
    {
      res = REC_TYPE_DATE;
    }
  if (strcmp (str, REC_TYPE_ENUM_NAME) == 0)
    {
      res = REC_TYPE_ENUM;
    }
  if (strcmp (str, REC_TYPE_FIELD_NAME) == 0)
    {
      res = REC_TYPE_FIELD;
    }
     
  return res;
}

static bool
rec_type_blank_p (char c)
{
  return ((c == ' ')
          || (c == '\n')
          || (c == '\t'));
}

static bool
rec_type_digit_p (char c)
{
  return ((c >= '0') && (c <= '9'));
}

static bool
rec_type_letter_p (char c)
{
  return (((c >= 'a') && (c <= 'z'))
          || ((c >= 'A') && (c <= 'Z')));
}

/* End of rec-types.c */
