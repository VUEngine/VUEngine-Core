/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

void CameraEffectManager_showCamera(CameraEffectManager this);
void CameraEffectManager_hideCamera(CameraEffectManager this);
void CameraEffectManager_FXFadeIn(CameraEffectManager this, u32 duration);
void CameraEffectManager_FXFadeOut(CameraEffectManager this, u32 duration);
void CameraEffectManager_FXFadeStart(CameraEffectManager this, int effect, int delay);
void CameraEffectManager_FXFadeAsync(CameraEffectManager this);
void CameraEffectManager_FXFadeAsyncStart(CameraEffectManager this, int initialDelay, const Brightness* targetBrightness, int delayBetweenSteps, void (*callback)(Object, Object), Object callbackScope);
void CameraEffectManager_FXFadeAsyncStop(CameraEffectManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// it's a singleton
__SINGLETON(CameraEffectManager);

// class's constructor
void CameraEffectManager_constructor(CameraEffectManager this)
{
	ASSERT(this, "CameraEffectManager::constructor: null this");

	// init class variables
	this->fxFadeTargetBrightness = (Brightness){0, 0, 0};
	this->fxFadeDelay = 0;
	this->fxFadeCallbackScope = NULL;

	// construct base object
	__CONSTRUCT_BASE(Object);
}

// class's destructor
void CameraEffectManager_destructor(CameraEffectManager this)
{
	ASSERT(this, "CameraEffectManager::destructor: null this");

	// stop any effects
	CameraEffectManager_stopEffect(this, kFadeTo);

	// destroy base
	__SINGLETON_DESTROY;
}

Brightness CameraEffectManager_getDefaultBrightness(CameraEffectManager this __attribute__ ((unused)))
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
	Stage stage = GameState_getStage(Game_getCurrentState(Game_getInstance()));
	if(stage != NULL)
	{
		StageDefinition* stageDefinition = Stage_getStageDefinition(stage);
		brightness = stageDefinition->rendering.colorConfig.brightness;
	}

	// convert brightness settings to vip format
	brightness.brightRed -= (brightness.darkRed + brightness.mediumRed);

	return brightness;
}

void CameraEffectManager_FXFadeStart(CameraEffectManager this, int effect, int delay)
{
	ASSERT(this, "CameraEffectManager::FXFadeStart: null this");

	Brightness defaultBrightness = CameraEffectManager_getDefaultBrightness(this);

	switch(effect)
	{
		case kFadeIn:

			CameraEffectManager_hideCamera(this);

			TimerManager_repeatMethodCall(
				TimerManager_getInstance(),
				defaultBrightness.darkRed,
				(delay * defaultBrightness.darkRed),
				__SAFE_CAST(Object, this),
				(void (*)(Object, u32))&CameraEffectManager_FXFadeIn
			);
			break;

		case kFadeOut:

			CameraEffectManager_showCamera(this);

			TimerManager_repeatMethodCall(
				TimerManager_getInstance(),
				defaultBrightness.darkRed,
				(delay * defaultBrightness.darkRed),
				__SAFE_CAST(Object, this),
				(void (*)(Object, u32))&CameraEffectManager_FXFadeOut
			);

			break;
	}
}

void CameraEffectManager_FXFadeAsyncStart(CameraEffectManager this, int initialDelay, const Brightness* targetBrightness, int delayBetweenSteps, void (*callback)(Object, Object), Object callbackScope)
{
	ASSERT(this, "CameraEffectManager::FXFadeAsyncStart: null this");

	// stop previous effect
	CameraEffectManager_stopEffect(this, kFadeTo);

	// set target brightness
	if(targetBrightness == NULL)
	{
		this->fxFadeTargetBrightness = CameraEffectManager_getDefaultBrightness(this);
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
			Object_addEventListener(__SAFE_CAST(Object, this), callbackScope, callback, kEventEffectFadeComplete);
		}
		else
		{
			NM_ASSERT(false, "CameraEffectManager::FXFadeAsyncStart: null callbackScope");
		}
	}

	// start effect
	// TODO: check if the message really needs to be delayed.
	initialDelay = 0 >= initialDelay ? 1 : initialDelay;
	MessageDispatcher_dispatchMessage(initialDelay, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kFadeTo, NULL);

	// fire effect started event
	Object_fireEvent(__SAFE_CAST(Object, this), kEventEffectFadeStart);
}

