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
 * Python version of the RadarDefinition API.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-08-31
 */
#include "Python.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "pyrave_debug.h"

#define PYRADARDEFINITION_MODULE    /**< to get correct part in pyarea.h */
#include "pyradardefinition.h"
#include "pyprojection.h"
#include "rave_alloc.h"

/**
 * Debug this module
 */
PYRAVE_DEBUG_MODULE("_radardef");

/**
 * Sets a python exception and goto tag
 */
#define raiseException_gotoTag(tag, type, msg) \
{PyErr_SetString(type, msg); goto tag;}

/**
 * Sets a python exception and return NULL
 */
#define raiseException_returnNULL(type, msg) \
{PyErr_SetString(type, msg); return NULL;}

/**
 * Error object for reporting errors to the python interpreeter
 */
static PyObject *ErrorObject;

/*@{ RadarDefinition */
/**
 * Returns the native RadarDefinition_t instance.
 * @param[in] radar - the python radar definition
 * @returns the native radar definition instance.
 */
static RadarDefinition_t*
PyRadarDefinition_GetNative(PyRadarDefinition* radar)
{
  RAVE_ASSERT((radar != NULL), "radar == NULL");
  return RAVE_OBJECT_COPY(radar->def);
}

/**
 * Creates a python radar definition from a native radar definition or will create an
 * initial radar definition if p is NULL.
 * @param[in] p - the native radar definition (or NULL)
 * @returns the python radar definition.
 */
static PyRadarDefinition*
PyRadarDefinition_New(RadarDefinition_t* p)
{
  PyRadarDefinition* result = NULL;
  RadarDefinition_t* cp = NULL;

  if (p == NULL) {
    cp = RAVE_OBJECT_NEW(&RadarDefinition_TYPE);
    if (cp == NULL) {
      RAVE_CRITICAL0("Failed to allocate memory for radar definition.");
      raiseException_returnNULL(PyExc_MemoryError, "Failed to allocate memory for radar definition.");
    }
  } else {
    cp = RAVE_OBJECT_COPY(p);
    result = RAVE_OBJECT_GETBINDING(p); // If p already have a binding, then this should only be increfed.
    if (result != NULL) {
      Py_INCREF(result);
    }
  }

  if (result == NULL) {
    result = PyObject_NEW(PyRadarDefinition, &PyRadarDefinition_Type);
    if (result != NULL) {
      PYRAVE_DEBUG_OBJECT_CREATED;
      result->def = RAVE_OBJECT_COPY(cp);
      RAVE_OBJECT_BIND(result->def, result);
    } else {
      RAVE_CRITICAL0("Failed to create PyRadarDefinition instance");
      raiseException_gotoTag(done, PyExc_MemoryError, "Failed to allocate memory for PyRadarDefinition.");
    }
  }

done:
  RAVE_OBJECT_RELEASE(cp);
  return result;
}

/**
 * Deallocates the radar definition
 * @param[in] obj the object to deallocate.
 */
static void _pyradardefinition_dealloc(PyRadarDefinition* obj)
{
  /*Nothing yet*/
  if (obj == NULL) {
    return;
  }
  PYRAVE_DEBUG_OBJECT_DESTROYED;
  RAVE_OBJECT_UNBIND(obj->def, obj);
  RAVE_OBJECT_RELEASE(obj->def);
  PyObject_Del(obj);
}

/**
 * Creates a new instance of the radar definition.
 * @param[in] self this instance.
 * @param[in] args arguments for creation (NOT USED).
 * @return the object on success, otherwise NULL
 */
static PyObject* _pyradardefinition_new(PyObject* self, PyObject* args)
{
  PyRadarDefinition* result = PyRadarDefinition_New(NULL);
  return (PyObject*)result;
}

/**
 * All methods a area can have
 */
static struct PyMethodDef _pyradardefinition_methods[] =
{
  {"id", NULL},
  {"description", NULL},
  {"longitude", NULL},
  {"latitude", NULL},
  {"height", NULL},
  {"elangles", NULL},
  {"nrays", NULL},
  {"nbins", NULL},
  {"scale", NULL},
  {"beamwidth", NULL},
  {"wavelength", NULL},
  {"projection", NULL},
  {NULL, NULL } /* sentinel */
};

/**
 * Returns the specified attribute in the radar definition
 * @param[in] self - the radar definition
 */
