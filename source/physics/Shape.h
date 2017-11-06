/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef SHAPE_H_
#define SHAPE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

__FORWARD_CLASS(SpatialObject)
__FORWARD_CLASS(Shape)

/**
 * Collision information
 *
 * @memberof Shape
 */
typedef struct CollisionInformation
{
	// shape detecting the collision
	Shape shape;

	// colliding shape
	Shape collidingShape;

	// minimum vector to solve the collision
	VBVec3D minimumTranslationVector;

	// help flag
	bool pendingSATCheck;

} CollisionInformation;


#define Shape_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, CollisionInformation, overlaps, Shape shape);																\
		__VIRTUAL_DEC(ClassName, void, setup, const VBVec3D* position, const Rotation* rotation, const Scale* scale, const Size* size);		\
		__VIRTUAL_DEC(ClassName, VBVec3D, computeMinimumTranslationVector, Shape collidingShape);		\
		__VIRTUAL_DEC(ClassName, bool, testIfCollision, Shape collidingShape, VBVec3D displacement);										\
		__VIRTUAL_DEC(ClassName, VBVec3D, getPosition);													\
		__VIRTUAL_DEC(ClassName, RightCuboid, getSurroundingRightCuboid);								\
		__VIRTUAL_DEC(ClassName, RightCuboid, getPositionedSurroundingRightCuboid);						\
		__VIRTUAL_DEC(ClassName, void, hide);															\
		__VIRTUAL_DEC(ClassName, void, show);															\
		__VIRTUAL_DEC(ClassName, void, print, int x, int y);											\

#define Shape_SET_VTABLE(ClassName)																		\
		Object_SET_VTABLE(ClassName)																	\

#define Shape_ATTRIBUTES																				\
		Object_ATTRIBUTES																				\
		/**
		 * @var SpatialObject 	owner
		 * @brief				the entity to which the shape belongs
		 * @memberof			Shape
		 */																								\
		SpatialObject owner;																			\
		/**
		 * @var u8 				checked
		 * @brief				whether it has been checked for collision in current update
		 * @memberof			Shape
		 */																								\
		u8 checked: 1;																					\
		/**
		 * @var u8 				ready
		 * @brief				flag to know if setup is needed
		 * @memberof			Shape
		 */																								\
		u8 ready: 1;																					\
		/**
		 * @var u8 				checkForCollisions
		 * @brief				flag to check against other shapes
		 * @memberof			Shape
		 */																								\
		u8 checkForCollisions: 1;																		\

__CLASS(Shape);

/**
 * Possible types of a Shape
 *
 * @memberof Shape
 */
enum ShapeTypes
{
	kNoShape = 0,
	kCircle,
	kCuboid,
	kInverseCuboid,
};

// defines a shape
typedef struct ShapeDefinition
{
	/// class allocator
	AllocatorPointer allocator;

	/// size
	Size size;

	/// displacement modifier
	VBVec3D displacement;

	/// rotation modifier
	Rotation rotation;

	/// scale modifier
	Scale scale;

	/// if true this shape checks for collisions against other shapes
	bool checkForCollisions;

} ShapeDefinition;

typedef const ShapeDefinition ShapeROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Shape_constructor(Shape this, SpatialObject owner);
void Shape_destructor(Shape this);

bool Shape_checkForCollisions(Shape this);
SpatialObject Shape_getOwner(Shape this);
void Shape_hide(Shape this);
bool Shape_isActive(Shape this);
bool Shape_isChecked(Shape this);
void Shape_setChecked(Shape this, bool checked);
bool Shape_isReady(Shape this);
void Shape_setMovesFlag(Shape this, bool moves);
void Shape_print(Shape this, int x, int y);
void Shape_setActive(Shape this, bool active);
void Shape_setChecked(Shape this, bool checked);
void Shape_setCheckForCollisions(Shape this, bool checkForCollisions);
void Shape_setReady(Shape this, bool ready);
void Shape_show(Shape this);


#endif
