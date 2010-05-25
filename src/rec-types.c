/* -*- mode: C -*-
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
#include <string.h>

#include <rec.h>

/*
 * Constants.
 */

/* Textual name of the types expected in the type description
   strings.  */
#define REC_TYPE_INT_NAME    "int"
#define REC_TYPE_BOOL_NAME   "bool"
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

/* Regular expressions denoting values.  */
#define REC_TYPE_INT_VALUE_RE                   \
  "^" REC_TYPE_ZBLANKS_RE "[0-9]+" REC_TYPE_ZBLANKS_RE "$"

#define REC_TYPE_BOOL_VALUE_RE                  \
  "^(yes|no|true|false|0|1)$"

#define REC_TYPE_REAL_VALUE_RE                  \
  "^" REC_TYPE_ZBLANKS_RE "[0-9]+(\\.[0-9]+)?" REC_TYPE_ZBLANKS_RE "$"

#define REC_TYPE_LINE_VALUE_RE                  \
  "^[^\n]*$"

#define REC_TYPE_REGEXP_VALUE_RE                \
  "XXX_TO_BE_DEFINED_DONT_USE"

#define REC_TYPE_DATE_VALUE_RE                  \
  "XXX_TO_BE_DEFINED_DONT_USE"

#define REC_TYPE_ENUM_NAME_RE                   \
  "[a-zA-Z0-9][a-zA-Z0-9_-]*"
#define REC_TYPE_ENUM_VALUE_RE                  \
  "^"                                           \
  REC_TYPE_ZBLANKS_RE                           \
  REC_TYPE_ENUM_NAME_RE                         \
  REC_TYPE_ZBLANKS_RE                           \
  "$"
  
/* Regular expression denoting a type name.  */
#define REC_TYPE_NAME_RE                               \
  "(" REC_TYPE_INT_NAME   "|" REC_TYPE_RANGE_NAME  "|" \
      REC_TYPE_REAL_NAME  "|" REC_TYPE_SIZE_NAME   "|" \
      REC_TYPE_LINE_NAME  "|" REC_TYPE_REGEXP_NAME "|" \
      REC_TYPE_DATE_NAME  "|" REC_TYPE_ENUM_NAME   "|" \
      REC_TYPE_FIELD_NAME "|" REC_TYPE_BOOL_NAME       \
  ")"

/* Regular expressions for the type descriptions.  */

/* int  */
#define REC_TYPE_INT_DESCR_RE                      \
  REC_TYPE_INT_NAME

/* bool */
#define REC_TYPE_BOOL_DESCR_RE                  \
  REC_TYPE_BOOL_NAME

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
#define REC_TYPE_ENUM_DESCR_RE                  \
  REC_TYPE_ENUM_NAME                            \
  REC_TYPE_BLANKS_RE                            \
  REC_TYPE_ENUM_NAME_RE                         \
  "(" REC_TYPE_BLANKS_RE REC_TYPE_ENUM_NAME_RE ")*"

/* field FIELD_NAME  */
#define REC_TYPE_FIELD_PART_RE                  \
  "[a-zA-Z%][a-zA-Z0-9_-]*"
#define REC_TYPE_FIELD_NAME_RE                  \
  REC_TYPE_FIELD_PART_RE                        \
  "(" ":" REC_TYPE_FIELD_PART_RE ")*" ":?"
#define REC_TYPE_FIELD_DESCR_RE                 \
  REC_TYPE_FIELD_NAME                           \
  REC_TYPE_BLANKS_RE                            \
  REC_TYPE_FIELD_NAME_RE

/* Regexp denoting any type description.  */
#define REC_TYPE_DESCR_RE                       \
  "^"                                           \
  REC_TYPE_ZBLANKS_RE                           \
  REC_TYPE_FIELD_NAME_RE                        \
  REC_TYPE_ZBLANKS_RE                           \
  "("                                           \
         "(" REC_TYPE_INT_DESCR_RE    ")"       \
     "|" "(" REC_TYPE_BOOL_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_RANGE_DESCR_RE  ")"       \
     "|" "(" REC_TYPE_REAL_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_SIZE_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_LINE_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_REGEXP_DESCR_RE ")"       \
     "|" "(" REC_TYPE_DATE_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_ENUM_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_FIELD_DESCR_RE  ")"       \
  ")"                                           \
  REC_TYPE_ZBLANKS_RE                           \
  "$"
      
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

