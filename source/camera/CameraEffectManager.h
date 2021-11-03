/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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

/// @ingroup camera
singleton class CameraEffectManager : Object
{
	// Target brightness for current fade effect
	Brightness fxFadeTargetBrightness;
	// Delay for current fade effect
	uint8 fxFadeDelay;
	// Callback scope for current fade effect
	Object fxFadeCallbackScope;

	/// @publicsection
	static CameraEffectManager getInstance();
	void constructor();
	Brightness getDefaultBrightness();
	virtual void startEffect(int32 effect, va_list args);
	virtual void stopEffect(int32 effect);
	override bool handleMessage(Telegram telegram);
}


#endif
