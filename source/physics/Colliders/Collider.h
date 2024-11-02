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

/// A ColliderSpec spec that is stored in ROM
/// @memberof Collider
typedef const ColliderSpec ColliderROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Collider
///
/// Inherits from Component
///
/// Checks collisions against other colliders.
/// @ingroup physics
abstract class Collider : Component
{
	/// @protectedsection

	/// Displaced position
	Vector3D position;
	
	/// List of colliding colliders
	VirtualList otherColliders;

	/// Layers on which this collider live
	uint32 layers;

	/// Layers to ignore when checking for collisions
	uint32 layersToIgnore;
	
	/// Wireframe to make the collider visible (mainly for debugging purposes)
	Wireframe wireframe;

	/// If false, it is ignored in all callision checks
	bool enabled;

	/// If false, it doesn't check collision against other colliders
	uint8 checkForCollisions;

	/// If true, it registers other colliders when a collision arises
	bool registerCollisions;
	
	/// Flag to destroy the collider
	bool destroyMe;

	/// Class index to avoid using __GET_CAST when checking for collisions
	uint8 classIndex;

	/// Flag to force the computation of the collider's position
	bool invalidPosition;

	/// @publicsection
	void constructor(SpatialObject owner, const ColliderSpec* colliderSpec);

	void reset();

	/// 
	CollisionResult collides(Collider collider);
	void resolveCollision(const CollisionInformation* collisionInformation, bool registerOtherCollider);
	bool canMoveTowards(Vector3D displacement, fixed_t sizeIncrement);

	SpatialObject getOwner();
	fixed_t getCollidingFrictionCoefficient();

	void enable(bool enable);
	bool isEnabled();

	void checkCollisions(bool activate);
	void registerCollisions(bool value);

	void setLayers(uint32 layers);
	uint32 getLayers();

	void setLayersToIgnore(uint32 layersToIgnore);
	uint32 getLayersToIgnore();

	void show();
	void hide();

	virtual void testForCollision(Collider collider, fixed_t sizeIncrement, CollisionInformation* collisionInformation);
	virtual Vector3D getNormal();
	virtual void configureWireframe() = 0;
	virtual void print(int32 x, int32 y) = 0;
	
	override bool handleMessage(Telegram telegram);
}


#endif
