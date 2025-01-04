/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CAMERA_EFFECT_MANAGER_H_
#define CAMERA_EFFECT_MANAGER_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <stdarg.h>
#include <ListenerObject.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Entity;
class Telegram;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

enum CameraFX
{
	kShow = 0,
	kHide,
	kFadeIn,
	kFadeOut,
	kFadeTo,

	kCameraLastFX
};


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class CameraEffectManager
///
/// Inherits from ListenerObject
///
/// Manages camera's special effects, brightness transitions, etc.
singleton class CameraEffectManager : ListenerObject
{
	/// Target brightness for the current fade effect
	Brightness fxFadeTargetBrightness;

	/// Callback scope for the current fade effect
	ListenerObject fxFadeCallbackScope;

	/// Delay for the current fade effect
	uint8 fxFadeDelay;

	/// Fade increment
	uint8 fadeEffectIncrement;

	/// Flag to signal that the current event listener has to be removed when the effect is complete
	bool startingANewEffect;

	/// @publicsection
	
	/// Method to retrieve the singleton instance
	/// @return CameraEffectManager singleton
	static CameraEffectManager getInstance();

	/// Class' constructor
	void constructor();

	/// Receive and process a Telegram.
	/// @param telegram: Received telegram to process
	/// @return True if the telegram was processed
	override bool handleMessage(Telegram telegram);

	/// Reset the manager's state
	void reset();

	/// Set the fade increment to apply on the next effect.
	/// @param fadeEffectIncrement: Fade increment
	void setFadeIncrement(uint8 fadeEffectIncrement);

	/// Retrieve the default brighness values for the current stage
	/// @return Struct with the brightness levels
	Brightness getDefaultBrightness();

	/// Start a camera effect.
	/// @param effect: Code of the effect to start
	/// @param args: Variable arguments list depending on the effect to start
	virtual void startEffect(int32 effect, va_list args);
	
	/// Stop a camera effect.
	/// @param effect: Code of the effect to stop
	virtual void stopEffect(int32 effect);
}


#endif
