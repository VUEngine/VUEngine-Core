/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ACTOR_H_
#define ACTOR_H_

//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <AnimatedEntity.h>

//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class Body;
class State;
class StateMachine;

//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// An Actor Spec
/// @memberof Actor
typedef struct ActorSpec
{
	/// AnimatedEntity spec
	AnimatedEntitySpec animatedEntitySpec;

	/// Flag to attach a physical body
	bool createBody;

	/// Axises around which to rotate the entity when syncronizing with body
	uint16 axisForSynchronizationWithBody;

} ActorSpec;

/// An Actor spec that is stored in ROM
/// @memberof Actor
typedef const ActorSpec ActorROMSpec;

//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Actor
///
/// Inherits from AnimatedEntity
///
/// Implements an animated entity that can have complex behavior and physical simulations.
class Actor : AnimatedEntity
{
	/// @protectedsection

	/// State machine to handle complex logic
	StateMachine stateMachine;

	/// Body for physics simulations
	Body body;

	/// @publicsection

	/// Class' constructor
	/// @param actorSpec: Specification that determines how to configure the actor
	/// @param internalId: ID to keep track internally of the new instance
	/// @param name: Name to assign to the new instance
	void constructor(const ActorSpec* actorSpec, int16 internalId, const char* const name);

	/// Process a Telegram.
	/// @param telegram: Telegram to process
	/// @return True if the Telegram was processed
	override bool handleMessage(Telegram telegram);

	/// A new component has been added to this actor.
	/// @param component: Added component
	override void addedComponent(Component component);

	/// A component has been removed from this actor.
	/// @param component: Removed component
	override void removedComponent(Component component);

	/// Retrieve the object's velocity vector.
	/// @return Pointer to the direction towards which the object is moving
	override const Vector3D* getVelocity();

	/// Retrieve the object's current speed (velocity vector's magnitude).
	/// @return Object's current speed (velocity vector's magnitude)
	override fixed_t getSpeed();

	/// Retrieve the object's bounciness factor.
	/// @return Object's bounciness factor
	override fixed_t getBounciness();

	/// Set the container's position.
	/// @param position: 3D vector defining the object's new position
	override void setPosition(const Vector3D* position);

	/// Set the direction towards which the object must move.
	/// @param direction: Pointer to a direction vector
	override void setDirection(const Vector3D* direction);

	/// Retrieve the direction towards which the object is moving.
	/// @return Pointer to the direction towards which the object is moving
	override const Vector3D* getDirection();

	/// Check if the object is subject to provided gravity vector.
	/// @return True if the provided gravity vector can affect the object; false otherwise
	override bool isSubjectToGravity(Vector3D gravity);

	/// Process a newly detected collision by one of the component colliders.
	/// @param collisionInformation: Information struct about the collision to resolve
	/// @return True if the collider must keep track of the collision to detect if it persists and when it
	/// ends; false otherwise
	override bool collisionStarts(const CollisionInformation* collisionInformation);

	/// Process when a previously detected collision by one of the component colliders stops.
	/// @param collisionInformation: Information struct about the collision to resolve
	override void collisionEnds(const CollisionInformation* collisionInformation);

	/// Set the local position.
	/// @param position: New local position
	override void setLocalPosition(const Vector3D* position);

	/// Update the local transformation in function of the provided environment transform.
	/// @param environmentTransform: New reference environment for the local transformation
	override void changeEnvironment(Transformation* environmentTransform);

	/// Update this instance's logic.
	override void update();

	/// Create the state machine and inintialize it with the provided state.
	/// @param state: State that the state machine must enter
	void createStateMachine(State state);

	/// Retrieve the actor's physical body.
	/// @return Actor's physical body
	Body getBody();

	/// Check if the actor is moving.
	/// @return True if the actor's body is moving; false otherwise
	bool isMoving();

	/// Stop all actor's movement.
	void stopAllMovement();

	/// Stop the actor's movement in the specified axis.
	/// @param axis: Axis on which to stop the movement of the actor's body
	void stopMovement(uint16 axis);

	/// Retrieve the actor's maximum speed.
	/// @return Maximum speed at which the actor's body is allowed to move
	fixed_t getMaximumSpeed();

	/// Set the actor's velocity vector.
	/// @param velocity: Velocity vector to assign to the actor's body
	/// @param checkIfCanMove: If true, the actor checks that none of its colliders will
	/// enter a collision if it were to move in the direction of the provided velocity
	/// @return True if the actor started to move in the direction specified by the
	/// provided velocity vector
	bool setVelocity(const Vector3D* velocity, bool checkIfCanMove);

	/// Apply a force to the actor's body.
	/// @param force: Force to be applied
	/// @param checkIfCanMove: If true, the actor checks that none of its colliders will
	/// @return True if the force was succesfully applied to the actor's body
	virtual bool applyForce(const Vector3D* force, bool checkIfCanMove);

	/// Check if the actor will enter a collision if it were to move in the provided direction
	/// @param direction: Direction vector to check
	virtual bool canMoveTowards(Vector3D direction);

	/// Check if the actor bounces when it collides with another object.
	/// @return True if the actor bounces when it collides with another object
	virtual bool isBouncy();

	/// Check if when the actor bounces it has to take into account the colliding object's bounciness.
	/// @return True if the actor has to take into account the colliding object's bounciness when bouncing
	virtual bool isSensibleToCollidingObjectBouncinessOnCollision(SpatialObject collidingObject);

	/// Check if when the actor bounces it has to take into account the colliding object's friction
	/// coefficient.
	/// @return True if the actor has to take into account the colliding object's friction coefficient when
	/// bouncing
	virtual bool isSensibleToCollidingObjectFrictionOnCollision(SpatialObject collidingObject);
}

#endif
