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
#include <regex.h>
#include <getdate.h>

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
#define REC_TYPE_EMAIL_NAME  "email"
#define REC_TYPE_FIELD_NAME  "field"

/* Regular expression denoting a blank character in a type
   description.  */
#define REC_TYPE_BLANK_RE "[ \t\n]"
#define REC_TYPE_NO_BLANK_RE "[^ \t\n]"
#define REC_TYPE_BLANKS_RE REC_TYPE_BLANK_RE "+"
#define REC_TYPE_NO_BLANKS_RE REC_TYPE_NO_BLANK_RE "+"
#define REC_TYPE_ZBLANKS_RE REC_TYPE_BLANK_RE "*"

/* Regular expressions denoting values.  */
#define REC_TYPE_INT_VALUE_RE                   \
  "^" REC_TYPE_ZBLANKS_RE "-?[0-9]+" REC_TYPE_ZBLANKS_RE "$"

#define REC_TYPE_BOOL_VALUE_RE                  \
  "^(yes|no|true|false|0|1)$"

#define REC_TYPE_REAL_VALUE_RE                  \
  "^" REC_TYPE_ZBLANKS_RE "[0-9]+(\\.[0-9]+)?" REC_TYPE_ZBLANKS_RE "$"

#define REC_TYPE_LINE_VALUE_RE                  \
  "^[^\n]*$"

#define REC_TYPE_EMAIL_VALUE_RE                 \
  "^[ \n\t]*"                                   \
  "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,4}" \
  "[ \n\t]*$"

#define REC_TYPE_ENUM_NAME_RE                   \
  "[a-zA-Z0-9][a-zA-Z0-9_-]*"
#define REC_TYPE_ENUM_VALUE_RE                  \
  "^"                                           \
  REC_TYPE_ZBLANKS_RE                           \
  REC_TYPE_ENUM_NAME_RE                         \
  REC_TYPE_ZBLANKS_RE                           \
  "$"

/* REC_FNAME_RE is defined in rec.h */
#define REC_TYPE_FIELD_VALUE_RE                 \
  "^"                                           \
  REC_TYPE_ZBLANKS_RE                           \
  REC_FNAME_RE                                  \
  REC_TYPE_ZBLANKS_RE                           \
  "$"
  
/* Regular expression denoting a type name.  */
#define REC_TYPE_NAME_RE                               \
  "(" REC_TYPE_INT_NAME   "|" REC_TYPE_RANGE_NAME  "|" \
      REC_TYPE_REAL_NAME  "|" REC_TYPE_SIZE_NAME   "|" \
      REC_TYPE_LINE_NAME  "|" REC_TYPE_REGEXP_NAME "|" \
      REC_TYPE_DATE_NAME  "|" REC_TYPE_ENUM_NAME   "|" \
      REC_TYPE_EMAIL_NAME "|" REC_TYPE_BOOL_NAME   "|" \
      REC_TYPE_FIELD_NAME \
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
  "-?[0-9]+"                                       \
  REC_TYPE_ZBLANKS_RE                              \
  "-?[0-9]+"

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
#define REC_TYPE_REGEXP_DESCR_RE                 \
  REC_TYPE_REGEXP_NAME                           \
  ".+"

/* date  */
#define REC_TYPE_DATE_DESCR_RE                  \
  REC_TYPE_DATE_NAME

/* enum NAME NAME NAME*/
#define REC_TYPE_ENUM_DESCR_RE                  \
  REC_TYPE_ENUM_NAME                            \
  REC_TYPE_BLANKS_RE                            \
  REC_TYPE_ENUM_NAME_RE                         \
  "(" REC_TYPE_BLANKS_RE REC_TYPE_ENUM_NAME_RE ")*"

  /* field */
#define REC_TYPE_FIELD_DESCR_RE                 \
  REC_TYPE_FIELD_NAME

/* email */
#define REC_TYPE_EMAIL_DESCR_RE                 \
  REC_TYPE_EMAIL_NAME

/* Regexp denoting any type description.  */
#define REC_TYPE_DESCR_RE                       \
  "^"                                           \
  REC_TYPE_ZBLANKS_RE                           \
  REC_FNAME_RE                                  \
  REC_TYPE_ZBLANKS_RE                           \
  "("                                           \
         "(" REC_TYPE_INT_DESCR_RE    ")"       \
     "|" "(" REC_TYPE_BOOL_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_RANGE_DESCR_RE  ")"       \
     "|" "(" REC_TYPE_REAL_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_SIZE_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_RANGE_DESCR_RE  ")"       \
     "|" "(" REC_TYPE_LINE_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_REGEXP_DESCR_RE ")"       \
     "|" "(" REC_TYPE_DATE_DESCR_RE   ")"       \
     "|" "(" REC_TYPE_EMAIL_DESCR_RE  ")"       \
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
    int range[2];             /* Range.  */
    regex_t regexp;           /* Regular expression.  */

