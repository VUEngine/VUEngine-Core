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

typedef enum CollisionResult
{
	kNoCollision = 0,
	kEnterCollision,
	kUpdateCollision,
	kExitCollision,

} CollisionResult;

/**
 * Collision information
 *
 * @memberof Shape
 */
typedef struct SolutionVector
{
	// collision's plane normal
	Vector3D direction;

	// minimum vector to solve the collision
	fix10_6 magnitude;

} SolutionVector;

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

	// information to solve the collision
	SolutionVector solutionVector;

} CollisionInformation;


/**
 * Collision shape registry
 *
 * @memberof Shape
 */
typedef struct CollidingShapeRegistry
{
	Shape shape;

	SolutionVector solutionVector;

	fix10_6 frictionCoefficient;

	bool isImpenetrable;

} CollidingShapeRegistry;

/**
 * Collision data
 *
 * @memberof Shape
 */
typedef struct CollisionData
{
	CollisionResult result;
	CollisionInformation collisionInformation;
	Shape shapeNotCollidingAnymore;
	bool isImpenetrableCollidingShape;

} CollisionData;

/**
 * Normals
 *
 * @memberof Shape
 */
typedef struct Normals
{
	Vector3D vectors[__SHAPE_NORMALS];
} Normals;

typedef struct VertexProjection
{
	fix10_6 min;
	fix10_6 max;
} VertexProjection;


#define Shape_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, void, setup, u32 layers, u32 layersToIgnore);							\
		__VIRTUAL_DEC(ClassName, void, position, const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size);		\
		__VIRTUAL_DEC(ClassName, bool, collides, Shape shape);											\
		__VIRTUAL_DEC(ClassName, CollisionInformation, testForCollision, Shape collidingShape, Vector3D displacement, fix10_6 sizeIncrement);	\
		__VIRTUAL_DEC(ClassName, Vector3D, getPosition);												\
		__VIRTUAL_DEC(ClassName, RightBox, getSurroundingRightBox);										\
		__VIRTUAL_DEC(ClassName, void, show);															\
		__VIRTUAL_DEC(ClassName, void, hide);															\
		__VIRTUAL_DEC(ClassName, void, print, int x, int y);											\
		__VIRTUAL_DEC(ClassName, bool, canMoveTowards, Vector3D displacement, fix10_6 sizeIncrement);	\

#define Shape_SET_VTABLE(ClassName)																		\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Shape, setup);															\
		__VIRTUAL_SET(ClassName, Shape, position);															\
		__VIRTUAL_SET(ClassName, Shape, collides);														\
		__VIRTUAL_SET(ClassName, Shape, canMoveTowards);												\

#define Shape_ATTRIBUTES																				\
		Object_ATTRIBUTES																				\
		/**
		 * @var SpatialObject 	owner
		 * @brief				the entity to which the shape belongs
		 * @memberof			Shape
		 */																								\
		SpatialObject owner;																			\
		/**
		 * @var VirtualList 	collidingShapes
		 * @brief				colliding shapes list
		 * @memberof			CollisionSolver
		 */																								\
		VirtualList collidingShapes;																	\
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
		/**
		 * @var u8 				isVisible
		 * @brief				flag to cull off shapes outside the screen
		 * @memberof			Shape
		 */																								\
		u8 isVisible;																					\

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

	/// size in pixels
	PixelSize pixelSize;

	/// displacement modifier
	PixelVector displacement;

	/// rotation modifier
	Rotation rotation;

	/// scale modifier
	Scale scale;

	/// if true this shape checks for collisions against other shapes
	bool checkForCollisions;

	/// layers in which I live
	u32 layers;

	/// layers to ignore when checking for collisions
	u32 layersToIgnore;

} ShapeDefinition;

typedef const ShapeDefinition ShapeROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void Shape_constructor(Shape this, SpatialObject owner);
void Shape_destructor(Shape this);

void Shape_enterCollision(Shape this, CollisionData* collisionData);
void Shape_updateCollision(Shape this, CollisionData* collisionData);
void Shape_exitCollision(Shape this, CollisionData* collisionData);
CollisionData Shape_collides(Shape this, Shape shape);
bool Shape_checkForCollisions(Shape this);
SpatialObject Shape_getOwner(Shape this);
void Shape_reset(Shape this);
void Shape_setup(Shape this, u32 layers, u32 layersToIgnore);
void Shape_position(Shape this, const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size);
void Shape_hide(Shape this);
bool Shape_isActive(Shape this);
bool Shape_isReady(Shape this);
void Shape_setActive(Shape this, bool active);
void Shape_setCheckForCollisions(Shape this, bool checkForCollisions);
void Shape_setReady(Shape this, bool ready);
bool Shape_canMoveTowards(Shape this, Vector3D displacement, fix10_6 sizeIncrement);
fix10_6 Shape_getCollidingFrictionCoefficient(Shape this);
void Shape_resolveCollision(Shape this, const CollisionInformation* collisionInformation);
u32 Shape_getLayers(Shape this);
void Shape_setLayers(Shape this, u32 layers);
u32 Shape_getLayersToIgnore(Shape this);
void Shape_setLayersToIgnore(Shape this, u32 layersToIgnore);
void Shape_print(Shape this, int x, int y);


#endif
