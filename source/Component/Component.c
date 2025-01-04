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
#include <GameObject.h>

#include "Component.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static const Transformation _dummyTransformation = 
{
	// position
	{0, 0, 0},
	// rotation
	{0, 0, 0},
	// scale
	{__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9},
	// invalidity
	__NON_TRANSFORMED
};


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Component::constructor(GameObject owner, const ComponentSpec* componentSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor();
	
	this->componentSpec = componentSpec;
	this->owner = owner;

	if(isDeleted(this->owner))
	{
		this->owner = NULL;
		this->transformation = &_dummyTransformation;
	}
	else
	{
		this->transformation = GameObject::getTransformation(this->owner);
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

GameObject Component::getOwner()
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

