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
#include <Prototypes.h>
#include <Game.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Actor
 * @extends AnimatedInGameEntity
 * @ingroup stage-entities
 */
__CLASS_DEFINITION(Actor, AnimatedInGameEntity);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenDisplacement;

void Actor_checkIfMustBounce(Actor this, int axisOfCollision);
static void Actor_resolveCollisions(Actor this, VirtualList collidingEntities);
static void Actor_resolveCollisionsAgainstMe(Actor this, SpatialObject collidingSpatialObject, VBVec3D* collidingSpatialObjectLastDisplacement);


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
	__CONSTRUCT_BASE(AnimatedInGameEntity, (AnimatedInGameEntityDefinition*)&actorDefinition->animatedInGameEntityDefinition, id, internalId, name);

	// save definition
	this->actorDefinition = actorDefinition;

	// construct the game state machine
	this->stateMachine = NULL;

	this->body = NULL;
	this->collisionSolver = NULL;
}

// class's destructor
void Actor_destructor(Actor this)
{
	ASSERT(this, "Actor::destructor: null this");

	// inform the screen I'm being removed
	Screen_onFocusEntityDeleted(Screen_getInstance(), __SAFE_CAST(InGameEntity, this));

	if(this->body)
	{
		// remove a body
		PhysicalWorld_unregisterBody(Game_getPhysicalWorld(Game_getInstance()), __SAFE_CAST(SpatialObject, this));
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
void Actor_setDefinition(Actor this, ActorDefinition* actorDefinition)
{
	ASSERT(this, "Actor::setDefinition: null this");
	ASSERT(actorDefinition, "Actor::setDefinition: null definition");

	// save definition
	this->actorDefinition = actorDefinition;

	AnimatedInGameEntity_setDefinition(__SAFE_CAST(AnimatedInGameEntity, this), &actorDefinition->animatedInGameEntityDefinition);
}

//set class's local position
void Actor_setLocalPosition(Actor this, const VBVec3D* position)
{
	ASSERT(this, "Actor::setLocalPosition: null this");

	VBVec3D displacement = this->transform.localPosition;
	Container_setLocalPosition(__SAFE_CAST(Container, this), position);

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

	this->invalidateGlobalTransformation = (displacement.x ? __XAXIS: 0) | (displacement.y ? __YAXIS: 0) | (displacement.y ? __ZAXIS: 0);
}

void Actor_syncWithBody(Actor this)
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

	// sync direction with velocity
	if(Body_isMoving(this->body))
	{
		Velocity velocity = Body_getVelocity(this->body);

		if(0 < velocity.x)
		{
			this->direction.x = __RIGHT;
		}
		else if(0 > velocity.x)
		{
			this->direction.x = __LEFT;
		}

		if(0 < velocity.y)
		{
			this->direction.y = __DOWN;
		}
		else if(0 > velocity.y)
		{
			this->direction.y = __UP;
		}

		if(0 < velocity.z)
		{
			this->direction.z = __FAR;
		}
		else if(0 > velocity.z)
		{
			this->direction.z = __NEAR;
		}
	}

	Entity_setLocalPosition(__SAFE_CAST(Entity, this), &localPosition);
}

// updates the animation attributes
// graphically refresh of characters that are visible
void Actor_transform(Actor this, const Transformation* environmentTransform)
{
	ASSERT(this, "Actor::transform: null this");

	// save previous position
	if(this->collisionSolver)
	{
		CollisionSolver_setOwnerPreviousPosition(this->collisionSolver, this->transform.globalPosition);
	}

	// apply environment transform
	Container_applyEnvironmentToTransformation(__SAFE_CAST(Container, this), environmentTransform);

	if(this->body)
	{
		Actor_syncWithBody(this);

		int bodyMovement = Body_isMoving(this->body);

		if(bodyMovement)
		{
			this->invalidateGlobalTransformation |= __INVALIDATE_POSITION;
		}

		if(__ZAXIS & bodyMovement)
		{
			this->invalidateGlobalTransformation |= __INVALIDATE_SCALE;
		}
	}

	// call base
	AnimatedInGameEntity_transform(__SAFE_CAST(AnimatedInGameEntity, this), environmentTransform);
}

void Actor_resume(Actor this)
{
	ASSERT(this, "Actor::resume: null this");

	AnimatedInGameEntity_resume(__SAFE_CAST(AnimatedInGameEntity, this));

	Entity_setSpritesDirection(__SAFE_CAST(Entity, this), __XAXIS, this->direction.x);
	Entity_setSpritesDirection(__SAFE_CAST(Entity, this), __YAXIS, this->direction.y);

	Actor_syncWithBody(this);
}


// execute character's logic
void Actor_update(Actor this, u32 elapsedTime)
{
	ASSERT(this, "Actor::update: null this");

	// call base
	AnimatedInGameEntity_update(__SAFE_CAST(AnimatedInGameEntity, this), elapsedTime);

	if(this->stateMachine)
	{
		StateMachine_update(this->stateMachine);
	}
}

// update colliding entities
void Actor_resetCollisionStatus(Actor this, int movementAxis)
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

	Force totalFriction = {this->actorDefinition->friction, this->actorDefinition->friction, this->actorDefinition->friction};

	if(this->collisionSolver)
	{
		Force surroundingFriction = CollisionSolver_getSurroundingFriction(this->collisionSolver);
		totalFriction.x += surroundingFriction.x;
		totalFriction.y += surroundingFriction.y;
		totalFriction.z += surroundingFriction.z;
	}

	Body_setFriction(this->body, totalFriction);
}

