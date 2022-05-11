/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <RumbleManager.h>
#include <CommunicationManager.h>
#ifdef __DEBUG_TOOLS
#include <Debug.h>
#endif
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS'S GLOBALS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
//												CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

static RumbleManager _rumbleManager = NULL;

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			RumbleManager::getInstance()
 * @memberof	RumbleManager
 * @public
 * @return		RumbleManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void RumbleManager::constructor()
{    
	Base::constructor();

    this->communicationManager = NULL;

    this->async = true;
	this->overridePreviousEffect = true;
    this->rumbleEffect = NULL;
    this->frequency = 0;
    this->sustainPositive = 0;
    this->sustainNegative = 0;
    this->overdrive = 0;
    this->breaking = 0;
    this->rumbleCommandIndex = 0;

    RumbleManager::reset(this);

    _rumbleManager = this;
}

/**
 * Class destructor
 */
void RumbleManager::destructor()
{
    _rumbleManager = NULL;

	// allow a new construct
	Base::destructor();
}

void RumbleManager::reset()
{
    this->communicationManager = CommunicationManager::getInstance();

    this->async = true;
    this->rumbleEffect = NULL;
    this->frequency = __RUMBLE_FREQ_160HZ + 1;
    this->sustainPositive = 0;
    this->sustainNegative = 0;
    this->overdrive = 0;
    this->breaking = 0;
    this->rumbleCommandIndex = 0;

    for(int32 i = 0; i < __RUMBLE_TOTAL_COMMANDS; i++)
    {
        this->rumbleCommands[i] = 0;
    }
}

static void RumbleManager::sendCode(uint8 code __attribute__((unused)))
{
    _rumbleManager->rumbleCommands[_rumbleManager->rumbleCommandIndex++] = code;
}

static void RumbleManager::execute()
{
// Rumble only is called in release mode since emulators that don't implement communications, 
// lock when trying to broadcast message throught the EXT port
#ifdef __EMU_ONLY
#if __EMU_ONLY
	return;
#endif
#endif

    if(isDeleted(_rumbleManager))
    {
        RumbleManager::getInstance();

        if(isDeleted(_rumbleManager))
        {
            RumbleManager::getInstance();
            return;
        }
    }

#ifdef __RELEASE
    if(_rumbleManager->async)
    {
		if(_rumbleManager->overridePreviousEffect)
		{
	        CommunicationManager::broadcastDataAsync(_rumbleManager->communicationManager, (BYTE*)_rumbleManager->rumbleCommands, _rumbleManager->rumbleCommandIndex, NULL, NULL);
		}
		else
		{
	        CommunicationManager::broadcastDataAsync(_rumbleManager->communicationManager, (BYTE*)_rumbleManager->rumbleCommands, _rumbleManager->rumbleCommandIndex, (EventListener)RumbleManager::onBroadcastDataDone, Object::safeCast(_rumbleManager));
		}
    }
    else
    {
        CommunicationManager::broadcastData(_rumbleManager->communicationManager, (BYTE*)_rumbleManager->rumbleCommands, _rumbleManager->rumbleCommandIndex);
        _rumbleManager->rumbleCommandIndex = 0;
    }
#endif
}

void RumbleManager::onBroadcastDataDone(Object eventFirer __attribute__ ((unused)))
{
    this->rumbleCommandIndex = 0;
}

void RumbleManager::toggleAsync()
{
    this->async = !this->async;
    RumbleManager::stopAllEffects(this);
}

void RumbleManager::setAsync(bool async)
{
	this->async = async;
    RumbleManager::stopAllEffects(this);
}

void RumbleManager::setOverridePreviousEffect(bool overridePreviousEffect)
{
	this->overridePreviousEffect = overridePreviousEffect;
}

static void RumbleManager::sendCommandWithValue(uint8 command, uint8 value)
{
    RumbleManager::sendCode(command);
    RumbleManager::sendCode(value);
}

static void RumbleManager::playEffect(uint8 effect)
{
    if(effect >= __RUMBLE_CMD_MIN_EFFECT && effect <= __RUMBLE_CMD_MAX_EFFECT)
	{
        RumbleManager::sendCode(effect);
    }
}

