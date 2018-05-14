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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Actor.h>
#include <Clock.h>
#include <MessageDispatcher.h>
#include <CollisionManager.h>
#include <Optics.h>
#include <Camera.h>
#include <Shape.h>
#include <PhysicalWorld.h>
#include <Body.h>
#include <Box.h>
#include <Game.h>
#include <debugUtilities.h>


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


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Actor, const ActorDefinition* actorDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(Actor, actorDefinition, id, internalId, name);

// class's constructor
void Actor::constructor(Actor this, const ActorDefinition* actorDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "Actor::constructor: null this");

	// construct base object
	Base::constructor(this, (AnimatedEntityDefinition*)&actorDefinition->animatedEntityDefinition, id, internalId, name);

	// save definition
	this->actorDefinition = actorDefinition;

	// construct the game state machine
	this->stateMachine = NULL;

	this->body = NULL;
	this->previousRotation = this->transformation.localRotation;

	// create body
	if(actorDefinition->createBody)
	{
		if(actorDefinition->animatedEntityDefinition.entityDefinition.physicalSpecification)
		{
			this->body = PhysicalWorld::createBody(Game::getPhysicalWorld(Game::getInstance()), (BodyAllocator)__TYPE(Body), __SAFE_CAST(SpatialObject, this), actorDefinition->animatedEntityDefinition.entityDefinition.physicalSpecification, actorDefinition->axesSubjectToGravity);
		}
		else
		{
			PhysicalSpecification defaultActorPhysicalSpecification = {__I_TO_FIX10_6(1), 0, 0, (Vector3D){0, 0, 0}};
			this->body = PhysicalWorld::createBody(Game::getPhysicalWorld(Game::getInstance()), (BodyAllocator)__TYPE(Body), __SAFE_CAST(SpatialObject, this), &defaultActorPhysicalSpecification, actorDefinition->axesSubjectToGravity);
		}
	}
}

