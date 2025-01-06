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
#include <MessageDispatcher.h>
#include <Telegram.h>
#include <TimerManager.h>
#include <VIPManager.h>
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
	// Init class variables
	this->fxFadeTargetBrightness = (Brightness){0, 0, 0};
	this->fxFadeDelay = 0;
	this->fxFadeCallbackScope = NULL;
	this->fadeEffectIncrement = __CAMERA_EFFECT_FADE_INCREMENT;
	this->startingANewEffect = false;
	
	// Always explicitly call the base's constructor 
	Base::constructor();
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
			CameraEffectManager::fxFadeAsync(this);
			break;
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

Brightness CameraEffectManager::getDefaultBrightness()
{
	// Default brightness settings
	Brightness brightness = (Brightness)
	{
		__BRIGHTNESS_DARK_RED,
		__BRIGHTNESS_MEDIUM_RED,
		__BRIGHTNESS_BRIGHT_RED,
	};

	if(!isDeleted(VUEngine::getCurrentState()))
	{
		// If exists, get brightness settings from stage spec
		Stage stage = GameState::getStage(VUEngine::getCurrentState());
		
		if(!isDeleted(stage))
		{
			StageSpec* stageSpec = Stage::getSpec(stage);
			brightness = stageSpec->rendering.colorConfig.brightness;
		}
	}

	return brightness;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::startEffect(int32 effect, va_list args)
{
	switch(effect)
	{
		case kShow:

			CameraEffectManager::showCamera(this);
			break;

		case kHide:

			CameraEffectManager::hideCamera(this);
			break;

		case kFadeIn:
		case kFadeOut:

			CameraEffectManager::fxFadeStart(this, effect, va_arg(args, int32));
			break;

		case kFadeTo:

			CameraEffectManager::fxFadeAsyncStart(this,
				va_arg(args, int32),
				va_arg(args, Brightness*),
				va_arg(args, int32),
				va_arg(args, EventListener),
				va_arg(args, ListenerObject)
			);
			break;
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
	Brightness defaultBrightness = CameraEffectManager::getDefaultBrightness(this);

	switch(effect)
	{
		case kFadeIn:

			CameraEffectManager::hideCamera(this);

			TimerManager::repeatMethodCall(
				
				defaultBrightness.darkRed,
				(delay * defaultBrightness.darkRed),
				ListenerObject::safeCast(this),
				(void (*)(ListenerObject, uint32))&CameraEffectManager::fxFadeIn
			);
			break;

		case kFadeOut:

			CameraEffectManager::showCamera(this);

			TimerManager::repeatMethodCall(
				
				defaultBrightness.darkRed,
				(delay * defaultBrightness.darkRed),
				ListenerObject::safeCast(this),
				(void (*)(ListenerObject, uint32))&CameraEffectManager::fxFadeOut
			);

			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeAsyncStart
(
	int32 initialDelay, const Brightness* targetBrightness, int32 delayBetweenSteps, EventListener callback, ListenerObject callbackScope
)
{
	// Stop previous effect
	CameraEffectManager::stopEffect(this, kFadeTo);

	this->startingANewEffect = true;	

	// Set target brightness
	if(targetBrightness == NULL)
	{
		this->fxFadeTargetBrightness = CameraEffectManager::getDefaultBrightness(this);
	}
	else
	{
		this->fxFadeTargetBrightness = *targetBrightness;
	}

	this->fxFadeTargetBrightness = CameraEffectManager::convertBrightnessToVipFormat(this, this->fxFadeTargetBrightness);

	// Set effect parameters
	this->fxFadeDelay = (0 >= delayBetweenSteps) ? 1 : (uint8)(delayBetweenSteps);

	// Set callback
	if(callback != NULL)
	{
		if(callbackScope != NULL)
		{
			this->fxFadeCallbackScope = callbackScope;
			CameraEffectManager::addEventListener(this, callbackScope, callback, kEventEffectFadeComplete);
		}
		else
		{
			NM_ASSERT(false, "CameraEffectManager::fxFadeAsyncStart: null callbackScope");
		}
	}

	// Start effect
	// TODO: check if the message really needs to be delayed.
	initialDelay = 0 >= initialDelay ? 1 : initialDelay;
	MessageDispatcher::dispatchMessage(initialDelay, ListenerObject::safeCast(this), ListenerObject::safeCast(this), kFadeTo, NULL);

	// Fire effect started event
	CameraEffectManager::fireEvent(this, kEventEffectFadeStart);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeAsyncStop()
{
	// Remove event listener
	CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeComplete);

	// Discard pending delayed messages to stop effect
	MessageDispatcher::discardDelayedMessagesForReceiver(ListenerObject::safeCast(this), kFadeTo);

	// Reset effect variables
	this->fxFadeTargetBrightness = (Brightness){0, 0, 0};
	this->fxFadeDelay = 0;
	this->fxFadeCallbackScope = NULL;

	// Fire effect stopped event
	CameraEffectManager::fireEvent(this, kEventEffectFadeStop);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeIn()
{
	Brightness incrementalBrightness =
	{
		_vipRegisters[__BRTA] + 1,
		_vipRegisters[__BRTB] + 2,
		_vipRegisters[__BRTC] - _vipRegisters[__BRTB] - _vipRegisters[__BRTA] + 1
	};
	
	VIPManager::setupBrightness(&incrementalBrightness);

#ifdef __DIMM_FOR_PROFILING

		_vipRegisters[__GPLT0] = 0x50;
		_vipRegisters[__GPLT1] = 0x50;
		_vipRegisters[__GPLT2] = 0x54;
		_vipRegisters[__GPLT3] = 0x54;
		_vipRegisters[__JPLT0] = 0x54;
		_vipRegisters[__JPLT1] = 0x54;
		_vipRegisters[__JPLT2] = 0x54;
		_vipRegisters[__JPLT3] = 0x54;

		_vipRegisters[0x30 | __PRINTING_PALETTE] = 0xE4;
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeOut()
{
	Brightness decrementalBrightness =
	{
		(_vipRegisters[__BRTA] > 0) ? _vipRegisters[__BRTA] - 1 : 0,
		(_vipRegisters[__BRTB] > 1) ? _vipRegisters[__BRTB] - 2 : 0,
		(_vipRegisters[__BRTC] > 0) ? _vipRegisters[__BRTC] - 1 : 0
	};
	
	VIPManager::setupBrightness(&decrementalBrightness);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeAsync()
{
	ASSERT(this, "CameraEffectManager::fxFadeAsync: invalid this");

	// Note: need to cast brightness registers to uint8 because only their lower 8 bits are valid
	bool lightRedDone 	= (uint8)_vipRegisters[__BRTC] == this->fxFadeTargetBrightness.brightRed;
	bool mediumRedDone 	= (uint8)_vipRegisters[__BRTB] == this->fxFadeTargetBrightness.mediumRed;
	bool darkRedDone 	= (uint8)_vipRegisters[__BRTA] == this->fxFadeTargetBrightness.darkRed;

	// Fade light red
	if(!lightRedDone)
	{
		if((uint8)_vipRegisters[__BRTC] + this->fadeEffectIncrement < this->fxFadeTargetBrightness.brightRed)
		{
			_vipRegisters[__BRTC] += this->fadeEffectIncrement;
		}
		else if((uint8)_vipRegisters[__BRTC] > this->fxFadeTargetBrightness.brightRed + this->fadeEffectIncrement)
		{
			_vipRegisters[__BRTC] -= this->fadeEffectIncrement;
		}
		else
		{
			_vipRegisters[__BRTC] = this->fxFadeTargetBrightness.brightRed;
			lightRedDone = true;
		}
	}

	if(!mediumRedDone)
	{
		// Fade medium red
		for(uint16 i = 0; i < 2; i++)
		{
			if((uint8)_vipRegisters[__BRTB] + this->fadeEffectIncrement < this->fxFadeTargetBrightness.mediumRed)
			{
				_vipRegisters[__BRTB] += this->fadeEffectIncrement;
			}
			else if((uint8)_vipRegisters[__BRTB] > this->fxFadeTargetBrightness.mediumRed + this->fadeEffectIncrement)
			{
				_vipRegisters[__BRTB] -= this->fadeEffectIncrement;
			}
			else
			{
				_vipRegisters[__BRTB] = this->fxFadeTargetBrightness.mediumRed;
				mediumRedDone = true;
				break;
			}
		}
	}

	if(!darkRedDone)
	{
		// Fade dark red
		if((uint8)_vipRegisters[__BRTA] + this->fadeEffectIncrement < this->fxFadeTargetBrightness.darkRed)
		{
			_vipRegisters[__BRTA] += this->fadeEffectIncrement;
		}
		else if((uint8)_vipRegisters[__BRTA] > this->fxFadeTargetBrightness.darkRed + this->fadeEffectIncrement)
		{
			_vipRegisters[__BRTA] -= this->fadeEffectIncrement;
		}
		else
		{
			_vipRegisters[__BRTA] = this->fxFadeTargetBrightness.darkRed;
			darkRedDone = true;
		}
	}

	// Finish effect or call next round
	if(lightRedDone && mediumRedDone && darkRedDone)
	{
		this->startingANewEffect = false;

		// Fire effect ended event
		CameraEffectManager::fireEvent(this, kEventEffectFadeComplete);

#ifdef __DIMM_FOR_PROFILING

		_vipRegisters[__GPLT0] = 0x50;
		_vipRegisters[__GPLT1] = 0x50;
		_vipRegisters[__GPLT2] = 0x54;
		_vipRegisters[__GPLT3] = 0x54;
		_vipRegisters[__JPLT0] = 0x54;
		_vipRegisters[__JPLT1] = 0x54;
		_vipRegisters[__JPLT2] = 0x54;
		_vipRegisters[__JPLT3] = 0x54;

		_vipRegisters[0x30 | __PRINTING_PALETTE] = 0xE4;
#endif

		if(!this->startingANewEffect)
		{
			CameraEffectManager::removeEventListenerScopes(this, NULL, kEventEffectFadeComplete);
		}
	}
	else
	{
		MessageDispatcher::dispatchMessage
		(
			this->fxFadeDelay, ListenerObject::safeCast(this), ListenerObject::safeCast(this), kFadeTo, NULL
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Brightness CameraEffectManager::convertBrightnessToVipFormat(Brightness brightness)
{
	brightness.brightRed -= (brightness.darkRed + brightness.mediumRed);

	return brightness;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::showCamera()
{
	Brightness defaultBrightness = CameraEffectManager::getDefaultBrightness(this);
	defaultBrightness = CameraEffectManager::convertBrightnessToVipFormat(this, defaultBrightness);
	VIPManager::setupBrightness(&defaultBrightness);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::hideCamera()
{
	VIPManager::lowerBrightness();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