static void RumbleManager::storeEffectChain(uint8 chainNumber, uint8* effectChain)
{
    uint8 i = 0;
    
    RumbleManager::sendCode(__RUMBLE_CMD_WRITE_EFFECT_CHAIN);
    
    RumbleManager::sendCode(chainNumber);
    
	for(i=0; effectChain[i] != __RUMBLE_EFFECT_CHAIN_END && i < __RUMBLE_MAX_EFFECTS_IN_CHAIN; i++)
	{
        RumbleManager::playEffect(effectChain[i]);
    }

    RumbleManager::sendCode(__RUMBLE_EFFECT_CHAIN_END);
}

static void RumbleManager::playEffectChain(uint8 effectChain)
{
    uint8 command = effectChain;

    if(command <= __RUMBLE_CHAIN_EFFECT_4)
	{
        command +=  __RUMBLE_CMD_CHAIN_EFFECT_0;
    }

    if(command >= __RUMBLE_CMD_CHAIN_EFFECT_0 && command <= __RUMBLE_CMD_CHAIN_EFFECT_4)
	{
        RumbleManager::sendCode(command);
    }
}

static void RumbleManager::setFrequency(uint8 value)
{
    if(_rumbleManager->frequency == value)
    {
        return;
    }

    _rumbleManager->frequency = value;
    
	if(value <= __RUMBLE_FREQ_400HZ)
	{
        value +=  __RUMBLE_CMD_FREQ_160HZ;
    }

    if(value >= __RUMBLE_CMD_FREQ_160HZ && value <= __RUMBLE_CMD_FREQ_400HZ)
	{
        RumbleManager::sendCode(value);
    }
}

static void RumbleManager::setOverdrive(uint8 value)
{
    if(_rumbleManager->overdrive == value)
    {
        return;
    }

    _rumbleManager->overdrive = value;

    RumbleManager::sendCommandWithValue(__RUMBLE_CMD_OVERDRIVE, value);
}

static void RumbleManager::setSustainPositive(uint8 value)
{
    if(_rumbleManager->sustainPositive == value)
    {
        return;
    }

    _rumbleManager->sustainPositive = value;

    RumbleManager::sendCommandWithValue(__RUMBLE_CMD_SUSTAIN_POS, value);
}

static void RumbleManager::setSustainNegative(uint8 value)
{
    if(_rumbleManager->sustainNegative == value)
    {
        return;
    }

    _rumbleManager->sustainNegative = value;

    RumbleManager::sendCommandWithValue(__RUMBLE_CMD_SUSTAIN_NEG, value);
}

static void RumbleManager::setBreak(uint8 value)
{
    if(_rumbleManager->breaking == value)
    {
        return;
    }

    _rumbleManager->breaking = value;

    RumbleManager::sendCommandWithValue(__RUMBLE_CMD_BREAK, value);
}

static void RumbleManager::play()
{
    RumbleManager::sendCode(__RUMBLE_CMD_PLAY);
}

static void RumbleManager::stop()
{
    RumbleManager::sendCode(__RUMBLE_CMD_STOP);
}

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

    if(_rumbleManager->rumbleEffect == rumbleEffect)
    {
        if(rumbleEffect->stop)
        {
            RumbleManager::stop();
        }

		RumbleManager::play();
        RumbleManager::execute();
        return;
    }

    _rumbleManager->rumbleEffect = rumbleEffect;

    if(rumbleEffect->stop)
    {
        RumbleManager::stop();
    }

    RumbleManager::setFrequency(rumbleEffect->frequency);
    RumbleManager::setSustainPositive(rumbleEffect->sustainPositive);
    RumbleManager::setSustainNegative(rumbleEffect->sustainNegative);
    RumbleManager::setOverdrive(rumbleEffect->overdrive);
    RumbleManager::setBreak(rumbleEffect->breaking);
    RumbleManager::playEffect(rumbleEffect->effect);
    RumbleManager::execute();
}

static void RumbleManager::stopEffect(const RumbleEffectSpec* rumbleEffect)
{
    if(NULL == rumbleEffect)
    {
        return;
    }

    if(_rumbleManager->rumbleEffect == rumbleEffect)
    {
        RumbleManager::stop();
        RumbleManager::execute();
    }
}

static void RumbleManager::stopAllEffects()
{
    RumbleManager::stop();
    RumbleManager::execute();
}
