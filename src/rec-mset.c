/* -*- mode: C -*- Time-stamp: "2010-04-02 14:56:20 jemarch"
 *
 *       File:         rec-mset.c
 *       Date:         Thu Apr  1 17:07:00 2010
 *
 *       GNU recutils - Ordered Heterogeneous Set
 *
 */

#include <config.h>

#include <string.h>

#include <gl_array_list.h>
#include <gl_list.h>

#include <rec-mset.h>

/*
 * Data types.
 */

#define MAX_NTYPES 4

struct rec_mset_elem_s
{
  int type;
  void *data;

  /* Containing set.  */
  rec_mset_t mset;
};

struct rec_mset_s
{
  int ntypes;

  /* Properties of the element types.  */
  char *name[MAX_NTYPES];
  rec_mset_disp_fn_t disp_fn[MAX_NTYPES];
  rec_mset_equal_fn_t equal_fn[MAX_NTYPES];

  /* Statistics.  */
  int count[MAX_NTYPES];

  gl_list_t elem_list;
};

/*
 * Forward rdelcarations of static functions.
 */

static bool rec_mset_elem_equal_fn (const void *e1,
                                    const void *e2);
static void rec_mset_elem_dispose_fn (const void *e);

/*
 * Public functions.
 */

rec_mset_t
rec_mset_new (void)
{
  rec_mset_t new;
  int i;

  new = malloc (sizeof (struct rec_mset_s));
  if (new)
    {
      new->ntypes = 1;
      new->name[0] = NULL;
      new->equal_fn[0] = NULL;
      new->disp_fn[0] = NULL;

      new->elem_list = gl_list_nx_create_empty (GL_ARRAY_LIST,
                                                rec_mset_elem_equal_fn,
                                                NULL,
                                                rec_mset_elem_dispose_fn,
                                                true);

      if (new->elem_list == NULL)
        {
          /* Out of memory.  */
          free (new);
          new = NULL;
        }
    }

  return new;
}

void
rec_mset_destroy (rec_mset_t mset)
{
  gl_list_free (mset->elem_list);
}

bool
rec_mset_type_p (rec_mset_t mset,
                 int type)
{
  return type < mset->ntypes;
}

int
rec_mset_register_type (rec_mset_t mset,
                        char *name,
                        rec_mset_disp_fn_t disp_fn,
                        rec_mset_equal_fn_t equal_fn)
{
  int new_type;

  new_type = mset->ntypes++;
  mset->count[new_type] = 0;
  mset->name[new_type] = strdup (name);
  mset->disp_fn[new_type] = disp_fn;
  mset->equal_fn[new_type] = equal_fn;

  return new_type;
}

int
rec_mset_count (rec_mset_t mset,
                int type)
{
  return mset->count[type];
}

rec_mset_elem_t
rec_mset_get (rec_mset_t mset,
              int type,
              int position)
{
  gl_list_iterator_t iter;
  gl_list_node_t node;
  rec_mset_elem_t elem;
  rec_mset_elem_t result;
  int count[MAX_NTYPES];

  if ((position < 0) || (position >= mset->count[type]))
    {
      /* Invalid order.  */
      return NULL;
    }
  

  result = NULL;
  memset (count, 0, MAX_NTYPES);
  iter = gl_list_iterator (mset->elem_list);
  while (gl_list_iterator_next (&iter, (const void **) &elem, &node))
    {
      if (count[elem->type] == position)
        {
          result = elem;
          break;
        }
      else
        {
          count[elem->type]++;
          if (elem->type != 0)
            {
              count[0]++;
            }
        }
    }
  
  return result;
}

bool
rec_mset_remove_at (rec_mset_t mset,
                    int position)
{
  rec_mset_elem_t elem;
  int elem_type;
  bool removed;

  removed = false;

  if (mset->count[0] > 0)
    {
      if (position < 0)
        {
          position = 0;
        }
      if (position >= mset->count[0])
        {
          position = mset->count[0] - 1;
        }
      
      elem = rec_mset_get (mset, MSET_ANY, position);
      elem_type = elem->type;

      if (gl_list_remove_at (mset->elem_list,
                             position))
        {
          mset->count[0]--;
          mset->count[elem->type]--;
          removed = true;
        }
    }

  return removed;
}

