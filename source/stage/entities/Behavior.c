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

#include "Behavior.h"


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void Behavior::constructor(SpatialObject owner, const BehaviorSpec* behaviorSpec)
{
	Base::constructor(owner, behaviorSpec);

	this->enabled = behaviorSpec->enabled;
}

/**
 * Class destructor
 */
void Behavior::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

bool Behavior::isEnabled()
{
	return this->enabled;
}

void Behavior::setEnabled(bool value)
{
	this->enabled = value;
}

static Behavior Behavior::create(SpatialObject owner, const BehaviorSpec* behaviorSpec)
{
	ASSERT(behaviorSpec, "Behavior::create: NULL behavior");
	ASSERT(behaviorSpec->allocator, "Behavior::create: no behavior allocator");

	if(!behaviorSpec || !behaviorSpec->allocator)
	{
		return NULL;
	}

	return 	((Behavior (*)(SpatialObject, BehaviorSpec**)) behaviorSpec->allocator)(owner, (BehaviorSpec**)behaviorSpec);
}