#define REC_ENUM_MAX_NAMES 50
    char *names[REC_ENUM_MAX_NAMES];   /* Names in enumeration.  */
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
static bool rec_type_check_email (rec_type_t type, char *str);
static bool rec_type_check_enum (rec_type_t type, char *str);
static bool rec_type_check_field (rec_type_t type, char *str);

/* Parsing routines.  */

static void rec_type_skip_blanks (char **str);
static bool rec_type_parse_int (char **str, int *num);
static bool rec_type_parse_regexp (char **str, char *re, char **result);

static bool rec_type_parse_size (char *str, rec_type_t type);
static bool rec_type_parse_enum (char *str, rec_type_t type);
static bool rec_type_parse_regexp_type (char *str, rec_type_t type);
static bool rec_type_parse_range (char *str, rec_type_t type);

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
  char *name;

  p = str;

  /* Skip blank characters.  */
  rec_type_skip_blanks (&p);

  /* Get the field name.  */
  if (rec_type_parse_regexp (&p, "^" REC_FNAME_RE, &name))
    {
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
  char *field_name_str = NULL;
  char *type_name_str = NULL;

  p = str;
  new = malloc (sizeof (struct rec_type_s));
  if (!new)
    {
      goto exit;
    }

  /* Skip the field name surrounded by blanks.  */
  rec_type_skip_blanks (&p);
  if (!rec_type_parse_regexp (&p, "^" REC_FNAME_RE, &field_name_str))
    {
      free (new);
      new = NULL;
      goto exit;
    }
  rec_type_skip_blanks (&p);

  /* Get the type name.  */
  if (!rec_type_parse_regexp (&p, "^" REC_TYPE_NAME_RE, &type_name_str))
    {
      free (new);
      new = NULL;
      goto exit;
    }

  /* Continue parsing depending on the kind of type.  */
  new->kind = rec_type_parse_type_kind (type_name_str);
  switch (new->kind)
    {
    case REC_TYPE_SIZE:
      {
        if (!rec_type_parse_size (p, new))
          {
            free (new);
            new = NULL;
          }        
        break;
      }
    case REC_TYPE_ENUM:
      {
        if (!rec_type_parse_enum (p, new))
          {
            free (new);
            new = NULL;
          }
        break;
      }
    case REC_TYPE_REGEXP:
      {
        if (!rec_type_parse_regexp_type (p, new))
          {
            free (new);
            new = NULL;
          }
        break;
      }
    case REC_TYPE_RANGE:
      {
        if (!rec_type_parse_range (p, new))
          {
            free (new);
            new = NULL;
          }
        break;
      }
    case REC_TYPE_INT:
    case REC_TYPE_BOOL:
    case REC_TYPE_REAL:
    case REC_TYPE_LINE:
    case REC_TYPE_FIELD:
      {
        /* We are done.  */
        break;
      }
    case REC_TYPE_NONE:
      {
        /* This point should not be reached.  */
        fprintf (stderr,
                 "rec-types: internal error: got REC_TYPE_NONE from"
                 "rec_type_parse_type kind in rec_type_new.\n");
        exit (1);
        break;
      }
    }

 exit:

  if (field_name_str)
    {
      free (field_name_str);
    }
  if (type_name_str)
    {
      free (type_name_str);
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
    case REC_TYPE_EMAIL:
      {
        res = REC_TYPE_EMAIL_NAME;
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
    case REC_TYPE_EMAIL:
      {
        res = rec_type_check_email (type, str);
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
      if (rec_field_name_equal_p (reg->entries[i].name, name)
          || rec_field_name_ref_p (reg->entries[i].name, name))
        {
          res = reg->entries[i].type;
          break;
        }
    }

  return res;
}

bool
rec_type_equal_p (rec_type_t type1,
                  rec_type_t type2)
{
  bool ret;
  size_t i;

  ret = true;

  if (type1->kind != type2->kind)
    {
      ret = false;
    }
  else
    {
      if (type1->kind == REC_TYPE_SIZE)
        {
          ret = (type1->data.max_size == type2->data.max_size);
        }
      else if (type1->kind == REC_TYPE_RANGE)
        {
          ret = ((type1->data.range[0] == type2->data.range[0])
                 && (type1->data.range[1] == type2->data.range[1]));
        }
      else if (type1->kind == REC_TYPE_ENUM)
        {
          i = 0;
          while (type1->data.names[i])
            {
              ret = (type2->data.names[i]
                     && (strcmp (type1->data.names[i],
                                 type2->data.names[i]) == 0));
            }
        }
      else if (type1->kind == REC_TYPE_REGEXP)
        {
          /* Since there is noway to determine whether two
             regex_t variables refer to equivalent regexps.  */
          ret = false;
        }
    }

  return ret;
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
  if (strcmp (str, REC_TYPE_EMAIL_NAME) == 0)
    {
      res = REC_TYPE_EMAIL;
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
rec_type_check_field (rec_type_t type,
                      char *str)
{
  return rec_type_check_re (REC_TYPE_FIELD_VALUE_RE, str);
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
  char *p;
  int num;

  p = str;

  rec_type_skip_blanks (&p);
  if (!rec_type_parse_int (&p, &num))
    {
      return false;
    }

  return ((num >= type->data.range[0])
          && (num <= type->data.range[1]));
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
  return (regexec (&type->data.regexp,
                   str,
                   0,
                   NULL,
                   0) == 0);
}

static bool
rec_type_check_date (rec_type_t type,
                     char *str)
{
  struct timespec tm;
  return get_date (&tm, str, NULL);
}

static bool
rec_type_check_email (rec_type_t type,
                      char *str)
{
  return rec_type_check_re (REC_TYPE_EMAIL_VALUE_RE, str);
}

static bool
rec_type_check_enum (rec_type_t type,
                     char *str)
{
  size_t i;
  char *p, *b;
  char name[100];

  if (!rec_type_check_re (REC_TYPE_ENUM_VALUE_RE, str))
    {
      return false;
    }

  /* Get the name from STR.  */
  p = str;

  while (p && rec_type_blank_p (*p))
    {
      p++;
    }

  b = p;
  while (p && (rec_type_digit_p (*p)
               || rec_type_letter_p (*p)
               || (*p == '_')
               || (*p == '-')))
    {
      name[p - b] = *p;
      p++;
    }
  name[p - b] = '\0';

  /* Check for the name in the enum types.  */
  i = 0;
  while (type->data.names[i])
    {
      if (strcmp (name, type->data.names[i]) == 0)
        {
          return true;
        }
      
      i++;
    }

  return false;
}

static void
rec_type_skip_blanks (char **str)
{
  char *p;

  p = *str;
  while (rec_type_blank_p (*p))
    {
      p++;
    }

  *str = p;
}

static bool
rec_type_parse_int (char **str, int *num)
{
  bool ret;
  char *p, *b;
  char number[30];

  ret = true;
  p = *str;

  b = p;
  while (rec_type_digit_p (*p) || (*p == '-'))
    {
      number[p - b] = *p;
      p++;
    }
  number[p - b] = '\0';

  if (!rec_atoi (number, num))
    {
      ret = false;
    }

  *str = p;
  return ret;
}

static bool
rec_type_parse_regexp (char **str, char *re, char **result)
{
  bool ret;
  char *p, *b;
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

static bool
rec_type_parse_size (char *str, rec_type_t type)
{
  bool ret;
  char *p;

  ret = true;
  p = str;

  /* Skip blanks.  */
  rec_type_skip_blanks (&p);

  /* Get the size.  */
  if (!rec_type_parse_int (&p, &(type->data.max_size)))
    {
      ret = false;
    }
  
  return ret;
}

static bool
rec_type_parse_enum (char *str, rec_type_t type)
{
  bool ret;
  char *p;
  size_t i, j;
  
  ret = true;
  p = str;

  i = 0;
  while (*p && (i < REC_ENUM_MAX_NAMES))
    {
      /* Skip blanks.  */
      /* XXX and comments as well!.  */
      rec_type_skip_blanks (&p);

      /* Parse an enum entry.  */
      if (!rec_type_parse_regexp (&p,
                                  "^" REC_TYPE_ENUM_NAME_RE,
                                  &(type->data.names[i++])))
        {
          ret = false;
          break;
        }
    }

  if (!ret)
    {
      /* Free memory.  */
      for (j = 0; j < i; j++)
        {
          free (type->data.names[j]);
        }
    }

  return ret;
}

static bool
rec_type_parse_regexp_type (char *str, rec_type_t type)
{
  bool ret;
  char *p;
  char re[200];
  bool escaping;
  bool end_regexp;
  size_t i;
  char delim_char;

  ret = true;
  p = str;

  /* The regexp type descriptor is like:
           
     BLANKS BEGIN_RE CHARS END_RE BLANKS
           
     where BEGIN_RE == END_RE and is the first non-blank
     character found in the string.  Escaped apperances of
     BEGIN_RE in CHARS are un-escaped.
  */

  /* Skip blanks.  */
  rec_type_skip_blanks (&p);
        
  end_regexp = false;
  delim_char = *p;
  p++;
        
  i = 0;
  while (*p)
    {
      if (*p == delim_char)
        {
          if (*(p + 1) == delim_char)
            {
              re[i++] = delim_char;
              p++;
            }
          else
            {
              /* End of the regexp.  */
              end_regexp = true;
              break;
            }
        }
      else
        {
          re[i++] = *p;
        }
            
      p++;
    }
  re[i] = '\0';
        
  if (!end_regexp)
    {
      /* Error.  */
      ret = false;
    }
  else
    {
      /* Compile the regexp.  */
      if (regcomp (&type->data.regexp, re,
                   REG_EXTENDED) != 0)
        {
          ret = false;
        }
    }

  return ret;
}

static bool
rec_type_parse_range (char *str, rec_type_t type)
{
  char *p;

  p = str;

  rec_type_skip_blanks (&p);

  if (!rec_type_parse_int (&p, &(type->data.range[0])))
    {
      return false;
    }

  rec_type_skip_blanks (&p);

  if (!rec_type_parse_int (&p, &(type->data.range[1])))
    {
      return false;
    }

  return true;
}

/* End of rec-types.c */
