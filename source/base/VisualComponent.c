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
#include <SpriteManager.h>
#include <VirtualList.h>
#include <WireframeManager.h>

#include "VisualComponent.h"

#include <Printing.h>
#include <Entity.h>
friend class Entity;


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class VirtualList;
friend class VirtualNode;


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static void VisualComponent::destroyComponents(SpatialObject owner, VirtualList components[])
{
	if(NULL == owner)
	{
		return;
	}

	ComponentManager::destroyComponents(owner, components, kSpriteComponent);
	ComponentManager::destroyComponents(owner, components, kWireframeComponent);
}
//---------------------------------------------------------------------------------------------------------
static bool VisualComponent::calculateRightBox(SpatialObject owner, RightBox* rightBox)
{
	bool modified = false;

	if(0 < SpriteManager::getCount(SpriteManager::getInstance(), owner))
	{
		VirtualList sprites = new VirtualList();

		SpriteManager::doGetComponents(SpriteManager::getInstance(), owner, sprites);

		VisualComponent::getRightBoxFromVisualComponents(sprites, rightBox);

		delete sprites;

		modified = true;
	}

	if(0 < WireframeManager::getCount(WireframeManager::getInstance(), owner))
	{
		VirtualList wireframes = new VirtualList();

		WireframeManager::doGetComponents(WireframeManager::getInstance(), owner, wireframes);

		VisualComponent::getRightBoxFromVisualComponents(wireframes, rightBox);

		delete wireframes;

		modified = true;
	}

	return modified;
}
//---------------------------------------------------------------------------------------------------------
static bool VisualComponent::isAnyVisible(SpatialObject owner)
{
	if(SpriteManager::isAnyVisible(SpriteManager::getInstance(), owner))
	{
		return true;
	}

	if(WireframeManager::isAnyVisible(WireframeManager::getInstance(), owner))
	{
		return true;
	}	

	return false;
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static void VisualComponent::getRightBoxFromVisualComponents(VirtualList visualComponents, RightBox* rightBox)
{
	if(isDeleted(visualComponents) || NULL == rightBox)
	{
		return;
	}

	for(VirtualNode node = visualComponents->head; node; node = node->next)
	{
		VisualComponent visualComponent = VisualComponent::safeCast(node->data);

		RightBox visualComponentRightBox = VisualComponent::getRightBox(visualComponent);

		NM_ASSERT(visualComponentRightBox.x0 < visualComponentRightBox.x1, "VisualComponent::getRightBoxFromVisualComponents: 0 width");
		NM_ASSERT(visualComponentRightBox.y0 < visualComponentRightBox.y1, "VisualComponent::getRightBoxFromVisualComponents: 0 height");
		NM_ASSERT(visualComponentRightBox.z0 < visualComponentRightBox.z1, "VisualComponent::getRightBoxFromVisualComponents: 0 depth");

		if(rightBox->x0 > visualComponentRightBox.x0)
		{
			rightBox->x0 = visualComponentRightBox.x0;
		}

		if(rightBox->x1 < visualComponentRightBox.x1)
		{
			rightBox->x1 = visualComponentRightBox.x1;
		}

		if(rightBox->y0 > visualComponentRightBox.y0)
		{
			rightBox->y0 = visualComponentRightBox.y0;
		}

		if(rightBox->y1 < visualComponentRightBox.y1)
		{
			rightBox->y1 = visualComponentRightBox.y1;
		}

		if(rightBox->z0 > visualComponentRightBox.z0)
		{
			rightBox->z0 = visualComponentRightBox.z0;
		}

		if(rightBox->z1 < visualComponentRightBox.z1)
		{
			rightBox->z1 = visualComponentRightBox.z1;
		}
	}
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PUBLIC METHODS
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
void VisualComponent::handleCommand(int32 command, va_list args)
{
	switch(command)
	{
		case cVisualComponentCommandShow:

			VisualComponent::show(this);
			break;

		case cVisualComponentCommandHide:

			VisualComponent::hide(this);
			break;

		case cVisualComponentCommandSetTransparency:

			VisualComponent::setTransparency(this, (uint8)va_arg(args, uint32));
			break;

		default:

			Base::handleCommand(this, command, args);
			break;
	}
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
	return this->transparency;
}
//---------------------------------------------------------------------------------------------------------
void VisualComponent::setTransparency(uint8 transparency)
{
	this->transparency = transparency;
}
//---------------------------------------------------------------------------------------------------------
RightBox VisualComponent::getRightBox()
{
	return (RightBox){-1, -1, -1, 1, 1, 1};
}
//---------------------------------------------------------------------------------------------------------
