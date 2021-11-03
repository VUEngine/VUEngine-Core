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
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void Behavior::constructor(const BehaviorSpec* behaviorSpec)
{
	Base::constructor();

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

void Behavior::start(Container owner __attribute__((unused)))
{
}

void Behavior::update(Container owner __attribute__((unused)), uint32 elapsedTime __attribute__((unused)))
{
}

void Behavior::pause(Container owner __attribute__((unused)))
{
}

void Behavior::resume(Container owner __attribute__((unused)))
{
}

static Behavior Behavior::create(const BehaviorSpec* behaviorSpec)
{
	ASSERT(behaviorSpec, "Behavior::create: NULL behavior");
	ASSERT(behaviorSpec->allocator, "Behavior::create: no behavior allocator");

	if(!behaviorSpec || !behaviorSpec->allocator)
	{
		return NULL;
	}

	return 	((Behavior (*)(BehaviorSpec**)) behaviorSpec->allocator)((BehaviorSpec**)behaviorSpec);
}
