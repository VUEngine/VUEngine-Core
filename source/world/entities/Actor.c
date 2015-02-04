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

static void Actor_alignToCollidingEntity(Actor this, InGameEntity collidingEntity, int axisOfCollision);
static void Actor_checkIfMustBounce(Actor this, InGameEntity collidingEntity, int axisOfCollision);
static void Actor_resolveCollision(Actor this, VirtualList collidingEntities);
static void Actor_resolveCollisionAgainstMe(Actor this, InGameEntity collidingEntity, VBVec3D* collidingEntityLastDisplacement);
static void Actor_updateCollisionStatus(Actor this, int movementAxis);
static void Actor_updateSourroundingFriction(Actor this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Actor, ActorDefinition* actorDefinition, s16 id)
__CLASS_NEW_END(Actor, actorDefinition, id);

// class's constructor
void Actor_constructor(Actor this, ActorDefinition* actorDefinition, s16 id)
{
	ASSERT(this, "Actor::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE((AnimatedInGameEntityDefinition*)&actorDefinition->inGameEntityDefinition, id);

	// construct the game state machine
	this->stateMachine = __NEW(StateMachine, this);

	this->lastCollidingEntity[kXAxis] = NULL;
	this->lastCollidingEntity[kYAxis] = NULL;
	this->lastCollidingEntity[kZAxis] = NULL;

	this->sensibleToFriction.x = true;
	this->sensibleToFriction.y = true;
	this->sensibleToFriction.z = true;

	this->body = NULL;
}

// class's destructor
void Actor_destructor(Actor this)
{
	ASSERT(this, "Actor::destructor: null this");

	// inform the screen I'm being removed
	Screen_focusEntityDeleted(Screen_getInstance(), (InGameEntity)this);

	// remove a body
	PhysicalWorld_unregisterBody(PhysicalWorld_getInstance(), __UPCAST(Entity, this));

	// destroy state machine
	__DELETE(this->stateMachine);

	// destroy the super object
	__DESTROY_BASE;
}

//set class's local position
void Actor_setLocalPosition(Actor this, VBVec3D position)
{
	ASSERT(this, "Actor::setLocalPosition: null this");

	Container_setLocalPosition(__UPCAST(Container, this), position);

	if (this->body)
    {
		VBVec3D globalPosition = Container_getGlobalPosition(__UPCAST(Container, this));

		Transformation environmentTransform =
        {
				// local position
				{0, 0, 0},
				// global position
				{0, 0, 0},
				// scale
				{1, 1},
				// rotation
				{0, 0, 0}
		};

		if (this->parent)
        {
			environmentTransform = Container_getEnvironmentTransform(this->parent);
			globalPosition.x = environmentTransform.globalPosition.x;
			globalPosition.y = environmentTransform.globalPosition.y;
			globalPosition.z = environmentTransform.globalPosition.z;
		}

		globalPosition.x += position.x;
		globalPosition.y += position.y;
		globalPosition.z += position.z;

		this->lastCollidingEntity[kXAxis] = NULL;
		this->lastCollidingEntity[kYAxis] = NULL;
		this->lastCollidingEntity[kZAxis] = NULL;

		Body_setPosition(this->body, &globalPosition, __UPCAST(Object, this));
	}
}

static void Actor_syncPositionWithBody(Actor this)
{
	// save previous position
	this->previousGlobalPosition = this->transform.globalPosition;

	Container_setLocalPosition(__UPCAST(Container, this), Body_getPosition(this->body));
}

// updates the animation attributes
// graphically refresh of characters that are visible
void Actor_transform(Actor this, Transformation* environmentTransform)
{
	ASSERT(this, "Actor::transform: null this");

	if (this->body && Body_isAwake(this->body))
    {
		Actor_syncPositionWithBody(this);
		
		// an Actor with a physical body is agnostic to parenting
		Transformation environmentAgnosticTransform =
	    {
				// local position
				{0, 0, 0},
				// global position
				{0, 0, 0},
				// scale
				{environmentTransform->scale.x, environmentTransform->scale.y},
				// rotation
				{0, 0, 0}
		};

		// call base
		AnimatedInGameEntity_transform((AnimatedInGameEntity)this, &environmentAgnosticTransform);
	}
	else
	{
		// call base
		AnimatedInGameEntity_transform((AnimatedInGameEntity)this, environmentTransform);
	}
}

// execute character's logic
void Actor_update(Actor this)
{
	ASSERT(this, "Actor::update: null this");

	// call base
	AnimatedInGameEntity_update((AnimatedInGameEntity)this);

	if (this->stateMachine)
	{
		StateMachine_update(this->stateMachine);
	}

	if (this->body)
	{
		Actor_updateCollisionStatus(this, Body_isMoving(this->body));
	}
}

// update colliding entities
static void Actor_updateCollisionStatus(Actor this, int movementAxis)
{
	ASSERT(this, "Actor::updateCollisionStatus: null this");
	ASSERT(this->body, "Actor::updateCollisionStatus: null body");

	if (__XAXIS & movementAxis)
	{
		this->lastCollidingEntity[kXAxis] = NULL;
	}
	if (__YAXIS & movementAxis)
	{
		this->lastCollidingEntity[kYAxis] = NULL;
	}
	if (__ZAXIS & movementAxis)
	{
		this->lastCollidingEntity[kZAxis] = NULL;
	}

	Actor_updateSourroundingFriction(this);
}

// retrieve friction of colliding objects
static void Actor_updateSourroundingFriction(Actor this)
{
	ASSERT(this, "Actor::updateSourroundingFriction: null this");
	ASSERT(this->body, "Actor::updateSourroundingFriction: null body");

	Force friction = {0, 0, 0};

	if (this->sensibleToFriction.x)
	{
		friction.x = this->lastCollidingEntity[kYAxis] ? __VIRTUAL_CALL(fix19_13, InGameEntity, getFriction, this->lastCollidingEntity[kYAxis]) : 0;
		friction.x += this->lastCollidingEntity[kZAxis] ? __VIRTUAL_CALL(fix19_13, InGameEntity, getFriction, this->lastCollidingEntity[kZAxis]) : 0;
	}

	if (this->sensibleToFriction.y)
	{
		friction.y = this->lastCollidingEntity[kXAxis] ? __VIRTUAL_CALL(fix19_13, InGameEntity, getFriction, this->lastCollidingEntity[kXAxis]) : 0;
		friction.y += this->lastCollidingEntity[kZAxis] ? __VIRTUAL_CALL(fix19_13, InGameEntity, getFriction, this->lastCollidingEntity[kZAxis]) : 0;
	}

	if (this->sensibleToFriction.z)
	{
		friction.z = this->lastCollidingEntity[kXAxis] ? __VIRTUAL_CALL(fix19_13, InGameEntity, getFriction, this->lastCollidingEntity[kXAxis]) : 0;
		friction.z += this->lastCollidingEntity[kYAxis] ? __VIRTUAL_CALL(fix19_13, InGameEntity, getFriction, this->lastCollidingEntity[kYAxis]) : 0;
	}

	Body_setFriction(this->body, friction);
}

// retrieve previous position
const VBVec3D* Actor_getPreviousPosition(Actor this)
{
	ASSERT(this, "Actor::getPreviousPosition: null this");
	
	return &this->previousGlobalPosition;
}

// change direction
void Actor_moveOpositeDirecion(Actor this, int axis)
{
	ASSERT(this, "Actor::moveOpositeDirecion: null this");

	switch (axis)
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

	switch (axis)
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

	if ((__XAXIS & axis))
	{
		if (__RIGHT == this->direction.x)
	    {
			this->direction.x = __LEFT;
		}
		else
	    {
			this->direction.x = __RIGHT;
		}
	}

	if ((__YAXIS & axis))
	{
		if (__NEAR == this->direction.y)
	    {
			this->direction.y = __FAR;
		}
		else
	    {
			this->direction.x = __NEAR;
		}
	}

	if ((__ZAXIS & axis))
	{
		if (__RIGHT == this->direction.z)
	    {
			this->direction.x = __LEFT;
		}
		else
	    {
			this->direction.x = __RIGHT;
		}
	}
}

// true if inside the screen range
bool Actor_isInsideGame(Actor this)
{
	ASSERT(this, "Actor::isInsideGame: null this");
	//Texture map = Sprite_getTexture(this->sprite);

	return 0;//!outsideScreenRange(this->transform.localPosition, Texture_getCols(map), Texture_getRows(map), __CHARACTERUNLOADPAD);
}

// check if gravity must apply to this actor
bool Actor_canMoveOverAxis(Actor this, const Acceleration* acceleration)
{
	ASSERT(this, "Actor::canMoveOverAxis: null this");

	bool axisFreeForMovement = __VIRTUAL_CALL(bool, Actor, getAxisFreeForMovement, this);

	int axisOfCollision = 0;

	if (axisFreeForMovement)
	{
		ASSERT(this->body, "Actor::resolveCollision: null body");

		int i = 0;
		// TODO: must still solve when there will be a collision with an object not yet in the list
		for (; i <= kZAxis; i++)
	    {
			if (this->lastCollidingEntity[i])
	        {
				VBVec3D displacement =
	            {
					kXAxis == i ? 0 < acceleration->x? FTOFIX19_13(1.5f): FTOFIX19_13(-1.5f): 0,
					kYAxis == i ? 0 < acceleration->y? FTOFIX19_13(1.5f): FTOFIX19_13(-1.5f): 0,
					kZAxis == i ? 0 < acceleration->z? FTOFIX19_13(1.5f): FTOFIX19_13(-1.5f): 0
				};

				axisOfCollision |= __VIRTUAL_CALL(bool, Shape, testIfCollision, this->shape, this->lastCollidingEntity[i], displacement);
			}
		}
	}

	return axisFreeForMovement & ~axisOfCollision;
}

// retrieve axis free for movement
int Actor_getAxisFreeForMovement(Actor this)
{
	ASSERT(this, "Actor::getAxisFreeForMovement: null this");

	bool movingState = Body_isMoving(this->body);

	return ((__XAXIS & ~(__XAXIS & movingState) )| (__YAXIS & ~(__YAXIS & movingState)) | (__ZAXIS & ~(__ZAXIS & movingState)));
}

// process a telegram
bool Actor_handleMessage(Actor this, Telegram telegram)
{
	ASSERT(this, "Actor::handleMessage: null this");

	if (!StateMachine_handleMessage(this->stateMachine, telegram))
	{
		// retrieve message
		int message = Telegram_getMessage(telegram);

		if (this->body)
	    {
			Object sender = Telegram_getSender(telegram);
			Actor atherActor = __GET_CAST(Actor, sender);

			if (true || (sender == __UPCAST(Object, this)) || __GET_CAST(Cuboid, sender) || __GET_CAST(Body, sender))
	        {
				switch (message)
	            {
					case kCollision:

						Actor_resolveCollision(this, (VirtualList)Telegram_getExtraInfo(telegram));
						return true;
						break;
						
					case kCollisionWithYou:

						Actor_resolveCollisionAgainstMe(this, (InGameEntity)Telegram_getSender(telegram), (VBVec3D*)Telegram_getExtraInfo(telegram));
						return true;
						break;

					case kBodyStartedMoving:

						CollisionManager_shapeStartedMoving(CollisionManager_getInstance(), this->shape);
						Actor_updateCollisionStatus(this, *(int*)Telegram_getExtraInfo(telegram));
						return true;
						break;

					case kBodyStoped:

						if (!Body_isMoving(this->body))
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
			else if (atherActor)
	        {
				__VIRTUAL_CALL(void, Actor, takeHitFrom, atherActor);

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
bool Actor_isMoving(Actor this)
{
	ASSERT(this, "Actor::isMoving: null this");

	return this->body ? Body_isMoving(this->body) : 0;
}

// retrieve global position
VBVec3D Actor_getPosition(Actor this)
{
	ASSERT(this, "Actor::getPosition: null this");

	if (this->body)
	{
		return Body_getPosition(this->body);
	}

	return Entity_getPosition(__UPCAST(Entity, this));
}

// check if must update sprite's position
int Actor_updateSpritePosition(Actor this)
{
	ASSERT(this, "Actor::updateSpritePosition: null this");

	return (this->invalidateGlobalPosition.x || this->invalidateGlobalPosition.y || this->invalidateGlobalPosition.z || Actor_isMoving(this) || _screenDisplacement->x || _screenDisplacement->y || _screenDisplacement->z);
}

// check if must update sprite's scale
int Actor_updateSpriteScale(Actor this)
{
	ASSERT(this, "Actor::updateSpriteScale: null this");

	if (this->body && Body_isAwake(this->body) &&  Body_getVelocity(this->body).z)
	{
		return true;
	}
	
	return AnimatedInGameEntity_updateSpriteScale((AnimatedInGameEntity)this);
}

// stop movement completelty
void Actor_stopMovement(Actor this)
{
	ASSERT(this, "Actor::stopMovement: null this");

	if (this->body)
	{
		Body_stopMovement(this->body, __XAXIS);
		Body_stopMovement(this->body, __YAXIS);
		Body_stopMovement(this->body, __ZAXIS);
	}
}

// align to colliding entity
static void Actor_alignToCollidingEntity(Actor this, InGameEntity collidingEntity, int axisOfCollision)
{
	Scale scale = Entity_getScale(__UPCAST(Entity,  this));
	int alignThreshold = FIX7_9TOI(FIX7_9_DIV(ITOFIX7_9(1), scale.y));

	if (1 > alignThreshold)
	{
		alignThreshold = 1;
	}
	alignThreshold = 1;

	if (__XAXIS & axisOfCollision)
    {
		Actor_alignTo(this, collidingEntity, __XAXIS, alignThreshold);
		this->lastCollidingEntity[kXAxis] = collidingEntity;
	}

	if (__YAXIS & axisOfCollision)
    {
		Actor_alignTo(this, collidingEntity, __YAXIS, alignThreshold);
		this->lastCollidingEntity[kYAxis] = collidingEntity;
	}

	if (__ZAXIS & axisOfCollision)
    {
		Actor_alignTo(this, collidingEntity, __ZAXIS, alignThreshold);
		this->lastCollidingEntity[kZAxis] = collidingEntity;
	}	
}

// start bouncing after collision with another inGameEntity
static void Actor_checkIfMustBounce(Actor this, InGameEntity collidingEntity, int axisOfCollision)
{
	ASSERT(this, "Actor::bounce: null this");

	if (collidingEntity && axisOfCollision)
	{
		// bounce over axis
		Body_bounce(this->body, axisOfCollision, __VIRTUAL_CALL(fix19_13, InGameEntity, getElasticity, collidingEntity));

		if (!(axisOfCollision & Body_isMoving(this->body)))
	    {
			MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, this), kBodyStoped, &axisOfCollision);
		}
		else
	    {
			MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, this), kBodyBounced, &axisOfCollision);
		}
	}
}

// resolve collision against other entities
static void Actor_resolveCollision(Actor this, VirtualList collidingEntities)
{
	ASSERT(this, "Actor::resolveCollision: null this");
	ASSERT(this->body, "Actor::resolveCollision: null body");
	ASSERT(collidingEntities, "Actor::resolveCollision: collidingEntities");

	int axisOfCollision = 0;

	// get last physical displacement
	VBVec3D displacement = Body_getLastDisplacement(this->body);

	VirtualNode node = VirtualList_begin(collidingEntities);

	InGameEntity collidingEntity = NULL;

	// TODO: solve when more than one entity has been touched
	for (; node && !axisOfCollision; node = VirtualNode_getNext(node))
	{
		collidingEntity = VirtualNode_getData(node);
		axisOfCollision = __VIRTUAL_CALL(int, Shape, getAxisOfCollision, this->shape, collidingEntity, displacement);
		Actor_alignToCollidingEntity(this, collidingEntity, axisOfCollision);
	}

	Actor_checkIfMustBounce(this, collidingEntity, axisOfCollision);
}

// resolve collision against me entities
static void Actor_resolveCollisionAgainstMe(Actor this, InGameEntity collidingEntity, VBVec3D* collidingEntityLastDisplacement)
{
	ASSERT(this, "Actor::resolveCollisionAgainstMe: null this");
	ASSERT(this->body, "Actor::resolveCollisionAgainstMe: null body");

	int axisOfCollision = 0;
	
	Shape collidingEntityShape = __VIRTUAL_CALL(Shape, Entity, getShape, collidingEntity);

	ASSERT(collidingEntityShape, "Actor::resolveCollision: null shape");
	
	if(collidingEntityShape)
	{
		axisOfCollision = __VIRTUAL_CALL(int, Shape, getAxisOfCollision, collidingEntityShape, this, collidingEntityLastDisplacement);
		Actor_alignToCollidingEntity(this, collidingEntity, axisOfCollision);
		Actor_checkIfMustBounce(this, collidingEntity, axisOfCollision);
	}
}

// align character to other entity on collision
void Actor_alignTo(Actor this, InGameEntity entity, int axis, int pad)
{
	ASSERT(this, "Actor::alignTo: null this");
	ASSERT(this->sprites, "Actor::alignTo: null sprites");

	// retrieve the colliding entity's position and gap
	VBVec3D otherPosition = Container_getGlobalPosition((Container) entity);
	Gap otherGap = InGameEntity_getGap(entity);

	// pointers to the dimensions to affect
	fix19_13 *myPositionAxisToCheck = NULL;
	fix19_13 *myPositionAxis = NULL;
	fix19_13 *otherPositionAxis = NULL;

	// used to the width, height or deep
	u16 myHalfSize = 0;
	u16 otherHalfSize = 0;

	// gap to use based on the axis
	int otherLowGap = 0;
	int otherHighGap = 0;
	int myLowGap = 0;
	int myHighGap = 0;

	// calculate gap again (scale, etc)
	InGameEntity_setGap((InGameEntity)this);

	// select the axis to affect
	switch (axis)
	{
		case __XAXIS:

			myPositionAxisToCheck = &this->transform.globalPosition.x;
			myPositionAxis = &this->transform.localPosition.x;
			otherPositionAxis = &otherPosition.x;

			myHalfSize = __VIRTUAL_CALL(u16, Entity, getWidth, this) >> 1;
			otherHalfSize = __VIRTUAL_CALL(u16, Entity, getWidth, entity) >> 1;

			otherLowGap = otherGap.left;
			otherHighGap = otherGap.right;
			myLowGap = this->gap.left;
			myHighGap = this->gap.right;
			break;

		case __YAXIS:

			myPositionAxisToCheck = &this->transform.globalPosition.y;
			myPositionAxis = &this->transform.localPosition.y;
			otherPositionAxis = &otherPosition.y;

			myHalfSize = __VIRTUAL_CALL(u16, Entity, getHeight, this) >> 1;
			otherHalfSize = __VIRTUAL_CALL(u16, Entity, getHeight, entity) >> 1;

			otherLowGap = otherGap.up;
			otherHighGap = otherGap.down;
			myLowGap = this->gap.up;
			myHighGap = this->gap.down;
			break;

		case __ZAXIS:

			myPositionAxisToCheck = &this->transform.globalPosition.z;
			myPositionAxis = &this->transform.localPosition.z;
			otherPositionAxis = &otherPosition.z;

			// TODO: must make deep work as the width and high
			if (this->transform.globalPosition.z < otherPosition.z)
			{
				myHalfSize = __VIRTUAL_CALL(u16, Entity, getDeep, this);
				otherHalfSize = 0;
			}
			else
			{
				myHalfSize = 0;
				otherHalfSize = __VIRTUAL_CALL(u16, Entity, getDeep, entity);
			}
			
			myLowGap = 0;
			myHighGap = 0;

			break;
	}

	// decide to which side of the entity align myself
	if (*myPositionAxisToCheck > *otherPositionAxis)
    {
        // pad -= (FIX19_13TOI(*myPositionAxis) > (screenSize >> 1) ? 1 : 0);
		// align right / below / behind
		*myPositionAxis = *otherPositionAxis +
							ITOFIX19_13(otherHalfSize - otherHighGap
							+ myHalfSize - myLowGap
							+ pad);
	}
	else
	{
		// align left / over / in front
		*myPositionAxis = *otherPositionAxis -
							ITOFIX19_13(otherHalfSize - otherLowGap
							+ myHalfSize - myHighGap
							+ pad);
	}

	if (this->body)
	{
		// force position
		Body_setPosition(this->body, &this->transform.localPosition, __UPCAST(Object, this));
		Actor_syncPositionWithBody(this);
	}

	Transformation environmentTransform = Container_getEnvironmentTransform(__UPCAST(Container, this));
	Actor_transform((Actor)this, &environmentTransform);

	__VIRTUAL_CALL(void, Shape, positione, this->shape);
	this->invalidateGlobalPosition.x = this->invalidateGlobalPosition.y = this->invalidateGlobalPosition.z = true;
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

	if (otherBody)
	{
		Body_takeHitFrom(this->body, otherBody);
	}
}

// get elasticiy
fix19_13 Actor_getElasticity(Actor this)
{
	ASSERT(this, "Actor::getElasticity: null this");

	return this->body ? Body_getElasticity(this->body) : InGameEntity_getElasticity((InGameEntity)this);
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
		velocity.x || (force->x && (__XAXIS & Actor_canMoveOverAxis((Actor)this, &acceleration)))? force->x: 0,
		velocity.y || (force->y && (__YAXIS & Actor_canMoveOverAxis((Actor)this, &acceleration)))? force->y: 0,
		velocity.z || (force->z && (__ZAXIS & Actor_canMoveOverAxis((Actor)this, &acceleration)))? force->z: 0
	};

	Body_addForce(this->body, &effectiveForceToApply);
}
