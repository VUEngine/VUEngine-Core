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

#include <CommunicationManager.h>
#ifdef __DEBUG_TOOL
#include <Debug.h>
#endif
#include <DebugConfig.h>

#include "RumbleManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static RumbleManager _rumbleManager = NULL;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::startEffect(const RumbleEffectSpec* rumbleEffect)
{
	if(isDeleted(_rumbleManager))
	{
		RumbleManager::getInstance();

		if(isDeleted(_rumbleManager))
		{
			RumbleManager::getInstance();
			return;
		}
	}

	_rumbleManager->rumbleCommandIndex = 0;

	if(!_rumbleManager->overridePreviousEffect && 0 != _rumbleManager->rumbleCommandIndex)
	{
		return;
	}

	if(NULL == rumbleEffect)
	{
		return;
	}

	if(_rumbleManager->rumbleEffectSpec == rumbleEffect)
	{
		if(rumbleEffect->stop)
		{
			RumbleManager::stop(_rumbleManager);
		}

		RumbleManager::play(_rumbleManager);
		RumbleManager::execute(_rumbleManager);
		return;
	}

	_rumbleManager->rumbleEffectSpec = rumbleEffect;

	if(rumbleEffect->stop)
	{
		RumbleManager::stop(_rumbleManager);
	}

	RumbleManager::setFrequency(_rumbleManager, rumbleEffect->frequency);
	RumbleManager::setSustainPositive(_rumbleManager, rumbleEffect->sustainPositive);
	RumbleManager::setSustainNegative(_rumbleManager, rumbleEffect->sustainNegative);
	RumbleManager::setOverdrive(_rumbleManager, rumbleEffect->overdrive);
	RumbleManager::setBreak(_rumbleManager, rumbleEffect->breaking);
	RumbleManager::setEffect(_rumbleManager, rumbleEffect->effect);
	RumbleManager::execute(_rumbleManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void RumbleManager::stopEffect(const RumbleEffectSpec* rumbleEffect)
{
	if(NULL == rumbleEffect || _rumbleManager->rumbleEffectSpec == rumbleEffect)
	{
		RumbleManager::stop(_rumbleManager);
		RumbleManager::execute(_rumbleManager);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::reset()
{
	this->communicationManager = CommunicationManager::getInstance();

	this->async = true;
	this->overridePreviousEffect = true;
	this->rumbleEffectSpec = NULL;
	this->rumbleCommandIndex = 0;
	this->cachedRumbleEffect.frequency = 0;
	this->cachedRumbleEffect.sustainPositive = 0;
	this->cachedRumbleEffect.sustainNegative = 0;
	this->cachedRumbleEffect.overdrive = 0;
	this->cachedRumbleEffect.breaking = 0;

	for(int32 i = 0; i < __RUMBLE_TOTAL_COMMANDS; i++)
	{
		this->rumbleCommands[i] = 0;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::setAsync(bool async)
{
	this->async = async;
	RumbleManager::stopAllEffects(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::setOverridePreviousEffect(bool overridePreviousEffect)
{
	this->overridePreviousEffect = overridePreviousEffect;
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

	this->communicationManager = NULL;

	this->async = true;
	this->overridePreviousEffect = true;
	this->rumbleEffectSpec = NULL;
	this->rumbleCommandIndex = 0;
	this->cachedRumbleEffect.frequency = 0;
	this->cachedRumbleEffect.sustainPositive = 0;
	this->cachedRumbleEffect.sustainNegative = 0;
	this->cachedRumbleEffect.overdrive = 0;
	this->cachedRumbleEffect.breaking = 0;

	RumbleManager::reset(this);

	_rumbleManager = this;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::destructor()
{
	_rumbleManager = NULL;

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::sendCode(uint8 code __attribute__((unused)))
{
	this->rumbleCommands[this->rumbleCommandIndex++] = code;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::execute()
{
// Rumble only is called in release mode since emulators that don't implement communications, 
// lock when trying to broadcast message throught the EXT port
#ifdef __EMU_ONLY
#if __EMU_ONLY
	return;
#endif
#endif

#ifdef __RELEASE
	if(this->async)
	{
		if(this->overridePreviousEffect)
		{
			CommunicationManager::broadcastDataAsync
			(
				this->communicationManager, (BYTE*)this->rumbleCommands, this->rumbleCommandIndex, NULL, NULL
			);
		}
		else
		{
			CommunicationManager::broadcastDataAsync
			(
				this->communicationManager, (BYTE*)this->rumbleCommands, this->rumbleCommandIndex, 
				(EventListener)RumbleManager::onBroadcastDataDone, ListenerObject::safeCast(this)
			);
		}
	}
	else
	{
		CommunicationManager::broadcastData(this->communicationManager, (BYTE*)this->rumbleCommands, this->rumbleCommandIndex);
		this->rumbleCommandIndex = 0;
	}
#endif
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool RumbleManager::onBroadcastDataDone(ListenerObject eventFirer __attribute__ ((unused)))
{
	this->rumbleCommandIndex = 0;

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::toggleAsync()
{
	this->async = !this->async;
	RumbleManager::stopAllEffects(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::sendCommandWithValue(uint8 command, uint8 value)
{
	RumbleManager::sendCode(this, command);
	RumbleManager::sendCode(this, value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::setEffect(uint8 effect)
{
	if(effect >= __RUMBLE_CMD_MIN_EFFECT && effect <= __RUMBLE_CMD_MAX_EFFECT)
	{
		RumbleManager::sendCode(this, effect);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::storeEffectChain(uint8 chainNumber, uint8* effectChain)
{
	uint8 i = 0;
	
	RumbleManager::sendCode(this, __RUMBLE_CMD_WRITE_EFFECT_CHAIN);
	
	RumbleManager::sendCode(this, chainNumber);
	
	for(i=0; effectChain[i] != __RUMBLE_EFFECT_CHAIN_END && i < __RUMBLE_MAX_EFFECTS_IN_CHAIN; i++)
	{
		RumbleManager::setEffect(this, effectChain[i]);
	}

	RumbleManager::sendCode(this, __RUMBLE_EFFECT_CHAIN_END);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::setEffectChain(uint8 effectChain)
{
	uint8 command = effectChain;

	if(command <= __RUMBLE_CHAIN_EFFECT_4)
	{
		command += __RUMBLE_CMD_CHAIN_EFFECT_0;
	}

	if(command >= __RUMBLE_CMD_CHAIN_EFFECT_0 && command <= __RUMBLE_CMD_CHAIN_EFFECT_4)
	{
		RumbleManager::sendCode(this, command);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::setFrequency(uint8 value)
{
	if(this->cachedRumbleEffect.frequency == value)
	{
		return;
	}

	this->cachedRumbleEffect.frequency = value;
	
	if(value <= __RUMBLE_FREQ_400HZ)
	{
		value += __RUMBLE_CMD_FREQ_50HZ;
	}

	if(value >= __RUMBLE_CMD_FREQ_50HZ && value <= __RUMBLE_CMD_FREQ_400HZ)
	{
		RumbleManager::sendCode(this, value);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::setOverdrive(uint8 value)
{
	if(this->cachedRumbleEffect.overdrive == value)
	{
		return;
	}

	if(__RUMBLE_MAX_OVERDRIVE < value)
	{
		value = __RUMBLE_MAX_OVERDRIVE;
	}

	this->cachedRumbleEffect.overdrive = value;

	RumbleManager::sendCommandWithValue(this, __RUMBLE_CMD_OVERDRIVE, value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::setSustainPositive(uint8 value)
{
	if(this->cachedRumbleEffect.sustainPositive == value)
	{
		return;
	}

	this->cachedRumbleEffect.sustainPositive = value;

	RumbleManager::sendCommandWithValue(this, __RUMBLE_CMD_SUSTAIN_POS, value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::setSustainNegative(uint8 value)
{
	if(this->cachedRumbleEffect.sustainNegative == value)
	{
		return;
	}

	this->cachedRumbleEffect.sustainNegative = value;

	RumbleManager::sendCommandWithValue(this, __RUMBLE_CMD_SUSTAIN_NEG, value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::setBreak(uint8 value)
{
	if(this->cachedRumbleEffect.breaking == value)
	{
		return;
	}

	this->cachedRumbleEffect.breaking = value;

	RumbleManager::sendCommandWithValue(this, __RUMBLE_CMD_BREAK, value);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::play()
{
	RumbleManager::sendCode(this, __RUMBLE_CMD_PLAY);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::stop()
{
	RumbleManager::sendCode(this, __RUMBLE_CMD_STOP);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void RumbleManager::stopAllEffects()
{
	RumbleManager::stop(this);
	RumbleManager::execute(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
