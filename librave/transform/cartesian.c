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
 * Defines the functions available when working with cartesian products
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-16
 */
#include "cartesian.h"
#include "area.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "rave_datetime.h"
#include "rave_data2d.h"
#include "raveobject_hashtable.h"
#include "rave_utilities.h"
#include <string.h>

/**
 * Represents the cartesian product.
 */
struct _Cartesian_t {
  RAVE_OBJECT_HEAD /** Always on top */

  // Where
  double xscale;     /**< xscale */
  double yscale;     /**< yscale */

  // x / ysize to use for parameters
  long xsize;        /**< xsize to use */
  long ysize;        /**< ysize to use */

  Rave_ProductType product;   /**< product */
  Rave_ObjectType objectType; /**< object type */

  double llX;        /**< lower left x-coordinate */
  double llY;        /**< lower left y-coordinate */
  double urX;        /**< upper right x-coordinate */
  double urY;        /**< upper right x-coordinate */

  // What
  RaveDateTime_t* datetime;  /**< the date and time */
  RaveDateTime_t* startdatetime;  /**< the start date and time */
  RaveDateTime_t* enddatetime;  /**< the end date and time */

  char* source;              /**< where does this data come from */

  RaveDataType datatype;     /**< the datatype to use */
  Projection_t* projection; /**< the projection */

  RaveObjectHashTable_t* attrs; /**< attributes */

  RaveObjectList_t* qualityfields; /**< quality fields */

  char* defaultParameter;                     /**< the default parameter */
  CartesianParam_t* currentParameter; /**< the current parameter */
  RaveObjectHashTable_t* parameters;  /**< the cartesian data fields */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int Cartesian_constructor(RaveCoreObject* obj)
{
  Cartesian_t* this = (Cartesian_t*)obj;
  this->xsize = 0;
  this->ysize = 0;
  this->xscale = 0.0;
  this->yscale = 0.0;
  this->llX = 0.0;
  this->llY = 0.0;
  this->urX = 0.0;
  this->urY = 0.0;
  this->datetime = NULL;
  this->product = Rave_ProductType_UNDEFINED;
  this->objectType = Rave_ObjectType_IMAGE;
  this->source = NULL;
  this->datatype = RaveDataType_UCHAR;
  this->projection = NULL;
  this->currentParameter = NULL;
  this->defaultParameter = RAVE_STRDUP("DBZH");
  this->datetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);
  this->startdatetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);
  this->enddatetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);
  this->attrs = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  this->qualityfields = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
  this->parameters = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  if (this->datetime == NULL || this->defaultParameter == NULL || this->attrs == NULL ||
      this->startdatetime == NULL || this->enddatetime == NULL || this->qualityfields == NULL ||
      this->parameters == NULL) {
    goto fail;
  }

  return 1;
fail:
  RAVE_OBJECT_RELEASE(this->currentParameter);
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->startdatetime);
  RAVE_OBJECT_RELEASE(this->enddatetime);
  RAVE_OBJECT_RELEASE(this->qualityfields);
  RAVE_OBJECT_RELEASE(this->parameters);
  return 0;
}

/**
 * Copy constructor.
 */
