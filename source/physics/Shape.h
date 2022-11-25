/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SHAPE_H_
#define SHAPE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
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
	fixed_t magnitude;

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

	fixed_t frictionCoefficient;

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
	fixed_t min;
	fixed_t max;
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
	PixelRotation pixelRotation;

	/// scale modifier
	Scale scale;

	/// if true this shape checks for collisions against other shapes
	bool checkForCollisions;

	/// layers in which I live
	uint32 layers;

	/// layers to ignore when checking for collisions
	uint32 layersToIgnore;

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
abstract class Shape : ListenerObject
{
	// the rectangle
	RightBox rightBox;
	// Position
	Vector3D position;
	// the entity to which the shape belongs
	SpatialObject owner;
	// colliding shapes list
	VirtualList collidingShapes;
	// layers on which this shape live
	uint32 layers;
	// layers to ignore when checking for collisions
	uint32 layersToIgnore;
	// for debugging purposes
	Wireframe wireframe;
	// flag to know if setup is needed
	uint8 ready;
	// flag to know if has moved
	uint8 moved;
	// flag to know if shape is reacting to collisions
	uint8 enabled;
	// flag to check against other shapes
	uint8 checkForCollisions;
	// flag to allow registration of colliding shapes
	bool registerCollisions;
	// flag to destroy it
	bool destroyMe;

	/// @publicsection
	void constructor(SpatialObject owner, const ShapeSpec* shapeSpec);
	void enterCollision(CollisionData* collisionData);
	void updateCollision(CollisionData* collisionData);
	void exitCollision(CollisionData* collisionData);
	CollisionResult collides(Shape shape);
	bool checkForCollisions();
	SpatialObject getOwner();
	void reset();
	bool isEnabled();
	bool isReady();
	void activeCollisionChecks(bool activate);
	void enable(bool enable);
	void setCheckForCollisions(bool checkForCollisions);
	void setReady(bool ready);
	bool canMoveTowards(Vector3D displacement, fixed_t sizeIncrement);
	fixed_t getCollidingFrictionCoefficient();
	void resolveCollision(const CollisionInformation* collisionInformation, bool registerCollidingShape);
	uint32 getLayers();
	void setLayers(uint32 layers);
	uint32 getLayersToIgnore();
	void setLayersToIgnore(uint32 layersToIgnore);
	void registerCollisions(bool value);
	void show();
	void hide();
	void setVisible(bool value);
	Vector3D getPosition();
	void setPosition(const Vector3D* position);

	RightBox getSurroundingRightBox();
	virtual void setup(uint32 layers, uint32 layersToIgnore);
	virtual void transform(const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size);
	virtual void updateRightBox();
	virtual Vector3D getNormal();
	virtual CollisionInformation testForCollision(Shape shape, Vector3D displacement, fixed_t sizeIncrement) = 0;
	virtual void configureWireframe() = 0;
	virtual void print(int32 x, int32 y) = 0;
}


#endif
