/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
void Actor::constructor(const ActorSpec* actorSpec, int16 internalId, const char* const name)
{
	// construct base object
	Base::constructor((AnimatedEntitySpec*)&actorSpec->animatedEntitySpec, internalId, name);

	// construct the game state machine
	this->stateMachine = NULL;

	this->body = NULL;
	this->previousRotation = this->transformation.localRotation;

	// create body
	if(actorSpec->createBody)
	{
		Actor::createBody(this, actorSpec->animatedEntitySpec.entitySpec.physicalSpecification, actorSpec->axisSubjectToGravity);
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

void Actor::createBody(PhysicalSpecification* physicalSpecification, uint16 axisSubjectToGravity)
{
	if(NULL != physicalSpecification)
	{
		this->body = PhysicalWorld::createBody(Game::getPhysicalWorld(Game::getInstance()), (BodyAllocator)__TYPE(Body), SpatialObject::safeCast(this), physicalSpecification, axisSubjectToGravity);
	}
	else
	{
		PhysicalSpecification defaultActorPhysicalSpecification = {__I_TO_FIX10_6(1), 0, 0, Vector3D::zero(), 0};
		this->body = PhysicalWorld::createBody(Game::getPhysicalWorld(Game::getInstance()), (BodyAllocator)__TYPE(Body), SpatialObject::safeCast(this), &defaultActorPhysicalSpecification, axisSubjectToGravity);
	}
}

void Actor::initializeStateMachine(State state)
{
	if(isDeleted(this->stateMachine))
	{
		this->stateMachine = new StateMachine(this);
	}

	StateMachine::swapState(this->stateMachine, state);

	this->update = true;
}

void Actor::iAmDeletingMyself()
{
	Base::iAmDeletingMyself(this);

	// destroy body to prevent any more physical interactions
	if(this->body)
	{
		Body::setActive(this->body, false);
	}
}

//set class's local position
void Actor::setLocalPosition(const Vector3D* position)
{
	Vector3D displacement = this->transformation.localPosition;

	// Must transform shapes after everything is setup
	bool transformShapes = this->transformShapes;
	this->transformShapes = false;
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

	this->invalidateGlobalTransformation |= (displacement.x ? __X_AXIS: 0) | (displacement.y ? __Y_AXIS: 0) | (displacement.y ? __Z_AXIS: 0);

	this->transformShapes = transformShapes;
	Actor::transformShapes(this);
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
	bodyLastDisplacement = Vector3D::get(this->transformation.globalPosition, bodyPosition);

//  Optimization: this doesn't seem to do anything useful
//	globalPosition.x += bodyLastDisplacement.x;
//	globalPosition.y += bodyLastDisplacement.y;
//	globalPosition.z += bodyLastDisplacement.z;

	// move the body to the new global position
	// to account for any parenting
//	Body::setPosition(this->body, &globalPosition, SpatialObject::safeCast(this));

	// sync local position with global position
	Vector3D localPosition = this->transformation.localPosition;
	localPosition.x += bodyLastDisplacement.x;
	localPosition.y += bodyLastDisplacement.y;
	localPosition.z += bodyLastDisplacement.z;

	Base::setLocalPosition(this, &localPosition);
}

void Actor::doSyncRotationWithBody()
{
	if(!isDeleted(this->body) && Body::getMovementOnAllAxis(this->body))
	{
		Direction3D direction3D = *Body::getDirection3D(this->body);

		if((uint16)__LOCK_AXIS == ((ActorSpec*)this->entitySpec)->axisForSynchronizationWithBody)
		{
			return;
		}

		if(__NO_AXIS == ((ActorSpec*)this->entitySpec)->axisForSynchronizationWithBody)
		{
			Direction direction = Actor::getDirection(this);

			if(0 > direction3D.x)
			{
				direction.x = __LEFT;
			}
			else if(0 < direction3D.x)
			{
				direction.x = __RIGHT;
			}

			if(0 > direction3D.y)
			{
				direction.y = __UP;
			}
			else if(0 < direction3D.y)
			{
				direction.y = __DOWN;
			}

			if(0 > direction3D.z)
			{
				direction.z = __NEAR;
			}
			else if(0 < direction3D.z)
			{
				direction.z = __FAR;
			}

			Actor::setDirection(this, direction);
		}
		else
		{
			Rotation localRotation = Actor::getRotationFromDirection(this, &direction3D, ((ActorSpec*)this->entitySpec)->axisForSynchronizationWithBody);
			Base::setLocalRotation(this, &localRotation);
		}
	}
}

void Actor::syncRotationWithBody()
{
	Actor::doSyncRotationWithBody(this);
}

void Actor::syncRotationWithBodyAfterBouncing(SpatialObject collidingObject __attribute__((unused)))
{
	Actor::doSyncRotationWithBody(this);
}

// updates the animation attributes
// graphically refresh of characters that are visible
void Actor::transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag)
{
	bool transformShapes = this->transformShapes;

	if(this->body)
	{
		Actor::syncWithBody(this);

		// Prevent transformation of shapes again when calling Base::transform
		this->transformShapes = false;

		uint16 bodyMovement = Body::getMovementOnAllAxis(this->body);

		if(bodyMovement)
		{
			this->invalidateGlobalTransformation |= __INVALIDATE_POSITION;

			if(__Z_AXIS & bodyMovement)
			{
				this->invalidateGlobalTransformation |= __INVALIDATE_SCALE;
			}
		}

		Transformation surrogateEnvironmentTransformation = *environmentTransform;

		surrogateEnvironmentTransformation.globalRotation = Rotation::zero();

		environmentTransform = &surrogateEnvironmentTransformation;

		// call base
		Base::transform(this, environmentTransform, invalidateTransformationFlag);
	}
	else
	{
		// call base
		Base::transform(this, environmentTransform, invalidateTransformationFlag);
	}

	this->transformShapes = transformShapes;

	this->previousRotation = this->transformation.localRotation;
}

void Actor::resume()
{
	Base::resume(this);

	Actor::syncWithBody(this);
}

// execute character's logic
void Actor::update(uint32 elapsedTime)
{
	// call base
	Base::update(this, elapsedTime);

	if(this->stateMachine)
	{
		StateMachine::update(this->stateMachine);
	}

//	Body::print(this->body, 1, 0);
//	Shape::print(VirtualList::front(this->shapes), 1, 20);
//	Printing::resetCoordinates(Printing::getInstance());
}

// whether changed direction in the last cycle or not
bool Actor::hasChangedDirection(uint16 axis)
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
void Actor::changeDirectionOnAxis(uint16 axis)
{
	if(this->body)
	{
		Actor::syncRotationWithBody(this);
	}
	else
	{
		// save current rotation
		this->previousRotation = this->transformation.localRotation;

		Direction direction = Actor::getDirection(this);

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

		Actor::setDirection(this, direction);
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

		for(; NULL != node; node = node->next)
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

		for(; NULL != node; node = node->next)
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

bool Actor::registerCollidingShapes()
{
	return true;
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
			Shape::resolveCollision(collisionInformation->shape, collisionInformation, Actor::registerCollidingShapes(this));

			SpatialObject collidingObject = Shape::getOwner(collisionInformation->collidingShape);

			fix10_6 bounciness = Actor::getBouncinessOnCollision(this, collidingObject, &collisionInformation->solutionVector.direction);
			fix10_6 frictionCoefficient = Actor::getFrictionOnCollision(this, collidingObject, &collisionInformation->solutionVector.direction);

			if(Actor::mustBounce(this))
			{
				Body::bounce(this->body, ListenerObject::safeCast(collisionInformation->collidingShape), collisionInformation->solutionVector.direction, frictionCoefficient, bounciness);

				Actor::syncRotationWithBodyAfterBouncing(this, collidingObject);

				Actor::fireEvent(this, kEventActorBounced);
				NM_ASSERT(!isDeleted(this), "Actor::enterCollision: deleted this during kEventActorBounced");
			}
			else
			{
				uint16 axis = __NO_AXIS;
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
		int32 message = Telegram::getMessage(telegram);

		if(this->body && Body::isActive(this->body))
		{
			switch(message)
			{
				case kMessageBodyStartedMoving:

					if(this->allowCollisions && this->shapes)
					{
						Entity::activeCollisionChecks(this, true);
						return true;
					}
					break;

				case kMessageBodyStopped:

					if(!Body::getMovementOnAllAxis(this->body) && this->shapes)
					{
						Entity::activeCollisionChecks(this, false);
					}
					break;

				case kMessageBodyChangedDirection:

					Actor::changeDirectionOnAxis(this, *(int32*)Telegram::getExtraInfo(telegram));
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
	Actor::stopMovement(this, __ALL_AXIS);
}

// stop movement completely
void Actor::stopMovement(uint16 axis)
{
	if(this->body)
	{
		Body::stopMovement(this->body, axis);
	}
}

void Actor::applyForce(const Force* force, bool checkIfCanMove)
{
	ASSERT(this->body, "Actor::applyForce: null body");

	if(!this->body)
	{
		return;
	}

	if(checkIfCanMove)
	{
		if(!Actor::canMoveTowards(this, *force))
		{
			return;
		}
	}

	Body::applyForce(this->body, force);
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
	return this->body ? Body::getMovementOnAllAxis(this->body) : 0;
}

uint16 Actor::getMovementState()
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
void Actor::initialTransform(const Transformation* environmentTransform, uint32 recursive)
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
	Base::setPosition(this, position);

	if(this->body)
	{
		Body::setPosition(this->body, &this->transformation.globalPosition, SpatialObject::safeCast(this));
	}

	Actor::transformShapes(this);
}

// retrieve global position
const Vector3D* Actor::getPosition()
{
	return this->body ? Body::getPosition(this->body) : Base::getPosition(this);
}

// get bounciness
fix10_6 Actor::getBounciness()
{
	PhysicalSpecification* physicalSpecification = ((ActorSpec*)this->entitySpec)->animatedEntitySpec.entitySpec.physicalSpecification;

	return this->body ? Body::getBounciness(this->body) : physicalSpecification ? physicalSpecification->bounciness : 0;
}

// get velocity
Velocity Actor::getVelocity()
{
	return this->body ? Body::getVelocity(this->body) : Base::getVelocity(this);
}

fix10_6 Actor::getSpeed()
{
	return this->body ? Body::getSpeed(this->body) : Base::getSpeed(this);
}

fix10_6 Actor::getMaximumSpeed()
{
	return this->body ? Body::getMaximumSpeed(this->body) : 0;
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
		Body::clearNormal(this->body, ListenerObject::safeCast(shapeNotCollidingAnymore));
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
		Body::clearNormal(this->body, ListenerObject::safeCast(shapeNotCollidingAnymore));
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
