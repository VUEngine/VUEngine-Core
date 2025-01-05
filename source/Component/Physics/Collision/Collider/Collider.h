/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COLLIDER_H_
#define COLLIDER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Component.h>
#include <Wireframe.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Entity;
class Collider;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __COLLIDER_NORMALS 3

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Collider commands
enum ColliderCommands
{
	cColliderComponentLastCommand = cComponentCommandLast + 1,
	cColliderComponentCommandShow,
	cColliderComponentCommandHide,
	cColliderComponentCommandCheckCollisions,
	cColliderComponentCommandRegisterCollisions,
	cColliderComponentCommandSetLayers,
	cColliderComponentCommandSetLayersToIgnore
};

/// Collision events
/// @memberof Collider
typedef enum CollisionResult
{
	kNoCollision = 0,
	kCollisionStarts,
	kCollisionPersists,
	kCollisionEnds,

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
/// @memberof Collider
typedef struct ColliderSpec
{
	/// Component spec
	ComponentSpec componentSpec;

	/// size in pixels
	PixelSize pixelSize;

	/// Displacement modifier
	PixelVector displacement;

	/// rotation modifier
	PixelRotation pixelRotation;

	/// scale modifier
	Scale scale;

	/// If true this collider checks for collisions against other colliders
	bool checkForCollisions;

	/// Layers in which I live
	uint32 layers;

	/// Layers to ignore when checking for collisions
	uint32 layersToIgnore;

} ColliderSpec;

/// A ColliderSpec spec that is stored in ROM
/// @memberof Collider
typedef const ColliderSpec ColliderROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Collider
///
/// Inherits from Component
///
/// Checks collisions against other colliders.
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

	/// Class index to avoid using __GET_CAST when checking for collisions
	uint8 classIndex;

	/// Flag to force the computation of the collider's position
	bool invalidPosition;

	/// @publicsection

	/// Class' constructor
	void constructor(Entity owner, const ColliderSpec* colliderSpec);

	/// Process a Telegram.
	/// @param telegram: Telegram to process
	/// @return True if the Telegram was processed
	override bool handleMessage(Telegram telegram);

	/// Handle a command.
	/// @param command: Command to handle
	/// @param args: Variable arguments list depending on the command to handle
	override void handleCommand(int32 command, va_list args);

	/// Enable the collider for collision checks.
	void enable();

	/// Disable the collider for collision checks.
	void disable();

	/// Set the layers in which this collider lives.
	/// @param layers: Layers in which the collider must live
	void setLayers(uint32 layers);

	/// Retrieve the layers in which this collider lives.
	/// @return Layers in which the collider must live
	uint32 getLayers();

	/// Set the layers in which live colliders to ignore when testing collisions.
	/// @param layersToIgnore: Layers to ignore when checking collisions
	void setLayersToIgnore(uint32 layersToIgnore);

	/// Retrieve the layers in which live colliders to ignore when testing collisions.
	/// @return Layers to ignore when checking collisions
	uint32 getLayersToIgnore();

	/// Make this collider to test collision against other colliders.
	/// @param checkCollisions: It true, this collider checks collision against others
	void checkCollisions(bool checkCollisions);

	/// Keep track of colliding colliders to detect when collisions exit.
	/// @param registerCollisions: If true, colliding colliders are registered
	void registerCollisions(bool registerCollisions);

	/// Check if there is there is a collision with the provided collider.
	/// @param collider: Collider to check collision against to
	CollisionResult collides(Collider collider);

	/// Resolve a collision by moving the owner to a position where the collision ceases.
	/// @param collisionInformation: Information struct about the collision to resolve
	void resolveCollision(const CollisionInformation* collisionInformation);

	/// Check if there is some collider blocking in the provided direction.
	/// @param displacement: Vector towards which to check if it is possible to move the owner
	/// @return True if there is no collision when moving the collider and increasing its size
	bool canMoveTowards(Vector3D displacement);

	/// Discard any registered collision.
	void discardCollisions();

	/// Get the total friction of colliding colliders.
	/// @return The sum of friction coefficients of the colliders colliding's owners
	fixed_t getCollidingFrictionCoefficient();

	/// Show the collider.
	void show();

	/// Hide the collider.
	void hide();

	/// Resize the colliders add the provided increment.
	/// @param sizeDelta: Delta to add to the collider's size
	virtual void resize(fixed_t sizeDelta);

	/// Retrieve the normal to the collider.
	/// @return Normal to the collider
	virtual Vector3D getNormal();

	/// Configure the wireframe used to show the collider.
	virtual void configureWireframe() = 0;

	/// Print collider's state.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	virtual void print(int32 x, int32 y);
}

#endif
