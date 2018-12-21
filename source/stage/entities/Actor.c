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

friend class VirtualList;
friend class VirtualNode;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Actor::constructor(const ActorSpec* actorSpec, s16 id, s16 internalId, const char* const name)
{
	// construct base object
	Base::constructor((AnimatedEntitySpec*)&actorSpec->animatedEntitySpec, id, internalId, name);

	// save spec
	this->actorSpec = actorSpec;

	// construct the game state machine
	this->stateMachine = NULL;

	this->body = NULL;
	this->previousRotation = this->transformation.localRotation;

	// create body
	if(actorSpec->createBody)
	{
		if(actorSpec->animatedEntitySpec.entitySpec.physicalSpecification)
		{
			this->body = PhysicalWorld::createBody(Game::getPhysicalWorld(Game::getInstance()), (BodyAllocator)__TYPE(Body), SpatialObject::safeCast(this), actorSpec->animatedEntitySpec.entitySpec.physicalSpecification, actorSpec->axesSubjectToGravity);
		}
		else
		{
			PhysicalSpecification defaultActorPhysicalSpecification = {__I_TO_FIX10_6(1), 0, 0, (Vector3D){0, 0, 0}};
			this->body = PhysicalWorld::createBody(Game::getPhysicalWorld(Game::getInstance()), (BodyAllocator)__TYPE(Body), SpatialObject::safeCast(this), &defaultActorPhysicalSpecification, actorSpec->axesSubjectToGravity);
		}
	}
}

