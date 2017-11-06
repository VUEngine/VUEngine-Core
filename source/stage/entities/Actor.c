/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Actor.h>
#include <Clock.h>
#include <MessageDispatcher.h>
#include <CollisionManager.h>
#include <Optics.h>
#include <Screen.h>
#include <Shape.h>
#include <PhysicalWorld.h>
#include <Body.h>
#include <Cuboid.h>
#include <Game.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Actor
 * @extends AnimatedEntity
 * @ingroup stage-entities
 */
__CLASS_DEFINITION(Actor, AnimatedEntity);

__CLASS_FRIEND_DEFINITION(VirtualList);
__CLASS_FRIEND_DEFINITION(VirtualNode);

//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global

void Actor_checkIfMustBounce(Actor this, const CollisionInformation* collisionInformation);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Actor, const ActorDefinition* actorDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(Actor, actorDefinition, id, internalId, name);

// class's constructor
void Actor_constructor(Actor this, const ActorDefinition* actorDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "Actor::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(AnimatedEntity, (AnimatedEntityDefinition*)&actorDefinition->animatedEntityDefinition, id, internalId, name);

	// save definition
	this->actorDefinition = actorDefinition;

	// construct the game state machine
	this->stateMachine = NULL;

	this->body = NULL;
	this->collisionSolver = NULL;
	this->previousRotation = this->transform.localRotation;
}

