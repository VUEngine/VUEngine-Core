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

#ifndef SHAPE_H_
#define SHAPE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Wireframe.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __SHAPE_NORMALS	3


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class SpatialObject;
class Shape;

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

// defines a shape
typedef struct ShapeSpec
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

} ShapeSpec;

typedef const ShapeSpec ShapeROMSpec;


//---------------------------------------------------------------------------------------------------------
//												ENUMS
//---------------------------------------------------------------------------------------------------------

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


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
abstract class Shape : Object
{
	// the entity to which the shape belongs
	SpatialObject owner;
	// colliding shapes list
	VirtualList collidingShapes;
	// layers on which this shape live
	u32 layers;
	// layers to ignore when checking for collisions
	u32 layersToIgnore;
	// for debugging purposes
	Wireframe wireframe;
	// flag to know if setup is needed
	u8 ready;
	// flag to know if has moved
	u8 moved;
	// flag to know if shape is reacting to collisions
	u8 enabled;
	// flag to check against other shapes
	u8 checkForCollisions;
	// flag to cull off shapes outside the screen
	u8 isVisible;

	/// @publicsection
	void constructor(SpatialObject owner);
	void enterCollision(CollisionData* collisionData);
	void updateCollision(CollisionData* collisionData);
	void exitCollision(CollisionData* collisionData);
	CollisionData collides(Shape shape);
	bool checkForCollisions();
	SpatialObject getOwner();
	void reset();
	bool isEnabled();
	bool isReady();
	void enable(bool enable);
	void setCheckForCollisions(bool checkForCollisions);
	void setReady(bool ready);
	bool canMoveTowards(Vector3D displacement, fix10_6 sizeIncrement);
	fix10_6 getCollidingFrictionCoefficient();
	void resolveCollision(const CollisionInformation* collisionInformation);
	u32 getLayers();
	void setLayers(u32 layers);
	u32 getLayersToIgnore();
	void setLayersToIgnore(u32 layersToIgnore);
	void show();
	void hide();
	virtual void setup(u32 layers, u32 layersToIgnore);
	virtual void position(const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size);
	virtual CollisionInformation testForCollision(Shape shape, Vector3D displacement, fix10_6 sizeIncrement) = 0;
	virtual Vector3D getPosition() = 0;
	virtual RightBox getSurroundingRightBox() = 0;
	virtual void configureWireframe() = 0;
	virtual void print(int x, int y) = 0;
}


#endif