static int Cartesian_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  Cartesian_t* this = (Cartesian_t*)obj;
  Cartesian_t* src = (Cartesian_t*)srcobj;
  this->xscale = src->xscale;
  this->yscale = src->yscale;
  this->xsize = src->xsize;
  this->ysize = src->ysize;
  this->llX = src->llX;
  this->llY = src->llY;
  this->urX = src->urX;
  this->urY = src->urY;
  this->product = src->product;
  this->objectType = src->objectType;
  this->datatype = src->datatype;
  this->source = NULL;
  this->projection = NULL;
  this->datetime = NULL;
  this->startdatetime = NULL;
  this->enddatetime = NULL;
  this->currentParameter = NULL;
  this->defaultParameter = NULL;

  this->datetime = RAVE_OBJECT_CLONE(src->datetime);
  this->startdatetime = RAVE_OBJECT_CLONE(src->startdatetime);
  this->enddatetime = RAVE_OBJECT_CLONE(src->enddatetime);
  this->currentParameter = RAVE_OBJECT_CLONE(src->currentParameter);
  this->attrs = RAVE_OBJECT_CLONE(src->attrs);
  this->qualityfields = RAVE_OBJECT_CLONE(src->qualityfields);
  this->parameters = RAVE_OBJECT_CLONE(src->parameters);

  if (this->datetime == NULL || (src->currentParameter != NULL && this->currentParameter == NULL) || this->attrs == NULL ||
      this->startdatetime == NULL || this->enddatetime == NULL || this->qualityfields == NULL ||
      this->parameters == NULL || !Cartesian_setDefaultParameter(this, Cartesian_getDefaultParameter(src))) {
    if (this->datetime == NULL) {
      fprintf(stderr, "Failed datetime\n");
    } else if (this->currentParameter == NULL) {
      fprintf(stderr, "currentParameter\n");
    } else if (this->attrs == NULL) {
      fprintf(stderr, "attrs\n");
    }
    fprintf(stderr, "Failed to clone something\n");
    goto fail;
  }

  Cartesian_setSource(this, Cartesian_getSource(src));

  if (src->projection != NULL) {
    this->projection = RAVE_OBJECT_CLONE(src->projection);
    if (this->projection == NULL) {
      goto fail;
    }
  }

  return 1;
fail:
  RAVE_FREE(this->source);
  RAVE_OBJECT_RELEASE(this->currentParameter);
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_OBJECT_RELEASE(this->startdatetime);
  RAVE_OBJECT_RELEASE(this->enddatetime);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->projection);
  RAVE_OBJECT_RELEASE(this->qualityfields);
  RAVE_OBJECT_RELEASE(this->parameters);
  RAVE_FREE(this->defaultParameter);
  return 0;
}

/**
 * Destroys the cartesian product
 * @param[in] scan - the cartesian product to destroy
 */
static void Cartesian_destructor(RaveCoreObject* obj)
{
  Cartesian_t* cartesian = (Cartesian_t*)obj;
  if (cartesian != NULL) {
    RAVE_OBJECT_RELEASE(cartesian->projection);
    RAVE_OBJECT_RELEASE(cartesian->datetime);
    RAVE_OBJECT_RELEASE(cartesian->startdatetime);
    RAVE_OBJECT_RELEASE(cartesian->enddatetime);
    RAVE_FREE(cartesian->source);
    RAVE_OBJECT_RELEASE(cartesian->currentParameter);
    RAVE_OBJECT_RELEASE(cartesian->attrs);
    RAVE_OBJECT_RELEASE(cartesian->qualityfields);
    RAVE_OBJECT_RELEASE(cartesian->parameters);
    RAVE_FREE(cartesian->defaultParameter);
  }
}

/*@} End of Private functions */

/*@{ Interface functions */
int Cartesian_setTime(Cartesian_t* cartesian, const char* value)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_setTime(cartesian->datetime, value);
}

const char* Cartesian_getTime(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_getTime(cartesian->datetime);
}

int Cartesian_setDate(Cartesian_t* cartesian, const char* value)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_setDate(cartesian->datetime, value);
}

const char* Cartesian_getDate(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_getDate(cartesian->datetime);
}

int Cartesian_setStartTime(Cartesian_t* cartesian, const char* value)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_setTime(cartesian->startdatetime, value);
}

const char* Cartesian_getStartTime(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (RaveDateTime_getTime(cartesian->startdatetime) == NULL) {
    return RaveDateTime_getTime(cartesian->datetime);
  }
  return RaveDateTime_getTime(cartesian->startdatetime);
}

int Cartesian_setStartDate(Cartesian_t* cartesian, const char* value)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_setDate(cartesian->startdatetime, value);
}

