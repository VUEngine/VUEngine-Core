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

#ifndef SCREEN_EFFECT_MANAGER_H_
#define SCREEN_EFFECT_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Telegram.h>
#include <InGameEntity.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

enum ScreenFX
{
	kShow = 0,
	kHide,
	kFadeIn,
	kFadeOut,
	kFadeTo,

	kScreenLastFX
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
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
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

ScreenEffectManager ScreenEffectManager_getInstance();

void ScreenEffectManager_constructor(ScreenEffectManager this);
void ScreenEffectManager_destructor(ScreenEffectManager this);
void ScreenEffectManager_startEffect(ScreenEffectManager this, int effect, va_list args);
void ScreenEffectManager_stopEffect(ScreenEffectManager this, int effect);
bool ScreenEffectManager_handleMessage(ScreenEffectManager this, Telegram telegram);
Brightness ScreenEffectManager_getDefaultBrightness(ScreenEffectManager this);


#endif
