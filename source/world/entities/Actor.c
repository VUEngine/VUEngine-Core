/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
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
const extern VBVec3D* _screenDisplacement;

static void Actor_checkIfMustBounce(Actor this, u8 axisOfCollision);
static void Actor_resolveCollision(Actor this, VirtualList collidingEntities);
static void Actor_resolveCollisionAgainstMe(Actor this, SpatialObject collidingSpatialObject, VBVec3D* collidingSpatialObjectLastDisplacement);
static void Actor_updateSourroundingFriction(Actor this);
static void Actor_resetCollisionStatus(Actor this, u8 movementAxis);

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
	__CONSTRUCT_BASE((AnimatedInGameEntityDefinition*)&actorDefinition->animatedInGameEntityDefinition, id, name);

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
	Screen_onFocusEntityDeleted(Screen_getInstance(), __GET_CAST(InGameEntity, this));

	if(this->body)
	{
		// remove a body
		PhysicalWorld_unregisterBody(PhysicalWorld_getInstance(), __GET_CAST(SpatialObject, this));
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
	__DESTROY_BASE;
}

//set class's local position
void Actor_setLocalPosition(Actor this, const VBVec3D* position)
{
	ASSERT(this, "Actor::setLocalPosition: null this");

	Container_setLocalPosition(__GET_CAST(Container, this), position);

	if(this->body)
    {
		VBVec3D globalPosition = *Container_getGlobalPosition(__GET_CAST(Container, this));

		Transformation environmentTransform =
        {
				// local position
				{0, 0, 0},
				// global position
				{0, 0, 0},
				// local rotation
				{0, 0, 0},
				// global rotation
				{0, 0, 0},
				// local scale
				{ITOFIX7_9(1), ITOFIX7_9(1)},
				// global scale
				{ITOFIX7_9(1), ITOFIX7_9(1)}
		};

		if(this->parent)
        {
			environmentTransform = Container_getEnvironmentTransform(this->parent);
			globalPosition.x = environmentTransform.globalPosition.x;
			globalPosition.y = environmentTransform.globalPosition.y;
			globalPosition.z = environmentTransform.globalPosition.z;
		}

		globalPosition.x += position->x;
		globalPosition.y += position->y;
		globalPosition.z += position->z;

		Actor_resetCollisionStatus(this, __XAXIS | __YAXIS | __ZAXIS);

		Body_setPosition(this->body, &globalPosition, __GET_CAST(SpatialObject, this));
		
		if(this->shape)
		{
			__VIRTUAL_CALL(void, Shape, positione, this->shape);
		}
	}
}

static void Actor_syncPositionWithBody(Actor this)
{
	// save previous position
	if(this->collisionSolver)
	{
		CollisionSolver_setOwnerPreviousPosition(this->collisionSolver, this->transform.globalPosition);
	}

	Container_setLocalPosition(__GET_CAST(Container, this), Body_getPosition(this->body));
}

// updates the animation attributes
// graphically refresh of characters that are visible
void Actor_transform(Actor this, const Transformation* environmentTransform)
{
	ASSERT(this, "Actor::transform: null this");

	if(this->body && Body_isAwake(this->body))
    {
		Actor_syncPositionWithBody(this);
		
		// an Actor with a physical body is agnostic to parenting
		Transformation environmentAgnosticTransform =
	    {
				// local position
				{0, 0, 0},
				// global position
				{0, 0, 0},
				// local ,rotation
				{0, 0, 0},
				// global rotation
				{0, 0, 0},
				// local scale
				{environmentTransform->localScale.x, environmentTransform->localScale.y},
				// global scale
				{environmentTransform->globalScale.x, environmentTransform->globalScale.y},
		};

		// call base
		AnimatedInGameEntity_transform(__GET_CAST(AnimatedInGameEntity, this), &environmentAgnosticTransform);
	}
	else
	{
		// call base
		AnimatedInGameEntity_transform(__GET_CAST(AnimatedInGameEntity, this), environmentTransform);
	}
}

