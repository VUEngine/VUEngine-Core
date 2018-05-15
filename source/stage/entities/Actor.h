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
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods

typedef struct ActorDefinition
{
	/// it has an Entity at the beginning
	AnimatedEntityDefinition animatedEntityDefinition;

	/// true to create a body
	bool createBody;

	// axes subject to gravity
	u16 axesSubjectToGravity;

} ActorDefinition;

typedef const ActorDefinition ActorROMDef;


class Actor : AnimatedEntity
{
	/* definition */
	const ActorDefinition* actorDefinition;
	/* a state machine to handle entity's logic	*/
	StateMachine stateMachine;
	/* a physical body	*/
	Body body;
	/* previous velocity */
	Rotation previousRotation;

	void constructor(Actor this, const ActorDefinition* actorDefinition, s16 id, s16 internalId, const char* const name);
	bool hasChangedDirection(Actor this, u16 axis);
	void changeDirectionOnAxis(Actor this, u16 axis);
	bool isInsideGame(Actor this);
	bool canMoveTowards(Actor this, Vector3D direction);
	StateMachine getStateMachine(Actor this);
	void addForce(Actor this, const Force* force);
	void moveUniformly(Actor this, Velocity* velocity);
	void stopAllMovement(Actor this);
	void stopMovement(Actor this, u16 axis);
	void resetCollisionStatus(Actor this);
	Body getBody(Actor this);
	virtual void takeHitFrom(Actor this, Actor other);
	virtual void syncPositionWithBody(Actor this);
	virtual void syncRotationWithBody(Actor this);
	virtual fix10_6 getBouncinessOnCollision(Actor this, SpatialObject collidingObject, const Vector3D* collidingObjectNormal);
	virtual fix10_6 getFrictionOnCollision(Actor this, SpatialObject collidingObject, const Vector3D* collidingObjectNormal);
	virtual fix10_6 getSurroundingFrictionCoefficient(Actor this);
	virtual bool mustBounce(Actor this);
	override void iAmDeletingMyself(Actor this);
	override void update(Actor this, u32 elapsedTime);
	override void transform(Actor this, const Transformation* environmentTransform, u8 invalidateTransformationFlag);
	override void initialTransform(Actor this, const Transformation* environmentTransform, u32 recursive);
	override void resume(Actor this);
	override bool handleMessage(Actor this, Telegram telegram);
	override bool isMoving(Actor this);
	override u16 getMovementState(Actor this);
	override void setLocalPosition(Actor this, const Vector3D* position);
	override fix10_6 getBounciness(Actor this);
	override const Vector3D* getPosition(Actor this);
	override void setPosition(Actor this, const Vector3D* position);
	override bool isSubjectToGravity(Actor this, Acceleration gravity);
	override Velocity getVelocity(Actor this);
	override void exitCollision(Actor this, Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	override void collidingShapeOwnerDestroyed(Actor this, Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	override void changeEnvironment(Actor this, Transformation* environmentTransform);
	override void setDefinition(Actor this, void* actorDefinition);
	override bool enterCollision(Actor this, const CollisionInformation* collisionInformation);
}


#endif
