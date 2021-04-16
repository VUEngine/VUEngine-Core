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

    this->communicationManager = CommunicationManager::getInstance();

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
    CommunicationManager::broadcastData(_rumblePakManager->communicationManager, &code, sizeof(code));
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

    if(command >= __RUMBLE_CHAIN_EFFECT_0 && command <= __RUMBLE_CHAIN_EFFECT_4)
	{
        command +=  __RUMBLE_CMD_CHAIN_EFFECT_0;
    }

    if(command >= __RUMBLE_CMD_CHAIN_EFFECT_0 && command <= __RUMBLE_CMD_CHAIN_EFFECT_4)
	{
        RumblePakManager::sendCode(command);
    }
}

static void RumblePakManager::setFrequency(u8 frequency)
{
    u8 command = frequency;
    
	if(command >= __RUMBLE_FREQ_160HZ && command <= __RUMBLE_FREQ_400HZ)
	{
        command +=  __RUMBLE_CMD_FREQ_160HZ;
    }

    if(command >= __RUMBLE_CMD_FREQ_160HZ && command <= __RUMBLE_CMD_FREQ_400HZ)
	{
        RumblePakManager::sendCode(command);
    }
}

static void RumblePakManager::setOverdrive(u8 value)
{
    RumblePakManager::sendCommandWithValue(__RUMBLE_CMD_OVERDRIVE, value);
}

static void RumblePakManager::setSustainPositive(u8 value)
{
    RumblePakManager::sendCommandWithValue(__RUMBLE_CMD_SUSTAIN_POS, value);
}

static void RumblePakManager::setSustainNegative(u8 value)
{
    RumblePakManager::sendCommandWithValue(__RUMBLE_CMD_SUSTAIN_NEG, value);
}

static void RumblePakManager::setBreak(u8 value)
{
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