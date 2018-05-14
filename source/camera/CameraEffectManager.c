/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <CameraEffectManager.h>
#include <Camera.h>
#include <Game.h>
#include <Clock.h>
#include <TimerManager.h>
#include <MessageDispatcher.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	CameraEffectManager
 * @extends Object
 * @ingroup camera
 */
__CLASS_DEFINITION(CameraEffectManager, Object);
__CLASS_FRIEND_DEFINITION(Camera);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void CameraEffectManager::constructor(CameraEffectManager this);
void CameraEffectManager::showCamera(CameraEffectManager this);
void CameraEffectManager::hideCamera(CameraEffectManager this);
void CameraEffectManager_FXFadeIn(CameraEffectManager this);
void CameraEffectManager_FXFadeOut(CameraEffectManager this);
void CameraEffectManager_FXFadeStart(CameraEffectManager this, int effect, int delay);
void CameraEffectManager_FXFadeAsync(CameraEffectManager this);
void CameraEffectManager_FXFadeAsyncStart(CameraEffectManager this, int initialDelay, const Brightness* targetBrightness, int delayBetweenSteps, void (*callback)(Object, Object), Object callbackScope);
void CameraEffectManager_FXFadeAsyncStop(CameraEffectManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			CameraEffectManager::getInstance()
 * @memberof	CameraEffectManager
 * @public
 *
 * @return		CameraEffectManager instance
 */
__SINGLETON(CameraEffectManager);

/**
 * Class constructor
 *
 * @memberof	CameraEffectManager
 * @public
 *
 * @param this	Function scope
 */
static void CameraEffectManager::constructor(CameraEffectManager this)
{
	ASSERT(this, "CameraEffectManager::constructor: null this");

	// init class variables
	this->fxFadeTargetBrightness = (Brightness){0, 0, 0};
	this->fxFadeDelay = 0;
	this->fxFadeCallbackScope = NULL;

	// construct base object
	__CONSTRUCT_BASE(Object);
}

/**
 * Class destructor
 *
 * @memberof	CameraEffectManager
 * @public
 *
 * @param this	Function scope
 */
void CameraEffectManager::destructor(CameraEffectManager this)
{
	ASSERT(this, "CameraEffectManager::destructor: null this");

	// stop any effects
	CameraEffectManager::stopEffect(this, kFadeTo);

	// destroy base
	__SINGLETON_DESTROY;
}

/**
 * Get the current default brightness settings
 *
 * @memberof	CameraEffectManager
 * @public
 *
 * @param this	Function scope
 *
 * @return		Brightness
 */
Brightness CameraEffectManager::getDefaultBrightness(CameraEffectManager this __attribute__ ((unused)))
{
	ASSERT(this, "CameraEffectManager::getDefaultBrightness: null this");

	// default brightness settings
	Brightness brightness = (Brightness)
	{
		__BRIGHTNESS_DARK_RED,
		__BRIGHTNESS_MEDIUM_RED,
		__BRIGHTNESS_BRIGHT_RED,
	};

	// if exists, get brightness settings from stage definition
	Stage stage = GameState::getStage(Game::getCurrentState(Game::getInstance()));
	if(stage != NULL)
	{
		StageDefinition* stageDefinition = Stage::getStageDefinition(stage);
		brightness = stageDefinition->rendering.colorConfig.brightness;
	}

	// convert brightness settings to vip format
	brightness.brightRed -= (brightness.darkRed + brightness.mediumRed);

	return brightness;
}

/**
 * Start an effect
 *
 * @memberof	CameraEffectManager
 * @public
 *
 * @param this		Function scope
 * @param effect	Effect Id
 * @param args		va_list of effect parameters
 */
void CameraEffectManager::startEffect(CameraEffectManager this, int effect, va_list args)
{
	ASSERT(this, "CameraEffectManager::startEffect: null this");

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

			CameraEffectManager_FXFadeStart(this, effect, va_arg(args, int));
			break;

		case kFadeTo:

			CameraEffectManager_FXFadeAsyncStart(this,
				va_arg(args, int),
				va_arg(args, Brightness*),
				va_arg(args, int),
				va_arg(args, void*),
				va_arg(args, Object)
			);
			break;
	}
}

/**
 * Stop an effect
 *
 * @memberof	CameraEffectManager
 * @public
 *
 * @param this		Function scope
 * @param effect	Effect Id
 */
void CameraEffectManager::stopEffect(CameraEffectManager this, int effect)
{
	ASSERT(this, "CameraEffectManager::stopEffect: null this");

	switch(effect)
	{
		case kFadeTo:

			CameraEffectManager_FXFadeAsyncStop(this);
			break;
	}
}

