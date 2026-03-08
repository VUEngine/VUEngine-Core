/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Camera.h>
#include <DebugConfig.h>
#include <Actor.h>
#include <GameState.h>
#include <Singleton.h>
#include <Telegram.h>
#include <Timer.h>
#include <DisplayUnit.h>
#include <VUEngine.h>

#include "CameraEffectManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Camera;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	// Init class variables
	//this->fxFadeTargetBrightness = (Brightness){0, 0, 0};
	this->fxFadeDelay = 0;
	this->fxFadeScope = NULL;
	this->fadeEffectIncrement = __CAMERA_EFFECT_FADE_INCREMENT;
	this->startingANewEffect = false;
	//this->currentBrightness = (Brightness){0, 0, 0};
	//DisplayUnit::configureBrightness(this->currentBrightness);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::destructor()
{
	// Stop any effects
	CameraEffectManager::stopEffect(this, kFadeTo);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool CameraEffectManager::handleMessage(Telegram telegram)
{
	switch(Telegram::getMessage(telegram))
	{
		case kFadeTo:
		{				
			CameraEffectManager::fxFadeAsync(this);
			break;
		}	
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::reset()
{
	this->fadeEffectIncrement = __CAMERA_EFFECT_FADE_INCREMENT;

	CameraEffectManager::stopEffect(this, kFadeTo);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::setFadeIncrement(uint8 fadeEffectIncrement)
{
	this->fadeEffectIncrement = fadeEffectIncrement;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Brightness CameraEffectManager::getDefaultBrightness()
{
	// Default brightness settings
	Brightness brightness = (Brightness)
	{
		__BRIGHTNESS_DARK_RED,
		__BRIGHTNESS_MEDIUM_RED,
		__BRIGHTNESS_BRIGHT_RED,
	};

/*
	// If exists, get brightness settings from stage spec
	Stage stage = VUEngine::getStage();
	
	if(!isDeleted(stage))
	{
		StageSpec* stageSpec = Stage::getSpec(stage);
		brightness = stageSpec->rendering.colorConfig.brightness;
	}
*/
	return brightness;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::startEffect(int32 effect, va_list args)
{
	switch(effect)
	{
		case kShow:
		{
			DisplayUnit::upBrightness();
			//this->currentBrightness = CameraEffectManager::getDefaultBrightness(this);
			//DisplayUnit::configureBrightness(this->currentBrightness);
			break;
		}

		case kHide:
		{
			DisplayUnit::lowerBrightness();
			//this->currentBrightness = (Brightness){0, 0, 0};
			//DisplayUnit::configureBrightness(this->currentBrightness);
			break;
		}
		
		case kFadeIn:
		case kFadeOut:
		{
			CameraEffectManager::fxFadeStart(this, effect, va_arg(args, int32));
			break;
		}

		case kFadeTo:
		{
//			this->currentBrightness = DisplayUnit::getBrightness();

			CameraEffectManager::fxFadeAsyncStart
			(
				this,
				va_arg(args, int32),
				va_arg(args, Brightness*),
				va_arg(args, int32),
				va_arg(args, ListenerObject)
			);
			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::stopEffect(int32 effect)
{
	switch(effect)
	{
		case kFadeTo:

			CameraEffectManager::fxFadeAsyncStop(this);
			break;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeStart(int32 effect, int32 delay)
{
	switch(effect)
	{
		case kFadeIn:
		{
			DisplayUnit::upBrightness();
			/*
			this->currentBrightness = (Brightness){0, 0, 0};
			DisplayUnit::configureBrightness(this->currentBrightness);

			Brightness defaultBrightness = CameraEffectManager::getDefaultBrightness(this);

			Timer::repeatMethodCall
			(
				defaultBrightness.darkRed,
				(delay * defaultBrightness.darkRed),
				ListenerObject::safeCast(this),
				(void (*)(ListenerObject, uint32))&CameraEffectManager::fxFadeIn
			);
			*/

			break;
		}

		case kFadeOut:
		{
			DisplayUnit::lowerBrightness();
			/*
			this->currentBrightness = CameraEffectManager::getDefaultBrightness(this);
			DisplayUnit::configureBrightness(this->currentBrightness);

			Timer::repeatMethodCall
			(
				this->currentBrightness.darkRed,
				(delay * this->currentBrightness.darkRed),
				ListenerObject::safeCast(this),
				(void (*)(ListenerObject, uint32))&CameraEffectManager::fxFadeOut
			);
*/
			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeAsyncStart
(
	int32 initialDelay, const Brightness* targetBrightness, int32 delayBetweenSteps, ListenerObject scope
)
{
	// Stop previous effect
	CameraEffectManager::stopEffect(this, kFadeTo);

	this->startingANewEffect = true;	

/*
	// Set target brightness
	if(NULL == targetBrightness)
	{
		this->fxFadeTargetBrightness = CameraEffectManager::getDefaultBrightness(this);
	}
	else
	{
		this->fxFadeTargetBrightness = *targetBrightness;
	}
*/	
	// Set effect parameters
	this->fxFadeDelay = (0 >= delayBetweenSteps) ? 1 : (uint8)(delayBetweenSteps);

/*
	if(scope != NULL)
	{
		this->fxFadeScope = scope;

		if
		(
			0 == this->currentBrightness.darkRed
			&&
			0 == this->currentBrightness.mediumRed
			&&
			0 == this->currentBrightness.brightRed
		)
		{		
			CameraEffectManager::addEventListener(this, scope, kEventEffectFadeInComplete);
		}
		else
		{		
			CameraEffectManager::addEventListener(this, scope, kEventEffectFadeInComplete);
			CameraEffectManager::addEventListener(this, scope, kEventEffectFadeOutComplete);
		}
	}
*/
	if(scope != NULL)
	{
		CameraEffectManager::addEventListener(this, scope, kEventEffectFadeInComplete);
		CameraEffectManager::addEventListener(this, scope, kEventEffectFadeOutComplete);
	}
	// Start effect
	// TODO: check if the message really needs to be delayed.
	initialDelay = 0 >= initialDelay ? 1 : initialDelay;
	CameraEffectManager::sendMessageToSelf(this, kFadeTo, initialDelay, 0);

	// Fire effect started event
	CameraEffectManager::fireEvent(this, kEventEffectFadeStart);
	CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeStart);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeAsyncStop()
{
	// Remove event listener
	CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeInComplete);
	CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeOutComplete);

	// Discard pending delayed messages to stop effect
	CameraEffectManager::discardMessages(this, kFadeTo);

	// Reset effect variables
//	this->fxFadeTargetBrightness = (Brightness){0, 0, 0};
	this->fxFadeDelay = 0;
	this->fxFadeScope = NULL;

	// Fire effect stopped event
	CameraEffectManager::fireEvent(this, kEventEffectFadeStop);
	CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeStop);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeIn(uint32 call __attribute__((unused)))
{
	DisplayUnit::upBrightness();
	/*
	this->currentBrightness.darkRed += 1;
	this->currentBrightness.mediumRed += 2;
	this->currentBrightness.brightRed += 1;
	
	DisplayUnit::configureBrightness(this->currentBrightness);
*/
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeOut()
{
	DisplayUnit::lowerBrightness();
	/*
	this->currentBrightness.darkRed -= 1;
	this->currentBrightness.mediumRed -= 2;
	this->currentBrightness.brightRed -= 1;
	
	DisplayUnit::configureBrightness(this->currentBrightness);
	*/
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CameraEffectManager::fxFadeAsync()
{
	ASSERT(this, "CameraEffectManager::fxFadeAsync: invalid this");

/*
	// Note: need to cast brightness registers to uint8 because only their lower 8 bits are valid
	bool lightRedDone 	= this->currentBrightness.brightRed == this->fxFadeTargetBrightness.brightRed;
	bool mediumRedDone 	= this->currentBrightness.mediumRed == this->fxFadeTargetBrightness.mediumRed;
	bool darkRedDone 	= this->currentBrightness.darkRed == this->fxFadeTargetBrightness.darkRed;

	// Fade light red
	if(!lightRedDone)
	{
		if(this->currentBrightness.brightRed + this->fadeEffectIncrement * 4 < this->fxFadeTargetBrightness.brightRed)
		{
			this->currentBrightness.brightRed += this->fadeEffectIncrement * 4;
		}
		else if(this->currentBrightness.brightRed > this->fxFadeTargetBrightness.brightRed + this->fadeEffectIncrement * 4)
		{
			this->currentBrightness.brightRed -= this->fadeEffectIncrement * 4;
		}
		else
		{
			this->currentBrightness.brightRed = this->fxFadeTargetBrightness.brightRed;
			lightRedDone = true;
		}
	}

	if(!mediumRedDone)
	{
		// Fade medium red
		for(uint16 i = 0; i < 2; i++)
		{
			if(this->currentBrightness.mediumRed + this->fadeEffectIncrement < this->fxFadeTargetBrightness.mediumRed)
			{
				this->currentBrightness.mediumRed += this->fadeEffectIncrement;
			}
			else if(this->currentBrightness.mediumRed > this->fxFadeTargetBrightness.mediumRed + this->fadeEffectIncrement)
			{
				this->currentBrightness.mediumRed -= this->fadeEffectIncrement;
			}
			else
			{
				this->currentBrightness.mediumRed = this->fxFadeTargetBrightness.mediumRed;
				mediumRedDone = true;
				break;
			}
		}
	}

	if(!darkRedDone)
	{
		// Fade dark red
		if(this->currentBrightness.darkRed + this->fadeEffectIncrement < this->fxFadeTargetBrightness.darkRed)
		{
			this->currentBrightness.darkRed += this->fadeEffectIncrement;
		}
		else if(this->currentBrightness.darkRed > this->fxFadeTargetBrightness.darkRed + this->fadeEffectIncrement)
		{
			this->currentBrightness.darkRed -= this->fadeEffectIncrement;
		}
		else
		{
			this->currentBrightness.darkRed = this->fxFadeTargetBrightness.darkRed;
			darkRedDone = true;
		}
	}
	DisplayUnit::configureBrightness(this->currentBrightness);
*/
	DisplayUnit::upBrightness();

	// Finish effect or call next round
	if(true)
//	if(lightRedDone && mediumRedDone && darkRedDone)
	{
		this->startingANewEffect = false;

		// Fire effect ended event
		CameraEffectManager::fireEvent(this, kEventEffectFadeInComplete);
		CameraEffectManager::fireEvent(this, kEventEffectFadeOutComplete);

		if(!this->startingANewEffect)
		{
			CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeInComplete);
			CameraEffectManager::removeEventListeners(this, NULL, kEventEffectFadeOutComplete);
		}
	}
	else
	{
		CameraEffectManager::sendMessageToSelf(this, kFadeTo, this->fxFadeDelay, 0);
	}
}


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
