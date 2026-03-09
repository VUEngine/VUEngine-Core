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

#include <Camera.h>
#include <DebugConfig.h>
#include <Actor.h>
#include <GameState.h>
#include <Singleton.h>
#include <Telegram.h>
#include <Timer.h>
#include <DisplayUnit.h>
#include <VUEngine.h>

#include "CameraEffectManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Camera;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Init class variables
	this->fadeDelay = 0;
	this->fadeScope = NULL;
	this->fadeEffectIncrement = __CAMERA_EFFECT_FADE_INCREMENT;
	this->startingANewEffect = false;
	this->targetDisplayColorConfig = DisplayUnit::getColorConfig();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::destructor()
{
	// Stop any effects
	CameraEffectManager::stopEffect(this, kFadeTo);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CameraEffectManager::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kFadeTo:
		{				
			CameraEffectManager::fxFadeAsync(this);
			break;
		}	
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::reset()
{
	this->fadeEffectIncrement = __CAMERA_EFFECT_FADE_INCREMENT;

	CameraEffectManager::stopEffect(this, kFadeTo);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::setFadeIncrement(uint8 fadeEffectIncrement)
{
	this->fadeEffectIncrement = fadeEffectIncrement;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::startEffect(int32 effect, va_list args)
{
	switch(effect)
	{
		case kShow:
		{
			DisplayUnit::upBrightness(0);
			break;
		}

		case kHide:
		{
			DisplayUnit::lowerBrightness(0);
			break;
		}
		
		case kFadeIn:
		case kFadeOut:
		{
			CameraEffectManager::fxFadeStart(this, effect, va_arg(args, int32));
			break;
		}

		case kFadeTo:
		{
			CameraEffectManager::fxFadeAsyncStart
			(
				this,
				va_arg(args, int32),
				va_arg(args, DisplayColorConfig*),
				va_arg(args, int32),
				va_arg(args, ListenerObject)
			);
			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::stopEffect(int32 effect)
{
	switch(effect)
	{
		case kFadeTo:

			CameraEffectManager::fxFadeAsyncStop(this);
			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeStart(int32 effect, int32 delay)
{
	switch(effect)
	{
		case kFadeIn:
		{
			DisplayUnit::lowerBrightness(0);
			while(DisplayUnit::upBrightness(1));
			break;
		}

		case kFadeOut:
		{
			while(DisplayUnit::lowerBrightness(1));
			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeAsyncStart
(
	int32 initialDelay, const DisplayColorConfig* targetDisplayColorConfig, int32 delayBetweenSteps, ListenerObject scope
)
{
	// Stop previous effect
	CameraEffectManager::stopEffect(this, kFadeTo);

	this->startingANewEffect = true;

	// Set target brightness
	if(NULL == targetDisplayColorConfig)
	{
		this->targetDisplayColorConfig = DisplayUnit::getColorConfig();
	}
	else
	{
		this->targetDisplayColorConfig = *targetDisplayColorConfig;
	}

	// Set effect parameters
	this->fadeDelay = (0 >= delayBetweenSteps) ? 1 : (uint8)(delayBetweenSteps);

	if(scope != NULL)
	{
		this->fadeScope = scope;

		if(0 < DisplayUnit::getBrightnessDirection(this->targetDisplayColorConfig))
		{		
			CameraEffectManager::addEventListener(this, scope, kEventEffectFadeInComplete);
		}
		else
		{		
			CameraEffectManager::addEventListener(this, scope, kEventEffectFadeOutComplete);
		}
	}
	
	// Start effect
	// TODO: check if the message really needs to be delayed.
	initialDelay = 0 >= initialDelay ? 1 : initialDelay;
	CameraEffectManager::sendMessageToSelf(this, kFadeTo, initialDelay, 0);

	// Fire effect started event
	CameraEffectManager::fireEvent(this, kEventEffectFadeStart);
	CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeStart);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeAsyncStop()
{
	// Remove event listener
	CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeInComplete);
	CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeOutComplete);

	// Discard pending delayed messages to stop effect
	CameraEffectManager::discardMessages(this, kFadeTo);

	// Reset effect variables
	this->fadeDelay = 0;
	this->fadeScope = NULL;

	// Fire effect stopped event
	CameraEffectManager::fireEvent(this, kEventEffectFadeStop);
	CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeStop);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeIn(uint32 call __attribute__((unused)))
{
	DisplayUnit::upBrightness(__ABS(this->fadeEffectIncrement));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeOut()
{
	DisplayUnit::lowerBrightness(-__ABS(this->fadeEffectIncrement));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeAsync()
{
	ASSERT(this, "CameraEffectManager::fxFadeAsync: invalid this");

	if(DisplayUnit::modifyBrightness(this->fadeEffectIncrement, this->targetDisplayColorConfig))
	{
		this->startingANewEffect = false;

		// Fire effect ended event
		CameraEffectManager::fireEvent(this, kEventEffectFadeInComplete);
		CameraEffectManager::fireEvent(this, kEventEffectFadeOutComplete);

		if(!this->startingANewEffect)
		{
			CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeInComplete);
			CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeOutComplete);
		}
	}
	else
	{
		CameraEffectManager::sendMessageToSelf(this, kFadeTo, this->fadeDelay, 0);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
