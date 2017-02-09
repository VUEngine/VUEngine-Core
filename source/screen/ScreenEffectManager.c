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

#include <ScreenEffectManager.h>
#include <Screen.h>
#include <Game.h>
#include <Clock.h>
#include <TimerManager.h>
#include <MessageDispatcher.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	ScreenEffectManager
 * @extends Object
 */
__CLASS_DEFINITION(ScreenEffectManager, Object);
__CLASS_FRIEND_DEFINITION(Screen);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

void ScreenEffectManager_showScreen(ScreenEffectManager this);
void ScreenEffectManager_hideScreen(ScreenEffectManager this);
void ScreenEffectManager_FXFadeIn(ScreenEffectManager this, u32 duration);
void ScreenEffectManager_FXFadeOut(ScreenEffectManager this, u32 duration);
void ScreenEffectManager_FXFadeStart(ScreenEffectManager this, int effect, int delay);
void ScreenEffectManager_FXFadeAsync(ScreenEffectManager this);
void ScreenEffectManager_FXFadeAsyncStart(ScreenEffectManager this, int initialDelay, const Brightness* targetBrightness, int delayBetweenSteps, void (*callback)(Object, Object), Object callbackScope);
void ScreenEffectManager_FXFadeAsyncStop(ScreenEffectManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// it's a singleton
__SINGLETON(ScreenEffectManager);

// class's constructor
void ScreenEffectManager_constructor(ScreenEffectManager this)
{
	ASSERT(this, "ScreenEffectManager::constructor: null this");

	// init class variables
	this->fxFadeTargetBrightness = (Brightness){0, 0, 0};
	this->fxFadeDelay = 0;
	this->fxFadeCallbackScope = NULL;

	// construct base object
	__CONSTRUCT_BASE(Object);
}

// class's destructor
void ScreenEffectManager_destructor(ScreenEffectManager this)
{
	ASSERT(this, "ScreenEffectManager::destructor: null this");

	// stop any effects
	ScreenEffectManager_stopEffect(this, kFadeTo);

	// destroy base
	__SINGLETON_DESTROY;
}

Brightness ScreenEffectManager_getDefaultBrightness(ScreenEffectManager this __attribute__ ((unused)))
{
	ASSERT(this, "ScreenEffectManager::getDefaultBrightness: null this");

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

void ScreenEffectManager_FXFadeStart(ScreenEffectManager this, int effect, int delay)
{
	ASSERT(this, "ScreenEffectManager::FXFadeStart: null this");

	Brightness defaultBrightness = ScreenEffectManager_getDefaultBrightness(this);

	switch(effect)
	{
		case kFadeIn:

			ScreenEffectManager_hideScreen(this);

			TimerManager_repeatMethodCall(
				TimerManager_getInstance(),
				defaultBrightness.darkRed,
				delay * defaultBrightness.darkRed,
				__SAFE_CAST(Object, this),
				(void (*)(Object, u32))&ScreenEffectManager_FXFadeIn
			);
			break;

		case kFadeOut:

			ScreenEffectManager_showScreen(this);

			TimerManager_repeatMethodCall(
				TimerManager_getInstance(),
				defaultBrightness.darkRed,
				delay * defaultBrightness.darkRed,
				__SAFE_CAST(Object, this),
				(void (*)(Object, u32))&ScreenEffectManager_FXFadeOut
			);

			break;
	}
}

void ScreenEffectManager_FXFadeAsyncStart(ScreenEffectManager this, int initialDelay, const Brightness* targetBrightness, int delayBetweenSteps, void (*callback)(Object, Object), Object callbackScope)
{
	ASSERT(this, "ScreenEffectManager::FXFadeAsyncStart: null this");

	// stop previous effect
	ScreenEffectManager_stopEffect(this, kFadeTo);

	// set target brightness
	if(targetBrightness == NULL)
	{
		this->fxFadeTargetBrightness = ScreenEffectManager_getDefaultBrightness(this);
	}
	else
	{
		this->fxFadeTargetBrightness = *targetBrightness;
	}

	// set effect parameters
	this->fxFadeDelay = 0 >= delayBetweenSteps ? 1 : (u8)delayBetweenSteps;

	// set callback
	if(callback != NULL && callbackScope != NULL)
	{
		this->fxFadeCallbackScope = callbackScope;
		Object_addEventListener(__SAFE_CAST(Object, this), callbackScope, callback, kEventEffectFadeComplete);
	}

	// start effect
	// TODO: check if the message really needs to be delayed.
	initialDelay = 0 >= initialDelay ? 1 : initialDelay;
	MessageDispatcher_dispatchMessage(initialDelay, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kFadeTo, NULL);

	// fire effect started event
	Object_fireEvent(__SAFE_CAST(Object, this), kEventEffectFadeStart);
}

void ScreenEffectManager_FXFadeAsyncStop(ScreenEffectManager this)
{
	ASSERT(this, "ScreenEffectManager::FXFadeAsyncStop: null this");

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

void ScreenEffectManager_startEffect(ScreenEffectManager this, int effect, va_list args)
{
	ASSERT(this, "ScreenEffectManager::startEffect: null this");

	switch(effect)
	{
		case kShow:

			ScreenEffectManager_showScreen(this);
			break;

		case kHide:

			ScreenEffectManager_hideScreen(this);
			break;

		case kFadeIn:
		case kFadeOut:

			ScreenEffectManager_FXFadeStart(this, effect, va_arg(args, int));
			break;

		case kFadeTo:

			ScreenEffectManager_FXFadeAsyncStart(this,
				va_arg(args, int),
				va_arg(args, Brightness*),
				va_arg(args, int),
				va_arg(args, void*),
				va_arg(args, Object)
			);
			break;
	}
}

void ScreenEffectManager_stopEffect(ScreenEffectManager this, int effect)
{
	ASSERT(this, "ScreenEffectManager::stopEffect: null this");

	switch(effect)
	{
		case kFadeTo:

			ScreenEffectManager_FXFadeAsyncStop(this);
			break;
	}
}

void ScreenEffectManager_showScreen(ScreenEffectManager this __attribute__ ((unused)))
{
	ASSERT(this, "ScreenEffectManager::showScreen: null this");

	Brightness defaultBrightness = ScreenEffectManager_getDefaultBrightness(this);

	__SET_BRIGHT(
		defaultBrightness.darkRed,
		defaultBrightness.mediumRed,
		defaultBrightness.brightRed
	);
}

void ScreenEffectManager_hideScreen(ScreenEffectManager this __attribute__ ((unused)))
{
	ASSERT(this, "ScreenEffectManager::hideScreen: null this");

	__SET_BRIGHT(0, 0, 0);
}

void ScreenEffectManager_FXFadeIn(ScreenEffectManager this __attribute__ ((unused)), u32 callNumber __attribute__ ((unused)))
{
	ASSERT(this, "ScreenEffectManager::FXFadeIn: null this");

	__SET_BRIGHT(
		_vipRegisters[__BRTA] + 1,
		_vipRegisters[__BRTB] + 2,
		_vipRegisters[__BRTC] + 1
	);
}

void ScreenEffectManager_FXFadeOut(ScreenEffectManager this __attribute__ ((unused)), u32 callNumber __attribute__ ((unused)))
{
	ASSERT(this, "ScreenEffectManager::FXFadeOut: null this");

	// decrease brightness
	__SET_BRIGHT(
		_vipRegisters[__BRTA] - 1,
		_vipRegisters[__BRTB] - 2,
		_vipRegisters[__BRTC] - 1
	);
}

// fade in the screen
void ScreenEffectManager_FXFadeAsync(ScreenEffectManager this)
{
	ASSERT(this, "ScreenEffectManager::FXFadeAsync: null this");
	ASSERT(__SAFE_CAST(ScreenEffectManager, this), "ScreenEffectManager::FXFadeAsync: invalid this");

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
bool ScreenEffectManager_handleMessage(ScreenEffectManager this, Telegram telegram)
{
	ASSERT(this, "ScreenEffectManager::handleMessage: null this");

	switch(Telegram_getMessage(telegram))
	{
		case kFadeTo:
			ScreenEffectManager_FXFadeAsync(this);
			break;
	}
	return true;
}
