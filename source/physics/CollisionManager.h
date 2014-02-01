/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef COLLISIONMANAGER_H_
#define COLLISIONMANAGER_H_


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Object.h>
#include <Shape.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DECLARATION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/* Defines as a pointer to a structure that
 * is not defined here and so is not accessible to the outside world
 */
// declare the virtual methods
#define CollisionManager_METHODS						\
		Object_METHODS									\


// declare the virtual methods which are redefined
#define CollisionManager_SET_VTABLE(ClassName)						\
		Object_SET_VTABLE(ClassName)								\


__CLASS(CollisionManager);

 
//spacial position
typedef struct CollisionInfo{
	
	//FPS increases a lot in hardware with ints
	InGameEntity inGameEntity;
	int axis;

}CollisionInfo;


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										PUBLIC INTERFACE
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// it is a singleton!
CollisionManager CollisionManager_getInstance();

// class's destructor
void CollisionManager_destructor(CollisionManager this);

// register a shape
Shape CollisionManager_registerShape(CollisionManager this, InGameEntity owner, int shapeType);

// remove a shape
void CollisionManager_unregisterShape(CollisionManager this, Shape shape);

// find a shape given an owner
Shape CollisionManager_getShape(CollisionManager this, InGameEntity owner);

// process removed shapes
void CollisionManager_processRemovedShapes(CollisionManager this);

// calculate collisions
void CollisionManager_update(CollisionManager this);

// update a shape
///void CollisionManager_updateShape(CollisionManager this, Shape shape, const VBVec3D* const position);

// unregister all shapes
void CollisionManager_reset(CollisionManager this);

// check if an entity has been registered
int CollisionManager_isEntityRegistered(CollisionManager this, InGameEntity owner);

#endif /*CollisionManager_H_*/
