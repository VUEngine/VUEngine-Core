/*
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


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Object.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class CommunicationManager;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __RUMBLE_MAX_EFFECTS_IN_CHAIN				8
#define __RUMBLE_MAX_OVERDRIVE						126
#define __RUMBLE_CHAIN_EFFECT_0						0x00
#define __RUMBLE_CHAIN_EFFECT_1						0x01
#define __RUMBLE_CHAIN_EFFECT_2						0x02
#define __RUMBLE_CHAIN_EFFECT_3						0x03
#define __RUMBLE_CHAIN_EFFECT_4						0x04
#define __RUMBLE_FREQ_50HZ							0x04
#define __RUMBLE_FREQ_95HZ							0x05
#define __RUMBLE_FREQ_130HZ							0x06
#define __RUMBLE_FREQ_160HZ							0x00
#define __RUMBLE_FREQ_240HZ							0x01
#define __RUMBLE_FREQ_320HZ							0x02
#define __RUMBLE_FREQ_400HZ							0x03
#define __RUMBLE_CMD_STOP							0x00
#define __RUMBLE_CMD_MIN_EFFECT						0x01
#define __RUMBLE_CMD_MAX_EFFECT						0x7B
#define __RUMBLE_CMD_PLAY							0x7C
#define __RUMBLE_CMD_CHAIN_EFFECT_0					0x80
#define __RUMBLE_CMD_CHAIN_EFFECT_1					0x81
#define __RUMBLE_CMD_CHAIN_EFFECT_2					0x82
#define __RUMBLE_CMD_CHAIN_EFFECT_3					0x83
#define __RUMBLE_CMD_CHAIN_EFFECT_4					0x84
#define __RUMBLE_CMD_FREQ_50HZ						0x87
#define __RUMBLE_CMD_FREQ_95HZ						0x88
#define __RUMBLE_CMD_FREQ_130HZ						0x89
#define __RUMBLE_CMD_FREQ_160HZ						0x90
#define __RUMBLE_CMD_FREQ_240HZ						0x91
#define __RUMBLE_CMD_FREQ_320HZ						0x92
#define __RUMBLE_CMD_FREQ_400HZ						0x93
#define __RUMBLE_CMD_OVERDRIVE						0xA0
#define __RUMBLE_CMD_SUSTAIN_POS					0xA1
#define __RUMBLE_CMD_SUSTAIN_NEG					0xA2
#define __RUMBLE_CMD_BREAK							0xA3
#define __RUMBLE_CMD_WRITE_EFFECT_CHAIN				0xB0
#define __RUMBLE_CMD_WRITE_EFFECT_LOOPS_CHAIN		0xB1
#define __RUMBLE_EFFECT_CHAIN_END				 	0xFF
#define __RUMBLE_TOTAL_COMMANDS						10


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// A rumble effect spec
/// @memberof RumbleManager
typedef struct RumbleEffectSpec
{
	/// Effect number
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

/// A A rumble effect spec that is stored in ROM
/// @memberof RumbleManager
typedef const RumbleEffectSpec RumbleEffectROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class RumbleManager
///
/// Inherits from Object
///
/// Manages rumble effects.
singleton class RumbleManager : Object
{
	/// @protectedsection

	/// Used to broadcast the rumble commands over the EXT port
	CommunicationManager communicationManager;

	/// Queue of commands to broadcast
	uint8 rumbleCommands[__RUMBLE_TOTAL_COMMANDS];

	/// Determines if the commands are broadcasted asynchronously,
	/// defaults to true
	bool async;

	/// Determines if the broadcast of new effects should wait or not 
	/// for a previous queue effect being completedly broadcasted
	bool overridePreviousEffect;

	/// Index of the command in the queue to broadcast next
	uint8 rumbleCommandIndex;

	/// Rumble effect spec being broadcasted
	const RumbleEffectSpec* rumbleEffectSpec;

	/// Cached rumble effect to prevent broadcasting again previous send commands
	RumbleEffectSpec cachedRumbleEffect;

	/// @publicsection

	/// Method to retrieve the singleton instance
	/// @return RumbleManager singleton
	static RumbleManager getInstance();

	/// Start a rumble effect configured with the provided spec.
	/// @param rumbleEffectSpec: Specification of the rumble effect to play
	static void startEffect(const RumbleEffectSpec* rumbleEffectSpec);

	/// Stop a rumble effect configured with the provided spec.
	/// @param rumbleEffectSpec: Specification of the rumble effect to stop; if NULL,
	/// any playing effect is stoped
	static void stopEffect(const RumbleEffectSpec* rumbleEffectSpec);
	
	/// Reset the manager's state.
	void reset();

	/// Set the async flag.
	/// @param async: If true, rumble commands are broadcasted asynchronously
	void setAsync(bool async);

	/// Set the flag to broadcast new effects regardless of if there is a previous 
	/// queue effect pending broadcasted
	/// @param overridePreviousEffect: If true, new effects are broadcasted regardless of if 
	/// there is a queued effect pending broadcasting
	void setOverridePreviousEffect(bool overridePreviousEffect);
}

#endif
