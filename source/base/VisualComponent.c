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

#include "VisualComponent.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void VisualComponent::constructor(SpatialObject owner, const VisualComponentSpec* visualComponentSpec)
{
	Base::constructor(owner, visualComponentSpec);

	this->show = __SHOW;
}

/**
 * Class destructor
 */
void VisualComponent::destructor()
{	
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Start being rendered
 */
void VisualComponent::show()
{
	this->rendered = __SHOW == this->show;
	this->show = __SHOW;
}

/**
 * Stop being rendered
 */
void VisualComponent::hide()
{
	this->rendered = false;
	this->show = __HIDE;
}

/**
 * Get transparency mode
 *
 * @return		Transparency mode
 */
uint8 VisualComponent::getTransparent()
{
	return this->transparent;
}

/**
 * Set transparency mode
 *
 * @param value	Transparency mode
 */
void VisualComponent::setTransparent(uint8 value)
{
	this->transparent = value;
}
