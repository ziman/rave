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
 * Implementation of a simple list.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-11-20
 */
#ifndef RAVE_LIST_H
#define RAVE_LIST_H
#include "rave_object.h"

/**
 * Defines a list
 */
typedef struct _RaveList_t RaveList_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveList_TYPE;

/**
 * Add one instance to the list.
 * @param[in] list - the list
 * @param[in] obj - the object
 * @returns 1 on success, otherwise 0
 */
int RaveList_add(RaveList_t* list, void* ob);

/**
 * Inserts the object at the specified index, if index < 0 or
 * index > size, then this function will add the object to the
 * end of the list.
 * @param[in] list - the list
 * @param[in] index - the index where to insert the object
 * @param[in] ob - the object to insert.
 */
int RaveList_insert(RaveList_t* list, int index, void* ob);

/**
 * Returns the number of items in this list.
 * @param[in] list - the list
 * @returns the number of items in this list.
 */
int RaveList_size(RaveList_t* list);

/**
 * Returns the item at the specified position.
 * @param[in] list - the list
 * @param[in] index - the index of the requested item
 * @returns the object
 */
void* RaveList_get(RaveList_t* list, int index);

/**
 * Returns the item at the end.
 * @param[in] list - the list
 * @returns the object
 */
void* RaveList_getLast(RaveList_t* list);

/**
 * Removes the item at the specified position and returns it.
 * @param[in] list - the list
 * @param[in] index - the index of the requested item
 * @returns the object
 */
void* RaveList_remove(RaveList_t* list, int index);

/**
 * Removes the last item.
 * @param[in] list - the list
 * @returns the object or NULL if there are no objects
 */
void* RaveList_removeLast(RaveList_t* list);

void RaveList_removeObject(RaveList_t* list, void* object);

void* RaveList_find(RaveList_t* list, void* expected, int (*findfunc)(void*, void*));

/**
 * Sorts the list according to the provided sort function.
 * The sort function should return an integer less than,
 * equal to or greater than zero depending on how the first
 * argument is in relation to the second argument.
 *
 * @param[in] list - the list
 * @param[in] sortfun - the sorting function.
 */
void RaveList_sort(RaveList_t* list, int (*sortfun)(const void*, const void*));

#endif /* RAVE_LIST_H */