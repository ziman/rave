/* --------------------------------------------------------------------
Copyright (C) 2009 Swedish Meteorological and Hydrological Institute, SMHI,

This file is part of RAVE.

RAVE is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RAVE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with RAVE.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/
/**
 * Python version of the Area API.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-12-10
 */
#include "Python.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define PYAREA_MODULE
#include "pyarea.h"
#include "rave_debug.h"
#include "rave_alloc.h"

/**
 * Some helpful exception defines.
 */
#define raiseException_gotoTag(tag, type, msg) \
{PyErr_SetString(type, msg); goto tag;}

#define raiseException_returnNULL(type, msg) \
{PyErr_SetString(type, msg); return NULL;}

/**
 * Error object for reporting errors to the python interpreeter
 */
static PyObject *ErrorObject;

/*@{ Area */
/**
 * Returns the native Area_t instance.
 * @param[in] pyarea - the python area instance
 * @returns the native area instance.
 */
static Area_t*
PyArea_GetNative(PyArea* pyarea)
{
  RAVE_ASSERT((pyarea != NULL), "pyarea == NULL");
  return RAVE_OBJECT_COPY(pyarea->area);
}

/**
 * Creates a python area from a native area or will create an
 * initial native area if p is NULL.
 * @param[in] p - the native area (or NULL)
 * @returns the python area product.
 */
static PyArea*
PyArea_New(Area_t* p)
{
  PyArea* result = NULL;
  Area_t* cp = NULL;

  if (p == NULL) {
    cp = RAVE_OBJECT_NEW(&Area_TYPE);
    if (cp == NULL) {
      RAVE_CRITICAL0("Failed to allocate memory for area.");
      raiseException_returnNULL(PyExc_MemoryError, "Failed to allocate memory for area.");
    }
  } else {
    cp = RAVE_OBJECT_COPY(p);
  }

  result = PyObject_NEW(PyArea, &PyArea_Type);
  if (result == NULL) {
    RAVE_CRITICAL0("Failed to create PyArea instance");
    raiseException_gotoTag(error, PyExc_MemoryError, "Failed to allocate memory for PyArea.");
  }

  result->area = RAVE_OBJECT_COPY(cp);
  RAVE_OBJECT_BIND(result->area, result);
error:
  RAVE_OBJECT_RELEASE(cp);
  return result;
}

/**
 * Deallocates the area
 * @param[in] obj the object to deallocate.
 */
static void _pyarea_dealloc(PyArea* obj)
{
  /*Nothing yet*/
  if (obj == NULL) {
    return;
  }
  RAVE_OBJECT_UNBIND(obj->area, obj);
  RAVE_OBJECT_RELEASE(obj->area);
  PyObject_Del(obj);
}

/**
 * Creates a new instance of the area.
 * @param[in] self this instance.
 * @param[in] args arguments for creation (NOT USED).
 * @return the object on success, otherwise NULL
 */
static PyObject* _pyarea_new(PyObject* self, PyObject* args)
{
  PyArea* result = PyArea_New(NULL);
  return (PyObject*)result;
}

/**
 * All methods a area can have
 */
static struct PyMethodDef _pyarea_methods[] =
{
  {NULL, NULL } /* sentinel */
};

/**
 * Returns the specified attribute in the area
 * @param[in] self - the area
 */
static PyObject* _pyarea_getattr(PyArea* self, char* name)
{
  PyObject* res = NULL;

  if (strcmp("xsize", name) == 0) {
    return PyInt_FromLong(Area_getXSize(self->area));
  } else if (strcmp("ysize", name) == 0) {
    return PyInt_FromLong(Area_getYSize(self->area));
  } else if (strcmp("xscale", name) == 0) {
    return PyFloat_FromDouble(Area_getXScale(self->area));
  } else if (strcmp("yscale", name) == 0) {
    return PyFloat_FromDouble(Area_getYScale(self->area));
  }

  res = Py_FindMethod(_pyarea_methods, (PyObject*) self, name);
  if (res)
    return res;

  PyErr_Clear();
  PyErr_SetString(PyExc_AttributeError, name);
  return NULL;
}

/**
 * Returns the specified attribute in the area
 */
static int _pyarea_setattr(PyArea* self, char* name, PyObject* val)
{
  int result = -1;
  if (name == NULL) {
    goto done;
  }
  if (strcmp("xsize", name)==0) {
    if (PyInt_Check(val)) {
      Area_setXSize(self->area, PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError,"xsize must be of type int");
    }
  } else if (strcmp("ysize", name)==0) {
    if (PyInt_Check(val)) {
      Area_setYSize(self->area, PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError,"ysize must be of type int");
    }
  } else if (strcmp("xscale", name)==0) {
    if (PyFloat_Check(val)) {
      Area_setXScale(self->area, PyFloat_AsDouble(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError,"xscale must be of type float");
    }
  } else if (strcmp("yscale", name)==0) {
    if (PyFloat_Check(val)) {
      Area_setYScale(self->area, PyFloat_AsDouble(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError,"yscale must be of type float");
    }
  }

  result = 0;
done:
  return result;
}

/*@} End of Area */

/*@{ Type definitions */
PyTypeObject PyArea_Type =
{
  PyObject_HEAD_INIT(NULL)0, /*ob_size*/
  "AreaCore", /*tp_name*/
  sizeof(PyArea), /*tp_size*/
  0, /*tp_itemsize*/
  /* methods */
  (destructor)_pyarea_dealloc, /*tp_dealloc*/
  0, /*tp_print*/
  (getattrfunc)_pyarea_getattr, /*tp_getattr*/
  (setattrfunc)_pyarea_setattr, /*tp_setattr*/
  0, /*tp_compare*/
  0, /*tp_repr*/
  0, /*tp_as_number */
  0,
  0, /*tp_as_mapping */
  0 /*tp_hash*/
};
/*@} End of Type definitions */

/*@{ Module setup */
static PyMethodDef functions[] = {
  {"new", (PyCFunction)_pyarea_new, 1},
  {NULL,NULL} /*Sentinel*/
};

PyMODINIT_FUNC
init_area(void)
{
  PyObject *module=NULL,*dictionary=NULL;
  static void *PyArea_API[PyArea_API_pointers];
  PyObject *c_api_object = NULL;
  PyArea_Type.ob_type = &PyType_Type;

  module = Py_InitModule("_area", functions);
  if (module == NULL) {
    return;
  }
  PyArea_API[PyArea_Type_NUM] = (void*)&PyArea_Type;
  PyArea_API[PyArea_GetNative_NUM] = (void *)PyArea_GetNative;
  PyArea_API[PyArea_New_NUM] = (void*)PyArea_New;

  c_api_object = PyCObject_FromVoidPtr((void *)PyArea_API, NULL);

  if (c_api_object != NULL) {
    PyModule_AddObject(module, "_C_API", c_api_object);
  }

  dictionary = PyModule_GetDict(module);
  ErrorObject = PyString_FromString("_area.error");
  if (ErrorObject == NULL || PyDict_SetItemString(dictionary, "error", ErrorObject) != 0) {
    Py_FatalError("Can't define _area.error");
  }

  Rave_initializeDebugger();
}
/*@} End of Module setup */