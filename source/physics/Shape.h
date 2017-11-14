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
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __SHAPE_NORMALS	3


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
typedef struct CollisionSolution
{
	// minimum vector to solve the collision
	Vector3D translationVector;

	// collision's plane normal
	Vector3D collisionPlaneNormal;

	// minimum vector to solve the collision
	fix19_13 translationVectorLength;

} CollisionSolution;

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

	// help flag
	bool isCollisionSolutionValid;

	// information to solve the collision
	CollisionSolution collisionSolution;

} CollisionInformation;

typedef struct Normals
{
	Vector3D vectors[__SHAPE_NORMALS];
} Normals;


#define Shape_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, CollisionInformation, overlaps, Shape shape);																\
		__VIRTUAL_DEC(ClassName, void, setup, const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size, u32 layers, u32 layersToIgnore);		\
		__VIRTUAL_DEC(ClassName, CollisionSolution, getCollisionSolution, Shape collidingShape);		\
		__VIRTUAL_DEC(ClassName, CollisionSolution, testForCollision, Shape collidingShape, Vector3D displacement, fix19_13 sizeIncrement);	\
		__VIRTUAL_DEC(ClassName, Vector3D, getPosition);													\
		__VIRTUAL_DEC(ClassName, RightBox, getSurroundingRightBox);										\
		__VIRTUAL_DEC(ClassName, void, show);															\
		__VIRTUAL_DEC(ClassName, void, hide);															\
		__VIRTUAL_DEC(ClassName, void, print, int x, int y);											\

#define Shape_SET_VTABLE(ClassName)																		\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Shape, setup);															\

#define Shape_ATTRIBUTES																				\
		Object_ATTRIBUTES																				\
		/**
		 * @var SpatialObject 	owner
		 * @brief				the entity to which the shape belongs
		 * @memberof			Shape
		 */																								\
		SpatialObject owner;																			\
		/**
		 * @var 32 				layers
		 * @brief				layers on which this shape live
		 * @memberof			Shape
		 */																								\
		u32 layers;																						\
		/**
		 * @var 32 				layersToIgnore
		 * @brief				layers to ignore when checking for collisions
		 * @memberof			Shape
		 */																								\
		u32 layersToIgnore;																				\
		/**
		 * @var u8 				ready
		 * @brief				flag to know if setup is needed
		 * @memberof			Shape
		 */																								\
		u8 ready;																						\
		/**
		 * @var u8 				checkForCollisions
		 * @brief				flag to check against other shapes
		 * @memberof			Shape
		 */																								\
		u8 checkForCollisions;																			\

__CLASS(Shape);

/**
 * Possible types of a Shape
 *
 * @memberof Shape
 */
enum ShapeTypes
{
	kNoShape = 0,
	kBall,
	kBox,
	kInverseBox,
};

// defines a shape
typedef struct ShapeDefinition
{
	/// class allocator
	AllocatorPointer allocator;

	/// size
	Size size;

	/// displacement modifier
	Vector3D displacement;

	/// rotation modifier
	Rotation rotation;

	/// scale modifier
	Scale scale;

	/// if true this shape checks for collisions against other shapes
	bool checkForCollisions;

	/// layers to ignore when checking for collisions
	u32 layers;

	/// if true this shape checks for collisions against other shapes
	u32 layersToIgnore;

} ShapeDefinition;

typedef const ShapeDefinition ShapeROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Shape_constructor(Shape this, SpatialObject owner);
void Shape_destructor(Shape this);

bool Shape_checkForCollisions(Shape this);
SpatialObject Shape_getOwner(Shape this);
void Shape_setup(Shape this, const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size, u32 layers, u32 layersToIgnore);
void Shape_hide(Shape this);
bool Shape_isActive(Shape this);
bool Shape_isReady(Shape this);
void Shape_setMovesFlag(Shape this, bool moves);
void Shape_print(Shape this, int x, int y);
void Shape_setActive(Shape this, bool active);
void Shape_setCheckForCollisions(Shape this, bool checkForCollisions);
void Shape_setReady(Shape this, bool ready);


#endif