// class's destructor
void Actor::destructor(Actor this)
{
	ASSERT(this, "Actor::destructor: null this");

	// inform the camera I'm being removed
	Camera::onFocusEntityDeleted(Camera::getInstance(), __SAFE_CAST(Entity, this));

	if(this->body)
	{
		// remove a body
		PhysicalWorld::destroyBody(Game::getPhysicalWorld(Game::getInstance()), this->body);
		this->body = NULL;
	}

	// destroy state machine
	if(this->stateMachine)
	{
		__DELETE(this->stateMachine);
		this->stateMachine = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void Actor::iAmDeletingMyself(Actor this)
{
	ASSERT(this, "Actor::iAmDeletingMyself: null this");

	Base::iAmDeletingMyself(this);

	// destroy body to prevent any more physical interactions
	if(this->body)
	{
		Body::setActive(this->body, false);

		// remove a body
//		PhysicalWorld::destroyBody(Game::getPhysicalWorld(Game::getInstance()), this->body);
//		this->body = NULL;
	}
}

// set definition
void Actor::setDefinition(Actor this, void* actorDefinition)
{
	ASSERT(this, "Actor::setDefinition: null this");
	ASSERT(actorDefinition, "Actor::setDefinition: null definition");

	// save definition
	this->actorDefinition = actorDefinition;

	Base::setDefinition(this, &((ActorDefinition*)actorDefinition)->animatedEntityDefinition);
}

//set class's local position
void Actor::setLocalPosition(Actor this, const Vector3D* position)
{
	ASSERT(this, "Actor::setLocalPosition: null this");

	Vector3D displacement = this->transformation.localPosition;
	Base::setLocalPosition(this, position);

	displacement.x -= this->transformation.localPosition.x;
	displacement.y -= this->transformation.localPosition.y;
	displacement.z -= this->transformation.localPosition.z;

	this->transformation.globalPosition.x -= displacement.x;
	this->transformation.globalPosition.y -= displacement.y;
	this->transformation.globalPosition.z -= displacement.z;

	if(this->body)
	{
		Body::setPosition(this->body, &this->transformation.globalPosition, __SAFE_CAST(SpatialObject, this));
	}

	this->invalidateGlobalTransformation = (displacement.x ? __X_AXIS: 0) | (displacement.y ? __Y_AXIS: 0) | (displacement.y ? __Z_AXIS: 0);

	Entity::transformShapes(__SAFE_CAST(Entity, this));
}

void Actor::syncWithBody(Actor this)
{
	ASSERT(this, "Actor::syncPositionWithBody: null this");

	 Actor::syncPositionWithBody(this);

	 Actor::syncRotationWithBody(this);
}

void Actor::syncPositionWithBody(Actor this)
{
	ASSERT(this, "Actor::syncPositionWithBody: null this");

	if(!this->body)
	{
		return;
	}

	// retrieve the body's displacement
	Vector3D bodyLastDisplacement = {0, 0, 0};
	Vector3D bodyPosition = this->transformation.globalPosition;

	if(!Clock::isPaused(Game::getPhysicsClock(Game::getInstance())) && Body::isActive(this->body) && Body::isAwake(this->body))
	{
		bodyPosition = *Body::getPosition(this->body);
	}

	// modify the global position according to the body's displacement
	Vector3D globalPosition = this->transformation.globalPosition;
	bodyLastDisplacement.x = bodyPosition.x - globalPosition.x;
	bodyLastDisplacement.y = bodyPosition.y - globalPosition.y;
	bodyLastDisplacement.z = bodyPosition.z - globalPosition.z;

	globalPosition.x += bodyLastDisplacement.x;
	globalPosition.y += bodyLastDisplacement.y;
	globalPosition.z += bodyLastDisplacement.z;

	// move the body to the new global position
	// to account for any parenting
	Body::setPosition(this->body, &globalPosition, __SAFE_CAST(SpatialObject, this));

	// sync local position with global position
	Vector3D localPosition = this->transformation.localPosition;
	localPosition.x += bodyLastDisplacement.x;
	localPosition.y += bodyLastDisplacement.y;
	localPosition.z += bodyLastDisplacement.z;

	Base::setLocalPosition(this, &localPosition);
}

void Actor::syncRotationWithBody(Actor this)
{
	ASSERT(this, "Actor::syncRotationWithBody: null this");

	if(this->body && Body::getMovementOnAllAxes(this->body))
	{
		Velocity velocity = Body::getVelocity(this->body);

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

		Entity::setDirection(__SAFE_CAST(Entity, this), direction);
	}
}

// updates the animation attributes
// graphically refresh of characters that are visible
void Actor::transform(Actor this, const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	ASSERT(this, "Actor::transform: null this");

	// apply environment transformation
	Container::applyEnvironmentToTransformation(__SAFE_CAST(Container, this), environmentTransform);

	if(this->body)
	{
		u16 bodyMovement = Body::getMovementOnAllAxes(this->body);

		if(bodyMovement)
		{
			Actor::syncWithBody(this);

			this->invalidateGlobalTransformation |= __INVALIDATE_POSITION;

			if(__Z_AXIS & bodyMovement)
			{
				this->invalidateGlobalTransformation |= __INVALIDATE_SCALE;
			}
		}
	}

	// call base
	Base::transform(this, environmentTransform, invalidateTransformationFlag);

	this->previousRotation = this->transformation.localRotation;
}

void Actor::resume(Actor this)
{
	ASSERT(this, "Actor::resume: null this");

	Base::resume(this);

	Actor::syncWithBody(this);
}

// execute character's logic
void Actor::update(Actor this, u32 elapsedTime)
{
	ASSERT(this, "Actor::update: null this");

	// call base
	Base::update(this, elapsedTime);

	if(this->stateMachine)
	{
		StateMachine::update(this->stateMachine);
	}

//	Body::print(this->body, 1, 0);
//	Shape::print(VirtualList::front(this->shapes), 1, 20);
//	Printing::resetWorldCoordinates(Printing::getInstance());
}

// whether changed direction in the last cycle or not
bool Actor::hasChangedDirection(Actor this, u16 axis)
{
	ASSERT(this, "Actor::changedDirection: null this");

	switch(axis)
	{
		case __X_AXIS:

			return this->transformation.localRotation.x != this->previousRotation.x;
			break;

		case __Y_AXIS:

			return this->transformation.localRotation.y != this->previousRotation.y;
			break;

		case __Z_AXIS:

			return this->transformation.localRotation.z != this->previousRotation.z;
			break;
	}

	return false;
}

// change direction over axis
void Actor::changeDirectionOnAxis(Actor this, u16 axis)
{
	ASSERT(this, "Actor::changeDirectionOnAxis: null this");

	if(this->body)
	{
		 Actor::syncRotationWithBody(this);
	}
	else
	{
		// save current rotation
		this->previousRotation = this->transformation.localRotation;

		Direction direction = Entity::getDirection(__SAFE_CAST(Entity, this));

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

		Entity::setDirection(__SAFE_CAST(Entity, this), direction);
	}
}


// check if gravity must apply to this actor
bool Actor::isSubjectToGravity(Actor this, Acceleration gravity)
{
	ASSERT(this, "Actor::isSubjectToGravity: null this");

	return Actor::canMoveTowards(this, gravity);
}

// check if gravity must apply to this actor
bool Actor::canMoveTowards(Actor this, Vector3D direction)
{
	ASSERT(this, "Actor::canMoveTowards: null this");

	fix10_6 collisionCheckDistance = __I_TO_FIX10_6(1);

	Vector3D displacement =
	{
		direction.x ? 0 < direction.x ? collisionCheckDistance : -collisionCheckDistance : 0,
		direction.y ? 0 < direction.y ? collisionCheckDistance : -collisionCheckDistance : 0,
		direction.z ? 0 < direction.z ? collisionCheckDistance : -collisionCheckDistance : 0
	};

	bool canMove = true;

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape shape = __SAFE_CAST(Shape, node->data);
			canMove &= Shape::canMoveTowards(shape, displacement, 0);
		}
	}

	return canMove;
}