static PyObject* _pyradardefinition_getattr(PyRadarDefinition* self, char* name)
{
  PyObject* res = NULL;

  if (strcmp("id", name) == 0) {
    if (RadarDefinition_getID(self->def) == NULL) {
      Py_RETURN_NONE;
    } else {
      return PyString_FromString(RadarDefinition_getID(self->def));
    }
  } else if (strcmp("description", name) == 0) {
    if (RadarDefinition_getDescription(self->def) == NULL) {
      Py_RETURN_NONE;
    } else {
      return PyString_FromString(RadarDefinition_getDescription(self->def));
    }
  } else if (strcmp("longitude", name) == 0) {
    return PyFloat_FromDouble(RadarDefinition_getLongitude(self->def));
  } else if (strcmp("latitude", name) == 0) {
    return PyFloat_FromDouble(RadarDefinition_getLatitude(self->def));
  } else if (strcmp("height", name) == 0) {
    return PyFloat_FromDouble(RadarDefinition_getHeight(self->def));
  } else if (strcmp("scale", name) == 0) {
    return PyFloat_FromDouble(RadarDefinition_getScale(self->def));
  } else if (strcmp("beamwidth", name) == 0) {
    return PyFloat_FromDouble(RadarDefinition_getBeamwidth(self->def));
  } else if (strcmp("wavelength", name) == 0) {
    return PyFloat_FromDouble(RadarDefinition_getWavelength(self->def));
  } else if (strcmp("nbins", name) == 0) {
    return PyLong_FromLong(RadarDefinition_getNbins(self->def));
  } else if (strcmp("nrays", name) == 0) {
    return PyLong_FromLong(RadarDefinition_getNrays(self->def));
  } else if (strcmp("elangles", name) == 0) {
    double* angles = NULL;
    unsigned int i = 0;
    unsigned int n = 0;
    PyObject* ret = PyList_New(0);
    if (ret == NULL) {
      return NULL;
    }
    if (!RadarDefinition_getElangles(self->def, &n, &angles)) {
      Py_DECREF(ret);
      return NULL;
    }
    for (i = 0; i < n; i++) {
      if (PyList_Append(ret, PyFloat_FromDouble(angles[i])) < 0) {
        Py_DECREF(ret);
        RAVE_FREE(angles);
        return NULL;
      }
    }
    RAVE_FREE(angles);
    return ret;
  } else if (strcmp("projection", name) == 0) {
    Projection_t* projection = RadarDefinition_getProjection(self->def);
    if (projection != NULL) {
      PyProjection* result = PyProjection_New(projection);
      RAVE_OBJECT_RELEASE(projection);
      return (PyObject*)result;
    } else {
      Py_RETURN_NONE;
    }
  }
  res = Py_FindMethod(_pyradardefinition_methods, (PyObject*) self, name);
  if (res)
    return res;

  PyErr_Clear();
  PyErr_SetString(PyExc_AttributeError, name);
  return NULL;
}

/**
 * Returns the specified attribute in the radar definition
 */
