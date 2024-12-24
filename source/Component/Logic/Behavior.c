/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include "Behavior.h"


//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static Behavior Behavior::create(SpatialObject owner, const BehaviorSpec* behaviorSpec)
{
	ASSERT(behaviorSpec, "Behavior::create: NULL behavior");
	ASSERT(behaviorSpec->componentSpec.allocator, "Behavior::create: no behavior allocator");

	if(NULL == behaviorSpec || !behaviorSpec->componentSpec.allocator)
	{
		return NULL;
	}

	return 	((Behavior (*)(SpatialObject, BehaviorSpec**)) behaviorSpec->componentSpec.allocator)(owner, (BehaviorSpec**)behaviorSpec);
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Behavior::constructor(SpatialObject owner, const BehaviorSpec* behaviorSpec)
{
	Base::constructor(owner, (const ComponentSpec*)&behaviorSpec->componentSpec);

	this->enabled = behaviorSpec->enabled;
}
//---------------------------------------------------------------------------------------------------------
void Behavior::destructor()
{
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void Behavior::enable()
{
	this->enabled = true;
}
//---------------------------------------------------------------------------------------------------------
void Behavior::disable()
{
	this->enabled = false;
}
//---------------------------------------------------------------------------------------------------------
bool Behavior::isEnabled()
{
	return this->enabled;
}
//---------------------------------------------------------------------------------------------------------