// change direction
void Actor_moveOppositeDirection(Actor this, int axis)
{
	ASSERT(this, "Actor::moveOpositeDirecion: null this");

	switch(axis)
	{
		case __XAXIS:

			this->direction.x *= -1;
			break;

		case __YAXIS:

			this->direction.y *= -1;
			break;

		case __ZAXIS:

			this->direction.z *= -1;
			break;
	}
}

// whether changed direction in the last cycle or not
int Actor_changedDirection(Actor this, int axis)
{
	ASSERT(this, "Actor::changedDirection: null this");

	switch(axis)
	{
		case __XAXIS:

			return this->direction.x != this->previousDirection.x;
			break;

		case __YAXIS:

			return this->direction.y != this->previousDirection.y;
			break;

		case __ZAXIS:

			return this->direction.z != this->previousDirection.z;
			break;
	}

	return false;
}

// change direction over axis
void Actor_changeDirectionOnAxis(Actor this, int axis)
{
	ASSERT(this, "Actor::changeDirectionOnAxis: null this");

	// save current direction
	this->previousDirection = this->direction;

	if((__XAXIS & axis))
	{
		if(__RIGHT == this->direction.x)
		{
			this->direction.x = __LEFT;
		}
		else
		{
			this->direction.x = __RIGHT;
		}
	}

	if((__YAXIS & axis))
	{
		if(__NEAR == this->direction.y)
		{
			this->direction.y = __FAR;
		}
		else
		{
			this->direction.x = __NEAR;
		}
	}

	if((__ZAXIS & axis))
	{
		if(__RIGHT == this->direction.z)
		{
			this->direction.x = __LEFT;
		}
		else
		{
			this->direction.x = __RIGHT;
		}
	}
}

// check if gravity must apply to this actor
int Actor_canMoveOverAxis(Actor this, const Acceleration* acceleration)
{
	ASSERT(this, "Actor::canMoveOverAxis: null this");

	if(this->collisionSolver)
	{
		return ~CollisionSolver_getAxisOfFutureCollision(this->collisionSolver, acceleration, this->shape);
	}

	return __VIRTUAL_CALL(Actor, getAxisFreeForMovement, this);
}

