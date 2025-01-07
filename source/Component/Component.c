/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <DebugConfig.h>
#include <Entity.h>

#include "Component.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static const Transformation _dummyTransformation = 
{
	// Position
	{0, 0, 0},
	// Rotation
	{0, 0, 0},
	// Scale
	{__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9},
	// Invalidity
	__NON_TRANSFORMED
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Component::constructor(Entity owner, const ComponentSpec* componentSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor();
	
	this->componentSpec = componentSpec;
	this->owner = owner;
	this->deleteMe = false;

	if(isDeleted(this->owner))
	{
		this->owner = NULL;
		this->transformation = &_dummyTransformation;
	}
	else
	{
		this->transformation = Entity::getTransformation(this->owner);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Component::destructor()
{	
	this->owner = NULL;

	if(NULL != this->events)
	{
		Component::fireEvent(this, kEventComponentDestroyed);
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ComponentSpec* Component::getSpec()
{
	return (ComponentSpec*)this->componentSpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Entity Component::getOwner()
{
	return this->owner;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 Component::getType()
{
	return this->componentSpec->componentType;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Component::handleCommand(int32 command __attribute__((unused)), va_list args __attribute__((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

RightBox Component::getRightBox()
{
	return (RightBox){-1, -1, -1, 1, 1, 1};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

