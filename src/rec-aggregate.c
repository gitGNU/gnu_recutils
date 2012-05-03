/* -*- mode: C -*- Time-stamp: "2012-05-03 20:48:41 jemarch"
 *
 *       File:         rec-aggregate.c
 *       Date:         Mon Apr 23 11:05:57 2012
 *
 *       GNU recutils - Support for aggregate functions
 *
 */

/* Copyright (C) 2012 Jose E. Marchesi */

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
#include <string.h>
#include <math.h>

#include <rec-utils.h>
#include <rec.h>

/*
 * Data structures.
 */

#define MAX_FUNCTIONS 40

struct rec_aggregate_reg_elem_s
{
  char *name;
  rec_aggregate_t function;
};

struct rec_aggregate_reg_s
{
  struct rec_aggregate_reg_elem_s functions[MAX_FUNCTIONS];
  size_t num_functions;
};

/* Static functions defined in this file.  */

static char *rec_aggregate_std_count (rec_rset_t rset,
                                      rec_record_t record,
                                      const char *field_name);

static char *rec_aggregate_std_sum (rec_rset_t rset,
                                    rec_record_t record,
                                    const char *field_name);
static double rec_aggregate_std_sum_record (rec_record_t record,
                                            const char *field_name);

/*
 * Public functions.
 */

rec_aggregate_reg_t
rec_aggregate_reg_new (void)
{
  rec_aggregate_reg_t new;

  new = malloc (sizeof (struct rec_aggregate_reg_s));
  if (new)
    {
      new->num_functions = 0;
    }

  return new;
}

void
rec_aggregate_reg_destroy (rec_aggregate_reg_t func_reg)
{
  if (func_reg)
    {
      size_t i = 0;
      for (i = 0; i < func_reg->num_functions; i++)
        {
          free (func_reg->functions[i].name);
        }

      free (func_reg);
    }
}

bool
rec_aggregate_reg_add (rec_aggregate_reg_t func_reg,
                       const char *name,
                       rec_aggregate_t function)
{
  bool function_replaced = false;
  size_t i = 0;

  for (i = 0; i < func_reg->num_functions; i++)
    {
      if (strcmp (name, func_reg->functions[i].name) == 0)
        {
          /* Replace the existing function.  */
          func_reg->functions[i].function = function;
          function_replaced = true;
          break;
        }
    }

  if (!function_replaced)
    {
      /* Insert the function into a new entry in the registry.  */

      if (func_reg->num_functions == MAX_FUNCTIONS)
        {
          /* FIXME: this is crappy as hell.  */
          return false;
        }

      func_reg->functions[func_reg->num_functions].name = strdup (name);
      func_reg->functions[func_reg->num_functions].function = function;
      func_reg->num_functions++;
    }

  return true;
}

rec_aggregate_t
rec_aggregate_reg_get (rec_aggregate_reg_t func_reg,
                       const char *name)
{
  size_t i = 0;
  rec_aggregate_t res = NULL;

  for (i = 0; i < func_reg->num_functions; i++)
    {
      if (strcasecmp (func_reg->functions[i].name, name) == 0)
        {
          res = func_reg->functions[i].function;
          break;
        }
    }

  return res;
}

void
rec_aggregate_reg_add_standard (rec_aggregate_reg_t func_reg)
{
  /* Please update the rec_aggregate_std_p function if you add a new
     standard aggregate function.  */

  rec_aggregate_reg_add (func_reg, "Count", &rec_aggregate_std_count);
  rec_aggregate_reg_add (func_reg, "Sum", &rec_aggregate_std_sum);
}

bool
rec_aggregate_std_p (const char *name)
{
  return ((strcasecmp (name, "Count") == 0)
          || (strcasecmp (name, "Sum") == 0));
}

/*
 * Private functions.
 */

static char *
rec_aggregate_std_count (rec_rset_t rset,
                         rec_record_t record,
                         const char *field_name)
{
  char *result = NULL;
  size_t count = 0;

  if (record)
    {
      count = rec_record_get_num_fields_by_name (record, field_name);
    }
  else if (rset)
    {
      rec_record_t rec = NULL;
      rec_mset_iterator_t iter = rec_mset_iterator (rec_rset_mset (rset));
      while (rec_mset_iterator_next (&iter, MSET_RECORD, (void *) &rec, NULL))
        {
          count = count + rec_record_get_num_fields_by_name (rec, field_name);
        }
      rec_mset_iterator_free (&iter);
    }

  /* Return the count as a string.  Note that if NULL is returned it
     will be returned by this function below to signal the
     end-of-memory condition.  */
      
  asprintf (&result, "%ld", count);
  return result;
}

static char *
rec_aggregate_std_sum (rec_rset_t rset,
                       rec_record_t record,
                       const char *field_name)
{
  char *result = NULL;
  double sum   = 0;

  if (record)
    {
      sum = rec_aggregate_std_sum_record (record, field_name);
    }
  else if (rset)
    {
      rec_record_t rec = NULL;
      rec_mset_iterator_t iter = rec_mset_iterator (rec_rset_mset (rset));
      while (rec_mset_iterator_next (&iter, MSET_RECORD, (void *) &rec, NULL))
        {
          sum = sum + rec_aggregate_std_sum_record (rec, field_name);
        }
      rec_mset_iterator_free (&iter);
    }

  /* Return the sum as a string.  Note that if NULL is returned it
     will be returned by this function below to signal the
     end-of-memory condition.  */

  if (sum == floor (sum))
    {
      asprintf (&result, "%ld", (size_t) sum);
    }
  else
    {
      asprintf (&result, "%f", sum);
    }

  return result;
}

static double
rec_aggregate_std_sum_record (rec_record_t record,
                              const char *field_name)
{
  /* Calculate the sum of the fields in a given record.  Fields not
     representing a real value are ignored.  */

  double sum = 0;
  rec_field_t field;
  rec_mset_iterator_t iter = rec_mset_iterator (rec_record_mset (record));

  while (rec_mset_iterator_next (&iter, MSET_FIELD, (void *) &field, NULL))
    {
      const char *field_value = rec_field_value (field);
      double field_value_double = 0;
      
      if (rec_field_name_equal_p (rec_field_name (field), field_name)
          && rec_atod (field_value, &field_value_double))
        {
          sum = sum + field_value_double;
        }
    }
  rec_mset_iterator_free (&iter);

  return sum;
}

/* End of rec-aggregate.c */