// retrieve axis free for movement
int Actor_getAxisFreeForMovement(Actor this)
{
	ASSERT(this, "Actor::getAxisFreeForMovement: null this");

	int movingState = Body_isMoving(this->body);

	return ((__XAXIS & ~(__XAXIS & movingState) )| (__YAXIS & ~(__YAXIS & movingState)) | (__ZAXIS & ~(__ZAXIS & movingState)));
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
				case kCollision:

					Actor_resolveCollisions(this, __SAFE_CAST(VirtualList, Telegram_getExtraInfo(telegram)));
					return true;
					break;

				case kCollisionWithYou:

					Actor_resolveCollisionsAgainstMe(this, __SAFE_CAST(SpatialObject, Telegram_getSender(telegram)), (VBVec3D*)Telegram_getExtraInfo(telegram));
					return true;
					break;

				case kBodyStartedMoving:

					ASSERT(this->shape, "Actor::handleMessage: null shape");
					CollisionManager_shapeStartedMoving(Game_getCollisionManager(Game_getInstance()), this->shape);
					Actor_resetCollisionStatus(this, *(int*)Telegram_getExtraInfo(telegram));
					return true;
					break;

				case kBodyStopped:

					if(!Body_isMoving(this->body))
					{
						ASSERT(this->shape, "Actor::handleMessage: null shape");
						CollisionManager_shapeStoppedMoving(Game_getCollisionManager(Game_getInstance()), this->shape);
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
void Actor_stopAllMovement(Actor this, u32 stopShape)
{
	ASSERT(this, "Actor::stopMovement: null this");

	Actor_stopMovement(this, __XAXIS | __YAXIS | __ZAXIS, stopShape);
}

// stop movement completely
void Actor_stopMovement(Actor this, int axis, u32 stopShape)
{
	ASSERT(this, "Actor::stopMovement: null this");

	if(this->body)
	{
		Body_stopMovement(this->body, axis);
	}

	if(stopShape && this->shape)
	{
		// unregister the shape for collision detections
		CollisionManager_shapeStoppedMoving(Game_getCollisionManager(Game_getInstance()), this->shape);
	}
}

void Actor_addForce(Actor this, const Force* force)
{
	ASSERT(this, "Actor::Force: null this");
	ASSERT(this->body, "Actor::Force: null body");

	Acceleration acceleration =
	{
		force->x,
		force->y,
		force->z
	};

	Velocity velocity = Body_getVelocity(this->body);

	Force effectiveForceToApply =
	{
		velocity.x || (force->x && (__XAXIS & Actor_canMoveOverAxis(this, &acceleration))) ? force->x : 0,
		velocity.y || (force->y && (__YAXIS & Actor_canMoveOverAxis(this, &acceleration))) ? force->y : 0,
		velocity.z || (force->z && (__ZAXIS & Actor_canMoveOverAxis(this, &acceleration))) ? force->z : 0
	};

	Body_addForce(this->body, &effectiveForceToApply);

	Actor_resetCollisionStatus(this, Body_isMoving(this->body));
	__VIRTUAL_CALL(Actor, updateSurroundingFriction, this);

	if(this->shape)
	{
		// register the shape for collision detections
		Shape_setActive(this->shape, true);
		CollisionManager_shapeStartedMoving(Game_getCollisionManager(Game_getInstance()), this->shape);
	}

}

void Actor_moveUniformly(Actor this, Velocity* velocity)
{
	ASSERT(this, "Actor::moveUniformly: null this");

	// move me with physics
	if(this->body)
	{
		Body_moveUniformly(this->body, *velocity);

		if(this->shape)
		{
			// register the shape for collision detections
			Shape_setActive(this->shape, true);
			CollisionManager_shapeStartedMoving(Game_getCollisionManager(Game_getInstance()), this->shape);
		}
	}
}

// does it move?
bool Actor_moves(Actor this __attribute__ ((unused)))
{
	ASSERT(this, "Actor::moves: null this");

	return true;
}

// is it moving?
int Actor_isMoving(Actor this)
{
	ASSERT(this, "Actor::isMoving: null this");

	return this->body ? Body_isMoving(this->body) : 0;
}

int Actor_getMovementState(Actor this)
{
	ASSERT(this, "Actor::getMovementState: null this");

	return Body_isMoving(this->body);
}

void Actor_changeEnvironment(Actor this, Transformation* environmentTransform)
{
	ASSERT(this, "Actor::changeEnvironment: null this");

	Container_changeEnvironment(__SAFE_CAST(Container, this), environmentTransform);

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
	this->updateSprites = __UPDATE_SPRITE_TRANSFORMATION;

	if(this->shape)
	{
		__VIRTUAL_CALL(Shape, position, this->shape);
	}
}

// retrieve global position
const VBVec3D* Actor_getPosition(Actor this)
{
	ASSERT(this, "Actor::getPosition: null this");

	return this->body ? Body_getPosition(this->body) : Entity_getPosition(__SAFE_CAST(Entity, this));
}

int Actor_getAxisAllowedForBouncing(Actor this __attribute__ ((unused)))
{
	ASSERT(this, "Actor::getAxisAllowedForBouncing: null this");

	return __XAXIS | __YAXIS | __ZAXIS;
}

// start bouncing after collision with another inGameEntity
void Actor_checkIfMustBounce(Actor this, int axisOfCollision)
{
	ASSERT(this, "Actor::bounce: null this");

	if(axisOfCollision)
	{
		fix19_13 otherSpatialObjectsElasticity = this->collisionSolver ? CollisionSolver_getCollidingSpatialObjectsTotalElasticity(this->collisionSolver, axisOfCollision) : __1I_FIX19_13;

		int axisAllowedForBouncing = __VIRTUAL_CALL(Actor, getAxisAllowedForBouncing, this);

		Body_bounce(this->body, axisOfCollision, axisAllowedForBouncing, otherSpatialObjectsElasticity);
	}
}

void Actor_alignTo(Actor this, SpatialObject spatialObject, bool registerObject)
{
	ASSERT(this, "Actor::alignTo: null this");

	int axisOfCollision = __VIRTUAL_CALL(Shape, getAxisOfCollision, this->shape, spatialObject, Body_getLastDisplacement(this->body), *CollisionSolver_getOwnerPreviousPosition(this->collisionSolver));

	if(axisOfCollision)
	{
		CollisionSolver_alignToCollidingSpatialObject(this->collisionSolver, spatialObject, axisOfCollision, registerObject);
	}
}

// resolve collision against other entities
static void Actor_resolveCollisions(Actor this, VirtualList collidingSpatialObjects)
{
	ASSERT(this, "Actor::resolveCollision: null this");
	ASSERT(this->body, "Actor::resolveCollision: null body");
	ASSERT(collidingSpatialObjects, "Actor::resolveCollision: collidingSpatialObjects");

	if(this->collisionSolver)
	{
		VBVec3D bodyLastDisplacement = Body_getLastDisplacement(this->body);

		if(bodyLastDisplacement.x | bodyLastDisplacement.y | bodyLastDisplacement.z)
		{
			int axisOfAllignement = CollisionSolver_resolveCollision(this->collisionSolver, collidingSpatialObjects, bodyLastDisplacement, true);

			Actor_checkIfMustBounce(this, axisOfAllignement);

			__VIRTUAL_CALL(Actor, updateSurroundingFriction, this);
		}
	}

	__VIRTUAL_CALL(Actor, collisionsProcessingDone, this, collidingSpatialObjects);
}

// resolve collision against me entities
static void Actor_resolveCollisionsAgainstMe(Actor this, SpatialObject collidingSpatialObject, VBVec3D* collidingSpatialObjectLastDisplacement)
{
	ASSERT(this, "Actor::resolveCollisionAgainstMe: null this");
	ASSERT(this->body, "Actor::resolveCollisionAgainstMe: null body");
	ASSERT(collidingSpatialObject, "Actor::resolveCollisionAgainstMe: collidingSpatialObject");

	if(this->collisionSolver)
	{
		// TODO: must retrieve the scale of the other object
		VirtualList collidingSpatialObjects = __NEW(VirtualList);
		VirtualList_pushBack(collidingSpatialObjects, collidingSpatialObject);

		VBVec3D fakeLastDisplacement =
		{
			-collidingSpatialObjectLastDisplacement->x,
			-collidingSpatialObjectLastDisplacement->y,
			-collidingSpatialObjectLastDisplacement->z,
		};

		// invent the colliding object's displacement to simulate that it was me
		int axisOfCollision = CollisionSolver_resolveCollision(this->collisionSolver, collidingSpatialObjects, fakeLastDisplacement, true);
		__DELETE(collidingSpatialObjects);

		Actor_checkIfMustBounce(this, axisOfCollision);

		__VIRTUAL_CALL(Actor, updateSurroundingFriction, this);
	}
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

	return this->body ? Body_getElasticity(this->body) : this->actorDefinition->elasticity;
}

// get friction
fix19_13 Actor_getFriction(Actor this)
{
	ASSERT(this, "Actor::getElasticity: null this");

	return this->actorDefinition->friction;
}

// get velocity
Velocity Actor_getVelocity(Actor this)
{
	ASSERT(this, "Actor::getVelocity: null this");

	return this->body ? Body_getVelocity(this->body) : SpatialObject_getVelocity(__SAFE_CAST(SpatialObject, this));
}

void Actor_collisionsProcessingDone(Actor this __attribute__ ((unused)), VirtualList collidingSpatialObjects __attribute__ ((unused)))
{
	ASSERT(this, "Actor::collisionsProcessingDone: null this");
}