// class's destructor
void Actor::destructor()
{
	// inform the camera I'm being removed
	Camera::onFocusEntityDeleted(Camera::getInstance(), Entity::safeCast(this));

	if(this->body)
	{
		// remove a body
		PhysicalWorld::destroyBody(Game::getPhysicalWorld(Game::getInstance()), this->body);
		this->body = NULL;
	}

	// destroy state machine
	if(this->stateMachine)
	{
		delete this->stateMachine;
		this->stateMachine = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void Actor::iAmDeletingMyself()
{
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

// set spec
void Actor::setSpec(void* actorSpec)
{
	ASSERT(actorSpec, "Actor::setSpec: null spec");

	// save spec
	this->actorSpec = actorSpec;

	Base::setSpec(this, &((ActorSpec*)actorSpec)->animatedEntitySpec);
}

//set class's local position
void Actor::setLocalPosition(const Vector3D* position)
{
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
		Body::setPosition(this->body, &this->transformation.globalPosition, SpatialObject::safeCast(this));
	}

	this->invalidateGlobalTransformation = (displacement.x ? __X_AXIS: 0) | (displacement.y ? __Y_AXIS: 0) | (displacement.y ? __Z_AXIS: 0);

	Entity::transformShapes(this);
}

void Actor::syncWithBody()
{
	Actor::syncPositionWithBody(this);
	Actor::syncRotationWithBody(this);
}

bool Actor::overrideParentingPositioningWhenBodyIsNotMoving()
{
	return true;
}

void Actor::syncPositionWithBody()
{
	if(!this->body)
	{
		return;
	}

	// retrieve the body's displacement
	Vector3D bodyLastDisplacement = {0, 0, 0};
	Vector3D bodyPosition = this->transformation.globalPosition;

	if(
		!Clock::isPaused(Game::getPhysicsClock(Game::getInstance()))
		&&
		(
			Actor::overrideParentingPositioningWhenBodyIsNotMoving(this) ||
			Body::isAwake(this->body)
		)
	)
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
	Body::setPosition(this->body, &globalPosition, SpatialObject::safeCast(this));

	// sync local position with global position
	Vector3D localPosition = this->transformation.localPosition;
	localPosition.x += bodyLastDisplacement.x;
	localPosition.y += bodyLastDisplacement.y;
	localPosition.z += bodyLastDisplacement.z;

	Base::setLocalPosition(this, &localPosition);
}

void Actor::syncRotationWithBody()
{
	if(this->body && Body::getMovementOnAllAxes(this->body))
	{
		Velocity velocity = Body::getVelocity(this->body);

		velocity = Vector3D::normalize(velocity);

//		this->transformation.localRotation.z = Math::getAngle(__FIX10_6_TO_FIX7_9(velocity.x), __FIX10_6_TO_FIX7_9(velocity.y));

//		Base::setLocalRotation(this, &this->transformation.localRotation);
//return;
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

		Entity::setDirection(this, direction);
	}
}

// updates the animation attributes
// graphically refresh of characters that are visible
void Actor::transform(const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	// apply environment transformation
	Container::applyEnvironmentToTransformation(this, environmentTransform);

	if(this->body)
	{
		Actor::syncWithBody(this);

		// Prevent transformation of shapes again when calling Base::transform
		this->transformShapes = false;

		u16 bodyMovement = Body::getMovementOnAllAxes(this->body);

		if(bodyMovement)
		{
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

void Actor::resume()
{
	Base::resume(this);

	Actor::syncWithBody(this);
}

// execute character's logic
void Actor::update(u32 elapsedTime)
{
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
bool Actor::hasChangedDirection(u16 axis)
{
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
void Actor::changeDirectionOnAxis(u16 axis)
{
	if(this->body)
	{
		Actor::syncRotationWithBody(this);
	}
	else
	{
		// save current rotation
		this->previousRotation = this->transformation.localRotation;

		Direction direction = Entity::getDirection(this);

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
			if(__UP == direction.y)
			{
				direction.y = __DOWN;
			}
			else
			{
				direction.y = __UP;
			}
		}

		if((__Z_AXIS & axis))
		{
			if(__NEAR == direction.z)
			{
				direction.x = __FAR;
			}
			else
			{
				direction.x = __NEAR;
			}
		}

		Entity::setDirection(this, direction);
	}
}

// check if gravity must apply to this actor
bool Actor::isSubjectToGravity(Acceleration gravity)
{
	return Actor::canMoveTowards(this, gravity);
}

// check if gravity must apply to this actor
bool Actor::canMoveTowards(Vector3D direction)
{
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
			Shape shape = Shape::safeCast(node->data);
			canMove &= Shape::canMoveTowards(shape, displacement, 0);
		}
	}

	return canMove;
}

fix10_6 Actor::getBouncinessOnCollision(SpatialObject collidingObject, const Vector3D* collidingObjectNormal __attribute__ ((unused)))
{
	return  SpatialObject::getBounciness(collidingObject);
}

fix10_6 Actor::getSurroundingFrictionCoefficient()
{
	fix10_6 totalFrictionCoefficient = 0;

	if(this->shapes)
	{
		VirtualNode node = this->shapes->head;

		for(; node; node = node->next)
		{
			Shape shape = Shape::safeCast(node->data);

			totalFrictionCoefficient += Shape::getCollidingFrictionCoefficient(shape);
		}
	}

	return totalFrictionCoefficient;
}

fix10_6 Actor::getFrictionOnCollision(SpatialObject collidingObject __attribute__ ((unused)), const Vector3D* collidingObjectNormal __attribute__ ((unused)))
{
	return  Actor::getSurroundingFrictionCoefficient(this);
}

bool Actor::enterCollision(const CollisionInformation* collisionInformation)
{
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
				Body::bounce(this->body, Object::safeCast(collisionInformation->collidingShape), collisionInformation->solutionVector.direction, frictionCoefficient, bounciness);
				//Actor::syncRotationWithBody(this);
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
bool Actor::handleMessage(Telegram telegram)
{
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
						Entity::activeCollisionChecks(this, true);
						return true;
					}
					break;

				case kBodyStopped:

					if(!Body::getMovementOnAllAxes(this->body) && this->shapes)
					{
						Entity::activeCollisionChecks(this, false);
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
StateMachine Actor::getStateMachine()
{
	return this->stateMachine;
}

// stop movement completely
void Actor::stopAllMovement()
{
	Actor::stopMovement(this, __X_AXIS | __Y_AXIS | __Z_AXIS);
}

// stop movement completely
void Actor::stopMovement(u16 axis)
{
	if(this->body)
	{
		Body::stopMovement(this->body, axis);
	}
}

void Actor::addForce(const Force* force)
{
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
		Entity::activeCollisionChecks(this, true);
	}
}

void Actor::moveUniformly(Velocity* velocity)
{
	// move me with physics
	if(this->body)
	{
		Body::moveUniformly(this->body, *velocity);

		Entity::activeCollisionChecks(this, true);
	}
}

// is it moving?
bool Actor::isMoving()
{
	return this->body ? Body::getMovementOnAllAxes(this->body) : 0;
}

u16 Actor::getMovementState()
{
	return Actor::isMoving(this);
}

void Actor::changeEnvironment(Transformation* environmentTransform)
{
	Base::changeEnvironment(this, environmentTransform);

	if(this->body)
	{
		Body::setPosition(this->body, &this->transformation.globalPosition, SpatialObject::safeCast(this));
	}
}

/**
 * Initial transformation
 *
 * @memberof					Actor
 * @public
 *
 * @param environmentTransform
 * @param recursive
 */
void Actor::initialTransform(const Transformation* environmentTransform, u32 recursive)
{
	// call base class's transformation method
	Base::initialTransform(this, environmentTransform, recursive);

	if(this->body)
	{
		Body::setPosition(this->body, &this->transformation.globalPosition, SpatialObject::safeCast(this));
	}
}

// set position
void Actor::setPosition(const Vector3D* position)
{
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
		Body::setPosition(this->body, &this->transformation.globalPosition, SpatialObject::safeCast(this));
	}

	this->invalidateGlobalTransformation = __INVALIDATE_TRANSFORMATION;
	this->invalidateSprites = __INVALIDATE_TRANSFORMATION;

	Entity::transformShapes(this);
}

// retrieve global position
const Vector3D* Actor::getPosition()
{
	return this->body ? Body::getPosition(this->body) : Base::getPosition(this);
}

// take hit
void Actor::takeHitFrom(Actor other)
{
	ASSERT(Actor::safeCast(other), "Actor::takeHitFrom: other is not actor");

	if(this->body && other->body)
	{
		Body::takeHitFrom(this->body, other->body);
	}
}

// get bounciness
fix10_6 Actor::getBounciness()
{
	PhysicalSpecification* physicalSpecification = this->actorSpec->animatedEntitySpec.entitySpec.physicalSpecification;

	return this->body ? Body::getBounciness(this->body) : physicalSpecification ? physicalSpecification->bounciness : 0;
}

// get velocity
Velocity Actor::getVelocity()
{
	return this->body ? Body::getVelocity(this->body) : Base::getVelocity(this);
}

void Actor::exitCollision(Shape shape  __attribute__ ((unused)), Shape shapeNotCollidingAnymore, bool isShapeImpenetrable)
{
	if(!this->body)
	{
		return;
	}

	Body::setSurroundingFrictionCoefficient(this->body,  Actor::getSurroundingFrictionCoefficient(this));

	if(isShapeImpenetrable)
	{
		Body::clearNormal(this->body, Object::safeCast(shapeNotCollidingAnymore));
	}
}

void Actor::collidingShapeOwnerDestroyed(Shape shape __attribute__ ((unused)), Shape shapeNotCollidingAnymore, bool isShapeImpenetrable)
{
	if(!this->body)
	{
		return;
	}

	Body::setSurroundingFrictionCoefficient(this->body,  Actor::getSurroundingFrictionCoefficient(this));

	if(isShapeImpenetrable)
	{
		Body::clearNormal(this->body, Object::safeCast(shapeNotCollidingAnymore));
	}
}

Body Actor::getBody()
{
	return this->body;
}

bool Actor::mustBounce()
{
	return true;
}
