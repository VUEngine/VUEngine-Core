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
#include <Body.h>
#include <StateMachine.h>
#include <Clock.h>


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

/// @ingroup stage-entities
class Actor : AnimatedEntity
{
	// a state machine to handle entity's logic
	StateMachine stateMachine;
	// a physical body
	Body body;

	/// @publicsection
	void constructor(const ActorSpec* actorSpec, int16 internalId, const char* const name);
	void createBody(const PhysicalSpecification* physicalSpecification, uint16 axisSubjectToGravity);
	void initializeStateMachine(State state);
	bool hasChangedDirection(uint16 axis, const Rotation* previousRotation);
	void changeDirectionOnAxis(uint16 axis);
	bool isInsideGame();
	StateMachine getStateMachine();
	void moveUniformly(Vector3D* velocity);
	void stopAllMovement();
	void resetCollisionStatus();
	Body getBody();
	void takeHitFrom(Actor other);
	bool isMoving();
	uint16 getMovementState();
	virtual bool applyForce(const Vector3D* force, bool checkIfCanMove);
	virtual bool canMoveTowards(Vector3D direction);
	virtual void stopMovement(uint16 axis);
	virtual void syncPositionWithBody();
	virtual void syncRotationWithBody();
	virtual void syncRotationWithBodyAfterBouncing(SpatialObject collidingObject);
	virtual fixed_t getBouncinessOnCollision(SpatialObject collidingObject, const Vector3D* collidingObjectNormal);
	virtual fixed_t getFrictionOnCollision(SpatialObject collidingObject, const Vector3D* collidingObjectNormal);
	virtual fixed_t getSurroundingFrictionCoefficient();
	virtual bool mustBounce();
	virtual bool registerCollidingShapes();
	virtual fixed_t getMaximumSpeed();
	override void iAmDeletingMyself();
	override void update();
	override void transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag);
	override void initialTransform(const Transformation* environmentTransform, uint32 createComponents);
	override void resume();
	override bool handleMessage(Telegram telegram);
	override void setLocalPosition(const Vector3D* position);
	override fixed_t getBounciness();
	override const Vector3D* getPosition();
	override void setPosition(const Vector3D* position);
	override bool isSubjectToGravity(Vector3D gravity);
	override const Vector3D* getVelocity();
	override fixed_t getSpeed();
	override void exitCollision(Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	override void collidingShapeOwnerDestroyed(Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	override void changeEnvironment(Transformation* environmentTransform);
	override bool enterCollision(const CollisionInformation* collisionInformation);
}


#endif
