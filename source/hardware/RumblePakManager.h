/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

/* This is based on Thunderstrucks' Rumble Pak library */

#ifndef __RUMBLE_PAK_MANAGER_H_
#define __RUMBLE_PAK_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <CommunicationManager.h>

//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __RUMBLE_MAX_EFFECTS_IN_CHAIN		 10

#define __RUMBLE_CHAIN_EFFECT_0				0x00
#define __RUMBLE_CHAIN_EFFECT_1				0x01
#define __RUMBLE_CHAIN_EFFECT_2				0x02
#define __RUMBLE_CHAIN_EFFECT_3				0x03
#define __RUMBLE_CHAIN_EFFECT_4				0x04

#define __RUMBLE_FREQ_160HZ					0x00
#define __RUMBLE_FREQ_240HZ					0x01
#define __RUMBLE_FREQ_320HZ					0x02
#define __RUMBLE_FREQ_400HZ					0x03

#define __RUMBLE_CMD_STOP					0x00
#define __RUMBLE_CMD_MIN_EFFECT				0x01
#define __RUMBLE_CMD_MAX_EFFECT				0x7B
#define __RUMBLE_CMD_PLAY					0x7C
#define __RUMBLE_CMD_CHAIN_EFFECT_0			0x80
#define __RUMBLE_CMD_CHAIN_EFFECT_1			0x81
#define __RUMBLE_CMD_CHAIN_EFFECT_2			0x82
#define __RUMBLE_CMD_CHAIN_EFFECT_3			0x83
#define __RUMBLE_CMD_CHAIN_EFFECT_4			0x84
#define __RUMBLE_CMD_FREQ_160HZ				0x90
#define __RUMBLE_CMD_FREQ_240HZ				0x91
#define __RUMBLE_CMD_FREQ_320HZ				0x92
#define __RUMBLE_CMD_FREQ_400HZ				0x93
#define __RUMBLE_CMD_OVERDRIVE				0xA0
#define __RUMBLE_CMD_SUSTAIN_POS			0xA1
#define __RUMBLE_CMD_SUSTAIN_NEG			0xA2
#define __RUMBLE_CMD_BREAK					0xA3
#define __RUMBLE_CMD_WRITE_EFFECT_CHAIN	 	0xB0

#define __RUMBLE_EFFECT_CHAIN_END		 	0xFF


#define __RUMBLE_TOTAL_COMMANDS				10

//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * A Rumble effect
 *
 * @memberof	Sprite
 */
typedef struct RumbleEffectSpec
{
	/// Effect #
	uint8 effect;
	/// Frequency
	uint8 frequency;
	/// Positive Sustain
	uint8 sustainPositive;
	/// Negative Sustain
	uint8 sustainNegative;
	/// Overdrive
	uint8 overdrive;
	/// Break
	uint8 breaking;
	/// Stop before starting
	bool stop;

} RumbleEffectSpec;

typedef const RumbleEffectSpec RumbleEffectROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup hardware
singleton class RumblePakManager : Object
{
	CommunicationManager communicationManager;
	uint8 rumbleCommands[__RUMBLE_TOTAL_COMMANDS];
	bool async;
	bool overridePreviousEffect;
	uint8 rumbleCommandIndex;
	const RumbleEffectSpec* rumbleEffect;
	uint8 frequency;
	uint8 sustainPositive;
	uint8 sustainNegative;
	uint8 overdrive;
	uint8 breaking;

	/// @publicsection
	static RumblePakManager getInstance();

	void reset();
	void setAsync(bool async);
	void setOverridePreviousEffect(bool overridePreviousEffect);
	static void startEffect(const RumbleEffectSpec* rumbleEffect);
	static void stopEffect(const RumbleEffectSpec* rumbleEffect);
	static void stopAllEffects();
}


#endif
