/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Body.h>
#include <BodyManager.h>
#include <Camera.h>
#include <Collider.h>
#include <Printing.h>
#include <State.h>
#include <StateMachine.h>
#include <Telegram.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VUEngine.h>

#include "Actor.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualList;
friend class VirtualNode;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::constructor(const ActorSpec* actorSpec, int16 internalId, const char* const name)
{
	// Always explicitly call the base's constructor 
	Base::constructor((AnimatedEntitySpec*)&actorSpec->animatedEntitySpec, internalId, name);

	// construct the game state machine
	this->stateMachine = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::destructor()
{
	// destroy state machine
	if(!isDeleted(this->stateMachine))
	{
		delete this->stateMachine;
		this->stateMachine = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Actor::handleMessage(Telegram telegram)
{
	if(!this->stateMachine || !StateMachine::handleMessage(this->stateMachine, telegram))
	{
		return Base::handleMessage(this, telegram);
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::setDirection(const Vector3D* direction)
{
	if(NULL == direction)
	{
		return;
	}

	if((uint16)__LOCK_AXIS == ((ActorSpec*)this->entitySpec)->axisForSynchronizationWithBody)
	{
		return;
	}
		
	if(__NO_AXIS == ((ActorSpec*)this->entitySpec)->axisForSynchronizationWithBody)
	{
		NormalizedDirection normalizedDirection = Actor::getNormalizedDirection(this);

		if(0 > direction->x)
		{
			normalizedDirection.x = __LEFT;
		}
		else if(0 < direction->x)
		{
			normalizedDirection.x = __RIGHT;
		}

		if(0 > direction->y)
		{
			normalizedDirection.y = __UP;
		}
		else if(0 < direction->y)
		{
			normalizedDirection.y = __DOWN;
		}

		if(0 > direction->z)
		{
			normalizedDirection.z = __NEAR;
		}
		else if(0 < direction->z)
		{
			normalizedDirection.z = __FAR;
		}

		Actor::setNormalizedDirection(this, normalizedDirection);
	}
	else
	{
		Rotation localRotation = Actor::getRotationFromDirection(this, direction, ((ActorSpec*)this->entitySpec)->axisForSynchronizationWithBody);
		Base::setLocalRotation(this, &localRotation);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::setLocalPosition(const Vector3D* position)
{
	Vector3D displacement = this->localTransformation.position;

	Base::setLocalPosition(this, position);

	displacement.x -= this->localTransformation.position.x;
	displacement.y -= this->localTransformation.position.y;
	displacement.z -= this->localTransformation.position.z;

	this->transformation.position.x -= displacement.x;
	this->transformation.position.y -= displacement.y;
	this->transformation.position.z -= displacement.z;

	if(!isDeleted(this->body))
	{
		Body::setPosition(this->body, &this->transformation.position, GameObject::safeCast(this));
	}

	this->transformation.invalid |= (displacement.x ? __X_AXIS: 0) | (displacement.y ? __Y_AXIS: 0) | (displacement.y ? __Z_AXIS: 0);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::changeEnvironment(Transformation* environmentTransform)
{
	Base::changeEnvironment(this, environmentTransform);

	if(!isDeleted(this->body))
	{
		Body::setPosition(this->body, &this->transformation.position, GameObject::safeCast(this));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::update()
{
	// call base
	Base::update(this);

	if(!isDeleted(this->stateMachine))
	{
		StateMachine::update(this->stateMachine);
	}

//	Body::print(this->body, 1, 0);
//	Printing::resetCoordinates(Printing::getInstance());
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void Actor::createStateMachine(State state)
{
	if(isDeleted(this->stateMachine))
	{
		this->stateMachine = new StateMachine(this);
	}

	StateMachine::swapState(this->stateMachine, state);

	this->update = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

Rotation Actor::getRotationFromDirection(const Vector3D* direction, uint8 axis)
{
	Rotation rotation = this->localTransformation.rotation;

	if(__X_AXIS & axis)
	{
		fixed_ext_t z = direction->z;

		if(direction->x)
		{
			z = Math::squareRootFixed(__FIXED_EXT_MULT(direction->x, direction->x) + __FIXED_EXT_MULT(direction->z, direction->z));

			z = 0 > direction->z ? -z : z;
		}

		rotation.x = __I_TO_FIXED(Math::getAngle(__FIXED_TO_FIX7_9(direction->y), __FIXED_TO_FIX7_9(z))) - __QUARTER_ROTATION_DEGREES;
	}
	
	if(__Y_AXIS & axis)
	{
		fixed_ext_t x = direction->x;

		if(direction->y)
		{
			x = Math::squareRootFixed(__FIXED_EXT_MULT(direction->y, direction->y) + __FIXED_EXT_MULT(direction->x, direction->x));

			x = 0 > direction->x ? -x : x;
		}

		rotation.y = __I_TO_FIXED(Math::getAngle(__FIXED_TO_FIX7_9((direction->z)), __FIXED_TO_FIX7_9(x)));
	}

	if(__Z_AXIS & axis)
	{
		fixed_ext_t y = direction->y;

		if(direction->z)
		{
			y = Math::squareRootFixed(__FIXED_EXT_MULT(direction->z, direction->z) + __FIXED_EXT_MULT(direction->y, direction->y));

			y = 0 > direction->y ? -y : y;
		}

		rotation.z = __I_TO_FIXED(Math::getAngle(__FIXED_TO_FIX7_9((direction->x)), __FIXED_TO_FIX7_9(y)));
	}

	if(__X_AXIS & axis)
	{
		if(__QUARTER_ROTATION_DEGREES < rotation.z)
		{
			rotation.x = rotation.x - __HALF_ROTATION_DEGREES;
		}
	}

	if(__Y_AXIS & axis)
	{
		if(__QUARTER_ROTATION_DEGREES < rotation.x)
		{
			rotation.y = rotation.y - __HALF_ROTATION_DEGREES;
		}
	}

	if(__Z_AXIS & axis)
	{
		if(__QUARTER_ROTATION_DEGREES < rotation.y)
		{
			rotation.z = rotation.z - __HALF_ROTATION_DEGREES;
		}
	}

	return Rotation::clamp(rotation.x, rotation.y, rotation.z);	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