/**
 * Process a telegram
 *
 * @memberof		CameraEffectManager
 * @public
 *
 * @param this		Function scope
 * @param telegram	Received telegram
 *
 * @return			True if successfully processed, false otherwise
 */
bool CameraEffectManager::handleMessage(CameraEffectManager this, Telegram telegram)
{
	ASSERT(this, "CameraEffectManager::handleMessage: null this");

	switch(Telegram::getMessage(telegram))
	{
		case kFadeTo:
			CameraEffectManager_FXFadeAsync(this);
			break;
	}
	return true;
}

/**
 * Start a synchronous fade effect
 *
 * @memberof		CameraEffectManager
 * @private
 *
 * @param this		Function scope
 * @param effect	Effect ID
 * @param delay		Start effect after this delay
 */
void CameraEffectManager_FXFadeStart(CameraEffectManager this, int effect, int delay)
{
	ASSERT(this, "CameraEffectManager::FXFadeStart: null this");

	Brightness defaultBrightness = CameraEffectManager::getDefaultBrightness(this);

	switch(effect)
	{
		case kFadeIn:

			CameraEffectManager::hideCamera(this);

			TimerManager::repeatMethodCall(
				TimerManager::getInstance(),
				defaultBrightness.darkRed,
				(delay * defaultBrightness.darkRed),
				__SAFE_CAST(Object, this),
				(void (*)(Object, u32))&CameraEffectManager_FXFadeIn
			);
			break;

		case kFadeOut:

			CameraEffectManager::showCamera(this);

			TimerManager::repeatMethodCall(
				TimerManager::getInstance(),
				defaultBrightness.darkRed,
				(delay * defaultBrightness.darkRed),
				__SAFE_CAST(Object, this),
				(void (*)(Object, u32))&CameraEffectManager_FXFadeOut
			);

			break;
	}
}

/**
 * Start an asynchronous fade effect
 *
 * @memberof				CameraEffectManager
 * @private
 *
 * @param this				Function scope
 * @param initialDelay		Start effect after this initial delay
 * @param targetBrightness	Fade to this target brightness
 * @param delayBetweenSteps	Delay between the individual fading steps
 * @param callback			Callback to execute after the fading is complete
 * @param callbackScope		Scope (class) of the callback to execute
 */
void CameraEffectManager_FXFadeAsyncStart(CameraEffectManager this, int initialDelay, const Brightness* targetBrightness, int delayBetweenSteps, void (*callback)(Object, Object), Object callbackScope)
{
	ASSERT(this, "CameraEffectManager::FXFadeAsyncStart: null this");

	// stop previous effect
	CameraEffectManager::stopEffect(this, kFadeTo);

	// set target brightness
	if(targetBrightness == NULL)
	{
		this->fxFadeTargetBrightness = CameraEffectManager::getDefaultBrightness(this);
	}
	else
	{
		this->fxFadeTargetBrightness = *targetBrightness;
	}

	// set effect parameters
	this->fxFadeDelay = (0 >= delayBetweenSteps) ? 1 : (u8)(delayBetweenSteps);

	// set callback
	if(callback != NULL)
	{
		if(callbackScope != NULL)
		{
			this->fxFadeCallbackScope = callbackScope;
			Object::addEventListener(__SAFE_CAST(Object, this), callbackScope, callback, kEventEffectFadeComplete);
		}
		else
		{
			NM_ASSERT(false, "CameraEffectManager::FXFadeAsyncStart: null callbackScope");
		}
	}

	// start effect
	// TODO: check if the message really needs to be delayed.
	initialDelay = 0 >= initialDelay ? 1 : initialDelay;
	MessageDispatcher::dispatchMessage(initialDelay, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kFadeTo, NULL);

	// fire effect started event
	Object::fireEvent(__SAFE_CAST(Object, this), kEventEffectFadeStart);
}

/**
 * Stop the current asynchronous fade effect
 *
 * @memberof	CameraEffectManager
 * @private
 *
 * @param this	Function scope
 */
void CameraEffectManager_FXFadeAsyncStop(CameraEffectManager this)
{
	ASSERT(this, "CameraEffectManager::FXFadeAsyncStop: null this");

	// remove event listener
	if(this->fxFadeCallbackScope)
	{
		Object::removeEventListeners(__SAFE_CAST(Object, this), this->fxFadeCallbackScope, kEventEffectFadeComplete);
	}

	// discard pending delayed messages to stop effect
	MessageDispatcher::discardDelayedMessagesFromSender(MessageDispatcher::getInstance(), __SAFE_CAST(Object, this), kFadeTo);

	// reset effect variables
	this->fxFadeTargetBrightness = (Brightness){0, 0, 0};
	this->fxFadeDelay = 0;
	this->fxFadeCallbackScope = NULL;

	// fire effect stopped event
	Object::fireEvent(__SAFE_CAST(Object, this), kEventEffectFadeStop);
}