const char* Cartesian_getStartDate(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (RaveDateTime_getDate(cartesian->startdatetime) == NULL) {
    return RaveDateTime_getDate(cartesian->datetime);
  }
  return RaveDateTime_getDate(cartesian->startdatetime);
}

int Cartesian_setEndTime(Cartesian_t* cartesian, const char* value)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_setTime(cartesian->enddatetime, value);
}

const char* Cartesian_getEndTime(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (RaveDateTime_getTime(cartesian->enddatetime) == NULL) {
    return RaveDateTime_getTime(cartesian->datetime);
  }
  return RaveDateTime_getTime(cartesian->enddatetime);
}

int Cartesian_setEndDate(Cartesian_t* cartesian, const char* value)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveDateTime_setDate(cartesian->enddatetime, value);
}

const char* Cartesian_getEndDate(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (RaveDateTime_getDate(cartesian->enddatetime) == NULL) {
    return RaveDateTime_getDate(cartesian->datetime);
  }
  return RaveDateTime_getDate(cartesian->enddatetime);
}

int Cartesian_setSource(Cartesian_t* cartesian, const char* value)
{
  char* tmp = NULL;
  int result = 0;
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (value != NULL) {
    tmp = RAVE_STRDUP(value);
    if (tmp != NULL) {
      RAVE_FREE(cartesian->source);
      cartesian->source = tmp;
      tmp = NULL;
      result = 1;
    }
  } else {
    RAVE_FREE(cartesian->source);
    result = 1;
  }
  return result;
}

const char* Cartesian_getSource(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return (const char*)cartesian->source;
}

int Cartesian_setObjectType(Cartesian_t* self, Rave_ObjectType type)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (type == Rave_ObjectType_IMAGE || type == Rave_ObjectType_COMP) {
    self->objectType = type;
    return 1;
  }
  return 0;
}

Rave_ObjectType Cartesian_getObjectType(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->objectType;
}

void Cartesian_setXSize(Cartesian_t* self, long xsize)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->xsize = xsize;

}

void Cartesian_setYSize(Cartesian_t* self, long ysize)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->ysize = ysize;
}

long Cartesian_getXSize(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->xsize;
}

long Cartesian_getYSize(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->ysize;
}

void Cartesian_setAreaExtent(Cartesian_t* cartesian, double llX, double llY, double urX, double urY)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  cartesian->llX = llX;
  cartesian->llY = llY;
  cartesian->urX = urX;
  cartesian->urY = urY;
}

void Cartesian_getAreaExtent(Cartesian_t* cartesian, double* llX, double* llY, double* urX, double* urY)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (llX != NULL) {
    *llX = cartesian->llX;
  }
  if (llY != NULL) {
    *llY = cartesian->llY;
  }
  if (urX != NULL) {
    *urX = cartesian->urX;
  }
  if (urY != NULL) {
    *urY = cartesian->urY;
  }
}

void Cartesian_setXScale(Cartesian_t* cartesian, double xscale)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  cartesian->xscale = xscale;
}

double Cartesian_getXScale(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return cartesian->xscale;
}

void Cartesian_setYScale(Cartesian_t* cartesian, double yscale)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  cartesian->yscale = yscale;
}

double Cartesian_getYScale(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return cartesian->yscale;
}

int Cartesian_setProduct(Cartesian_t* cartesian, Rave_ProductType type)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  cartesian->product = type;
  return 1;
}

Rave_ProductType Cartesian_getProduct(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return cartesian->product;
}

double Cartesian_getNodata(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->currentParameter != NULL) {
    return CartesianParam_getNodata(self->currentParameter);
  }
  return 0.0;
}

double Cartesian_getUndetect(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->currentParameter != NULL) {
    return CartesianParam_getUndetect(self->currentParameter);
  }
  return 0.0;
}

double Cartesian_getLocationX(Cartesian_t* cartesian, long x)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return cartesian->llX + cartesian->xscale * (double)x;
}

double Cartesian_getLocationY(Cartesian_t* cartesian, long y)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return cartesian->urY - cartesian->yscale * (double)y;
}

