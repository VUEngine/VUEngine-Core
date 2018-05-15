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
//											CLASS'S DECLARATION
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


abstract class Shape : Object
{
	/**
	* @var SpatialObject 	owner
	* @brief				the entity to which the shape belongs
	* @memberof			Shape
	*/
	SpatialObject owner;
	/**
	* @var VirtualList 	collidingShapes
	* @brief				colliding shapes list
	* @memberof			CollisionSolver
	*/
	VirtualList collidingShapes;
	/**
	* @var 32 				layers
	* @brief				layers on which this shape live
	* @memberof			Shape
	*/
	u32 layers;
	/**
	* @var 32 				layersToIgnore
	* @brief				layers to ignore when checking for collisions
	* @memberof			Shape
	*/
	u32 layersToIgnore;
	/**
	* @var Sphere		wireframe
	* @brief			for debugging purposes
	* @memberof 		Ball
	*/
	Wireframe wireframe;
	/**
	* @var u8 				ready
	* @brief				flag to know if setup is needed
	* @memberof			Shape
	*/
	u8 ready;
	/**
	* @var u8 				moved
	* @brief				flag to know if has moved
	* @memberof			Shape
	*/
	u8 moved;
	/**
	* @var u8 				isActive
	* @brief				flag to know if shape is reacting to collisions
	* @memberof			Shape
	*/
	u8 isActive;
	/**
	* @var u8 				checkForCollisions
	* @brief				flag to check against other shapes
	* @memberof			Shape
	*/
	u8 checkForCollisions;
	/**
	* @var u8 				isVisible
	* @brief				flag to cull off shapes outside the screen
	* @memberof			Shape
	*/
	u8 isVisible;

	void constructor(Shape this, SpatialObject owner);
	void enterCollision(Shape this, CollisionData* collisionData);
	void updateCollision(Shape this, CollisionData* collisionData);
	void exitCollision(Shape this, CollisionData* collisionData);
	CollisionData collides(Shape this, Shape shape);
	bool checkForCollisions(Shape this);
	SpatialObject getOwner(Shape this);
	void reset(Shape this);
	bool isActive(Shape this);
	bool isReady(Shape this);
	void setActive(Shape this, bool active);
	void setCheckForCollisions(Shape this, bool checkForCollisions);
	void setReady(Shape this, bool ready);
	bool canMoveTowards(Shape this, Vector3D displacement, fix10_6 sizeIncrement);
	fix10_6 getCollidingFrictionCoefficient(Shape this);
	void resolveCollision(Shape this, const CollisionInformation* collisionInformation);
	u32 getLayers(Shape this);
	void setLayers(Shape this, u32 layers);
	u32 getLayersToIgnore(Shape this);
	void setLayersToIgnore(Shape this, u32 layersToIgnore);
	void show(Shape this);
	void hide(Shape this);
	virtual void setup(Shape this, u32 layers, u32 layersToIgnore);
	virtual void position(Shape this, const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size);
	virtual CollisionInformation testForCollision(Shape this, Shape shape, Vector3D displacement, fix10_6 sizeIncrement) = 0;
	virtual Vector3D getPosition(Shape this) = 0;
	virtual RightBox getSurroundingRightBox(Shape this) = 0;
	virtual void configureWireframe(Shape this) = 0;
	virtual void print(Shape this, int x, int y) = 0;
}


#endif
