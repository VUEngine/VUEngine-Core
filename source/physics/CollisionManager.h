/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef COLLISION_MANAGER_H_
#define COLLISION_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <SpatialObject.h>
#include <Shape.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define CollisionManager_METHODS																		\
		Object_METHODS																					\

// declare the virtual methods which are redefined
#define CollisionManager_SET_VTABLE(ClassName)															\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(CollisionManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(CollisionManager);

void CollisionManager_constructor(CollisionManager this);
void CollisionManager_destructor(CollisionManager this);
Shape CollisionManager_registerShape(CollisionManager this, SpatialObject owner, int shapeType);
void CollisionManager_unregisterShape(CollisionManager this, Shape shape);
Shape CollisionManager_getShape(CollisionManager this, SpatialObject owner);
void CollisionManager_processRemovedShapes(CollisionManager this);
void CollisionManager_update(CollisionManager this, Clock clock);
void CollisionManager_reset(CollisionManager this);
void CollisionManager_shapeStartedMoving(CollisionManager this, Shape shape);
void CollisionManager_shapeStoppedMoving(CollisionManager this, Shape shape);
void CollisionManager_shapeBecameInactive(CollisionManager this, Shape shape);
void CollisionManager_shapeBecameActive(CollisionManager this, Shape shape);
void CollisionManager_drawShapes(CollisionManager this);
void CollisionManager_flushShapesDirectDrawData(CollisionManager this);
void CollisionManager_print(CollisionManager this, int x, int y);


#endif