long Cartesian_getIndexX(Cartesian_t* cartesian, double x)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  RAVE_ASSERT((cartesian->xscale != 0.0), "xcale == 0.0, would result in Division by zero");
  return (long)((x - cartesian->llX) / cartesian->xscale);
}

long Cartesian_getIndexY(Cartesian_t* cartesian, double y)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  RAVE_ASSERT((cartesian->yscale != 0.0), "ycale == 0.0, would result in Division by zero");
  return (long)((cartesian->urY - y)/cartesian->yscale);
}

int Cartesian_setDefaultParameter(Cartesian_t* cartesian, const char* name)
{
  int result = 0;
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (name != NULL) {
    char* tmp = RAVE_STRDUP(name);
    if (tmp == NULL) {
      RAVE_CRITICAL0("Failed to allocate memory");
      goto done;
    }
    RAVE_FREE(cartesian->defaultParameter);
    cartesian->defaultParameter = tmp;
    RAVE_OBJECT_RELEASE(cartesian->currentParameter);
    if (RaveObjectHashTable_exists(cartesian->parameters, name)) {
      cartesian->currentParameter = (CartesianParam_t*)RaveObjectHashTable_get(cartesian->parameters, name);
    }
    result = 1;
  } else {
    RAVE_WARNING0("Not supported parameter name");
  }

done:
  return result;
}

const char* Cartesian_getDefaultParameter(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  return (const char*)cartesian->defaultParameter;
}

void Cartesian_setProjection(Cartesian_t* cartesian, Projection_t* projection)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  RAVE_OBJECT_RELEASE(cartesian->projection);
  if (projection != NULL) {
    cartesian->projection = RAVE_OBJECT_COPY(projection);
  }
}

Projection_t* Cartesian_getProjection(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (cartesian->projection != NULL) {
    return RAVE_OBJECT_COPY(cartesian->projection);
  }
  return NULL;
}

const char* Cartesian_getProjectionString(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (cartesian->projection != NULL) {
    return Projection_getDefinition(cartesian->projection);
  }
  return NULL;
}

int Cartesian_setValue(Cartesian_t* cartesian, long x, long y, double v)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (cartesian->currentParameter != NULL) {
    return CartesianParam_setValue(cartesian->currentParameter, x, y, v);
  }
  return 0;
}

int Cartesian_setConvertedValue(Cartesian_t* cartesian, long x, long y, double v)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian was NULL");
  if (cartesian->currentParameter != NULL) {
    return CartesianParam_setConvertedValue(cartesian->currentParameter, x, y, v);
  }
  return 0;
}

RaveValueType Cartesian_getValue(Cartesian_t* cartesian, long x, long y, double* v)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (cartesian->currentParameter != NULL) {
    return CartesianParam_getValue(cartesian->currentParameter, x, y, v);
  }
  return RaveValueType_UNDEFINED;
}

RaveValueType Cartesian_getConvertedValue(Cartesian_t* cartesian, long x, long y, double* v)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (cartesian->currentParameter != NULL) {
    return CartesianParam_getConvertedValue(cartesian->currentParameter, x, y, v);
  }
  return RaveValueType_UNDEFINED;
}

void Cartesian_init(Cartesian_t* self, Area_t* area)
{
  double llX = 0.0L, llY = 0.0L, urX = 0.0L, urY = 0.0L;
  Projection_t* projection = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((area != NULL), "area == NULL");

  Cartesian_setXScale(self, Area_getXScale(area));
  Cartesian_setYScale(self, Area_getYScale(area));
  Cartesian_setXSize(self, Area_getXSize(area));
  Cartesian_setYSize(self, Area_getYSize(area));
  projection = Area_getProjection(area);
  Cartesian_setProjection(self, projection);
  Area_getExtent(area, &llX, &llY, &urX, &urY);
  Cartesian_setAreaExtent(self, llX, llY, urX, urY);
  RAVE_OBJECT_RELEASE(projection);
}

