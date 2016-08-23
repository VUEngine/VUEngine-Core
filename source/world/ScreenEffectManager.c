/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ScreenEffectManager.h>
#include <Screen.h>
#include <Game.h>
#include <Clock.h>
#include <TimerManager.h>
#include <MessageDispatcher.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(ScreenEffectManager, Object);
__CLASS_FRIEND_DEFINITION(Screen);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

void ScreenEffectManager_FXFadeIn(ScreenEffectManager this, u32 duration);
void ScreenEffectManager_FXFadeOut(ScreenEffectManager this, u32 duration);
void ScreenEffectManager_FXFadeStart(ScreenEffectManager this, int effect, int duration);
void ScreenEffectManager_FXFadeAsync(ScreenEffectManager this);
void ScreenEffectManager_FXFadeAsyncStart(ScreenEffectManager this, int effect, int delay, const Brightness* targetBrightness, void (*callback)(Object, Object), Object callbackScope);
void ScreenEffectManager_FXFadeAsyncStop(ScreenEffectManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
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
    ScreenEffectManager_stopEffect(this, kFadeToAsync);

	// destroy base
	__SINGLETON_DESTROY;
}

Brightness ScreenEffectManager_getDefaultTargetBrightness(ScreenEffectManager this __attribute__ ((unused)))
{
	ASSERT(this, "ScreenEffectManager::getDefaultTargetBrightness: null this");

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
        StageDefinition* stageDefinition = Stage_stageDefinition(stage);
        brightness = stageDefinition->rendering.colorConfig.brightness;
    }

    // convert brightness settings to vip format
    brightness.brightRed -= (brightness.darkRed + brightness.mediumRed);

    return brightness;
}

void ScreenEffectManager_FXFadeStart(ScreenEffectManager this, int effect, int duration)
{
	ASSERT(this, "ScreenEffectManager::FXFadeStart: null this");

#ifdef __DEBUG_NO_FADE
    return;
#endif

    Brightness targetBrightness = ScreenEffectManager_getDefaultTargetBrightness(this);

    switch(effect)
    {
        case kFadeIn:
            TimerManager_repeatMethodCall(TimerManager_getInstance(), targetBrightness.darkRed, duration / 32, __SAFE_CAST(Object, this), (void (*)(Object, u32))&ScreenEffectManager_FXFadeIn);
            break;

        case kFadeOut:
            TimerManager_repeatMethodCall(TimerManager_getInstance(), targetBrightness.darkRed, duration / 32, __SAFE_CAST(Object, this), (void (*)(Object, u32))&ScreenEffectManager_FXFadeOut);
            break;
    }
}

void ScreenEffectManager_FXFadeAsyncStart(ScreenEffectManager this, int effect, int delay, const Brightness* targetBrightness, void (*callback)(Object, Object), Object callbackScope)
{
	ASSERT(this, "ScreenEffectManager::FXFadeAsyncStart: null this");

#ifdef __DEBUG_NO_FADE
    return;
#endif

    // stop previous effect
    ScreenEffectManager_stopEffect(this, kFadeToAsync);

    // set target brightness according to effect
    if(targetBrightness)
    {
        this->fxFadeTargetBrightness = *targetBrightness;
    }
    else if(effect == kFadeInAsync)
    {
        this->fxFadeTargetBrightness = ScreenEffectManager_getDefaultTargetBrightness(this);
    }
    else if(effect == kFadeOutAsync)
    {
        this->fxFadeTargetBrightness = (Brightness) {0, 0, 0};
    }

   // set effect parameters
    this->fxFadeDelay = 0 >= delay? 1: delay;

    // fire effect started event
    Object_fireEvent(__SAFE_CAST(Object, this), __EVENT_EFFECT_FADE_START);

    // set callback
    if(callback != NULL && callbackScope != NULL)
    {
        this->fxFadeCallbackScope = callbackScope;
        Object_addEventListener(__SAFE_CAST(Object, this), callbackScope, callback, __EVENT_EFFECT_FADE_COMPLETE);
    }

    // start effect
    // TODO: check if the message really needs to be delayed.
    MessageDispatcher_dispatchMessage(1, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kFadeToAsync, NULL);
}

