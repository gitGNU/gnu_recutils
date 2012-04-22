/* -*- mode: C -*- Time-stamp: "2012-04-22 13:50:41 jemarch"
 *
 *       File:         recmodule.c
 *       Date:         Sun Feb 26 20:27:56 2012
 *
 *       GNU recutils - Python bindings for librec
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

#include <Python.h>
#include <rec.h>

/*
 * Version method.
 */

static char prec_version_doc[] =
"version()\n\
\n\
Return the version of librec.";

static PyObject *
prec_version (PyObject *self, PyObject *args)
{
  if (!PyArg_ParseTuple (args, ""))
    {
      return NULL;
    }

  return (Py_BuildValue("II", REC_VERSION_MAJOR, REC_VERSION_MINOR));
}

/*
 * Definition of the global functions implemented by the module.
 * Other functions are defined in the PyTypeObject structure of the
 * corresponding type (class).
 */

static PyMethodDef prec_functions[] =
  {
    {"version", prec_version, METH_VARARGS, prec_version_doc},
    {NULL, NULL}
  };

/*
 * Initialization function, which is called when the module is loaded.
 */

static char prec_doc[] =
  "This module provides bindings to the librec library (GNU recutils).";

PyMODINIT_FUNC
initrec (void)
{
  /* Initialize constant containers.  */
  /* Initialize types.  */
  /* Internally created/read-only type overrides.  */
  /* Initialize module.  */
  PyObject *mod = Py_InitModule3 ("rec", prec_functions, prec_doc);
  if (!mod)
    {
      return;
    }

  /* Add types to the module.  */
  /*  PyModule_AddObject (mod, "Field",  (PyObject*) &prec_field_type); */
  /* PyModule_AddObject (mod, "Record", (PyObject*) &prec_record_type); */

  /* Add constants.  */
}

/* End of recmodule.c */