fix10_6 Actor::getBouncinessOnCollision(Actor this __attribute__ ((unused)), SpatialObject collidingObject, const Vector3D* collidingObjectNormal __attribute__ ((unused)))
{
	ASSERT(this, "Actor::getBouncinessOnCollision: null this");

	return  SpatialObject::getBounciness(collidingObject);
}

fix10_6 Actor::getSurroundingFrictionCoefficient(Actor this)
{
	ASSERT(this, "Actor::getSurroundingFrictionCoefficient: null this");

	fix10_6 totalFrictionCoefficient = 0;

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape shape = __SAFE_CAST(Shape, node->data);

			totalFrictionCoefficient += Shape::getCollidingFrictionCoefficient(shape);
		}
	}

	return totalFrictionCoefficient;
}

fix10_6 Actor::getFrictionOnCollision(Actor this, SpatialObject collidingObject __attribute__ ((unused)), const Vector3D* collidingObjectNormal __attribute__ ((unused)))
{
	ASSERT(this, "Actor::getFrictionOnCollision: null this");

	return  Actor::getSurroundingFrictionCoefficient(this);
}

bool Actor::enterCollision(Actor this, const CollisionInformation* collisionInformation)
{
	ASSERT(this, "Actor::enterCollision: null this");
	ASSERT(collisionInformation->collidingShape, "Actor::enterCollision: collidingShapes");

	if(!this->body)
	{
		return false;
	}

	bool returnValue = false;

	if(collisionInformation->shape && collisionInformation->collidingShape)
	{
		if(collisionInformation->solutionVector.magnitude)
		{
			Shape::resolveCollision(collisionInformation->shape, collisionInformation);

			SpatialObject collidingObject = Shape::getOwner(collisionInformation->collidingShape);

			fix10_6 bounciness =  Actor::getBouncinessOnCollision(this, collidingObject, &collisionInformation->solutionVector.direction);
			fix10_6 frictionCoefficient =  Actor::getFrictionOnCollision(this, collidingObject, &collisionInformation->solutionVector.direction);

			if( Actor::mustBounce(this))
			{
				Body::bounce(this->body, __SAFE_CAST(Object, collisionInformation->collidingShape), collisionInformation->solutionVector.direction, frictionCoefficient, bounciness);
			}
			else
			{
				u16 axis = __NO_AXIS;
				axis |= collisionInformation->solutionVector.direction.x ? __X_AXIS : __NO_AXIS;
				axis |= collisionInformation->solutionVector.direction.y ? __Y_AXIS : __NO_AXIS;
				axis |= collisionInformation->solutionVector.direction.z ? __Z_AXIS : __NO_AXIS;
				Actor::stopMovement(this, axis);
			}

			returnValue = true;
		}
	}

	return returnValue;
}

