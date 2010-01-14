/* -*- mode: C -*- Time-stamp: "10/01/14 21:06:36 jemarch"
 *
 *       File:         rec-db.c
 *       Date:         Thu Jan 14 15:35:27 2010
 *
 *       GNU recutils - Databases
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
#include <gl_array_list.h>
#include <gl_list.h>

#include <rec.h>

/*
 * Data structures.
 */

struct rec_db_s
{
  int size;             /* Number of record sets contained in this
                           database.  */
  gl_list_t rset_list;  /* List of record sets.  */
};

/* Static functions defined in this file.  */
static bool rec_db_rset_equals_fn (const void *elt1,
                                   const void *elt2);
static void rec_db_rset_dispose_fn (const void *elt);

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
  gl_list_free (db->rset_list);
  free (db);
}

int
rec_db_size (rec_db_t db)
{
  return db->size;
}

rec_rset_t
rec_db_get_rset (rec_db_t db,
                 int position)
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

      rset = (rec_rset_t) gl_list_get_at (db->rset_list,
                                          position);
    }

  return rset;
}

bool
rec_db_insert_rset (rec_db_t db,
                    rec_rset_t rset,
                    int position)
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
rec_db_remove_rset (rec_db_t db, int position)
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
               char *type)
{
  return (rec_db_get_rset_by_type (db, type) != NULL);
}

rec_rset_t
rec_db_get_rset_by_type (rec_db_t db,
                         char *type)
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
          continue;
        }

      if (strcmp (rtype, type) == 0)
        {
          found = true;
          break;
        }
    }

  if (!found)
    {
      rset = NULL;
    }
  
  return rset;
}

/*
 * Private functions.
 */

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