void CameraEffectManager_FXFadeAsyncStop(CameraEffectManager this)
{
	ASSERT(this, "CameraEffectManager::FXFadeAsyncStop: null this");

	// remove event listener
	if(this->fxFadeCallbackScope)
	{
		Object_removeEventListeners(__SAFE_CAST(Object, this), this->fxFadeCallbackScope, kEventEffectFadeComplete);
	}

	// discard pending delayed messages to stop effect
	MessageDispatcher_discardDelayedMessagesFromSender(MessageDispatcher_getInstance(), __SAFE_CAST(Object, this), kFadeTo);

	// reset effect variables
	this->fxFadeTargetBrightness = (Brightness){0, 0, 0};
	this->fxFadeDelay = 0;
	this->fxFadeCallbackScope = NULL;

	// fire effect stopped event
	Object_fireEvent(__SAFE_CAST(Object, this), kEventEffectFadeStop);
}

void CameraEffectManager_startEffect(CameraEffectManager this, int effect, va_list args)
{
	ASSERT(this, "CameraEffectManager::startEffect: null this");

	switch(effect)
	{
		case kShow:

			CameraEffectManager_showCamera(this);
			break;

		case kHide:

			CameraEffectManager_hideCamera(this);
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

void CameraEffectManager_stopEffect(CameraEffectManager this, int effect)
{
	ASSERT(this, "CameraEffectManager::stopEffect: null this");

	switch(effect)
	{
		case kFadeTo:

			CameraEffectManager_FXFadeAsyncStop(this);
			break;
	}
}

void CameraEffectManager_showCamera(CameraEffectManager this __attribute__ ((unused)))
{
	ASSERT(this, "CameraEffectManager::showCamera: null this");

	Brightness defaultBrightness = CameraEffectManager_getDefaultBrightness(this);

	__SET_BRIGHT(
		defaultBrightness.darkRed,
		defaultBrightness.mediumRed,
		defaultBrightness.brightRed
	);
}

void CameraEffectManager_hideCamera(CameraEffectManager this __attribute__ ((unused)))
{
	ASSERT(this, "CameraEffectManager::hideCamera: null this");

	__SET_BRIGHT(0, 0, 0);
}

void CameraEffectManager_FXFadeIn(CameraEffectManager this __attribute__ ((unused)), u32 callNumber __attribute__ ((unused)))
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

void CameraEffectManager_FXFadeOut(CameraEffectManager this __attribute__ ((unused)), u32 callNumber __attribute__ ((unused)))
{
	ASSERT(this, "CameraEffectManager::FXFadeOut: null this");

	// decrease brightness
	__SET_BRIGHT(
		_vipRegisters[__BRTA] - 1,
		_vipRegisters[__BRTB] - 2,
		_vipRegisters[__BRTC] - 1
	);
}

// fade in the camera
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
		Object_fireEvent(__SAFE_CAST(Object, this), kEventEffectFadeComplete);

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
			Object_removeEventListeners(__SAFE_CAST(Object, this), this->fxFadeCallbackScope, kEventEffectFadeComplete);
		}
	}
	else
	{
		MessageDispatcher_dispatchMessage(this->fxFadeDelay, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kFadeTo, NULL);
	}
}

// process a telegram
bool CameraEffectManager_handleMessage(CameraEffectManager this, Telegram telegram)
{
	ASSERT(this, "CameraEffectManager::handleMessage: null this");

	switch(Telegram_getMessage(telegram))
	{
		case kFadeTo:
			CameraEffectManager_FXFadeAsync(this);
			break;
	}
	return true;
}
