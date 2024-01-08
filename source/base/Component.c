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

static const Vector3D _dummyPosition = {0, 0, 0};
static const Rotation _dummyRotation = {0, 0, 0};
static const Scale _dummyScale = {__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9};


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

	this->center = (PixelVector){0, 0, 0, 0};

	this->show = __SHOW;

	if(!isDeleted(this->owner))
	{
		this->position = SpatialObject::getPosition(this->owner);
		this->rotation = SpatialObject::getRotation(this->owner);
		this->scale = SpatialObject::getScale(this->owner);
	}
	else
	{
		this->position = &_dummyPosition;
		this->rotation = &_dummyRotation;
		this->scale = &_dummyScale;
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

/**
 * Get Sprite's transparency mode
 *
 * @return		Transparency mode
 */
uint8 Component::getTransparent()
{
	return this->transparent;
}

/**
 * Set Sprite transparent
 *
 * @param value	Transparency mode
 */
void Component::setTransparent(uint8 value)
{
	this->transparent = value;

	this->renderFlag = true;
}

/**
 * Start being rendered
 */
void Component::show()
{
	this->show = __SHOW;
}

/**
 * Stop being rendered
 */
void Component::hide()
{
	this->draw = false;
	this->positioned = false;
	this->show = __HIDE;
}
