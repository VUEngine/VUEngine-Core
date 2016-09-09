/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
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
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(Actor, AnimatedInGameEntity);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenDisplacement;

void Actor_checkIfMustBounce(Actor this, int axisOfCollision);
static void Actor_resolveCollisions(Actor this, VirtualList collidingEntities);
static void Actor_resolveCollisionsAgainstMe(Actor this, SpatialObject collidingSpatialObject, VBVec3D* collidingSpatialObjectLastDisplacement);



//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Actor, const ActorDefinition* actorDefinition, s16 id, const char* const name)
__CLASS_NEW_END(Actor, actorDefinition, id, name);

// class's constructor
void Actor_constructor(Actor this, const ActorDefinition* actorDefinition, s16 id, const char* const name)
{
	ASSERT(this, "Actor::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(AnimatedInGameEntity, (AnimatedInGameEntityDefinition*)&actorDefinition->animatedInGameEntityDefinition, id, name);

	// save definition
	this->actorDefinition = actorDefinition;

	// construct the game state machine
	this->stateMachine = __NEW(StateMachine, this);

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
	__DELETE(this->stateMachine);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
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

	this->invalidateGlobalPosition = (displacement.x ? __XAXIS: 0) | (displacement.y ? __YAXIS: 0) | (displacement.y ? __ZAXIS: 0);
}

void Actor_syncWithBody(Actor this)
{
	ASSERT(this, "Actor::syncPositionWithBody: null this");

	// retrieve the body's displacement
	VBVec3D bodyLastDisplacement = {0, 0, 0};

	if(!Clock_isPaused(Game_getPhysicsClock(Game_getInstance())) && Body_isActive(this->body))
	{
		bodyLastDisplacement = Body_getLastDisplacement(this->body);
	}

	// modify the global position accorging to the body's displacement
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
	Container_applyEnvironmentToTranformation(__SAFE_CAST(Container, this), environmentTransform);

	if(this->body)
	{
		Actor_syncWithBody(this);
    }

	// call base
	AnimatedInGameEntity_transform(__SAFE_CAST(AnimatedInGameEntity, this), environmentTransform);
/*
  	if(this->shape)
	{
		__VIRTUAL_CALL(Shape, draw, this->shape);
	}
*/
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
void Actor_update(Actor this)
{
	ASSERT(this, "Actor::update: null this");

	// call base
	AnimatedInGameEntity_update(__SAFE_CAST(AnimatedInGameEntity, this));

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
void Actor_moveOpositeDirecion(Actor this, int axis)
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

	if(!StateMachine_handleMessage(this->stateMachine, telegram))
	{
		// retrieve message
		int message = Telegram_getMessage(telegram);

		if(this->body)
	    {
			Object sender = Telegram_getSender(telegram);
			Actor otherActor = __SAFE_CAST(Actor, sender);

			if(true || (sender == __SAFE_CAST(Object, this)) || __SAFE_CAST(Cuboid, sender) || __SAFE_CAST(Body, sender))
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
			else if(otherActor)
	        {
				__VIRTUAL_CALL(Actor, takeHitFrom, this, otherActor);

				return true;
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

// does it moves?
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

    this->invalidateGlobalPosition = true;
    this->updateSprites = __UPDATE_SPRITE_TRANSFORMATIONS | __UPDATE_SPRITE_POSITION;
}

// retrieve global position
const VBVec3D* Actor_getPosition(Actor this)
{
	ASSERT(this, "Actor::getPosition: null this");

	return this->body? Body_getPosition(this->body) : Entity_getPosition(__SAFE_CAST(Entity, this));
}

// check if necessary to update sprite's position
bool Actor_updateSpritePosition(Actor this)
{
	ASSERT(this, "Actor::updateSpritePosition: null this");

	return (
	    this->invalidateGlobalPosition ||
	    Actor_isMoving(this) ||
	    _screenDisplacement->x ||
	    _screenDisplacement->y ||
	    _screenDisplacement->z
	);
}

// check if necessary to update sprite's scale
bool Actor_updateSpriteTransformations(Actor this)
{
	ASSERT(this, "Actor::updateSpriteTransformations: null this");

	if(this->body && Body_isAwake(this->body) &&  Body_getVelocity(this->body).z)
	{
		return true;
	}

	return Entity_updateSpriteTransformations(__SAFE_CAST(Entity, this));
}

// stop movement completelty
void Actor_stopMovement(Actor this)
{
	ASSERT(this, "Actor::stopMovement: null this");

	if(this->body)
	{
		Body_stopMovement(this->body, __XAXIS);
		Body_stopMovement(this->body, __YAXIS);
		Body_stopMovement(this->body, __ZAXIS);
	}
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
		fix19_13 otherSpatialObjectsElasticity = this->collisionSolver? CollisionSolver_getCollisingSpatialObjectsTotalElasticity(this->collisionSolver, axisOfCollision): ITOFIX19_13(1);

		int axisAllowedForBouncing = __VIRTUAL_CALL(Actor, getAxisAllowedForBouncing, this);

		Body_bounce(this->body, axisOfCollision, axisAllowedForBouncing, otherSpatialObjectsElasticity);

		if(!(axisOfCollision & Body_isMoving(this->body)))
	    {
			MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kBodyStopped, &axisOfCollision);
		}
		else
	    {
			MessageDispatcher_dispatchMessage(0, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kBodyBounced, &axisOfCollision);
		}
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

		if(bodyLastDisplacement.x || bodyLastDisplacement.y || bodyLastDisplacement.z)
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

// get elasticiy
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
}

// get velocity
Velocity Actor_getVelocity(Actor this)
{
	ASSERT(this, "Actor::getVelocity: null this");

	return this->body? Body_getVelocity(this->body) : SpatialObject_getVelocity(__SAFE_CAST(SpatialObject, this));
}

void Actor_collisionsProcessingDone(Actor this __attribute__ ((unused)), VirtualList collidingSpatialObjects __attribute__ ((unused)))
{
	ASSERT(this, "Actor::collisionsProcessingDone: null this");
}

