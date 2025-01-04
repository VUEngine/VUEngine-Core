/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <AnimationController.h>
#include <ComponentManager.h>
#include <DebugConfig.h>
#include <Printing.h>
#include <GameObject.h>
#include <SpriteManager.h>
#include <VirtualList.h>
#include <WireframeManager.h>

#include "VisualComponent.h"



//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualList;
friend class VirtualNode;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void VisualComponent::propagateCommand(int32 command, GameObject owner, ...)
{
	va_list args;
	va_start(args, owner);
	int32 value = va_arg(args, int32);
	va_end(args);

	SpriteManager::propagateCommand(SpriteManager::getInstance(), command, owner, value);
	WireframeManager::propagateCommand(WireframeManager::getInstance(), command, owner, value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VisualComponent::calculateRightBox(GameObject owner, RightBox* rightBox)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool VisualComponent::isAnyVisible(GameObject owner)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::constructor(GameObject owner, const VisualComponentSpec* visualComponentSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, (const ComponentSpec*)visualComponentSpec);

	this->show = __SHOW;
	this->rendered = false;
	this->animationController = NULL;
	this->updateAnimationFrame = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::destructor()
{	
	if(!isDeleted(this->animationController))
	{
		delete this->animationController;
		this->animationController = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

		case cVisualComponentCommandPlay:

			VisualComponent::play
			(
				this, (const AnimationFunction**)va_arg(args, AnimationFunction**), (const char*)va_arg(args, char*), 
				va_arg(args, ListenerObject), (EventListener)va_arg(args, EventListener)
			);
			break;

		case cVisualComponentCommandPause:

			VisualComponent::pause(this, (bool)va_arg(args, uint32));
			break;

		case cVisualComponentCommandStop:

			VisualComponent::stop(this);
			break;

		case cVisualComponentCommandSetFrame:

			VisualComponent::setActualFrame(this, (int16)va_arg(args, uint32));
			break;

		case cVisualComponentCommandNextFrame:

			VisualComponent::nextFrame(this);
			break;

		case cVisualComponentCommandPreviousFrame:

			VisualComponent::previousFrame(this);
			break;

		default:

			Base::handleCommand(this, command, args);
			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

AnimationController VisualComponent::getAnimationController()
{
	return this->animationController;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::createAnimationController()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::forceChangeOfFrame(int16 actualFrame __attribute__((unused)))
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VisualComponent::play
(
	const AnimationFunction* animationFunctions[], const char* animationName, ListenerObject scope, EventListener callback
)
{
	ASSERT(NULL != animationFunctions, "VisualComponent::play: null animationFunctions");
	ASSERT(NULL != animationName, "VisualComponent::play: null animationName");

	bool playBackStarted = false;

	if(!isDeleted(this->animationController))
	{
		playBackStarted = AnimationController::play(this->animationController, animationFunctions, animationName, scope, callback);
		this->rendered = this->rendered && !this->updateAnimationFrame;
	}

	return playBackStarted;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VisualComponent::replay(const AnimationFunction* animationFunctions[])
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::replay(this->animationController, animationFunctions);
		this->rendered = this->rendered && !this->updateAnimationFrame;

		return this->updateAnimationFrame;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::pause(bool pause)
{
	if(!isDeleted(this->animationController))
	{
		// first animate the frame
		AnimationController::pause(this->animationController, pause);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::stop()
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::stop(this->animationController);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VisualComponent::isPlaying()
{
	if(!isDeleted(this->animationController))
	{
		// first animate the frame
		return AnimationController::isPlaying(this->animationController);
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool VisualComponent::isPlayingAnimation(char* animationName)
{
	if(!isDeleted(this->animationController))
	{
		return AnimationController::isPlayingFunction(this->animationController, animationName);
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::nextFrame()
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::nextFrame(this->animationController);
		this->updateAnimationFrame = true;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::previousFrame()
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::previousFrame(this->animationController);
		this->updateAnimationFrame = true;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::setActualFrame(int16 actualFrame)
{
	if(!isDeleted(this->animationController))
	{
		this->updateAnimationFrame = 
			this->updateAnimationFrame || AnimationController::setActualFrame(this->animationController, actualFrame);
	}
	else
	{
		VisualComponent::forceChangeOfFrame(this, actualFrame);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 VisualComponent::getActualFrame()
{
	if(!isDeleted(this->animationController))
	{
		return AnimationController::getActualFrame(this->animationController);
	}

	return -1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::setFrameDuration(uint8 frameDuration)
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::setFrameDuration(this->animationController, frameDuration);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint8 VisualComponent::getFrameDuration()
{
	if(!isDeleted(this->animationController))
	{
		return AnimationController::getFrameDuration(this->animationController);
	}

	return -1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::setFrameDurationDecrement(uint8 frameDurationDecrement)
{
	if(!isDeleted(this->animationController))
	{
		AnimationController::setFrameDurationDecrement(this->animationController, frameDurationDecrement);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const char* VisualComponent::getPlayingAnimationName()
{
	if(!isDeleted(this->animationController))
	{
		return AnimationController::getPlayingAnimationName(this->animationController);
	}

	return "None";
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::show()
{
	this->rendered = __SHOW == this->show;
	this->show = __SHOW;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::hide()
{
	this->rendered = false;
	this->show = __HIDE;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint8 VisualComponent::getTransparent()
{
	return this->transparency;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void VisualComponent::setTransparency(uint8 transparency)
{
	this->transparency = transparency;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

RightBox VisualComponent::getRightBox()
{
	return (RightBox){-1, -1, -1, 1, 1, 1};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