struct rec_type_reg_entry_s
{
  rec_field_name_t name;
  rec_type_t type;
};

#define REC_TYPE_REG_MAX_ENTRIES 100

struct rec_type_reg_s
{
  size_t num_entries;
  struct rec_type_reg_entry_s entries[REC_TYPE_REG_MAX_ENTRIES];
};

/*
 * Forward declarations.
 */

static bool rec_type_check_re (char *regexp_str, char *str);
static enum rec_type_kind_e rec_type_parse_type_kind (char *str);
static bool rec_type_blank_p (char c);
static bool rec_type_digit_p (char c);
static bool rec_type_letter_p (char c);

static bool rec_type_check_int (rec_type_t type, char *str);
static bool rec_type_check_bool (rec_type_t type, char *str);
static bool rec_type_check_range (rec_type_t type, char *str);
static bool rec_type_check_real (rec_type_t type, char *str);
static bool rec_type_check_size (rec_type_t type, char *str);
static bool rec_type_check_line (rec_type_t type, char *str);
static bool rec_type_check_regexp (rec_type_t type, char *str);
static bool rec_type_check_date (rec_type_t type, char *str);
static bool rec_type_check_enum (rec_type_t type, char *str);
static bool rec_type_check_field (rec_type_t type, char *str);

/*
 * Public functions.
 */

bool
rec_type_descr_p (char *str)
{
  return rec_type_check_re (REC_TYPE_DESCR_RE, str);
}

rec_field_name_t
rec_type_descr_field_name (char *str)
{
  rec_field_name_t field_name = NULL;
  char *p, *b;
  char name[100];

  if (rec_type_descr_p (str))
    {
      /* Skip blank characters.  */
      p = str;
      while (p && rec_type_blank_p (*p))
        {
          p++;
        }

      /* Get the field name.  */
      b = p;
      while (p && (rec_type_letter_p (*p)
                   || rec_type_digit_p (*p)
                   || (*p == '%') || (*p == '_') || (*p == ':')
                   || (*p == '-')))
        {
          name[p - b] = *p;
          p++;
        }
      name[p - b] = '\0';

      /* Parse the field name.  */
      field_name = rec_parse_field_name_str (name);
    }

  return field_name;
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

      /* Skip field name.  */
      while (p && (rec_type_letter_p (*p)
                   || rec_type_digit_p (*p)
                   || (*p == '%') || (*p == '_') || (*p == ':')
                   || (*p == '-')))
        {
          p++;
        }

      /* Skip blanks.  */
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
                     "rec-types: internal error: got REC_TYPE_NONE from rec_type_parse_type kind in rec_type_new.\n");
            exit (1);
            break;
          }
        case REC_TYPE_INT:
        case REC_TYPE_BOOL:
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
            /* XXX: Not implemented yet.  */
            break;
          }
        }
    }

  return new;
}

enum rec_type_kind_e
rec_type_kind (rec_type_t type)
{
  return type->kind;
}

char *
rec_type_kind_str (rec_type_t type)
{
  char *res;

  switch (type->kind)
    {
    case REC_TYPE_NONE:
      {
        res = "";
        break;
      }
    case REC_TYPE_INT:
      {
        res = REC_TYPE_INT_NAME;
        break;
      }
    case REC_TYPE_BOOL:
      {
        res = REC_TYPE_BOOL_NAME;
        break;
      }
    case REC_TYPE_RANGE:
      {
        res = REC_TYPE_RANGE_NAME;
        break;
      }
    case REC_TYPE_REAL:
      {
        res = REC_TYPE_REAL_NAME;
        break;
      }
    case REC_TYPE_SIZE:
      {
        res = REC_TYPE_SIZE_NAME;
        break;
      }
    case REC_TYPE_LINE:
      {
        res = REC_TYPE_LINE_NAME;
        break;
      }
    case REC_TYPE_REGEXP:
      {
        res = REC_TYPE_REGEXP_NAME;
        break;
      }
    case REC_TYPE_DATE:
      {
        res = REC_TYPE_DATE_NAME;
        break;
      }
    case REC_TYPE_ENUM:
      {
        res = REC_TYPE_ENUM_NAME;
        break;
      }
    case REC_TYPE_FIELD:
      {
        res = REC_TYPE_FIELD_NAME;
        break;
      }
    }

  return res;
}

