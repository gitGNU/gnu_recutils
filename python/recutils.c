//  #include <config.h>

#include <Python.h>
#include <rec.h>
#include "structmember.h"


typedef struct {
    PyObject_HEAD
    rec_db_t RDb;  
} RecDb;


/* Create an empty database.  */
static PyObject *
RecDb_new (PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    RecDb *self;
    self = (RecDb *)type->tp_alloc(type, 0);
    if (self != NULL) 
    {
        self->RDb = rec_db_new();
        if (self->RDb == NULL) 
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
RecDb_dealloc (RecDb* self)
{
    rec_db_destroy (self->RDb);
    self->ob_type->tp_free((PyObject*)self);
}


/* Return the number of record sets contained in a given record
   set.  */
static PyObject*
RecDb_size(RecDb* self)
{
    int s = rec_db_size (self->RDb);
    return Py_BuildValue("i", s);
}


/* Load a file into the Database */
static PyObject*
RecDb_loadfile(RecDb *self, PyObject *args, PyObject *kwds)
{

    char *string = NULL;
    static char *kwlist[] = {"filename", NULL};
    printf("Before parsing\n");
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &string)) {
      return NULL;
    }
    printf("Parsed filename: %s\n", string);
    FILE *in = fopen(string,"r");
    rec_parser_t parser = rec_parser_new (in, string);
    bool success = rec_parse_db (parser, &(self->RDb));
    printf("success - %d\n", (int)success);
    fclose(in);
    Py_DECREF(args);
    int num = rec_db_size (self->RDb);
    return Py_BuildValue("i", num);
}

static PyObject*
RecDb_writefile(RecDb *self, PyObject *args, PyObject *kwds)
{
    char *string = NULL;
    static char *kwlist[] = {"filename", NULL};
    printf("Before parsing\n");
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &string)) {
      return NULL;
    }
    printf("Parsed filename: %s\n", string);
    FILE *out = fopen(string,"w");
    rec_writer_t writer = rec_writer_new (out);
    printf("Created writer\n");
    bool success = rec_write_db (writer, self->RDb);
    printf("success - %d\n", (int)success);
    fclose(out);
    return Py_BuildValue("");

}



/*RecDb doc string */
static char RecDb_doc[] =
  "This module provides bindings to the librec library (GNU recutils).";


static PyMethodDef RecDb_methods[] = {
    {"size", (PyCFunction)RecDb_size, METH_NOARGS,
     "Return the size of the DB"
    },
    {"loadfile", (PyCFunction)RecDb_loadfile, 
     METH_VARARGS, 
     "Load data from file into DB"
    },
    {"writefile", (PyCFunction)RecDb_writefile, 
     METH_VARARGS, 
     "Write data from DB to file"
    },
    {NULL}  
};



/* Define the RecDb object type */
static PyTypeObject RecDbType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "recutils.RecDb",              /*tp_name*/
    sizeof(RecDb),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)RecDb_dealloc, /*tp_dealloc*/
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
    RecDb_doc,                 /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    RecDb_methods,             /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    RecDb_new,                 /* tp_new */
};




/*
 * Initialization function, which is called when the module is loaded.
 */

PyMODINIT_FUNC
initrecutils (void) 
{
    PyObject* m;

    if (PyType_Ready(&RecDbType) < 0)
        return;

    m = Py_InitModule3("recutils", RecDb_methods, RecDb_doc);

    if (m == NULL)
      return;

    Py_INCREF(&RecDbType);
    PyModule_AddObject(m, "RecDb", (PyObject *)&RecDbType);
}
    