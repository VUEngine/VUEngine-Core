/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COLLIDER_H_
#define COLLIDER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Component.h>
#include <Wireframe.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define __COLLIDER_NORMALS	3


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class SpatialObject;
class Collider;

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
 * @memberof Collider
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
 * @memberof Collider
 */
typedef struct CollisionInformation
{
	// collider detecting the collision
	Collider collider;

	// colliding collider
	Collider otherCollider;

	// information to solve the collision
	SolutionVector solutionVector;

} CollisionInformation;


/**
 * Collision collider registry
 *
 * @memberof Collider
 */
typedef struct OtherColliderRegistry
{
	Collider collider;

	SolutionVector solutionVector;

	fixed_t frictionCoefficient;

	bool isImpenetrable;

} OtherColliderRegistry;

/**
 * Collision data
 *
 * @memberof Collider
 */
typedef struct CollisionData
{
	CollisionResult result;
	CollisionInformation collisionInformation;
	Collider colliderNotCollidingAnymore;
	bool isImpenetrableOtherCollider;

} CollisionData;

/**
 * Normals
 *
 * @memberof Collider
 */
typedef struct Normals
{
	Vector3D vectors[__COLLIDER_NORMALS];
} Normals;

typedef struct VertexProjection
{
	fixed_t min;
	fixed_t max;
} VertexProjection;

// defines a collider
typedef struct ColliderSpec
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

	/// if true this collider checks for collisions against other colliders
	bool checkForCollisions;

	/// layers in which I live
	uint32 layers;

	/// layers to ignore when checking for collisions
	uint32 layersToIgnore;

} ColliderSpec;

typedef const ColliderSpec ColliderROMSpec;


//---------------------------------------------------------------------------------------------------------
//												ENUMS
//---------------------------------------------------------------------------------------------------------

/**
 * Possible types of a Collider
 *
 * @memberof Collider
 */
enum ColliderTypes
{
	kNoCollider = 0,
	kBall,
	kBox,
	kInverseBox,
};

enum ColliderClassIndexes
{
	kColliderBallIndex = 0,
	kColliderBoxIndex,
	kColliderInverseBoxIndex,
	kColliderLineFieldIndex,
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
abstract class Collider : Component
{
	// colliding colliders list
	VirtualList otherColliders;
	// layers on which this collider live
	uint32 layers;
	// layers to ignore when checking for collisions
	uint32 layersToIgnore;
	// for debugging purposes
	Wireframe wireframe;
	// flag to know if setup is needed
	uint8 ready;
	// flag to know if collider is reacting to collisions
	uint8 enabled;
	// flag to check against other colliders
	uint8 checkForCollisions;
	// flag to allow registration of colliding colliders
	bool registerCollisions;
	// flag to destroy it
	bool destroyMe;
	// class index 
	uint8 classIndex;

	/// @publicsection
	void constructor(SpatialObject owner, const ColliderSpec* colliderSpec);
	void enterCollision(CollisionData* collisionData);
	void updateCollision(CollisionData* collisionData);
	void exitCollision(CollisionData* collisionData);
	CollisionResult collides(Collider collider);
	bool checkForCollisions();
	SpatialObject getOwner();
	void reset();
	bool isEnabled();
	bool isReady();
	void checkCollisions(bool activate);
	void enable(bool enable);
	void setCheckForCollisions(bool checkForCollisions);
	void setReady(bool ready);
	bool canMoveTowards(Vector3D displacement, fixed_t sizeIncrement);
	fixed_t getCollidingFrictionCoefficient();
	void resolveCollision(const CollisionInformation* collisionInformation, bool registerOtherCollider);
	uint32 getLayers();
	void setLayers(uint32 layers);
	uint32 getLayersToIgnore();
	void setLayersToIgnore(uint32 layersToIgnore);
	void registerCollisions(bool value);
	void show();
	void hide();
	void setVisible(bool value);

//	virtual void setup(uint32 layers, uint32 layersToIgnore);
	virtual Vector3D getNormal();
	virtual void testForCollision(Collider collider, fixed_t sizeIncrement, CollisionInformation* collisionInformation);
	virtual void configureWireframe() = 0;
	virtual void print(int32 x, int32 y) = 0;
}


#endif
