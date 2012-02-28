/* -*- mode: C -*-
 *
 *       File:         rec-db.c
 *       Date:         Thu Jan 14 15:35:27 2010
 *
 *       GNU recutils - Databases
 *
 */

/* Copyright (C) 2010, 2011, 2012 Jose E. Marchesi */

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

#include <string.h>
#include <stdlib.h>
#include <gl_array_list.h>
#include <gl_list.h>

#include <rec.h>

/*
 * Data structures.
 */

struct rec_db_s
{
  size_t size;          /* Number of record sets contained in this
                           database.  */
  gl_list_t rset_list;  /* List of record sets.  */
};

/* Static functions defined in this file.  */

static bool rec_db_rset_equals_fn (const void *elt1,
                                   const void *elt2);
static void rec_db_rset_dispose_fn (const void *elt);

static rec_record_t rec_db_process_fex (rec_record_t record, rec_fex_t fex);
static bool rec_db_record_selected_p (rec_record_t record,
                                      rec_sex_t sex,
                                      const char *fast_string,
                                      bool case_insensitive_p);

/*
 * Public functions.
 */

rec_db_t
rec_db_new (void)
{
  rec_db_t new;

  new = malloc (sizeof (struct rec_db_s));
  if (new)
    {
      new->size = 0;
      new->rset_list = gl_list_nx_create_empty (GL_ARRAY_LIST,
                                                rec_db_rset_equals_fn,
                                                NULL,
                                                rec_db_rset_dispose_fn,
                                                true);
      
      if (new->rset_list == NULL)
        {
          /* Out of memory.  */
          free (new);
          new = NULL;
        }
    }

  return new;
}

void
rec_db_destroy (rec_db_t db)
{
  if (db)
    {
      gl_list_free (db->rset_list);
      free (db);
    }
}

size_t
rec_db_size (rec_db_t db)
{
  return db->size;
}

rec_rset_t
rec_db_get_rset (rec_db_t db,
                 size_t position)
{
  rec_rset_t rset;

  rset = NULL;

  if (db->size > 0)
    {
      if (position < 0)
        {
          position = 0;
        }
      if (position >= db->size)
        {
          position = db->size - 1;
        }

      rset = (rec_rset_t) gl_list_get_at (db->rset_list, position);
    }

  return rset;
}

bool
rec_db_insert_rset (rec_db_t db,
                    rec_rset_t rset,
                    size_t position)
{
  gl_list_node_t node;

  node = NULL;

  if (position < 0)
    {
      node = gl_list_nx_add_first (db->rset_list,
                                   (void *) rset);
    }
  else if (position >= db->size)
    {
      node = gl_list_nx_add_last (db->rset_list,
                                  (void *) rset);
    }
  else
    {
      node = gl_list_nx_add_at (db->rset_list,
                                position,
                                (void *) rset);
    }

  if (node != NULL)
    {
      db->size++;
      return true;
    }

  return false;
}

bool
rec_db_remove_rset (rec_db_t db, size_t position)
{
  bool removed;

  removed = false;
  
  if (db->size > 0)
    {
      if (position < 0)
        {
          position = 0;
        }
      if (position >= db->size)
        {
          position = db->size - 1;
        }
      
      if (gl_list_remove_at (db->rset_list,
                             position))
        {
          db->size--;
          removed = true;
        }
    }
  
  return removed;
}

bool
rec_db_type_p (rec_db_t db,
               const char *type)
{
  return (rec_db_get_rset_by_type (db, type) != NULL);
}

rec_rset_t
rec_db_get_rset_by_type (rec_db_t db,
                         const char *type)
{
  int i;
  rec_rset_t rset;
  bool found;
  char *rtype;

  found = false;
  for (i = 0; i < rec_db_size (db); i++)
    {
      rset = rec_db_get_rset (db, i);
      rtype = rec_rset_type (rset);
      if (rtype == NULL)
        {
          if (type == NULL)
            {
              /* Return the default rset.  */
              found = true;
              break;
            }
        }
      else
        { 
          if ((type != NULL)
              && (strcmp (rtype, type) == 0))
            {
              found = true;
              break;
            }
        }
    }

  if (!found)
    {
      rset = NULL;
    }
  
  return rset;
}

