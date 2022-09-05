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

#include <CameraEffectManager.h>
#include <Camera.h>
#include <VUEngine.h>
#include <Clock.h>
#include <TimerManager.h>
#include <MessageDispatcher.h>
#include <VIPManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Camera;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			CameraEffectManager::getInstance()
 * @memberof	CameraEffectManager
 * @public
 * @return		CameraEffectManager instance
 */


/**
 * Class constructor
 */
void CameraEffectManager::constructor()
{
	// init class variables
	this->fxFadeTargetBrightness = (Brightness){0, 0, 0};
	this->fxFadeDelay = 0;
	this->fxFadeCallbackScope = NULL;
	this->fadeEffectIncrement = __CAMERA_EFFECT_FADE_INCREMENT;
	this->startingANewEffect = false;
	
	// construct base object
	Base::constructor();
}

/**
 * Class destructor
 */
void CameraEffectManager::destructor()
{
	// stop any effects
	CameraEffectManager::stopEffect(this, kFadeTo);

	// destroy base
	Base::destructor();
}

/**
 * Reset
 */
void CameraEffectManager::reset()
{
	this->fadeEffectIncrement = __CAMERA_EFFECT_FADE_INCREMENT;

	CameraEffectManager::stopEffect(this, kFadeTo);
}

/**
 * Set fade increment
 * @param increment 
 */
void CameraEffectManager::setFadeIncrement(uint8 fadeEffectIncrement)
{
	this->fadeEffectIncrement = fadeEffectIncrement;
}

/**
 * Get the current default brightness settings
 *
 * @return		Brightness
 */
Brightness CameraEffectManager::getDefaultBrightness()
{
	// default brightness settings
	Brightness brightness = (Brightness)
	{
		__BRIGHTNESS_DARK_RED,
		__BRIGHTNESS_MEDIUM_RED,
		__BRIGHTNESS_BRIGHT_RED,
	};

	if(!isDeleted(VUEngine::getCurrentState(VUEngine::getInstance())))
	{
		// if exists, get brightness settings from stage spec
		Stage stage = GameState::getStage(VUEngine::getCurrentState(VUEngine::getInstance()));
		if(stage != NULL)
		{
			StageSpec* stageSpec = Stage::getStageSpec(stage);
			brightness = stageSpec->rendering.colorConfig.brightness;
		}
	}

	return brightness;
}

/**
 * Convert brightness to VIP format
 *
 * @return		Brightness
 */
Brightness CameraEffectManager::convertBrightnessToVipFormat(Brightness brightness)
{
	brightness.brightRed -= (brightness.darkRed + brightness.mediumRed);

	return brightness;
}

/**
 * Start an effect
 *
 * @param effect	Effect Id
 * @param args		va_list of effect parameters
 */
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
				va_arg(args, void*),
				va_arg(args, ListenerObject)
			);
			break;
	}
}

/**
 * Stop an effect
 *
 * @param effect	Effect Id
 */
void CameraEffectManager::stopEffect(int32 effect)
{
	switch(effect)
	{
		case kFadeTo:

			CameraEffectManager::fxFadeAsyncStop(this);
			break;
	}
}

/**
 * Process a telegram
 *
 * @param telegram	Received telegram
 * @return			True if successfully processed, false otherwise
 */
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

/**
 * Start a synchronous fade effect
 *
 * @private
 * @param effect	Effect ID
 * @param delay		Start effect after this delay
 */
void CameraEffectManager::fxFadeStart(int32 effect, int32 delay)
{
	Brightness defaultBrightness = CameraEffectManager::getDefaultBrightness(this);

	switch(effect)
	{
		case kFadeIn:

			CameraEffectManager::hideCamera(this);

			TimerManager::repeatMethodCall(
				TimerManager::getInstance(),
				defaultBrightness.darkRed,
				(delay * defaultBrightness.darkRed),
				ListenerObject::safeCast(this),
				(void (*)(ListenerObject, uint32))&CameraEffectManager::fxFadeIn
			);
			break;

		case kFadeOut:

			CameraEffectManager::showCamera(this);

			TimerManager::repeatMethodCall(
				TimerManager::getInstance(),
				defaultBrightness.darkRed,
				(delay * defaultBrightness.darkRed),
				ListenerObject::safeCast(this),
				(void (*)(ListenerObject, uint32))&CameraEffectManager::fxFadeOut
			);

			break;
	}
}

/**
 * Start an asynchronous fade effect
 *
 * @private
 * @param initialDelay		Start effect after this initial delay
 * @param targetBrightness	Fade to this target brightness
 * @param delayBetweenSteps	Delay between the individual fading steps
 * @param callback			Callback to execute after the fading is complete
 * @param callbackScope		Scope (class) of the callback to execute
 */
