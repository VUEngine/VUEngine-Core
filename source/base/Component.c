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

	this->center = (PixelVector){0, 0, 0, 0};

	this->show = __SHOW;

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
}

/**
 * Start being rendered
 */
void Component::show()
{
	this->rendered = __SHOW == this->show;
	this->show = __SHOW;
}

/**
 * Stop being rendered
 */
void Component::hide()
{
	this->rendered = false;
	this->show = __HIDE;
}

/**
 * Retrieve the position
 *
 * @return Vector3D			Collider's position
 */
Vector3D Component::getPosition()
{
	return this->transformation->position;
}

/**
 * Retrieve the rotation
 *
 * @return Rotation			Collider's rotation
 */
Rotation Component::getRotation()
{
	return this->transformation->rotation;
}

/**
 * Retrieve the rotation
 *
 * @return Scale			Collider's scale
 */
Scale Component::getScale()
{
	return this->transformation->scale;
}