rec_rset_t
rec_db_query (rec_db_t db,
              const char  *type,
              size_t       min,
              size_t       max,
              rec_sex_t    sex,
              const char  *fast_string,
              size_t       random,
              rec_fex_t    fex,
              const char  *password,
              size_t      *counter,
              int          flags)
{
  rec_rset_t res = NULL;
  rec_rset_t rset = NULL;
  size_t processed = 0;
  size_t n_rset = 0;
  
  /* Create a new, empty, record set, that will contain the contents
     of the selection.  */

  res = rec_rset_new ();
  if (!res)
    {
      /* Out of memory.  */
      return NULL;
    }

  /* Search for the rset containing records of the requested type.  If
     type equals to NULL then the default record set is used.  */
  
  rset = rec_db_get_rset_by_type (db, type);
  if (!rset)
    {
      /* Type not found, so return an empty record set.  */
      return res;
    }

  /* If a descriptor is requested then get a copy of the descriptor of
     the referred record set, which exists only if it is not the
     default.  */

  if (type && (flags & REC_Q_DESCRIPTOR))
    {
      rec_record_t descriptor = rec_record_dup (rec_rset_descriptor (rset));
      if (!descriptor)
        {
          /* Out of memory.  */
          free (res);
          return NULL;
        }

      rec_rset_set_descriptor (res,
                               rec_record_dup (rec_rset_descriptor (rset)));
    }
  
  /* Process this record set.  This means that every record of this
     record set which is selected by some of the selection arguments
     (a sex, an index, a random selection or a "fast string") will be
     duplicated and added to the 'res' record set.  */

  /* TODO: generate a list of random indexes here.  */

  {
    rec_record_t record = NULL;
    size_t num_rec = -1;
    
    rec_mset_iterator_t iter = rec_mset_iterator (rec_rset_mset (rset));
    while (rec_mset_iterator_next (&iter, MSET_RECORD, (const void **) &record, NULL))
      {
        bool selected = false;
        num_rec++;
        
        /* Determine whether we must skip this record.  */
        
        if (!rec_db_record_selected_p (record,
                                       sex,
                                       fast_string,
                                       flags & REC_Q_ICASE))
          {
            continue;
          }
              
        /* Process this record.  */
        
        if (counter != NULL)
          {
            /* Just increase the counter of selected records.  */
            processed++;
          }
        else
          {
            /* Transform the record through the field expression
               and add it to the result record set.  */
            
            rec_record_t res_record
              = rec_db_process_fex (record, fex);
            
            /* Decrypt the confidential fields in the record if some
               of the fields are declared as "confidential", but only
               do that if the user provided a password.  Note that we
               use 'rset' instead of 'res' to cover cases where (flags
               & REC_Q_DESCRIPTOR) == 0.  */
            
            if (password)
              {
                if (!rec_decrypt_record (rset, res_record, password))
                  {
                    /* Out of memory.  */
                    return NULL;
                  }
              }
            
            if (!res_record
                || !rec_mset_append (rec_rset_mset (rset),
                                     MSET_RECORD,
                                     (void *) res_record,
                                     MSET_RECORD))
              {
                /* Out of memory.  */
                return NULL;
              }
            
          }
      }
    rec_mset_iterator_free (&iter);
  }

  return res;
}

/*
 * Private functions.
 */

static bool
rec_db_record_selected_p (rec_record_t record,
                          rec_sex_t sex,
                          const char *fast_string,
                          bool case_insensitive_p)
{
  /* Note that the logic in this function assumes that SEX and
     FAST_STRING are mutually exclusive.  If they are not then
     FAST_STRING takes precedence.  */

  /* The record is searched for instances of the "fast string", which
     can appear as a substring.  */

  if (fast_string)
    {
      return rec_record_contains_value (record,
                                        fast_string,
                                        case_insensitive_p);
    }

  /* The selected expression is evaluated in the record.  If there is
     an error evaluating the expression, or if the expression does not
     evaluate to 'true', then 'false' is returned.  */

  if (sex)
    {
      bool eval_status;
      return rec_sex_eval (sex, record, &eval_status);
    }

  return true;
}

static rec_record_t
rec_db_process_fex (rec_record_t record,
                    rec_fex_t fex)
{
  rec_record_t res = NULL;
  size_t fex_size, i, j = 0;

  /* If fex is NULL then just return a copy of RECORD.  Otherwise
     create an empty result record.  */
  
  if (!fex)
    {
      return rec_record_dup (record);
    }

  res = rec_record_new ();
  if (!res)
    {
      /* Out of memory.  */
      return NULL;
    }

  /* Iterate on the elements of the FEX, picking and transforming the
     fields of RECORD that must be copied and inserted into RES.  */

  fex_size = rec_fex_size (fex);
  for (i = 0; i < fex_size; i++)
    {
      rec_fex_elem_t elem = rec_fex_get (fex, i);
      const char *field_name = rec_fex_elem_field_name (elem);
      const char *alias = rec_fex_elem_rewrite_to (elem);
      size_t min = rec_fex_elem_min (elem);
      size_t max = rec_fex_elem_max (elem);

      if ((min == -1) && (max == -1))
        {
          /* Add all the fields with that name.  */
             min = 0;
             max = rec_record_get_num_fields_by_name (record, field_name);
        }
      else if (max == -1)
        {
          /* Add just one field: Field[min].  */
          max = min + 1;
        }
      else
        {
          /* Add the interval min..max, max inclusive.  */
          max++;
        }

      /* Add the selected fields to the result record.  */

      for (j = min; j < max; j++)
        {
          rec_field_t res_field = NULL;
          rec_field_t field =
            rec_record_get_field_by_name (record, field_name, j);

          if (!field)
            {
              continue;
            }

          /* Duplicate the field and append it into 'res'.  If there
             is a rewrite rule defined in this fex entry then use it
             instead of the original name of the field.  */

          res_field = rec_field_dup (field);
          if (alias)
            {
              if (!rec_field_set_name (field, alias))
                {
                  /* Out of memory.  */
                  return NULL;
                }
            }

          if (!rec_mset_append (rec_record_mset (res),
                                MSET_FIELD,
                                (void *) res_field,
                                MSET_FIELD))
            {
              /* Out of memory.  */
              return NULL;
            }
        }
    }

  return res;
}

static bool
rec_db_rset_equals_fn (const void *elt1,
                       const void *elt2)
{
  return false;
}

static void
rec_db_rset_dispose_fn (const void *elt)
{
  rec_rset_t rset;

  rset = (rec_rset_t) elt;
  rec_rset_destroy (rset);
}

/* End of rec-db.c */
