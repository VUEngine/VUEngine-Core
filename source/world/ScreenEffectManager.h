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

#ifndef SCREEN_EFFECT_MANAGER_H_
#define SCREEN_EFFECT_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Telegram.h>
#include <InGameEntity.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

enum ScreenFX
{
	kFadeIn = 0,
	kFadeOut,
	kFadeInAsync,
	kFadeOutAsync,
	kFadeToAsync,

	kScreenLastFX
};

#define __EVENT_EFFECT_FADE_START		"eventFadeStart"
#define __EVENT_EFFECT_FADE_STOP		"eventFadeStop"
#define __EVENT_EFFECT_FADE_COMPLETE	"eventFadeComplete"


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define ScreenEffectManager_METHODS(ClassName)															\
    	Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, void, startEffect, int effect, va_list args);							\
		__VIRTUAL_DEC(ClassName, void, stopEffect, int effect);											\

// declare the virtual methods which are redefined
#define ScreenEffectManager_SET_VTABLE(ClassName)														\
    	Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, ScreenEffectManager, startEffect);										\
		__VIRTUAL_SET(ClassName, ScreenEffectManager, stopEffect);										\
		__VIRTUAL_SET(ClassName, ScreenEffectManager, handleMessage);									\

#define ScreenEffectManager_ATTRIBUTES																	\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* target brightness for current fade effect */													\
        Brightness fxFadeTargetBrightness;																\
        /* delay for current fade effect */																\
        u8 fxFadeDelay;																					\
        /* callback scope for current fade effect */													\
        Object fxFadeCallbackScope;

// declare a ScreenEffectManager
__CLASS(ScreenEffectManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

ScreenEffectManager ScreenEffectManager_getInstance();

void ScreenEffectManager_constructor(ScreenEffectManager this);
void ScreenEffectManager_destructor(ScreenEffectManager this);
void ScreenEffectManager_startEffect(ScreenEffectManager this, int effect, va_list args);
void ScreenEffectManager_stopEffect(ScreenEffectManager this, int effect);
bool ScreenEffectManager_handleMessage(ScreenEffectManager this, Telegram telegram);


#endif