bool
rec_type_check (rec_type_t type,
                char *str)
{
  bool res;

  switch (type->kind)
    {
    case REC_TYPE_NONE:
      {
        res = true;
        break;
      }
    case REC_TYPE_INT:
      {
        res = rec_type_check_int (type, str);
        break;
      }
    case REC_TYPE_BOOL:
      {
        res = rec_type_check_bool (type, str);
        break;
      }
    case REC_TYPE_RANGE:
      {
        res = rec_type_check_range (type, str);
        break;
      }
    case REC_TYPE_REAL:
      {
        res = rec_type_check_real (type, str);
        break;
      }
    case REC_TYPE_SIZE:
      {
        res = rec_type_check_size (type, str);
        break;
      }
    case REC_TYPE_LINE:
      {
        res = rec_type_check_line (type, str);
        break;
      }
    case REC_TYPE_REGEXP:
      {
        res = rec_type_check_regexp (type, str);
        break;
      }
    case REC_TYPE_DATE:
      {
        res = rec_type_check_date (type, str);
        break;
      }
    case REC_TYPE_ENUM:
      {
        res = rec_type_check_enum (type, str);
        break;
      }
    case REC_TYPE_FIELD:
      {
        res = rec_type_check_field (type, str);
        break;
      }
    }

  return res;
}

void
rec_type_destroy (rec_type_t type)
{
  free (type);
}

rec_type_reg_t
rec_type_reg_new (void)
{
  rec_type_reg_t new;

  new = malloc (sizeof (struct rec_type_reg_s));
  if (new)
    {
      new->num_entries = 0;
    }

  return new;
}

void
rec_type_reg_destroy (rec_type_reg_t reg)
{
  size_t i;

  for (i = 0; i < reg->num_entries; i++)
    {
      rec_field_name_destroy (reg->entries[i].name);
      rec_type_destroy (reg->entries[i].type);
    }
  
  free (reg);
}

void
rec_type_reg_register (rec_type_reg_t reg,
                       rec_field_name_t name,
                       rec_type_t type)
{
  reg->entries[reg->num_entries].name = rec_field_name_dup (name);
  reg->entries[reg->num_entries].type = type;
  reg->num_entries++;
}

rec_type_t
rec_type_reg_get (rec_type_reg_t reg,
                  rec_field_name_t name)
{
  rec_type_t res;
  size_t i;

  res = NULL;
  for (i = 0; i < reg->num_entries; i++)
    {
      if (rec_field_name_equal_p (reg->entries[i].name, name))
        {
          res = reg->entries[i].type;
          break;
        }
    }

  return res;
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
      printf("YYY: %s\n", regexp_str);
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
  if (strcmp (str, REC_TYPE_BOOL_NAME) == 0)
    {
      res = REC_TYPE_BOOL;
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

static bool
rec_type_check_int (rec_type_t type,
                    char *str)
{
  return rec_type_check_re (REC_TYPE_INT_VALUE_RE, str);
}

static bool
rec_type_check_bool (rec_type_t type,
                     char *str)
{
  return rec_type_check_re (REC_TYPE_BOOL_VALUE_RE, str);
}

static bool
rec_type_check_range (rec_type_t type,
                      char *str)
{
  /* XXX: TODO.  */
  /* Check string.  */
  /* Get min and max numbers.  */
  /* Cross-check with the data in 'type'.  */
  
  return false;
}

static bool
rec_type_check_real (rec_type_t type,
                     char *str)
{
  return rec_type_check_re (REC_TYPE_REAL_VALUE_RE, str);
}

static bool
rec_type_check_size (rec_type_t type,
                     char *str)
{
  return (strlen (str) <= type->data.max_size);
}

static bool
rec_type_check_line (rec_type_t type,
                     char *str)
{
  return rec_type_check_re (REC_TYPE_LINE_VALUE_RE, str);
}

static bool
rec_type_check_regexp (rec_type_t type,
                       char *str)
{
  return rec_type_check_re (type->data.regexp, str);
}

static bool
rec_type_check_date (rec_type_t type,
                     char *str)
{
  return rec_type_check_re (REC_TYPE_DATE_VALUE_RE, str);
}

static bool
rec_type_check_enum (rec_type_t type,
                     char *str)
{
  /* XXX: todo.  */
  return false;
}

static bool
rec_type_check_field (rec_type_t type,
                      char *str)
{
  /* XXX: todo.  */
  return false;
}

/* End of rec-types.c */
