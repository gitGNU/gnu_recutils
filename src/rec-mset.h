/* -*- mode: C -*- Time-stamp: ""
 *
 *       File:         rec-mset.h
 *       Date:         Thu Apr  1 16:31:37 2010
 *
 *       GNU recutils - Ordered Heterogeneous Set
 *
 */

#ifndef REC_MSET_H
#define REC_MSET_H

#include <config.h>
#include <stdbool.h>

/*
 * Ordered Heterogenous Sets (multi-sets).
 *
 * Element types: A, B, C
 *
 *    type   value     next_A   next_B   next_C
 *   +-----+----------+-------+--------+--------+
 *   |     |          |       |        |        |
 *   +-----+----------+-------+--------+--------+
 *   |     |          |       |        |        |
 *   .     .          .       .        .        .
 *   .     .          .       .        .        .
 *   |     |          |       |        |        |
 *   +-----+----------+-------+--------+--------+
 */

/*
 * Data types.
 */

typedef struct rec_mset_s *rec_mset_t;
typedef struct rec_mset_elem_s *rec_mset_elem_t;

typedef void (*rec_mset_disp_fn_t) (void *data);
typedef bool (*rec_mset_equal_fn_t) (void *data1, void *data2);
typedef void *(*rec_mset_dup_fn_t) (void *data);

/*
 * Constants.
 */
#define MSET_ANY 0

/*
 * Functions.
 */

rec_mset_t rec_mset_new (void);
void rec_mset_destroy (rec_mset_t mset);

rec_mset_t rec_mset_dup (rec_mset_t mset);

/* Types management.  */
bool rec_mset_type_p (rec_mset_t mset, int type);
int rec_mset_register_type (rec_mset_t mset,
                            char *name,
                            rec_mset_disp_fn_t disp_fn,
                            rec_mset_equal_fn_t equal_fn,
                            rec_mset_dup_fn_t dup_fn);

/* Statistics.  */
int rec_mset_count (rec_mset_t mset, int type);

/* Getting and setting elements.  */
rec_mset_elem_t rec_mset_get (rec_mset_t mset, int type, int position);

bool rec_mset_remove_at (rec_mset_t mset, int position);
void rec_mset_insert_at (rec_mset_t mset, rec_mset_elem_t elem, int position);
void rec_mset_append (rec_mset_t mset, rec_mset_elem_t elem);

rec_mset_elem_t rec_mset_remove (rec_mset_t mset, rec_mset_elem_t elem);
void rec_mset_insert_after (rec_mset_t mset, rec_mset_elem_t elem, rec_mset_elem_t new_elem);

/* Searching.  */
rec_mset_elem_t rec_mset_search (rec_mset_t mset, void *data);

/* Iterating.  */
rec_mset_elem_t rec_mset_first (rec_mset_t mset, int type);
rec_mset_elem_t rec_mset_next (rec_mset_t mset, rec_mset_elem_t elem, int type);

/* Elements.  */
rec_mset_elem_t rec_mset_elem_new (rec_mset_t mset, int type);
void rec_mset_elem_destroy (rec_mset_elem_t elem);
int rec_mset_elem_type (rec_mset_elem_t elem);
void *rec_mset_elem_data (rec_mset_elem_t elem);
void rec_mset_elem_set_data (rec_mset_elem_t elem, void *data);
bool rec_mset_elem_equal_p (rec_mset_elem_t elem1, rec_mset_elem_t elem2);

/* Debug.  */
void rec_mset_dump (rec_mset_t mset);

#endif /* rec-mset.h */

/* End of rec-mset.h */
