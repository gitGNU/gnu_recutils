/* -*- mode: C -*- Time-stamp: "2012-05-03 14:35:55 jco"
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
                                      const char *field_name,
                                      size_t min,
                                      size_t max);

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
      if (strcmp (func_reg->functions[i].name, name) == 0)
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
  rec_aggregate_reg_add (func_reg, "Count", &rec_aggregate_std_count);
}

/*
 * Private functions.
 */

static char *
rec_aggregate_std_count (rec_rset_t rset,
                         rec_record_t record,
                         const char *field_name,
                         size_t min,
                         size_t max)
{
  char *result = NULL;

  /* Get the value of the first occurrence of a field called
     FIELD_NAME and then count the number of records in RSET
     having such a field.  In case no such field exists in the
     current record then we are done.  */
  
  rec_field_t count_field =
    rec_record_get_field_by_name (record, field_name, 0);
  
  if (count_field)
    {
      size_t count = 0;
      rec_record_t rec = NULL;
      rec_mset_iterator_t rset_iter =
        rec_mset_iterator (rec_rset_mset (rset));
      
      while (rec_mset_iterator_next (&rset_iter, MSET_RECORD, (void *) &rec, NULL))
        {
          rec_field_t field = NULL;
          rec_mset_iterator_t rec_iter =
            rec_mset_iterator (rec_record_mset (rec));
          
          if (rec == record)
            {
              /* Small optimization.  */
              count++;
              continue;
            }
          
          while (rec_mset_iterator_next (&rec_iter, MSET_FIELD, (void *) &field, NULL))
            {
              const char *fname = rec_field_name (field);
              if (rec_field_name_equal_p (rec_field_name (field), field_name)
                  && (strcmp (rec_field_value (field), rec_field_value (count_field)) == 0))
                {
                  count++;
                  break; /* We count 1 per record containing a
                            field with the same value.  */
                }                  
            }
          rec_mset_iterator_free (&rec_iter);
        }
      rec_mset_iterator_free (&rset_iter);
      
      /* Return the count as a string.  Note that if NULL is returned
         it will be returned by this function below to signal the
         end-of-memory condition.  */
      
      asprintf (&result, "%ld", count);
    }

  return result;
}

/* End of rec-aggregate.c */