// process a telegram
bool Actor::handleMessage(Actor this, Telegram telegram)
{
	ASSERT(this, "Actor::handleMessage: null this");

	if(!this->stateMachine || !StateMachine::handleMessage(this->stateMachine, telegram))
	{
		// retrieve message
		int message = Telegram::getMessage(telegram);

		if(this->body && Body::isActive(this->body))
		{
			switch(message)
			{
				case kBodyStartedMoving:

					if(this->shapes)
					{
						Entity::informShapesThatStartedMoving(__SAFE_CAST(Entity, this));
						return true;
					}
					break;

				case kBodyStopped:

					if(!Body::getMovementOnAllAxes(this->body) && this->shapes)
					{
						Entity::informShapesThatStoppedMoving(__SAFE_CAST(Entity, this));
					}
					break;

				case kBodyChangedDirection:

					Actor::changeDirectionOnAxis(this, *(int*)Telegram::getExtraInfo(telegram));
					return true;
					break;
			}
		}
	}
	return false;
}

// retrieve state machine
StateMachine Actor::getStateMachine(Actor this)
{
	ASSERT(this, "Actor::getStateMachine: null this");

	return this->stateMachine;
}

// stop movement completely
void Actor::stopAllMovement(Actor this)
{
	ASSERT(this, "Actor::stopMovement: null this");

	Actor::stopMovement(this, __X_AXIS | __Y_AXIS | __Z_AXIS);
}

// stop movement completely
void Actor::stopMovement(Actor this, u16 axis)
{
	ASSERT(this, "Actor::stopMovement: null this");

	if(this->body)
	{
		Body::stopMovement(this->body, axis);
	}
}

void Actor::addForce(Actor this, const Force* force)
{
	ASSERT(this, "Actor::addForce: null this");
	ASSERT(this->body, "Actor::addForce: null body");

	if(!this->body)
	{
		return;
	}

	if(!Actor::canMoveTowards(this, *force))
	{
		return;
	}

	Body::addForce(this->body, force);

	if(this->shapes)
	{
		// register the shape for collision detections
		Entity::informShapesThatStartedMoving(__SAFE_CAST(Entity, this));
	}
}

void Actor::moveUniformly(Actor this, Velocity* velocity)
{
	ASSERT(this, "Actor::moveUniformly: null this");

	// move me with physics
	if(this->body)
	{
		Body::moveUniformly(this->body, *velocity);

		Entity::informShapesThatStartedMoving(__SAFE_CAST(Entity, this));
	}
}

// is it moving?
bool Actor::isMoving(Actor this)
{
	ASSERT(this, "Actor::isMoving: null this");

	return this->body ? Body::getMovementOnAllAxes(this->body) : 0;
}

u16 Actor::getMovementState(Actor this)
{
	ASSERT(this, "Actor::getMovementState: null this");

	return Actor::isMoving(this);
}

void Actor::changeEnvironment(Actor this, Transformation* environmentTransform)
{
	ASSERT(this, "Actor::changeEnvironment: null this");

	Base::changeEnvironment(this, environmentTransform);

	if(this->body)
	{
		Body::setPosition(this->body, &this->transformation.globalPosition, __SAFE_CAST(SpatialObject, this));
	}
}

/**
 * Initial transformation
 *
 * @memberof					Actor
 * @public
 *
 * @param this					Function scope
 * @param environmentTransform
 * @param recursive
 */