void
rec_mset_insert_at (rec_mset_t mset,
                    rec_mset_elem_t elem,
                    int position)
{
  gl_list_node_t node;

  node = NULL;

  if (position < 0)
    {
      node = gl_list_nx_add_first (mset->elem_list,
                                   (void *) elem);
    }
  else if (position >= mset->count[0])
    {
      node = gl_list_nx_add_last (mset->elem_list,
                                  (void *) elem);
    }
  else
    {
      node = gl_list_nx_add_at (mset->elem_list,
                                position,
                                (void *) elem);
    }

  if (node != NULL)
    {
      mset->count[0]++;
      mset->count[elem->type]++;
    }
}

void
rec_mset_append (rec_mset_t mset,
                 rec_mset_elem_t elem)
{
  rec_mset_insert_at (mset, MSET_ANY, rec_mset_count (mset, MSET_ANY));
}

rec_mset_elem_t
rec_mset_remove (rec_mset_t mset,
                 rec_mset_elem_t elem)
{
  rec_mset_elem_t next_elem;
  gl_list_node_t node;
  gl_list_node_t next_node;

  next_elem = NULL;
  node = gl_list_search (mset->elem_list, (void *) elem);
  if (node)
    {
      mset->count[0]--;
      mset->count[elem->type]--;

      next_node = gl_list_next_node (mset->elem_list, node);
      if (next_node)
        {
          next_elem = (rec_mset_elem_t) gl_list_node_value (mset->elem_list,
                                                            next_node);
        }

      gl_list_remove_node (mset->elem_list, node);
    }

  return next_elem;
}

void
rec_mset_insert_after (rec_mset_t mset,
                       rec_mset_elem_t elem,
                       rec_mset_elem_t new_elem)
{
  gl_list_node_t node;

  node = gl_list_search (mset->elem_list, (void *) elem);
  if (node)
    {
      gl_list_nx_add_after (mset->elem_list,
                            node,
                            (void *) new_elem);

      mset->count[0]++;
      mset->count[new_elem->type]++;
    }
}

rec_mset_elem_t
rec_mset_search (rec_mset_t mset,
                 void *data)
{
  rec_mset_elem_t result;
  rec_mset_elem_t elem;

  result = NULL;
  elem = rec_mset_get (mset, MSET_ANY, 0);
  do
    {
      if (elem->data == data)
        {
          result = elem;
          break;
        }

      elem = rec_mset_elem_next (elem, MSET_ANY);
    }
  while (elem);

  return result;
}

rec_mset_elem_t
rec_mset_elem_first (rec_mset_t mset,
                     int type)
{
  return rec_mset_get (mset, type, 0);
}

rec_mset_elem_t
rec_mset_elem_next (rec_mset_elem_t elem,
                    int type)
{
  rec_mset_elem_t result;
  rec_mset_elem_t next_elem;
  gl_list_node_t node;

  result = NULL;
  node = gl_list_search (elem->mset->elem_list, (void *) elem);
  while (node = gl_list_next_node (elem->mset->elem_list, node))
    {
      next_elem = (rec_mset_elem_t) gl_list_node_value (elem->mset->elem_list,
                                                        node);
      if (next_elem->type == type)
        {
          result = next_elem;
          break;
        }
    }

  return result;
}

rec_mset_elem_t
rec_mset_elem_new (rec_mset_t mset,
                   int type)
{
  rec_mset_elem_t new;

  if (type >= mset->ntypes)
    {
      return NULL;
    }

  new = malloc (sizeof (struct rec_mset_elem_s));
  if (new)
    {
      new->type = type;
      new->data = NULL;
      new->mset = mset;
    }

  return new;
}

void
rec_mset_elem_destroy (rec_mset_elem_t elem)
{
  (elem->mset->disp_fn[elem->type]) (elem->data);
  free (elem);
}

int
rec_mset_elem_type (rec_mset_elem_t elem)
{
  return elem->type;
}

void *
rec_mset_elem_data (rec_mset_elem_t elem)
{
  return elem->data;
}

void
rec_mset_elem_set_data (rec_mset_elem_t elem,
                        void *data)
{
  elem->data = data;
}

/*
 * Private functions.
 */

static bool
rec_mset_elem_equal_fn (const void *e1,
                        const void *e2)
{
  rec_mset_elem_t elem1;
  rec_mset_elem_t elem2;

  elem1 = (rec_mset_elem_t) e1;
  elem2 = (rec_mset_elem_t) e2;

  if ((elem1->mset != elem2->mset)
      || (elem1->type != elem2->type))
    {
      return false;
    }

  return (elem1->mset->equal_fn[elem1->type]) (elem1->data,
                                               elem2->data);
}

static void
rec_mset_elem_dispose_fn (const void *e)
{
  rec_mset_elem_t elem;

  elem = (rec_mset_elem_t) e;
  rec_mset_elem_destroy (elem);
}

/* End of rec-mset.c */