// execute character's logic
void Actor_update(Actor this)
{
	ASSERT(this, "Actor::update: null this");

	// call base
	AnimatedInGameEntity_update(__GET_CAST(AnimatedInGameEntity, this));

	if(this->stateMachine)
	{
		StateMachine_update(this->stateMachine);
	}
}

// update colliding entities
static void Actor_resetCollisionStatus(Actor this, u8 movementAxis)
{
	ASSERT(this, "Actor::updateCollisionStatus: null this");

	if(this->collisionSolver)
	{
		CollisionSolver_resetCollisionStatusOnAxis(this->collisionSolver, movementAxis);
	}
}

// retrieve friction of colliding objects
static void Actor_updateSourroundingFriction(Actor this)
{
	ASSERT(this, "Actor::updateSourroundingFriction: null this");
	ASSERT(this->body, "Actor::updateSourroundingFriction: null body");

	Force totalFriction = {this->actorDefinition->friction, this->actorDefinition->friction, this->actorDefinition->friction};
	
	if(this->collisionSolver)
	{
		Force sourroundingFriction = CollisionSolver_getSourroundingFriction(this->collisionSolver);
		totalFriction.x += sourroundingFriction.x;
		totalFriction.y += sourroundingFriction.y;
		totalFriction.z += sourroundingFriction.z;
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

			return this->direction.x != this->previousDirection.x;
			break;

		case __ZAXIS:

			return this->direction.x != this->previousDirection.x;
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
u8 Actor_canMoveOverAxis(Actor this, const Acceleration* acceleration)
{
	ASSERT(this, "Actor::canMoveOverAxis: null this");

	u8 axisFreeForMovement = __VIRTUAL_CALL(bool, Actor, getAxisFreeForMovement, this);

	if(this->collisionSolver)
	{
		return axisFreeForMovement & ~CollisionSolver_getAxisOfFutureCollision(this->collisionSolver, acceleration, this->shape);
	}

	return axisFreeForMovement;
}

// retrieve axis free for movement
u8 Actor_getAxisFreeForMovement(Actor this)
{
	ASSERT(this, "Actor::getAxisFreeForMovement: null this");

	u8 movingState = Body_isMoving(this->body);
	
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
			Actor otherActor = __GET_CAST(Actor, sender);

			if(true || (sender == __GET_CAST(Object, this)) || __GET_CAST(Cuboid, sender) || __GET_CAST(Body, sender))
	        {
				switch(message)
	            {
					case kCollision:

						Actor_resolveCollision(this, __GET_CAST(VirtualList, Telegram_getExtraInfo(telegram)));
						return true;
						break;
						
					case kCollisionWithYou:

						Actor_resolveCollisionAgainstMe(this, __GET_CAST(SpatialObject, Telegram_getSender(telegram)), (VBVec3D*)Telegram_getExtraInfo(telegram));
						return true;
						break;

					case kBodyStartedMoving:

						CollisionManager_shapeStartedMoving(CollisionManager_getInstance(), this->shape);
						Actor_resetCollisionStatus(this, *(u8*)Telegram_getExtraInfo(telegram));
						return true;
						break;

					case kBodyStoped:

						if(!Body_isMoving(this->body))
	                    {
							CollisionManager_shapeStopedMoving(CollisionManager_getInstance(), this->shape);
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
				__VIRTUAL_CALL(void, Actor, takeHitFrom, otherActor);

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
bool Actor_moves(Actor this)
{
	ASSERT(this, "Actor::moves: null this");

	return true;
}

// is it moving?
u8 Actor_isMoving(Actor this)
{
	ASSERT(this, "Actor::isMoving: null this");

	return this->body ? Body_isMoving(this->body) : 0;
}

// set position
void Actor_setPosition(Actor this, const VBVec3D* position)
{
	ASSERT(this, "Actor::setPosition: null this");

	Actor_setLocalPosition(this, position);
}

// retrieve global position
const VBVec3D* Actor_getPosition(Actor this)
{
	ASSERT(this, "Actor::getPosition: null this");

	if(this->body)
	{
		return Body_getPosition(this->body);
	}

	return Entity_getPosition(__GET_CAST(Entity, this));
}

// check if must update sprite's position
bool Actor_updateSpritePosition(Actor this)
{
	ASSERT(this, "Actor::updateSpritePosition: null this");

	return (this->invalidateGlobalPosition.x || this->invalidateGlobalPosition.y || this->invalidateGlobalPosition.z || Actor_isMoving(this) || _screenDisplacement->x || _screenDisplacement->y || _screenDisplacement->z);
}

// check if must update sprite's scale
bool Actor_updateSpriteTransformations(Actor this)
{
	ASSERT(this, "Actor::updateSpriteTransformations: null this");

	if(this->body && Body_isAwake(this->body) &&  Body_getVelocity(this->body).z)
	{
		return true;
	}
	
	return Entity_updateSpriteTransformations(__GET_CAST(Entity, this));
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

// start bouncing after collision with another inGameEntity
static void Actor_checkIfMustBounce(Actor this, u8 axisOfCollision)
{
	ASSERT(this, "Actor::bounce: null this");

	if(axisOfCollision)
	{
		fix19_13 otherSpatialObjectsElasticity = this->collisionSolver? CollisionSolver_getCollisingSpatialObjectsTotalElasticity(this->collisionSolver, axisOfCollision): ITOFIX19_13(1);

		Body_bounce(this->body, axisOfCollision, otherSpatialObjectsElasticity);
		
		if(!(axisOfCollision & Body_isMoving(this->body)))
	    {
			MessageDispatcher_dispatchMessage(0, __GET_CAST(Object, this), __GET_CAST(Object, this), kBodyStoped, &axisOfCollision);
		}
		else
	    {
			MessageDispatcher_dispatchMessage(0, __GET_CAST(Object, this), __GET_CAST(Object, this), kBodyBounced, &axisOfCollision);
		}
	}
}

// resolve collision against other entities
static void Actor_resolveCollision(Actor this, VirtualList collidingSpatialObjects)
{
	ASSERT(this, "Actor::resolveCollision: null this");
	ASSERT(this->body, "Actor::resolveCollision: null body");
	ASSERT(collidingSpatialObjects, "Actor::resolveCollision: collidingSpatialObjects");

	if(this->collisionSolver)
	{
		u8 axisOfAllignement = CollisionSolver_resolveCollision(this->collisionSolver, collidingSpatialObjects, Body_isMoving(this->body), Body_getLastDisplacement(this->body), &this->transform.globalScale);

		Actor_checkIfMustBounce(this, axisOfAllignement);
		
		Actor_updateSourroundingFriction(this);
	}
}

// resolve collision against me entities
static void Actor_resolveCollisionAgainstMe(Actor this, SpatialObject collidingSpatialObject, VBVec3D* collidingSpatialObjectLastDisplacement)
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
		u8 axisOfCollision = CollisionSolver_resolveCollision(this->collisionSolver, collidingSpatialObjects, Body_isMoving(this->body), fakeLastDisplacement, &this->transform.globalScale);
		__DELETE(collidingSpatialObjects);
		
		Actor_checkIfMustBounce(this, axisOfCollision);
		
		Actor_updateSourroundingFriction(this);
	}
}

// retrieve body
const Body Actor_getBody(Actor this)
{
	ASSERT(this, "Actor::getBodys: null this");

	return this->body;
}

// take hit
void Actor_takeHitFrom(Actor this, Actor other)
{
	ASSERT(this, "Actor::takeHitFrom: null this");

	const Body otherBody = Actor_getBody(other);

	if(otherBody)
	{
		Body_takeHitFrom(this->body, otherBody);
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

	return this->body ? Body_getFriction(this->body).x : this->actorDefinition->friction;
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
	Actor_updateSourroundingFriction(this);
}
