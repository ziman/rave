/* --------------------------------------------------------------------
Copyright (C) 2010 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Provides support for reading and writing areas to and from
 * an xml-file.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-12-08
 */
#ifndef AREAREGISTRY_H
#define AREAREGISTRY_H
#include "area.h"
#include "rave_object.h"
#include "projectionregistry.h"

/**
 * Defines the area registry
 */
typedef struct _AreaRegistry_t AreaRegistry_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType AreaRegistry_TYPE;

/**
 * Loads an area registry from a xml file
 * @param[in] self - self
 * @param[in] filename - the xml file to load
 * @returns 1 on success 0 otherwise
 */
int AreaRegistry_loadRegistry(AreaRegistry_t* self, const char* filename);

/**
 * Simplified loading function, takes filename and a projection registry
 * @param[in] filename - the area file name
 * @param[in] pRegistry - the projection registry
 * @returns an area registry
 */
AreaRegistry_t* AreaRegistry_load(const char* filename, ProjectionRegistry_t* pRegistry);

/**
 * Returns the number of registered areas
 * @param[in] self - self
 * @returns the number of registered areas
 */
int AreaRegistry_size(AreaRegistry_t* self);

/**
 * Returns the area at specified index
 * @param[in] self - self
 * @param[in] index - the index
 * @returns the area or NULL if no such area
 */
Area_t* AreaRegistry_get(AreaRegistry_t* self, int index);

/**
 * Sets a projection registry to be able to fetch projections.
 * @param[in] self - self
 * @param[in] registry - the projection registry
 */
void AreaRegistry_setProjectionRegistry(AreaRegistry_t* self, ProjectionRegistry_t* registry);

/**
 * Sets a projection registry to be able to fetch projections.
 * @param[in] self - self
 * @returns the projection registry
 */
ProjectionRegistry_t* AreaRegistry_getProjectionRegistry(AreaRegistry_t* self);

#endif /* AREAREGISTRY_H */