/**
 * Set the screen to the default brightness
 *
 * @memberof	CameraEffectManager
 * @private
 *
 * @param this	Function scope
 */
void CameraEffectManager::showCamera(CameraEffectManager this)
{
	ASSERT(this, "CameraEffectManager::showCamera: null this");

	Brightness defaultBrightness = CameraEffectManager::getDefaultBrightness(this);

	__SET_BRIGHT(
		defaultBrightness.darkRed,
		defaultBrightness.mediumRed,
		defaultBrightness.brightRed
	);
}

/**
 * Set the screen to black
 *
 * @memberof	CameraEffectManager
 * @private
 *
 * @param this	Function scope
 */
void CameraEffectManager::hideCamera(CameraEffectManager this __attribute__ ((unused)))
{
	ASSERT(this, "CameraEffectManager::hideCamera: null this");

	__SET_BRIGHT(0, 0, 0);
}

/**
 * Do a synchronous fade in step
 *
 * @memberof	CameraEffectManager
 * @private
 *
 * @param this	Function scope
 */
void CameraEffectManager_FXFadeIn(CameraEffectManager this __attribute__ ((unused)))
{
	ASSERT(this, "CameraEffectManager::FXFadeIn: null this");

	__SET_BRIGHT(
		_vipRegisters[__BRTA] + 1,
		_vipRegisters[__BRTB] + 2,
		_vipRegisters[__BRTC] + 1
	);

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
 * @memberof	CameraEffectManager
 * @private
 *
 * @param this	Function scope
 */
void CameraEffectManager_FXFadeOut(CameraEffectManager this __attribute__ ((unused)))
{
	ASSERT(this, "CameraEffectManager::FXFadeOut: null this");

	// decrease brightness
	__SET_BRIGHT(
		_vipRegisters[__BRTA] - 1,
		_vipRegisters[__BRTB] - 2,
		_vipRegisters[__BRTC] - 1
	);
}

/**
 * Do an asynchronous fading step
 *
 * @memberof	CameraEffectManager
 * @private
 *
 * @param this	Function scope
 */
void CameraEffectManager_FXFadeAsync(CameraEffectManager this)
{
	ASSERT(this, "CameraEffectManager::FXFadeAsync: null this");
	ASSERT(this, "CameraEffectManager::FXFadeAsync: invalid this");

	bool lightRedDone = false;
	bool mediumRedDone = false;
	bool darkRedDone = false;

	// note: need to cast brightness registers to u8 because only their lower 8 bits are valid

	// fade light red
	if((u8)_vipRegisters[__BRTC] < this->fxFadeTargetBrightness.brightRed)
	{
		_vipRegisters[__BRTC] += 1;
	}
	else if((u8)_vipRegisters[__BRTC] > this->fxFadeTargetBrightness.brightRed)
	{
		_vipRegisters[__BRTC] -= 1;
	}
	else
	{
		lightRedDone = true;
	}

	// fade medium red
	u8 i;
	for(i = 0; i < 2; i++) {
		if((u8)_vipRegisters[__BRTB] < this->fxFadeTargetBrightness.mediumRed)
		{
			_vipRegisters[__BRTB] += 1;
		}
		else if((u8)_vipRegisters[__BRTB] > this->fxFadeTargetBrightness.mediumRed)
		{
			_vipRegisters[__BRTB] -= 1;
		}
		else
		{
			mediumRedDone = true;
		}
	}

	// fade dark red
	if((u8)_vipRegisters[__BRTA] < this->fxFadeTargetBrightness.darkRed)
	{
		_vipRegisters[__BRTA] += 1;
	}
	else if((u8)_vipRegisters[__BRTA] > this->fxFadeTargetBrightness.darkRed)
	{
		_vipRegisters[__BRTA] -= 1;
	}
	else
	{
		darkRedDone = true;
	}

	// finish effect or call next round
	if(lightRedDone && mediumRedDone && darkRedDone)
	{
		// fire effect ended event
		Object::fireEvent(__SAFE_CAST(Object, this), kEventEffectFadeComplete);

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

		// remove callback event listener
		if(this->fxFadeCallbackScope)
		{
			Object::removeEventListeners(__SAFE_CAST(Object, this), this->fxFadeCallbackScope, kEventEffectFadeComplete);
		}
	}
	else
	{
		MessageDispatcher::dispatchMessage(this->fxFadeDelay, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kFadeTo, NULL);
	}
}
