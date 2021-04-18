/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <RumblePakManager.h>
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

static RumblePakManager _rumblePakManager = NULL;

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			RumblePakManager::getInstance()
 * @memberof	RumblePakManager
 * @public
 * @return		RumblePakManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void RumblePakManager::constructor()
{    
	Base::constructor();

    this->communicationManager = NULL;

    this->rumbleEffect = NULL;
    this->frequency = 0;
    this->sustainPositive = 0;
    this->sustainNegative = 0;
    this->overdrive = 0;
    this->breaking = 0;

    RumblePakManager::reset(this);

    _rumblePakManager = this;
}

/**
 * Class destructor
 */
void RumblePakManager::destructor()
{
    _rumblePakManager = NULL;

	// allow a new construct
	Base::destructor();
}

void RumblePakManager::reset()
{
    this->communicationManager = CommunicationManager::getInstance();

    this->rumbleEffect = NULL;
    this->frequency = __RUMBLE_FREQ_160HZ + 1;
    this->sustainPositive = 0;
    this->sustainNegative = 0;
    this->overdrive = 0;
    this->breaking = 0;

    RumblePakManager::setFrequency(__RUMBLE_FREQ_160HZ);
    RumblePakManager::setSustainPositive(255);
    RumblePakManager::setSustainNegative(255);
    RumblePakManager::setOverdrive(255);
    RumblePakManager::setBreak(255);
}

static void RumblePakManager::sendCode(u8 code __attribute__((unused)))
{
// Rumble only is called in release mode since emulators that don't implement communications, 
// lock when trying to broadcast message throught the EXT port
#ifdef __EMU_ONLY
#if __EMU_ONLY
	return;
#endif
#endif

    if(isDeleted(_rumblePakManager))
    {
        RumblePakManager::getInstance();

        if(isDeleted(_rumblePakManager))
        {
            RumblePakManager::getInstance();
            return;
        }
    }

#ifdef __RELEASE
    CommunicationManager::broadcastData(_rumblePakManager->communicationManager, code);
#endif
}

static void RumblePakManager::sendCommandWithValue(u8 command, u8 value)
{
    RumblePakManager::sendCode(command);
    RumblePakManager::sendCode(value);
}

static void RumblePakManager::playEffect(u8 effect)
{
    if(effect >= __RUMBLE_CMD_MIN_EFFECT && effect <= __RUMBLE_CMD_MAX_EFFECT)
	{
        RumblePakManager::sendCode(effect);
    }
}

static void RumblePakManager::storeEffectChain(u8 chainNumber, u8* effectChain)
{
    u8 i = 0;
    
    RumblePakManager::sendCode(__RUMBLE_CMD_WRITE_EFFECT_CHAIN);
    
    RumblePakManager::sendCode(chainNumber);
    
	for(i=0; effectChain[i] != __RUMBLE_EFFECT_CHAIN_END && i < __RUMBLE_MAX_EFFECTS_IN_CHAIN; i++)
	{
        RumblePakManager::playEffect(effectChain[i]);
    }

    RumblePakManager::sendCode(__RUMBLE_EFFECT_CHAIN_END);
}

static void RumblePakManager::playEffectChain(u8 effectChain)
{
    u8 command = effectChain;

    if(command <= __RUMBLE_CHAIN_EFFECT_4)
	{
        command +=  __RUMBLE_CMD_CHAIN_EFFECT_0;
    }

    if(command >= __RUMBLE_CMD_CHAIN_EFFECT_0 && command <= __RUMBLE_CMD_CHAIN_EFFECT_4)
	{
        RumblePakManager::sendCode(command);
    }
}

static void RumblePakManager::setFrequency(u8 value)
{
    if(_rumblePakManager->frequency == value)
    {
        return;
    }

    _rumblePakManager->frequency = value;
    
	if(value <= __RUMBLE_FREQ_400HZ)
	{
        value +=  __RUMBLE_CMD_FREQ_160HZ;
    }

    if(value >= __RUMBLE_CMD_FREQ_160HZ && value <= __RUMBLE_CMD_FREQ_400HZ)
	{
        RumblePakManager::sendCode(value);
    }
}

static void RumblePakManager::setOverdrive(u8 value)
{
    if(_rumblePakManager->overdrive == value)
    {
        return;
    }

    _rumblePakManager->overdrive = value;

    RumblePakManager::sendCommandWithValue(__RUMBLE_CMD_OVERDRIVE, value);
}

static void RumblePakManager::setSustainPositive(u8 value)
{
    if(_rumblePakManager->sustainPositive == value)
    {
        return;
    }

    _rumblePakManager->sustainPositive = value;

    RumblePakManager::sendCommandWithValue(__RUMBLE_CMD_SUSTAIN_POS, value);
}

static void RumblePakManager::setSustainNegative(u8 value)
{
    if(_rumblePakManager->sustainNegative == value)
    {
        return;
    }

    _rumblePakManager->sustainNegative = value;

    RumblePakManager::sendCommandWithValue(__RUMBLE_CMD_SUSTAIN_NEG, value);
}

static void RumblePakManager::setBreak(u8 value)
{
    if(_rumblePakManager->breaking == value)
    {
        return;
    }

    _rumblePakManager->breaking = value;

    RumblePakManager::sendCommandWithValue(__RUMBLE_CMD_BREAK, value);
}

static void RumblePakManager::play()
{
    RumblePakManager::sendCode(__RUMBLE_CMD_PLAY);
}

static void RumblePakManager::stop()
{
    RumblePakManager::sendCode(__RUMBLE_CMD_STOP);
}

static void RumblePakManager::startEffect(const RumbleEffectSpec* rumbleEffect)
{
    if(isDeleted(_rumblePakManager))
    {
        RumblePakManager::getInstance();

        if(isDeleted(_rumblePakManager))
        {
            RumblePakManager::getInstance();
            return;
        }
    }

    if(NULL == rumbleEffect)
    {
        return;
    }

    if(_rumblePakManager->rumbleEffect == rumbleEffect)
    {
        if(rumbleEffect->stop)
        {
            RumblePakManager::stop();
        }

		RumblePakManager::play();

        return;
    }

    _rumblePakManager->rumbleEffect = rumbleEffect;

    if(rumbleEffect->stop)
    {
        RumblePakManager::stop();
    }

    RumblePakManager::setFrequency(rumbleEffect->frequency);
    RumblePakManager::setSustainPositive(rumbleEffect->sustainPositive);
    RumblePakManager::setSustainNegative(rumbleEffect->sustainNegative);
    RumblePakManager::setOverdrive(rumbleEffect->overdrive);
    RumblePakManager::setBreak(rumbleEffect->breaking);
    RumblePakManager::playEffect(rumbleEffect->effect);
}

static void RumblePakManager::stopEffect(const RumbleEffectSpec* rumbleEffect)
{
    if(NULL == rumbleEffect)
    {
        return;
    }

    if(_rumblePakManager->rumbleEffect == rumbleEffect)
    {
        RumblePakManager::stop();
    }
}

static void RumblePakManager::stopAllEffects()
{
    RumblePakManager::stop();
}
