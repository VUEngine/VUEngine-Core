/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

void Behavior::update(Container owner __attribute__((unused)), u32 elapsedTime __attribute__((unused)))
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
	
	return 	((Behavior (*)(BehaviorSpec*)) behaviorSpec->allocator)((BehaviorSpec*)behaviorSpec);
}
