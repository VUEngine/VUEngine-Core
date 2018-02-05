/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef COLLISION_MANAGER_H_
#define COLLISION_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <SpatialObject.h>
#include <Shape.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define CollisionManager_METHODS(ClassName)																\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define CollisionManager_SET_VTABLE(ClassName)															\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(CollisionManager);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(CollisionManager);

void CollisionManager_constructor(CollisionManager this);
void CollisionManager_destructor(CollisionManager this);

void CollisionManager_hideShapes(CollisionManager this);
void CollisionManager_print(CollisionManager this, int x, int y);
Shape CollisionManager_createShape(CollisionManager this, SpatialObject owner, const ShapeDefinition* shapeDefinition);
void CollisionManager_destroyShape(CollisionManager this, Shape shape);
void CollisionManager_reset(CollisionManager this);
void CollisionManager_shapeBecameActive(CollisionManager this, Shape shape);
void CollisionManager_shapeBecameInactive(CollisionManager this, Shape shape);
void CollisionManager_shapeStartedMoving(CollisionManager this, Shape shape);
void CollisionManager_shapeStoppedMoving(CollisionManager this, Shape shape);
void CollisionManager_showShapes(CollisionManager this);
u32 CollisionManager_update(CollisionManager this, Clock clock);


#endif
