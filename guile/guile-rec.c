/* -*- mode: C -*- Time-stamp: "2012-07-21 01:45:15 jemarch"
 *
 *       File:         guile-rec.c
 *       Date:         Fri Jul 20 19:59:24 2012
 *
 *       GNU recutils - Guile bindings.
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

#include <libguile.h>
#include <rec.h>

/* Definition of SMOBS  */

SCM_GLOBAL_SMOB (scm_tc16_rec_db, "db", 0);
SCM_SMOB_FREE (scm_tc16_rec_db, db_free, obj)
{
  rec_db_t db = (rec_db_t) SCM_SMOB_DATA (obj);
  rec_db_destroy (db);
  return 0;
}
SCM_DEFINE (scm_rec_db_p, "db?", 1, 0, 0,
            (SCM obj),
            "Return true if @var{obj} is of type @code{db}.")
#define FUNC_NAME s_scm_rec_db_p
{
  return (scm_from_bool (SCM_SMOB_PREDICATE (scm_tc16_rec_db, obj)));
}
#undef FUNC_NAME

SCM_GLOBAL_SMOB (scm_tc16_rec_rset, "rset", 0);
SCM_SMOB_FREE (scm_tc16_rec_rset, rset_free, obj)
{
  rec_rset_t rset = (rec_rset_t) SCM_SMOB_DATA (obj);
  rec_rset_destroy (rset);
  return 0;
}
SCM_DEFINE (scm_rec_rset_p, "rset?", 1, 0, 0,
            (SCM obj),
            "Return true if @var{obj} is of type @code{rset}.")
#define FUNC_NAME s_scm_rec_rset_p
{
  return (scm_from_bool (SCM_SMOB_PREDICATE (scm_tc16_rec_rset, obj)));
}
#undef FUNC_NAME

/* Procedures.  */

SCM_DEFINE (scm_rec_db_new, "make-db", 0, 0, 0,
              (void),
              "Return a new empty database.")
#define FUNC_NAME s_scm_rec_db_new
{
  rec_db_t db;
  SCM db_data;

  db = rec_db_new ();
  if (!db)
    {
      return SCM_BOOL_F;
    }

/*  SCM_GNUTLS_SET_SESSION_DATA (db, db_data); */
  SCM_RETURN_NEWSMOB (scm_tc16_rec_db, (scm_t_bits) db);
}
#undef FUNC_NAME

SCM_DEFINE (scm_rec_db_size, "db-size", 1, 0, 0,
            (SCM obj),
            "Get the number of record sets stored in a given database.")
#define FUNC_NAME s_scm_rec_db_size
{
  rec_db_t db = (rec_db_t) SCM_SMOB_DATA (obj);
  return scm_from_unsigned_integer (rec_db_size (db));
}

/* Initialization.  */

void
scm_init_rec (void)
{
#include "guile-rec.x"

  rec_init ();
}

/* End of guile-rec.c */
