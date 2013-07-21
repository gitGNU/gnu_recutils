//#include <config.h>
#include <Python.h>
#include <rec.h>
#include "structmember.h"

typedef struct {
    PyObject_HEAD
    rec_db_t rdb;  
} recdb;


typedef struct {
    PyObject_HEAD
    rec_rset_t rst;  
} rset;

typedef struct {
    PyObject_HEAD
    rec_record_t rcd;  
} record;

typedef struct {
    PyObject_HEAD
    rec_sex_t sx;  
} sex;

typedef struct {
    PyObject_HEAD
    rec_fex_t fx;  
} fex;

staticforward PyTypeObject rsetType;
staticforward PyTypeObject recordType;


/* Create an empty database.  */

static PyObject *
recdb_new (PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  recdb *self;
  self = (recdb *)type->tp_alloc(type, 0);
  if (self != NULL) 
    {
      self->rdb = rec_db_new();
      if (self->rdb == NULL) 
        {
           /* Out of memory.  */
          Py_DECREF(self);
          return NULL;
        }
    }
  return (PyObject *)self;
}


/* Destroy a database, freeing any used memory.
 *
 * This means that all the record sets contained in the database are
 * also destroyed.
 */

static void
recdb_dealloc (recdb* self)
{
  rec_db_destroy (self->rdb);
  self->ob_type->tp_free((PyObject*)self);
}


/* Return the number of record sets contained in a given record
   set.  */
static PyObject*
recdb_size(recdb* self)
{
  int s = rec_db_size (self->rdb);
  return Py_BuildValue("i", s);
}


/* Load a file into a Database object */

static PyObject*
recdb_loadfile(recdb *self, PyObject *args, PyObject *kwds)
{
  char *string = NULL;
  static char *kwlist[] = {"filename", NULL};
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &string)) 
    {
      return NULL;
    }
  printf("Parsed filename: %s\n", string);
  FILE *in = fopen(string,"r");
  rec_parser_t parser = rec_parser_new (in, string);
  bool success = rec_parse_db (parser, &(self->rdb));
  printf("success - %d\n", (int)success);
  fclose(in);
  Py_DECREF(args);
  int num = rec_db_size (self->rdb);
  return Py_BuildValue("i", num);
}


/*Write to file from a DB object */ 

static PyObject*
recdb_writefile(recdb *self, PyObject *args, PyObject *kwds)
{
  char *string = NULL;
  static char *kwlist[] = {"filename", NULL};
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &string))
    {
      return NULL;
    }
  printf("Parsed filename: %s\n", string);
  FILE *out = fopen(string,"w");
  rec_writer_t writer = rec_writer_new (out);
  bool success = rec_write_db (writer, self->rdb);
  printf("success - %d\n", (int)success);
  fclose(out);
  return Py_BuildValue("");

}


/* Return the record set occupying the given position in the database.
   If no such record set is contained in the database then NULL is
   returned.  */

static PyObject*
recdb_get_rset(recdb *self, PyObject *args, PyObject *kwds)
{
  int pos;
  rec_rset_t res;
  PyObject *result;
  rset *tmp = PyObject_NEW(rset, &rsetType);
  //tmp->rst = rec_rset_new();
  static char *kwlist[] = {"position", NULL};
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &pos)) 
    {
      return NULL;
    }
  res = rec_db_get_rset (self->rdb, pos);
  tmp->rst = res;
  result =  Py_BuildValue("O",tmp);
  return result;
}


/*recdb doc string */
static char recdb_doc[] =
  "This type refers to the database structure of recutils";


static PyMethodDef recdb_methods[] = {
    {"size", (PyCFunction)recdb_size, METH_NOARGS,
     "Return the size of the DB"
    },
    {"loadfile", (PyCFunction)recdb_loadfile, 
     METH_VARARGS, 
     "Load data from file into DB"
    },
    {"writefile", (PyCFunction)recdb_writefile, 
     METH_VARARGS, 
     "Write data from DB to file"
    },
    {"get_rset", (PyCFunction)recdb_get_rset, 
     METH_VARARGS, 
     "Get rset by position"
    },
    {NULL}  
};

/* Define the recdb object type */
static PyTypeObject recdbType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "recutils.recdb",              /*tp_name*/
    sizeof(recdb),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)recdb_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    recdb_doc,                 /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    recdb_methods,             /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    recdb_new,                 /* tp_new */
};


/* Create a new empty record set and return a reference to it.  NULL
   is returned if there is no enough memory to perform the
   operation.  */