// class's destructor
void Actor_destructor(Actor this)
{
	ASSERT(this, "Actor::destructor: null this");

	// inform the screen I'm being removed
	Screen_onFocusEntityDeleted(Screen_getInstance(), __SAFE_CAST(Entity, this));

	if(this->body)
	{
		// remove a body
		PhysicalWorld_destroyBody(Game_getPhysicalWorld(Game_getInstance()), this->body);
		this->body = NULL;
	}

	if(this->collisionSolver)
	{
		__DELETE(this->collisionSolver);
		this->collisionSolver = NULL;
	}

	// destroy state machine
	if(this->stateMachine)
	{
		__DELETE(this->stateMachine);
		this->stateMachine = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// set definition
void Actor_setDefinition(Actor this, void* actorDefinition)
{
	ASSERT(this, "Actor::setDefinition: null this");
	ASSERT(actorDefinition, "Actor::setDefinition: null definition");

	// save definition
	this->actorDefinition = actorDefinition;

	__CALL_BASE_METHOD(AnimatedEntity, setDefinition, this, &((ActorDefinition*)actorDefinition)->animatedEntityDefinition);
}

//set class's local position
void Actor_setLocalPosition(Actor this, const VBVec3D* position)
{
	ASSERT(this, "Actor::setLocalPosition: null this");

	VBVec3D displacement = this->transform.localPosition;
	__CALL_BASE_METHOD(AnimatedEntity, setLocalPosition, this, position);

	displacement.x -= this->transform.localPosition.x;
	displacement.y -= this->transform.localPosition.y;
	displacement.z -= this->transform.localPosition.z;

	this->transform.globalPosition.x += displacement.x;
	this->transform.globalPosition.y += displacement.y;
	this->transform.globalPosition.z += displacement.z;

	if(this->body)
	{
		Body_setPosition(this->body, &this->transform.globalPosition, __SAFE_CAST(SpatialObject, this));
	}

	this->invalidateGlobalTransformation = (displacement.x ? __X_AXIS: 0) | (displacement.y ? __Y_AXIS: 0) | (displacement.y ? __Z_AXIS: 0);
}

void Actor_syncWithBody(Actor this)
{
	ASSERT(this, "Actor::syncPositionWithBody: null this");

	__VIRTUAL_CALL(Actor, syncPositionWithBody, this);

	__VIRTUAL_CALL(Actor, syncRotationWithBody, this);
}

void Actor_syncPositionWithBody(Actor this)
{
	ASSERT(this, "Actor::syncPositionWithBody: null this");

	// retrieve the body's displacement
	VBVec3D bodyLastDisplacement = {0, 0, 0};

	if(!Clock_isPaused(Game_getPhysicsClock(Game_getInstance())) && Body_isActive(this->body) && Body_isAwake(this->body))
	{
		bodyLastDisplacement = Body_getLastDisplacement(this->body);
	}

	// modify the global position according to the body's displacement
	VBVec3D globalPosition = this->transform.globalPosition;
	globalPosition.x += bodyLastDisplacement.x;
	globalPosition.y += bodyLastDisplacement.y;
	globalPosition.z += bodyLastDisplacement.z;

	// move the body to the new global position
	// to account for any parenting
	Body_setPosition(this->body, &globalPosition, __SAFE_CAST(SpatialObject, this));

	// sync local position with global position
	VBVec3D localPosition = this->transform.localPosition;
	localPosition.x += bodyLastDisplacement.x;
	localPosition.y += bodyLastDisplacement.y;
	localPosition.z += bodyLastDisplacement.z;

	__CALL_BASE_METHOD(AnimatedEntity, setLocalPosition, this, &localPosition);
}

void Actor_syncRotationWithBody(Actor this)
{
	ASSERT(this, "Actor::syncRotationWithBody: null this");

	if(Body_getMovementOverAllAxis(this->body))
	{
		Velocity velocity = Body_getVelocity(this->body);

		Direction direction =
		{
			__RIGHT, __DOWN, __FAR
		};

		if(0 > velocity.x)
		{
			direction.x = __LEFT;
		}

		if(0 > velocity.y)
		{
			direction.y = __UP;
		}

		if(0 > velocity.z)
		{
			direction.z = __NEAR;
		}

		Entity_setDirection(__SAFE_CAST(Entity, this), direction);
	}
}

// updates the animation attributes
// graphically refresh of characters that are visible
void Actor_transform(Actor this, const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	ASSERT(this, "Actor::transform: null this");

	// apply environment transform
	Container_applyEnvironmentToTransformation(__SAFE_CAST(Container, this), environmentTransform);

	if(this->body)
	{
		Actor_syncWithBody(this);

		u16 bodyMovement = Body_getMovementOverAllAxis(this->body);

		if(bodyMovement)
		{
			this->invalidateGlobalTransformation |= __INVALIDATE_POSITION;
		}

		if(__Z_AXIS & bodyMovement)
		{
			this->invalidateGlobalTransformation |= __INVALIDATE_SCALE;
		}
	}

	// call base
	__CALL_BASE_METHOD(AnimatedEntity, transform, this, environmentTransform, invalidateTransformationFlag);

	this->previousRotation = this->transform.localRotation;
}

void Actor_resume(Actor this)
{
	ASSERT(this, "Actor::resume: null this");

	__CALL_BASE_METHOD(AnimatedEntity, resume, this);

	Actor_syncWithBody(this);
}


// execute character's logic
void Actor_update(Actor this, u32 elapsedTime)
{
	ASSERT(this, "Actor::update: null this");

	// call base
	__CALL_BASE_METHOD(AnimatedEntity, update, this, elapsedTime);

	if(this->stateMachine)
	{
		StateMachine_update(this->stateMachine);
	}
}

// update colliding entities
void Actor_resetCollisionStatus(Actor this, u16 movementAxis)
{
	ASSERT(this, "Actor::updateCollisionStatus: null this");

	if(this->collisionSolver)
	{
		CollisionSolver_resetCollisionStatusOnAxis(this->collisionSolver, movementAxis);
	}
}

// retrieve friction of colliding objects
void Actor_updateSurroundingFriction(Actor this)
{
	ASSERT(this, "Actor::updateSurroundingFriction: null this");
	ASSERT(this->body, "Actor::updateSurroundingFriction: null body");

	PhysicalSpecification* physicalSpecification = this->actorDefinition->animatedEntityDefinition.entityDefinition.physicalSpecification;

	Force totalFriction = physicalSpecification ? (Force){physicalSpecification->friction, physicalSpecification->friction, physicalSpecification->friction} : (Force){0, 0, 0};

	if(this->collisionSolver)
	{
		Force surroundingFriction = CollisionSolver_getSurroundingFriction(this->collisionSolver);
		totalFriction.x += surroundingFriction.x;
		totalFriction.y += surroundingFriction.y;
		totalFriction.z += surroundingFriction.z;
	}

	Body_setFriction(this->body, totalFriction);
}

// whether changed direction in the last cycle or not
int Actor_changedDirection(Actor this, int axis)
{
	ASSERT(this, "Actor::changedDirection: null this");

	switch(axis)
	{
		case __X_AXIS:

			return this->transform.localRotation.x != this->previousRotation.x;
			break;

		case __Y_AXIS:

			return this->transform.localRotation.y != this->previousRotation.y;
			break;

		case __Z_AXIS:

			return this->transform.localRotation.z != this->previousRotation.z;
			break;
	}

	return false;
}

// change direction over axis
void Actor_changeDirectionOnAxis(Actor this, u16 axis)
{
	ASSERT(this, "Actor::changeDirectionOnAxis: null this");

	// save current rotation
	this->previousRotation = this->transform.localRotation;

	Direction direction = Entity_getDirection(__SAFE_CAST(Entity, this));

	if((__X_AXIS & axis))
	{
		if(__RIGHT == direction.x)
		{
			direction.x = __LEFT;
		}
		else
		{
			direction.x = __RIGHT;
		}
	}

	if((__Y_AXIS & axis))
	{
		if(__NEAR == direction.y)
		{
			direction.y = __FAR;
		}
		else
		{
			direction.x = __NEAR;
		}
	}

	if((__Z_AXIS & axis))
	{
		if(__RIGHT == direction.z)
		{
			direction.x = __LEFT;
		}
		else
		{
			direction.x = __RIGHT;
		}
	}

	Entity_setDirection(__SAFE_CAST(Entity, this), direction);
}

// check if gravity must apply to this actor
u16 Actor_getAxisAllowedForMovement(Actor this, const Acceleration* acceleration)
{
	ASSERT(this, "Actor::getAxisAllowedForMovement: null this");

	if(this->collisionSolver)
	{
		VirtualNode node = this->shapes->head;

		u16 axisFreeForMovement = __X_AXIS | __Y_AXIS | __Z_AXIS;

		for(; node; node = node->next)
		{
			axisFreeForMovement &= ~CollisionSolver_getAxisOfFutureCollision(this->collisionSolver, acceleration, __SAFE_CAST(Shape, node->data));
		}

		return axisFreeForMovement;
	}

	return __VIRTUAL_CALL(Actor, getAxisFreeForMovement, this);
}

// retrieve axis free for movement
u16 Actor_getAxisFreeForMovement(Actor this)
{
	ASSERT(this, "Actor::getAxisFreeForMovement: null this");

	u16 movingState = Body_getMovementOverAllAxis(this->body);

	return ((__X_AXIS & ~(__X_AXIS & movingState) )| (__Y_AXIS & ~(__Y_AXIS & movingState)) | (__Z_AXIS & ~(__Z_AXIS & movingState)));
}

bool Actor_processCollision(Actor this, const CollisionInformation* collisionInformation)
{
	ASSERT(this, "Actor::processCollision: null this");
	ASSERT(this->body, "Actor::processCollision: null body");
	ASSERT(collisionInformation->collidingShape, "Actor::processCollision: collidingShapes");

	bool returnValue = false;

	if(this->collisionSolver && collisionInformation->collidingShape)
	{
		CollisionSolver_resolveCollision(this->collisionSolver, collisionInformation);

		VBVec3D bodyLastDisplacement = Body_getLastDisplacement(this->body);

		if(bodyLastDisplacement.x | bodyLastDisplacement.y | bodyLastDisplacement.z)
		{
			Actor_checkIfMustBounce(this, collisionInformation);

			__VIRTUAL_CALL(Actor, updateSurroundingFriction, this);

			returnValue = true;
		}

		__VIRTUAL_CALL(Actor, collisionsProcessingDone, this, collisionInformation);
	}

	return returnValue;
}

// process a telegram
bool Actor_handleMessage(Actor this, Telegram telegram)
{
	ASSERT(this, "Actor::handleMessage: null this");

	if(!this->stateMachine || !StateMachine_handleMessage(this->stateMachine, telegram))
	{
		// retrieve message
		int message = Telegram_getMessage(telegram);

		if(this->body)
		{
			switch(message)
			{
				case kBodyStartedMoving:

					ASSERT(this->shapes, "Actor::handleMessage: null shapes");
					Entity_informShapesThatStartedMoving(__SAFE_CAST(Entity, this));
					Actor_resetCollisionStatus(this, *(int*)Telegram_getExtraInfo(telegram));
					return true;
					break;

				case kBodyStopped:

					if(!Body_getMovementOverAllAxis(this->body))
					{
						ASSERT(this->shapes, "Actor::handleMessage: null shapes");
						Entity_informShapesThatStoppedMoving(__SAFE_CAST(Entity, this));
					}
					break;

				case kBodyBounced:

					Actor_changeDirectionOnAxis(this, *(int*)Telegram_getExtraInfo(telegram));
					return true;
					break;
			}
		}
	}
	return false;
}

// retrieve state machine
StateMachine Actor_getStateMachine(Actor this)
{
	ASSERT(this, "Actor::getStateMachine: null this");

	return this->stateMachine;
}

// stop movement completely
void Actor_stopAllMovement(Actor this)
{
	ASSERT(this, "Actor::stopMovement: null this");

	Actor_stopMovement(this, __X_AXIS | __Y_AXIS | __Z_AXIS);
}

// stop movement completely
void Actor_stopMovement(Actor this, int axis)
{
	ASSERT(this, "Actor::stopMovement: null this");

	if(this->body)
	{
		Body_stopMovement(this->body, axis);
	}
}

void Actor_addForce(Actor this, const Force* force, bool informAboutBodyAwakening)
{
	ASSERT(this, "Actor::addForce: null this");
	ASSERT(this->body, "Actor::addForce: null body");

	Acceleration acceleration =
	{
		force->x,
		force->y,
		force->z
	};

	Velocity velocity = Body_getVelocity(this->body);

	Force effectiveForceToApply =
	{
		velocity.x || (force->x && (__X_AXIS & Actor_getAxisAllowedForMovement(this, &acceleration))) ? force->x : 0,
		velocity.y || (force->y && (__Y_AXIS & Actor_getAxisAllowedForMovement(this, &acceleration))) ? force->y : 0,
		velocity.z || (force->z && (__Z_AXIS & Actor_getAxisAllowedForMovement(this, &acceleration))) ? force->z : 0
	};

	Body_addForce(this->body, &effectiveForceToApply, informAboutBodyAwakening);

	Actor_resetCollisionStatus(this, Body_getMovementOverAllAxis(this->body));
	__VIRTUAL_CALL(Actor, updateSurroundingFriction, this);

	if(this->shapes)
	{
		// register the shape for collision detections
		Entity_informShapesThatStartedMoving(__SAFE_CAST(Entity, this));
	}
}

void Actor_moveUniformly(Actor this, Velocity* velocity)
{
	ASSERT(this, "Actor::moveUniformly: null this");

	// move me with physics
	if(this->body)
	{
		Body_moveUniformly(this->body, *velocity);

		Entity_informShapesThatStartedMoving(__SAFE_CAST(Entity, this));
	}
}

// does it move?
bool Actor_moves(Actor this __attribute__ ((unused)))
{
	ASSERT(this, "Actor::moves: null this");

	return true;
}

// is it moving?
bool Actor_isMoving(Actor this)
{
	ASSERT(this, "Actor::isMoving: null this");

	return this->body ? Body_getMovementOverAllAxis(this->body) : 0;
}

u16 Actor_getMovementState(Actor this)
{
	ASSERT(this, "Actor::getMovementState: null this");

	return Body_getMovementOverAllAxis(this->body);
}

void Actor_changeEnvironment(Actor this, Transformation* environmentTransform)
{
	ASSERT(this, "Actor::changeEnvironment: null this");

	__CALL_BASE_METHOD(AnimatedEntity, changeEnvironment, this, environmentTransform);

	if(this->body)
	{
		Body_setPosition(this->body, &this->transform.globalPosition, __SAFE_CAST(SpatialObject, this));
	}
}

// set position
void Actor_setPosition(Actor this, const VBVec3D* position)
{
	ASSERT(this, "Actor::setPosition: null this");

	VBVec3D displacement = this->transform.globalPosition;
	this->transform.globalPosition = *position;

	displacement.x -= this->transform.globalPosition.x;
	displacement.y -= this->transform.globalPosition.y;
	displacement.z -= this->transform.globalPosition.z;

	this->transform.localPosition.x -= displacement.x;
	this->transform.localPosition.y -= displacement.y;
	this->transform.localPosition.z -= displacement.z;

	if(this->body)
	{
		Body_setPosition(this->body, &this->transform.globalPosition, __SAFE_CAST(SpatialObject, this));
	}

	this->invalidateGlobalTransformation = __INVALIDATE_TRANSFORMATION;
	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;

	Entity_transformShapes(__SAFE_CAST(Entity, this));
}

// retrieve global position
const VBVec3D* Actor_getPosition(Actor this)
{
	ASSERT(this, "Actor::getPosition: null this");

	return this->body ? Body_getPosition(this->body) : __CALL_BASE_METHOD(AnimatedEntity, getPosition, this);
}

int Actor_getAxisAllowedForBouncing(Actor this __attribute__ ((unused)))
{
	ASSERT(this, "Actor::getAxisAllowedForBouncing: null this");

	return __X_AXIS | __Y_AXIS | __Z_AXIS;
}

// start bouncing after collision with another Entity
void Actor_checkIfMustBounce(Actor this, const CollisionInformation* collisionInformation)
{
	ASSERT(this, "Actor::bounce: null this");

/*
	if(axisOfCollision)
	{
		fix19_13 otherSpatialObjectsElasticity = this->collisionSolver ? CollisionSolver_getCollidingTotalElasticity(this->collisionSolver, axisOfCollision) : __1I_FIX19_13;

		int axisAllowedForBouncing = __VIRTUAL_CALL(Actor, getAxisAllowedForBouncing, this);

		Body_bounce(this->body, axisOfCollision, axisAllowedForBouncing, otherSpatialObjectsElasticity);
	}
	*/
}

// take hit
void Actor_takeHitFrom(Actor this, Actor other)
{
	ASSERT(this, "Actor::takeHitFrom: null this");
	ASSERT(__SAFE_CAST(Actor, other), "Actor::takeHitFrom: other is not actor");

	if(other->body)
	{
		Body_takeHitFrom(this->body, other->body);
	}
}

// get elasticity
fix19_13 Actor_getElasticity(Actor this)
{
	ASSERT(this, "Actor::getElasticity: null this");

	PhysicalSpecification* physicalSpecification = this->actorDefinition->animatedEntityDefinition.entityDefinition.physicalSpecification;

	return this->body ? Body_getElasticity(this->body) : physicalSpecification ? physicalSpecification->elasticity : 0;
}

// get velocity
Velocity Actor_getVelocity(Actor this)
{
	ASSERT(this, "Actor::getVelocity: null this");

	return this->body ? Body_getVelocity(this->body) : __CALL_BASE_METHOD(AnimatedEntity, getVelocity, this);
}

void Actor_collisionsProcessingDone(Actor this __attribute__ ((unused)), const CollisionInformation* collisionInformation __attribute__ ((unused)))
{
	ASSERT(this, "Actor::collisionsProcessingDone: null this");
}