RaveValueType Cartesian_getMean(Cartesian_t* cartesian, long x, long y, int N, double* v)
{
  RaveValueType xytype = RaveValueType_NODATA;
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  xytype = Cartesian_getValue(cartesian, x, y, v);
  if (xytype == RaveValueType_DATA) {
    long xk = 0, yk = 0;
    double sum = 0.0L;
    int pts = 0;
    int k = N/2;
    double value = 0.0L;

    for (yk = -k; yk < k; yk++) {
      for (xk = -k; xk < k; xk++) {
        xytype = Cartesian_getValue(cartesian, xk + x, yk + y, &value);
        if (xytype == RaveValueType_DATA) {
          sum += value;
          pts++;
        }
      }
    }
    *v = sum / (double)pts; // we have at least 1 at pts so division by zero will not occur
  }

  return xytype;
}

int Cartesian_isTransformable(Cartesian_t* cartesian)
{
  int result = 0;
  int ncount = 0;
  int i = 0;
  RaveObjectList_t* params = NULL;

  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  params = RaveObjectHashTable_values(cartesian->parameters);
  if (params == NULL) {
    goto done;
  }
  ncount = RaveObjectList_size(params);
  if (ncount <= 0 || cartesian->xscale <= 0.0 || cartesian->yscale <= 0.0 || cartesian->projection == NULL) {
    goto done;
  }

  result = 1;
  for (i = 0; result == 1 && i < ncount; i++) {
    CartesianParam_t* param = (CartesianParam_t*)RaveObjectList_get(params, i);
    if (param != NULL) {
      result = CartesianParam_isTransformable(param);
    } else {
      result = 0;
    }
    RAVE_OBJECT_RELEASE(param);
  }

done:
  RAVE_OBJECT_RELEASE(params);
  return result;
}

int Cartesian_addAttribute(Cartesian_t* cartesian, RaveAttribute_t* attribute)
{
  const char* name = NULL;
  char* aname = NULL;
  char* gname = NULL;
  int result = 0;
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  RAVE_ASSERT((attribute != NULL), "attribute == NULL");

  name = RaveAttribute_getName(attribute);
  if (name != NULL) {
    if (!RaveAttributeHelp_extractGroupAndName(name, &gname, &aname)) {
      RAVE_ERROR1("Failed to extract group and name from %s", name);
      goto done;
    }
    if (strcasecmp("how", gname)==0 &&
      strchr(aname, '/') == NULL) {
      result = RaveObjectHashTable_put(cartesian->attrs, name, (RaveCoreObject*)attribute);
    } else if (strcasecmp("what/prodpar", name)==0) {
      result = RaveObjectHashTable_put(cartesian->attrs, name, (RaveCoreObject*)attribute);
    } else {
      RAVE_WARNING1("You are not allowed to add dynamic attributes in other groups than 'how': '%s'", name);
      goto done;
    }
  }

done:
  RAVE_FREE(aname);
  RAVE_FREE(gname);
  return result;
}

RaveAttribute_t* Cartesian_getAttribute(Cartesian_t* cartesian, const char* name)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  if (name == NULL) {
    RAVE_ERROR0("Trying to get an attribute with NULL name");
    return NULL;
  }
  return (RaveAttribute_t*)RaveObjectHashTable_get(cartesian->attrs, name);
}

RaveList_t* Cartesian_getAttributeNames(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveObjectHashTable_keys(cartesian->attrs);
}

RaveObjectList_t* Cartesian_getAttributeValues(Cartesian_t* cartesian)
{
  RaveObjectList_t* result = NULL;
  RaveObjectList_t* attrs = NULL;

  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  attrs = RaveObjectHashTable_values(cartesian->attrs);
  if (attrs == NULL) {
    goto error;
  }
  result = RAVE_OBJECT_CLONE(attrs);
error:
  RAVE_OBJECT_RELEASE(attrs);
  return result;
}