void Actor::initialTransform(Actor this, const Transformation* environmentTransform, u32 recursive)
{
	ASSERT(this, "Entity::initialTransform: null this");

	// call base class's transformation method
	Base::initialTransform(this, environmentTransform, recursive);

	if(this->body)
	{
		Body::setPosition(this->body, &this->transformation.globalPosition, __SAFE_CAST(SpatialObject, this));
	}
}

// set position
void Actor::setPosition(Actor this, const Vector3D* position)
{
	ASSERT(this, "Actor::setPosition: null this");

	Vector3D displacement = this->transformation.globalPosition;
	this->transformation.globalPosition = *position;

	displacement.x -= this->transformation.globalPosition.x;
	displacement.y -= this->transformation.globalPosition.y;
	displacement.z -= this->transformation.globalPosition.z;

	this->transformation.localPosition.x -= displacement.x;
	this->transformation.localPosition.y -= displacement.y;
	this->transformation.localPosition.z -= displacement.z;

	if(this->body)
	{
		Body::setPosition(this->body, &this->transformation.globalPosition, __SAFE_CAST(SpatialObject, this));
	}

	this->invalidateGlobalTransformation = __INVALIDATE_TRANSFORMATION;
	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;

	Entity::transformShapes(__SAFE_CAST(Entity, this));
}

// retrieve global position
const Vector3D* Actor::getPosition(Actor this)
{
	ASSERT(this, "Actor::getPosition: null this");

	return this->body ? Body::getPosition(this->body) : Base::getPosition(this);
}

// take hit
void Actor::takeHitFrom(Actor this, Actor other)
{
	ASSERT(this, "Actor::takeHitFrom: null this");
	ASSERT(__SAFE_CAST(Actor, other), "Actor::takeHitFrom: other is not actor");

	if(this->body && other->body)
	{
		Body::takeHitFrom(this->body, other->body);
	}
}

// get bounciness
fix10_6 Actor::getBounciness(Actor this)
{
	ASSERT(this, "Actor::getBounciness: null this");

	PhysicalSpecification* physicalSpecification = this->actorDefinition->animatedEntityDefinition.entityDefinition.physicalSpecification;

	return this->body ? Body::getBounciness(this->body) : physicalSpecification ? physicalSpecification->bounciness : 0;
}

// get velocity
Velocity Actor::getVelocity(Actor this)
{
	ASSERT(this, "Actor::getVelocity: null this");

	return this->body ? Body::getVelocity(this->body) : Base::getVelocity(this);
}

void Actor::exitCollision(Actor this, Shape shape  __attribute__ ((unused)), Shape shapeNotCollidingAnymore, bool isShapeImpenetrable)
{
	ASSERT(this, "Actor::exitCollision: null this");

	if(!this->body)
	{
		return;
	}

	Body::setSurroundingFrictionCoefficient(this->body,  Actor::getSurroundingFrictionCoefficient(this));

	if(isShapeImpenetrable)
	{
		Body::clearNormal(this->body, __SAFE_CAST(Object, shapeNotCollidingAnymore));
	}
}

void Actor::collidingShapeOwnerDestroyed(Actor this, Shape shape __attribute__ ((unused)), Shape shapeNotCollidingAnymore, bool isShapeImpenetrable)
{
	ASSERT(this, "Actor::collidingShapeOwnerDestroyed: null this");

	if(!this->body)
	{
		return;
	}

	Body::setSurroundingFrictionCoefficient(this->body,  Actor::getSurroundingFrictionCoefficient(this));

	if(isShapeImpenetrable)
	{
		Body::clearNormal(this->body, __SAFE_CAST(Object, shapeNotCollidingAnymore));
	}
}

Body Actor::getBody(Actor this)
{
	ASSERT(this, "Actor::getBody: null this");

	return this->body;
}

bool Actor::mustBounce(Actor this __attribute__ ((unused)))
{
	ASSERT(this, "Actor::mustBounce: null this");

	return true;
}