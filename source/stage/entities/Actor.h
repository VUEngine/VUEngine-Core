/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ACTOR_H_
#define ACTOR_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <AnimatedEntity.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

typedef struct ActorSpec
{
	/// it has an Entity at the beginning
	AnimatedEntitySpec animatedEntitySpec;

	/// true to create a body
	bool createBody;

	/// axis subject to gravity
	uint16 axisSubjectToGravity;

	/// axis around which to rotate the entity when syncronizing with body
	uint16 axisForSynchronizationWithBody;

} ActorSpec;

typedef const ActorSpec ActorROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class Body;
class State;
class StateMachine;

/// @ingroup stage-entities
class Actor : AnimatedEntity
{
	// a state machine to handle entity's logic
	StateMachine stateMachine;
	// a physical body
	Body body;

	/// @publicsection
	void constructor(const ActorSpec* actorSpec, int16 internalId, const char* const name);
	void createBody(const PhysicalProperties* physicalProperties, uint16 axisSubjectToGravity);
	void initializeStateMachine(State state);
	bool hasChangedDirection(uint16 axis, const Rotation* previousRotation);
	void changeDirectionOnAxis(uint16 axis);
	bool isInsideGame();
	StateMachine getStateMachine();
	void stopAllMovement();
	void resetCollisionStatus();
	Body getBody();
	bool isMoving();
	uint16 getMovementState();
	bool setVelocity(const Vector3D* velocity, bool checkIfCanMove);
	virtual bool applyForce(const Vector3D* force, bool checkIfCanMove);
	virtual bool canMoveTowards(Vector3D direction);
	virtual void stopMovement(uint16 axis);
	virtual fixed_t getBouncinessOnCollision(SpatialObject collidingObject, const Vector3D* collidingObjectNormal);
	virtual fixed_t getFrictionOnCollision(SpatialObject collidingObject, const Vector3D* collidingObjectNormal);
	virtual fixed_t getSurroundingFrictionCoefficient();
	virtual bool mustBounce();
	virtual fixed_t getMaximumSpeed();
	override void update();
	override bool handleMessage(Telegram telegram);
	override void setPosition(const Vector3D* position);
	override void setLocalPosition(const Vector3D* position);
	override void setDirection(const Vector3D* direction);
	override fixed_t getBounciness();
	override bool isSubjectToGravity(Vector3D gravity);
	override const Vector3D* getVelocity();
	override const Vector3D* getDirection();
	override fixed_t getSpeed();
	override void changeEnvironment(Transformation* environmentTransform);
	override bool collisionStarts(const CollisionInformation* collisionInformation);
	override void collisionEnds(const CollisionInformation* collisionInformation);
	override void destroyComponents();
}


#endif