static PyObject *
rset_new (PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  rset *self;
  self = (rset *)type->tp_alloc(type, 0);
  if (self != NULL) 
    {
      self->rst = rec_rset_new();
      if (self->rst == NULL) 
        {
            /*Out of memory*/
          Py_DECREF(self);
          return NULL;
        }
    }
  return (PyObject *)self;
}


/* Destroy a record set, freeing all user resources.  This disposes
   all the memory used by the record internals, including any stored
   record or comment.  */

static void
rset_dealloc (rset* self)
{
  rec_rset_destroy (self->rst);
  self->ob_type->tp_free((PyObject*)self);
}


static PyObject*
rset_num_records (rset* self)
{
  if (self->rst == NULL)
    {
        PyErr_SetString(PyExc_AttributeError, "rst");
        return NULL;
    }
  int num = rec_rset_num_records (self->rst);
  return Py_BuildValue("i",num);


}


static PyObject*
rset_descriptor (rset* self)
{
  PyObject *result;
  rec_record_t reco;
  if (self->rst == NULL) 
    {
      PyErr_SetString(PyExc_AttributeError, "rst");
      return NULL;
    }
  record *tmp = PyObject_NEW(record, &recordType);
  reco = rec_rset_descriptor (self->rst);
  //printf("The desc is: %s", reco->source);
  tmp->rcd = reco;
  result = Py_BuildValue("O",tmp);
  return result;
}


/*rset doc string */
static char rset_doc[] =
  "This type refers to the record set structure of recutils";


static PyMethodDef rset_methods[] = {
    {"num_records", (PyCFunction)rset_num_records, METH_NOARGS,
     "Return the number of records in the record set"  
    },
    {"descriptor", (PyCFunction)rset_descriptor, METH_NOARGS,
     "Return the descriptor of the record set"  
    },
    {NULL}
};


/* Define the rset object type */
static PyTypeObject rsetType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "recutils.rset",              /*tp_name*/
    sizeof(rset),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)rset_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    rset_doc,                 /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    rset_methods,             /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    rset_new,                 /* tp_new */
};


/* Create a new empty record and return a reference to it.  NULL is
   returned if there is no enough memory to perform the operation.  */

static PyObject *
record_new (PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  record *self;
  self = (record *)type->tp_alloc(type, 0);
  if (self != NULL) 
    {
      self->rcd = rec_record_new();
      if (self->rcd == NULL) 
        {
           /* Out of memory.  */
          Py_DECREF(self);
          return NULL;
        }
    }
  return (PyObject *)self;
}

/* Destroy a record, freeing all used resources.  This disposes all
   the memory used by the record internals, including any stored field
   or comment.  */

static void
record_dealloc (record* self)
{
  rec_record_destroy (self->rcd);
  self->ob_type->tp_free((PyObject*)self);
}  


static PyObject*
record_num_fields (record* self)
{
  int num = rec_record_num_fields (self->rcd);
  return Py_BuildValue("i",num);

}


/*record doc string */
static char record_doc[] =
  "This type refers to the record structure of recutils";


static PyMethodDef record_methods[] = {
    {"num_fields", (PyCFunction)record_num_fields, METH_NOARGS,
     "Return the number of fields in the record"  
    },
    {NULL}
};


/* Define the record object type */
static PyTypeObject recordType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "recutils.record",              /*tp_name*/
    sizeof(record),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)record_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    record_doc,                 /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    record_methods,             /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    record_new,                 /* tp_new */
};


/* Create a new selection expression and return it.  If there is not
   enough memory to create the sex, then return NULL.  */


static PyMethodDef recutils_methods[] = {
    {NULL}  /* Sentinel */
};

static char recutils_doc[] =
  "This module provides bindings to the librec library (GNU recutils).";

/*
 * Initialization function, which is called when the module is loaded.
 */

PyMODINIT_FUNC
initrecutils (void) 
{
    PyObject* m;

    if (PyType_Ready(&recdbType) < 0)
        return;

    if (PyType_Ready(&rsetType) < 0)
        return;

    if (PyType_Ready(&recordType) < 0)
        return;

    m = Py_InitModule3("recutils", recutils_methods, recutils_doc);

    if (m == NULL)
      return;

    Py_INCREF(&recdbType);
    PyModule_AddObject(m, "recdb", (PyObject *)&recdbType);

    Py_INCREF(&rsetType);
    PyModule_AddObject(m, "rset", (PyObject *)&rsetType);

    Py_INCREF(&recordType);
    PyModule_AddObject(m, "record", (PyObject *)&recordType);
}
    