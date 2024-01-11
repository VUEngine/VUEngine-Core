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

#include <DebugConfig.h>
#include <DebugUtilities.h>
#include <SpatialObject.h>

#include "Component.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

static const Transformation _dummyTransformation = 
{
	// position
	{0, 0, 0},
	// rotation
	{0, 0, 0},
	// scale
	{__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9}
};


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void Component::constructor(SpatialObject owner, const ComponentSpec* componentSpec)
{
	Base::constructor();
	
	this->componentSpec = componentSpec;
	this->owner = owner;

	if(!isDeleted(this->owner))
	{
		this->transformation = SpatialObject::getTransformation(this->owner);
	}
	else
	{
		this->transformation = &_dummyTransformation;
	}
}

/**
 * Class destructor
 */
void Component::destructor()
{	
	// must always be called at the end of the destructor
	Base::destructor();
}
