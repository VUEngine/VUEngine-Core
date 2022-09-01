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

#include <ListenerObject.h>
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
singleton class CameraEffectManager : ListenerObject
{
	// Target brightness for current fade effect
	Brightness fxFadeTargetBrightness;
	// Callback scope for current fade effect
	ListenerObject fxFadeCallbackScope;
	// Delay for current fade effect
	uint8 fxFadeDelay;
	// fade increment
	uint8 fadeEffectIncrement;

	/// @publicsection
	static CameraEffectManager getInstance();
	void constructor();
	void reset();
	void setFadeIncrement(uint8 fadeEffectIncrement);
	Brightness getDefaultBrightness();
	virtual void startEffect(int32 effect, va_list args);
	virtual void stopEffect(int32 effect);
	override bool handleMessage(Telegram telegram);
}


#endif
