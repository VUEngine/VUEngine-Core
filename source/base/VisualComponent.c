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

#include <DebugConfig.h>
#include <SpatialObject.h>

#include "VisualComponent.h"


//=========================================================================================================
// CLASS'S PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void VisualComponent::constructor(SpatialObject owner, const VisualComponentSpec* visualComponentSpec)
{
	Base::constructor(owner, visualComponentSpec);

	this->show = __SHOW;
	this->rendered = false;
}
//---------------------------------------------------------------------------------------------------------
void VisualComponent::destructor()
{	
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void VisualComponent::show()
{
	this->rendered = __SHOW == this->show;
	this->show = __SHOW;
}
//---------------------------------------------------------------------------------------------------------
void VisualComponent::hide()
{
	this->rendered = false;
	this->show = __HIDE;
}
//---------------------------------------------------------------------------------------------------------
uint8 VisualComponent::getTransparent()
{
	return this->transparent;
}
//---------------------------------------------------------------------------------------------------------
void VisualComponent::setTransparent(uint8 value)
{
	this->transparent = value;
}
//---------------------------------------------------------------------------------------------------------
