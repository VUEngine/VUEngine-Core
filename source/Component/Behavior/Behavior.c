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

#include "Behavior.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Behavior::constructor(Entity owner, const BehaviorSpec* behaviorSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, (const ComponentSpec*)&behaviorSpec->componentSpec);

	this->enabled = behaviorSpec->enabled;

	if(NULL != behaviorSpec->targetClass)
	{
		Entity::mutateTo(owner, behaviorSpec->targetClass);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Behavior::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Behavior::enable()
{
	this->enabled = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Behavior::disable()
{
	this->enabled = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Behavior::isEnabled()
{
	return this->enabled;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
