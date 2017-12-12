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

#ifndef CAMERA_EFFECT_MANAGER_H_
#define CAMERA_EFFECT_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Telegram.h>
#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

enum CameraFX
{
	kShow = 0,
	kHide,
	kFadeIn,
	kFadeOut,
	kFadeTo,

	kCameraLastFX
};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define CameraEffectManager_METHODS(ClassName)															\
		Object_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, void, startEffect, int effect, va_list args);							\
		__VIRTUAL_DEC(ClassName, void, stopEffect, int effect);											\

// declare the virtual methods which are redefined
#define CameraEffectManager_SET_VTABLE(ClassName)														\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, CameraEffectManager, startEffect);										\
		__VIRTUAL_SET(ClassName, CameraEffectManager, stopEffect);										\
		__VIRTUAL_SET(ClassName, CameraEffectManager, handleMessage);									\

#define CameraEffectManager_ATTRIBUTES																	\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* target brightness for current fade effect */													\
		Brightness fxFadeTargetBrightness;																\
		/* delay for current fade effect */																\
		u8 fxFadeDelay;																					\
		/* callback scope for current fade effect */													\
		Object fxFadeCallbackScope;

// declare a CameraEffectManager
__CLASS(CameraEffectManager);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

CameraEffectManager CameraEffectManager_getInstance();

void CameraEffectManager_constructor(CameraEffectManager this);
void CameraEffectManager_destructor(CameraEffectManager this);
void CameraEffectManager_startEffect(CameraEffectManager this, int effect, va_list args);
void CameraEffectManager_stopEffect(CameraEffectManager this, int effect);
bool CameraEffectManager_handleMessage(CameraEffectManager this, Telegram telegram);
Brightness CameraEffectManager_getDefaultBrightness(CameraEffectManager this);


#endif