void CameraEffectManager::fxFadeAsyncStart(int32 initialDelay, const Brightness* targetBrightness, int32 delayBetweenSteps, void (*callback)(ListenerObject, ListenerObject), ListenerObject callbackScope)
{
	// stop previous effect
	CameraEffectManager::stopEffect(this, kFadeTo);

	this->startingANewEffect = true;	

	// set target brightness
	if(targetBrightness == NULL)
	{
		this->fxFadeTargetBrightness = CameraEffectManager::getDefaultBrightness(this);
	}
	else
	{
		this->fxFadeTargetBrightness = *targetBrightness;
	}

	this->fxFadeTargetBrightness = CameraEffectManager::convertBrightnessToVipFormat(this, this->fxFadeTargetBrightness);

	// set effect parameters
	this->fxFadeDelay = (0 >= delayBetweenSteps) ? 1 : (uint8)(delayBetweenSteps);

	// set callback
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

	// start effect
	// TODO: check if the message really needs to be delayed.
	initialDelay = 0 >= initialDelay ? 1 : initialDelay;
	MessageDispatcher::dispatchMessage(initialDelay, ListenerObject::safeCast(this), ListenerObject::safeCast(this), kFadeTo, NULL);

	// fire effect started event
	CameraEffectManager::fireEvent(this, kEventEffectFadeStart);
}

/**
 * Stop the current asynchronous fade effect
 *
 * @private
 */
void CameraEffectManager::fxFadeAsyncStop()
{
	// remove event listener
	if(!isDeleted(this->fxFadeCallbackScope))
	{
		CameraEffectManager::removeEventListenerScopes(this, this->fxFadeCallbackScope, kEventEffectFadeComplete);
	}

	// discard pending delayed messages to stop effect
	MessageDispatcher::discardDelayedMessagesForReceiver(MessageDispatcher::getInstance(), ListenerObject::safeCast(this), kFadeTo);

	// reset effect variables
	this->fxFadeTargetBrightness = (Brightness){0, 0, 0};
	this->fxFadeDelay = 0;
	this->fxFadeCallbackScope = NULL;

	// fire effect stopped event
	CameraEffectManager::fireEvent(this, kEventEffectFadeStop);
}

/**
 * Set the screen to the default brightness
 *
 * @private
 */
void CameraEffectManager::showCamera()
{
	Brightness defaultBrightness = CameraEffectManager::getDefaultBrightness(this);
	defaultBrightness = CameraEffectManager::convertBrightnessToVipFormat(this, defaultBrightness);
	VIPManager::setupBrightness(VIPManager::getInstance(), &defaultBrightness);
}

/**
 * Set the screen to black
 *
 * @private
 */
void CameraEffectManager::hideCamera()
{
	VIPManager::lowerBrightness(VIPManager::getInstance());
}

/**
 * Do a synchronous fade in step
 *
 * @private
 */
void CameraEffectManager::fxFadeIn()
{
	Brightness incrementalBrightness =
	{
		_vipRegisters[__BRTA] + 1,
		_vipRegisters[__BRTB] + 2,
		_vipRegisters[__BRTC] - _vipRegisters[__BRTB] - _vipRegisters[__BRTA] + 1
	};
	
	VIPManager::setupBrightness(VIPManager::getInstance(), &incrementalBrightness);

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

/**
 * Do a synchronous fade out step
 *
 * @private
 */
void CameraEffectManager::fxFadeOut()
{
	Brightness decrementalBrightness =
	{
		(_vipRegisters[__BRTA] > 0) ? _vipRegisters[__BRTA] - 1 : 0,
		(_vipRegisters[__BRTB] > 1) ? _vipRegisters[__BRTB] - 2 : 0,
		(_vipRegisters[__BRTC] > 0) ? _vipRegisters[__BRTC] - 1 : 0
	};
	
	VIPManager::setupBrightness(VIPManager::getInstance(), &decrementalBrightness);
}

/**
 * Do an asynchronous fading step
 *
 * @private
 */
void CameraEffectManager::fxFadeAsync()
{
	ASSERT(this, "CameraEffectManager::fxFadeAsync: invalid this");

	// note: need to cast brightness registers to uint8 because only their lower 8 bits are valid
	bool lightRedDone 	= (uint8)_vipRegisters[__BRTC] == this->fxFadeTargetBrightness.brightRed;
	bool mediumRedDone 	= (uint8)_vipRegisters[__BRTB] == this->fxFadeTargetBrightness.mediumRed;
	bool darkRedDone 	= (uint8)_vipRegisters[__BRTA] == this->fxFadeTargetBrightness.darkRed;

	// fade light red
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
		// fade medium red
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
		// fade dark red
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

	// finish effect or call next round
	if(lightRedDone && mediumRedDone && darkRedDone)
	{
		this->startingANewEffect = false;

		// fire effect ended event
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

		if(this->fxFadeCallbackScope && !this->startingANewEffect)
		{
			CameraEffectManager::removeEventListenerScopes(this, this->fxFadeCallbackScope, kEventEffectFadeComplete);
		}
	}
	else
	{
		MessageDispatcher::dispatchMessage(this->fxFadeDelay, ListenerObject::safeCast(this), ListenerObject::safeCast(this), kFadeTo, NULL);
	}
}