void ScreenEffectManager_FXFadeAsyncStop(ScreenEffectManager this)
{
	ASSERT(this, "ScreenEffectManager::FXFadeAsyncStop: null this");

    // discard pending delayed messages to stop effect
    MessageDispatcher_discardDelayedMessagesFromSender(MessageDispatcher_getInstance(), __SAFE_CAST(Object, this), kFadeToAsync);

    // reset effect variables
    this->fxFadeTargetBrightness = (Brightness){0, 0, 0};
    this->fxFadeDelay = 0;
    this->fxFadeCallbackScope = NULL;

    // fire effect stopped event
    Object_fireEvent(__SAFE_CAST(Object, this), __EVENT_EFFECT_FADE_STOP);
}

void ScreenEffectManager_startEffect(ScreenEffectManager this, int effect, va_list args)
{
	ASSERT(this, "ScreenEffectManager::startEffect: null this");

	switch(effect)
	{
		case kFadeIn:
		case kFadeOut:

            ScreenEffectManager_FXFadeStart(this, effect, va_arg(args, int));
			break;

		case kFadeInAsync:
		case kFadeOutAsync:
		case kFadeToAsync:

            ScreenEffectManager_FXFadeAsyncStart(this, effect, va_arg(args, int), va_arg(args, Brightness*), va_arg(args, void*), va_arg(args, Object));
            break;
	}
}

void ScreenEffectManager_stopEffect(ScreenEffectManager this, int effect)
{
	ASSERT(this, "ScreenEffectManager::stopEffect: null this");

	switch(effect)
	{
		case kFadeInAsync:
		case kFadeOutAsync:
		case kFadeToAsync:

            ScreenEffectManager_FXFadeAsyncStop(this);
	        break;
	}
}

void ScreenEffectManager_FXFadeIn(ScreenEffectManager this __attribute__ ((unused)), u32 callNumber)
{
    // increase brightness
    __SET_BRIGHT(callNumber, callNumber << 1, callNumber);
}

void ScreenEffectManager_FXFadeOut(ScreenEffectManager this __attribute__ ((unused)), u32 callNumber)
{
    // increase brightness
    __SET_BRIGHT(32 - callNumber, (32 - callNumber) << 1, 32 -callNumber);
}

// fade in the screen
void ScreenEffectManager_FXFadeAsync(ScreenEffectManager this)
{
	ASSERT(this, "ScreenEffectManager::FXFadeAsync: null this");

#ifdef __DEBUG_NO_FADE
	return;
#endif

    // TODO rework this logic for freely defined target brightness
    if(_vipRegisters[__BRTC] < this->fxFadeTargetBrightness.brightRed)
    {
        _vipRegisters[__BRTA] += 1;
        _vipRegisters[__BRTB] += 2;
        _vipRegisters[__BRTC] += 1;
        MessageDispatcher_dispatchMessage(this->fxFadeDelay, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kFadeToAsync, NULL);
    }
    else if(_vipRegisters[__BRTC] > this->fxFadeTargetBrightness.brightRed)
    {
        _vipRegisters[__BRTA] -= 1;
        _vipRegisters[__BRTB] -= 2;
        _vipRegisters[__BRTC] -= 1;
        MessageDispatcher_dispatchMessage(this->fxFadeDelay, __SAFE_CAST(Object, this), __SAFE_CAST(Object, this), kFadeToAsync, NULL);
    }
    else
    {
        _vipRegisters[__BRTA] = this->fxFadeTargetBrightness.brightRed;
        _vipRegisters[__BRTB] = this->fxFadeTargetBrightness.brightRed * 2;
        _vipRegisters[__BRTC] = this->fxFadeTargetBrightness.brightRed;

        // fire effect ended event
	    Object_fireEvent(__SAFE_CAST(Object, this), __EVENT_EFFECT_FADE_COMPLETE);

        // remove callback event listener
        if(this->fxFadeCallbackScope)
        {
            Object_removeEventListeners(__SAFE_CAST(Object, this), this->fxFadeCallbackScope, __EVENT_EFFECT_FADE_COMPLETE);
        }
    }
}

// process a telegram
bool ScreenEffectManager_handleMessage(ScreenEffectManager this, Telegram telegram)
{
	ASSERT(this, "ScreenEffectManager::handleMessage: null this");

	switch(Telegram_getMessage(telegram))
	{
		case kFadeToAsync:
			ScreenEffectManager_FXFadeAsync(this);
			break;
	}
	return true;
}
