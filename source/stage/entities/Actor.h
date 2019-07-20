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
	u16 axisSubjectToGravity;

	/// axis around which to rotate the entity when syncronizing with body
	u16 axisForSynchronizationWithBody;

} ActorSpec;

typedef const ActorSpec ActorROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage-entities
class Actor : AnimatedEntity
{
	// spec
	const ActorSpec* actorSpec;
	// a state machine to handle entity's logic
	StateMachine stateMachine;
	// a physical body
	Body body;
	// previous velocity
	Rotation previousRotation;

	/// @publicsection
	void constructor(const ActorSpec* actorSpec, s16 id, s16 internalId, const char* const name);
	bool hasChangedDirection(u16 axis);
	void changeDirectionOnAxis(u16 axis);
	bool isInsideGame();
	StateMachine getStateMachine();
	void moveUniformly(Velocity* velocity);
	void stopAllMovement();
	void resetCollisionStatus();
	Body getBody();
	virtual void addForce(const Force* force, bool checkIfCanMove);
	virtual bool canMoveTowards(Vector3D direction);
	virtual void stopMovement(u16 axis);
	virtual void takeHitFrom(Actor other);
	virtual void syncPositionWithBody();
	virtual void syncRotationWithBody();
	virtual void syncRotationWithBodyAfterBouncing(SpatialObject collidingObject);
	virtual fix10_6 getBouncinessOnCollision(SpatialObject collidingObject, const Vector3D* collidingObjectNormal);
	virtual fix10_6 getFrictionOnCollision(SpatialObject collidingObject, const Vector3D* collidingObjectNormal);
	virtual fix10_6 getSurroundingFrictionCoefficient();
	virtual bool mustBounce();
	virtual bool overrideParentingPositioningWhenBodyIsNotMoving();
	fix10_6_ext getSpeedSquare();
	override void iAmDeletingMyself();
	override void update(u32 elapsedTime);
	override void transform(const Transformation* environmentTransform, u8 invalidateTransformationFlag);
	override void initialTransform(const Transformation* environmentTransform, u32 recursive);
	override void resume();
	override bool handleMessage(Telegram telegram);
	override bool isMoving();
	override u16 getMovementState();
	override void setLocalPosition(const Vector3D* position);
	override fix10_6 getBounciness();
	override const Vector3D* getPosition();
	override void setPosition(const Vector3D* position);
	override bool isSubjectToGravity(Acceleration gravity);
	override Velocity getVelocity();
	override fix10_6 getSpeed();
	override fix10_6 getMaximumSpeed();
	override void exitCollision(Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	override void collidingShapeOwnerDestroyed(Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	override void changeEnvironment(Transformation* environmentTransform);
	override void setSpec(void* actorSpec);
	override bool enterCollision(const CollisionInformation* collisionInformation);
}


#endif