int Cartesian_hasAttribute(Cartesian_t* cartesian, const char* name)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveObjectHashTable_exists(cartesian->attrs, name);
}

int Cartesian_addQualityField(Cartesian_t* cartesian, RaveField_t* field)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveObjectList_add(cartesian->qualityfields, (RaveCoreObject*)field);
}

RaveField_t* Cartesian_getQualityField(Cartesian_t* cartesian, int index)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return (RaveField_t*)RaveObjectList_get(cartesian->qualityfields, index);
}

int Cartesian_getNumberOfQualityFields(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return RaveObjectList_size(cartesian->qualityfields);
}

void Cartesian_removeQualityField(Cartesian_t* cartesian, int index)
{
  RaveField_t* field = NULL;
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  field = (RaveField_t*)RaveObjectList_remove(cartesian->qualityfields, index);
  RAVE_OBJECT_RELEASE(field);
}

RaveObjectList_t* Cartesian_getQualityFields(Cartesian_t* cartesian)
{
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  return (RaveObjectList_t*)RAVE_OBJECT_COPY(cartesian->qualityfields);
}

int Cartesian_addParameter(Cartesian_t* self, CartesianParam_t* param)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (param != NULL) {
    const char* p = CartesianParam_getQuantity(param);
    if (p == NULL) {
      RAVE_ERROR0("Parameter does not contain any quantity");
      goto done;
    }
    if (RaveObjectHashTable_size(self->parameters) == 0) {
      self->xsize = CartesianParam_getXSize(param);
      self->ysize = CartesianParam_getYSize(param);
    }

    if (CartesianParam_getXSize(param) != self->xsize ||
        CartesianParam_getYSize(param) != self->ysize) {
      RAVE_ERROR0("Inconsistent x/y size between parameters");
      goto done;
    }

    if (!RaveObjectHashTable_put(self->parameters, p, (RaveCoreObject*)param)) {
      RAVE_ERROR0("Could not add parameter to cartesian");
      goto done;
    }

    if (strcmp(self->defaultParameter, p) == 0) {
      RAVE_OBJECT_RELEASE(self->currentParameter);
      self->currentParameter = RAVE_OBJECT_COPY(param);
    }
    result = 1;
  }

done:
  return result;
}

CartesianParam_t* Cartesian_getParameter(Cartesian_t* self, const char* name)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (RaveObjectHashTable_exists(self->parameters, name)) {
    return (CartesianParam_t*)RaveObjectHashTable_get(self->parameters, name);
  }
  return NULL;
}

int Cartesian_hasParameter(Cartesian_t* self, const char* quantity)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveObjectHashTable_exists(self->parameters, quantity);
}


void Cartesian_removeParameter(Cartesian_t* self, const char* name)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  CartesianParam_t* param = (CartesianParam_t*)RaveObjectHashTable_remove(self->parameters, name);
  RAVE_OBJECT_RELEASE(param);
}

int Cartesian_getParameterCount(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveObjectHashTable_size(self->parameters);
}

RaveList_t* Cartesian_getParameterNames(Cartesian_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveObjectHashTable_keys(self->parameters);
}

CartesianParam_t* Cartesian_createParameter(Cartesian_t* self, const char* quantity, RaveDataType type)
{
  CartesianParam_t* result = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->xsize > 0 && self->ysize > 0 && quantity != NULL && type != RaveDataType_UNDEFINED) {
    result = RAVE_OBJECT_NEW(&CartesianParam_TYPE);
    if (result == NULL ||
        !CartesianParam_createData(result, self->xsize, self->ysize, type) ||
        !CartesianParam_setQuantity(result, quantity) ||
        !Cartesian_addParameter(self, result)) {
      RAVE_OBJECT_RELEASE(result);
    }
  }
  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType Cartesian_TYPE = {
    "Cartesian",
    sizeof(Cartesian_t),
    Cartesian_constructor,
    Cartesian_destructor,
    Cartesian_copyconstructor
};

