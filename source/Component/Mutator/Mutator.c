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

#include "Mutator.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Mutator::constructor(Entity owner, const MutatorSpec* mutatorSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, (const ComponentSpec*)&mutatorSpec->componentSpec);

	this->enabled = mutatorSpec->enabled;

	if(NULL != mutatorSpec->targetClass)
	{
		Entity::mutateTo(owner, mutatorSpec->targetClass);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Mutator::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Mutator::enable()
{
	this->enabled = true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Mutator::disable()
{
	this->enabled = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Mutator::isEnabled()
{
	return this->enabled;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
