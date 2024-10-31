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


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Component.h>
#include <Wireframe.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class SpatialObject;
class Collider;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __COLLIDER_NORMALS	3


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// Collision events
/// @memberof Collider
typedef enum CollisionResult
{
	kNoCollision = 0,
	kEnterCollision,
	kUpdateCollision,
	kExitCollision,

} CollisionResult;

/// Possible types of a colliders
/// @memberof Collider
enum ColliderClassIndexes
{
	kColliderBallIndex = 0,
	kColliderBoxIndex,
	kColliderInverseBoxIndex,
	kColliderLineFieldIndex,
};

/// Collision solution vector struct
/// @memberof Collider
typedef struct SolutionVector
{
	/// Direction of vector to solve the collision
	Vector3D direction;

	// Minimum distance to solve the collision
	fixed_t magnitude;

} SolutionVector;

/// Collision information struct
/// @memberof Collider
typedef struct CollisionInformation
{
	/// Collider detecting the collision
	Collider collider;

	/// Colliding collider
	Collider otherCollider;

	/// Vector to solve the collision
	SolutionVector solutionVector;

	/// Raised when the collider solves the collision
	bool isImpenetrable;

} CollisionInformation;


/// Registry to keep track of collisions
/// @memberof Collider
typedef struct OtherColliderRegistry
{
	/// Colliding collider
	Collider collider;

	/// Collision solution vector
	SolutionVector solutionVector;

	/// Friction coeficient of the colliding collider's owner
	fixed_t frictionCoefficient;

	/// Raised when the collider solves the collision
	bool isImpenetrable;

} OtherColliderRegistry;

/// Collision struct
/// @memberof Collider
typedef struct Collision
{
	CollisionResult result;
	CollisionInformation collisionInformation;

} Collision;

/// Normals
/// @memberof Collider
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



//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup physics
abstract class Collider : Component
{
	// Displaced position
	Vector3D position;
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
	bool enabled;
	// flag to check against other colliders
	uint8 checkForCollisions;
	// flag to allow registration of colliding colliders
	bool registerCollisions;
	// flag to destroy it
	bool destroyMe;
	// class index 
	uint8 classIndex;
	// flag to compute the displaced position
	bool dirty;

	/// @publicsection
	void constructor(SpatialObject owner, const ColliderSpec* colliderSpec);
	void enterCollision(Collision* collision);
	void updateCollision(Collision* collision);
	void exitCollision(Collision* collision);
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
	override bool handleMessage(Telegram telegram);
}


#endif
