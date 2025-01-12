/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with rumbleManager source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <CommunicationManager.h>
#ifdef __DEBUG_TOOL
#include <Debug.h>
#endif
#include <DebugConfig.h>

#include "RumbleManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::startEffect(const RumbleEffectSpec* rumbleEffect)
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	if(isDeleted(rumbleManager))
	{
		RumbleManager::getInstance();

		if(isDeleted(rumbleManager))
		{
			RumbleManager::getInstance();
			return;
		}
	}

	rumbleManager->rumbleCommandIndex = 0;

	if(!rumbleManager->overridePreviousEffect && 0 != rumbleManager->rumbleCommandIndex)
	{
		return;
	}

	if(NULL == rumbleEffect)
	{
		return;
	}

	if(rumbleManager->rumbleEffectSpec == rumbleEffect)
	{
		if(rumbleEffect->stop)
		{
			RumbleManager::stop(rumbleManager);
		}

		RumbleManager::play(rumbleManager);
		RumbleManager::execute(rumbleManager);
		return;
	}

	rumbleManager->rumbleEffectSpec = rumbleEffect;

	if(rumbleEffect->stop)
	{
		RumbleManager::stop(rumbleManager);
	}

	RumbleManager::setFrequency(rumbleEffect->frequency);
	RumbleManager::setSustainPositive(rumbleEffect->sustainPositive);
	RumbleManager::setSustainNegative(rumbleEffect->sustainNegative);
	RumbleManager::setOverdrive(rumbleEffect->overdrive);
	RumbleManager::setBreak(rumbleEffect->breaking);
	RumbleManager::setEffect(rumbleEffect->effect);
	RumbleManager::execute(rumbleManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::stopEffect(const RumbleEffectSpec* rumbleEffect)
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	if(NULL == rumbleEffect || rumbleManager->rumbleEffectSpec == rumbleEffect)
	{
		RumbleManager::stop(rumbleManager);
		RumbleManager::execute(rumbleManager);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::reset()
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	rumbleManager->async = true;
	rumbleManager->overridePreviousEffect = true;
	rumbleManager->rumbleEffectSpec = NULL;
	rumbleManager->rumbleCommandIndex = 0;
	rumbleManager->cachedRumbleEffect.frequency = 0;
	rumbleManager->cachedRumbleEffect.sustainPositive = 0;
	rumbleManager->cachedRumbleEffect.sustainNegative = 0;
	rumbleManager->cachedRumbleEffect.overdrive = 0;
	rumbleManager->cachedRumbleEffect.breaking = 0;

	for(int32 i = 0; i < __RUMBLE_TOTAL_COMMANDS; i++)
	{
		rumbleManager->rumbleCommands[i] = 0;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::setAsync(bool async)
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	rumbleManager->async = async;
	RumbleManager::stopAllEffects(rumbleManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::setOverridePreviousEffect(bool overridePreviousEffect)
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	rumbleManager->overridePreviousEffect = overridePreviousEffect;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::sendCode(uint8 code __attribute__((unused)))
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	rumbleManager->rumbleCommands[rumbleManager->rumbleCommandIndex++] = code;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::execute()
{
// Rumble only is called in release mode since emulators that don't implement communications, 
// lock when trying to broadcast message throught the EXT port
#ifdef __EMU_ONLY
#if __EMU_ONLY
	return;
#endif
#endif

#ifdef __RELEASE
	RumbleManager rumbleManager = RumbleManager::getInstance();

	if(rumbleManager->async)
	{
		if(rumbleManager->overridePreviousEffect)
		{
			CommunicationManager::broadcastDataAsync
			(
				(BYTE*)rumbleManager->rumbleCommands, rumbleManager->rumbleCommandIndex, NULL, NULL
			);
		}
		else
		{
			CommunicationManager::broadcastDataAsync
			(
				(BYTE*)rumbleManager->rumbleCommands, rumbleManager->rumbleCommandIndex, 
				(EventListener)RumbleManager::onBroadcastDataDone, ListenerObject::safeCast(rumbleManager)
			);
		}
	}
	else
	{
		CommunicationManager::broadcastData((BYTE*)rumbleManager->rumbleCommands, rumbleManager->rumbleCommandIndex);
		rumbleManager->rumbleCommandIndex = 0;
	}
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::toggleAsync()
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	rumbleManager->async = !rumbleManager->async;
	RumbleManager::stopAllEffects(rumbleManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::sendCommandWithValue(uint8 command, uint8 value)
{
	RumbleManager::sendCode(command);
	RumbleManager::sendCode(value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::setEffect(uint8 effect)
{
	if(effect >= __RUMBLE_CMD_MIN_EFFECT && effect <= __RUMBLE_CMD_MAX_EFFECT)
	{
		RumbleManager::sendCode(effect);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::storeEffectChain(uint8 chainNumber, uint8* effectChain)
{
	uint8 i = 0;
	
	RumbleManager::sendCode(__RUMBLE_CMD_WRITE_EFFECT_CHAIN);
	
	RumbleManager::sendCode(chainNumber);
	
	for(i=0; effectChain[i] != __RUMBLE_EFFECT_CHAIN_END && i < __RUMBLE_MAX_EFFECTS_IN_CHAIN; i++)
	{
		RumbleManager::setEffect(effectChain[i]);
	}

	RumbleManager::sendCode(__RUMBLE_EFFECT_CHAIN_END);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::setEffectChain(uint8 effectChain)
{
	uint8 command = effectChain;

	if(command <= __RUMBLE_CHAIN_EFFECT_4)
	{
		command += __RUMBLE_CMD_CHAIN_EFFECT_0;
	}

	if(command >= __RUMBLE_CMD_CHAIN_EFFECT_0 && command <= __RUMBLE_CMD_CHAIN_EFFECT_4)
	{
		RumbleManager::sendCode(command);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::setFrequency(uint8 value)
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	if(rumbleManager->cachedRumbleEffect.frequency == value)
	{
		return;
	}

	rumbleManager->cachedRumbleEffect.frequency = value;
	
	if(value <= __RUMBLE_FREQ_400HZ)
	{
		value += __RUMBLE_CMD_FREQ_50HZ;
	}

	if(value >= __RUMBLE_CMD_FREQ_50HZ && value <= __RUMBLE_CMD_FREQ_400HZ)
	{
		RumbleManager::sendCode(value);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::setOverdrive(uint8 value)
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	if(rumbleManager->cachedRumbleEffect.overdrive == value)
	{
		return;
	}

	if(__RUMBLE_MAX_OVERDRIVE < value)
	{
		value = __RUMBLE_MAX_OVERDRIVE;
	}

	rumbleManager->cachedRumbleEffect.overdrive = value;

	RumbleManager::sendCommandWithValue(__RUMBLE_CMD_OVERDRIVE, value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::setSustainPositive(uint8 value)
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	if(rumbleManager->cachedRumbleEffect.sustainPositive == value)
	{
		return;
	}

	rumbleManager->cachedRumbleEffect.sustainPositive = value;

	RumbleManager::sendCommandWithValue(__RUMBLE_CMD_SUSTAIN_POS, value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::setSustainNegative(uint8 value)
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	if(rumbleManager->cachedRumbleEffect.sustainNegative == value)
	{
		return;
	}

	rumbleManager->cachedRumbleEffect.sustainNegative = value;

	RumbleManager::sendCommandWithValue(__RUMBLE_CMD_SUSTAIN_NEG, value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::setBreak(uint8 value)
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	if(rumbleManager->cachedRumbleEffect.breaking == value)
	{
		return;
	}

	rumbleManager->cachedRumbleEffect.breaking = value;

	RumbleManager::sendCommandWithValue(__RUMBLE_CMD_BREAK, value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::play()
{
	RumbleManager::sendCode(__RUMBLE_CMD_PLAY);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::stop()
{
	RumbleManager::sendCode(__RUMBLE_CMD_STOP);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::stopAllEffects()
{
	RumbleManager::stop();
	RumbleManager::execute();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::constructor()
{
	
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->async = true;
	this->overridePreviousEffect = true;
	this->rumbleEffectSpec = NULL;
	this->rumbleCommandIndex = 0;
	this->cachedRumbleEffect.frequency = 0;
	this->cachedRumbleEffect.sustainPositive = 0;
	this->cachedRumbleEffect.sustainNegative = 0;
	this->cachedRumbleEffect.overdrive = 0;
	this->cachedRumbleEffect.breaking = 0;

	RumbleManager::reset();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::destructor()
{
	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool RumbleManager::onBroadcastDataDone(ListenerObject eventFirer __attribute__ ((unused)))
{
	RumbleManager rumbleManager = RumbleManager::getInstance();

	rumbleManager->rumbleCommandIndex = 0;

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