static int _pyradardefinition_setattr(PyRadarDefinition* self, char* name, PyObject* val)
{
  int result = -1;
  if (name == NULL) {
    goto done;
  }
  if (strcmp("id", name) == 0) {
    if (PyString_Check(val)) {
      RadarDefinition_setID(self->def, PyString_AsString(val));
    } else if (val == Py_None) {
      RadarDefinition_setID(self->def, NULL);
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "id must be a string");
    }
  } else if (strcmp("description", name) == 0) {
    if (PyString_Check(val)) {
      RadarDefinition_setDescription(self->def, PyString_AsString(val));
    } else if (val == Py_None) {
      RadarDefinition_setDescription(self->def, NULL);
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "description must be a string");
    }
  } else if (strcmp("longitude", name) == 0) {
    if (PyFloat_Check(val)) {
      RadarDefinition_setLongitude(self->def, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      RadarDefinition_setLongitude(self->def, PyLong_AsDouble(val));
    } else if (PyInt_Check(val)) {
      RadarDefinition_setLongitude(self->def, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "longitude must be a number");
    }
  } else if (strcmp("latitude", name) == 0) {
    if (PyFloat_Check(val)) {
      RadarDefinition_setLatitude(self->def, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      RadarDefinition_setLatitude(self->def, PyLong_AsDouble(val));
    } else if (PyInt_Check(val)) {
      RadarDefinition_setLatitude(self->def, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "longitude must be a number");
    }
  } else if (strcmp("height", name) == 0) {
    if (PyFloat_Check(val)) {
      RadarDefinition_setHeight(self->def, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      RadarDefinition_setHeight(self->def, PyLong_AsDouble(val));
    } else if (PyInt_Check(val)) {
      RadarDefinition_setHeight(self->def, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "height must be a number");
    }
  } else if (strcmp("scale", name) == 0) {
    if (PyFloat_Check(val)) {
      RadarDefinition_setScale(self->def, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      RadarDefinition_setScale(self->def, PyLong_AsDouble(val));
    } else if (PyInt_Check(val)) {
      RadarDefinition_setScale(self->def, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "scale must be a number");
    }
  } else if (strcmp("beamwidth", name) == 0) {
    if (PyFloat_Check(val)) {
      RadarDefinition_setBeamwidth(self->def, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      RadarDefinition_setBeamwidth(self->def, PyLong_AsDouble(val));
    } else if (PyInt_Check(val)) {
      RadarDefinition_setBeamwidth(self->def, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "beamwidth must be a number");
    }
  } else if (strcmp("wavelength", name) == 0) {
    if (PyFloat_Check(val)) {
      RadarDefinition_setWavelength(self->def, PyFloat_AsDouble(val));
    } else if (PyLong_Check(val)) {
      RadarDefinition_setWavelength(self->def, PyLong_AsDouble(val));
    } else if (PyInt_Check(val)) {
      RadarDefinition_setWavelength(self->def, (double)PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "wavelength must be a number");
    }
  } else if (strcmp("nrays", name) == 0) {
    if (PyLong_Check(val)) {
      RadarDefinition_setNrays(self->def, PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      RadarDefinition_setNrays(self->def, PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "nrays must be a integer");
    }
  } else if (strcmp("nbins", name) == 0) {
    if (PyLong_Check(val)) {
      RadarDefinition_setNbins(self->def, PyLong_AsLong(val));
    } else if (PyInt_Check(val)) {
      RadarDefinition_setNbins(self->def, PyInt_AsLong(val));
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "nbins must be a integer");
    }
  } else if (strcmp("elangles", name) == 0) {
    if (PySequence_Check(val)) {
      PyObject* v = NULL;
      Py_ssize_t n = PySequence_Size(val);
      Py_ssize_t i = 0;
      double* angles = RAVE_MALLOC(n * sizeof(double));
      if (angles == NULL) {
        raiseException_gotoTag(done, PyExc_MemoryError, "Could not allocate memory for angles");
      }
      for (i = 0; i < n; i++) {
        v = PySequence_GetItem(val, i);
        if (v != NULL) {
          if (PyFloat_Check(v)) {
            angles[i] = PyFloat_AsDouble(v);
          } else if (PyLong_Check(v)) {
            angles[i] = PyLong_AsDouble(v);
          } else if (PyInt_Check(v)) {
            angles[i] = (double)PyInt_AsLong(v);
          } else {
            Py_XDECREF(v);
            raiseException_gotoTag(done, PyExc_TypeError, "height must be a number");
          }
        }
        Py_XDECREF(v);
      }
      if (!RadarDefinition_setElangles(self->def, (unsigned int)n, angles)) {
        raiseException_gotoTag(done, PyExc_MemoryError, "Could not set angles");
      }
      RAVE_FREE(angles);
    } else {
      raiseException_gotoTag(done, PyExc_TypeError, "elangles must be a sequence of numbers");
    }
  } else if (strcmp("projection", name)==0) {
    if (PyProjection_Check(val)) {
      RadarDefinition_setProjection(self->def, ((PyProjection*)val)->projection);
    } else if (val == Py_None) {
      RadarDefinition_setProjection(self->def, NULL);
    } else {
      raiseException_gotoTag(done, PyExc_TypeError,"projection must be of ProjectionCore type");
    }
  }
  result = 0;
done:
  return result;
}

/*@} End of RadarDefinition */

/*@{ Type definitions */
PyTypeObject PyRadarDefinition_Type =
{
  PyObject_HEAD_INIT(NULL)0, /*ob_size*/
  "RadarDefinitionCore", /*tp_name*/
  sizeof(PyRadarDefinition), /*tp_size*/
  0, /*tp_itemsize*/
  /* methods */
  (destructor)_pyradardefinition_dealloc, /*tp_dealloc*/
  0, /*tp_print*/
  (getattrfunc)_pyradardefinition_getattr, /*tp_getattr*/
  (setattrfunc)_pyradardefinition_setattr, /*tp_setattr*/
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
  {"new", (PyCFunction)_pyradardefinition_new, 1},
  {NULL,NULL} /*Sentinel*/
};

PyMODINIT_FUNC
init_radardef(void)
{
  PyObject *module=NULL,*dictionary=NULL;
  static void *PyRadarDefinition_API[PyRadarDefinition_API_pointers];
  PyObject *c_api_object = NULL;
  PyRadarDefinition_Type.ob_type = &PyType_Type;

  module = Py_InitModule("_radardef", functions);
  if (module == NULL) {
    return;
  }
  PyRadarDefinition_API[PyRadarDefinition_Type_NUM] = (void*)&PyRadarDefinition_Type;
  PyRadarDefinition_API[PyRadarDefinition_GetNative_NUM] = (void *)PyRadarDefinition_GetNative;
  PyRadarDefinition_API[PyRadarDefinition_New_NUM] = (void*)PyRadarDefinition_New;

  c_api_object = PyCObject_FromVoidPtr((void *)PyRadarDefinition_API, NULL);

  if (c_api_object != NULL) {
    PyModule_AddObject(module, "_C_API", c_api_object);
  }

  dictionary = PyModule_GetDict(module);
  ErrorObject = PyString_FromString("_radardef.error");
  if (ErrorObject == NULL || PyDict_SetItemString(dictionary, "error", ErrorObject) != 0) {
    Py_FatalError("Can't define _radardef.error");
  }

  import_pyprojection();
  PYRAVE_DEBUG_INITIALIZE;
}
/*@} End of Module setup */
