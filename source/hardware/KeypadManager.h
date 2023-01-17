/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef KEY_PAD_MANAGER_H_
#define KEY_PAD_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>


//---------------------------------------------------------------------------------------------------------
//											MACROS
//---------------------------------------------------------------------------------------------------------

// hardware reg __SCR specs
#define	__S_INTDIS		0x80 	// Disable Interrupts
#define	__S_SW			0x20 	// Software Reading
#define	__S_SWCK		0x10 	// Software Clock, Interrupt?
#define	__S_HW			0x04 	// Hardware Reading
#define	__S_STAT		0x02 	// Hardware Reading Status
#define	__S_HWDIS		0x01	// Disable Hardware Reading

// keypad specs
#define	K_PWR			0x0001	// Low Power
#define	K_SGN			0x0002	// Signature; 1 = Standard Pad
#define	K_A				0x0004	// A Button
#define	K_B				0x0008	// B Button
#define	K_RT			0x0010	// R Trigger
#define	K_LT			0x0020	// L Trigger
#define	K_RU			0x0040	// Right Pad, Up
#define	K_RR			0x0080	// Right Pad, Right
#define	K_LR			0x0100	// Left Pad, Right
#define	K_LL			0x0200	// Left Pad, Left
#define	K_LD			0x0400	// Left Pad, Down
#define	K_LU			0x0800	// Left Pad, Up
#define	K_STA			0x1000	// Start Button
#define	K_SEL			0x2000	// Select Button
#define	K_RL			0x4000	// Right Pad, Left
#define	K_RD			0x8000	// Right Pad, Down
#define	K_ANY			0xFFFC	// All keys, without pwr & sgn
#define	K_BTNS			0x303C	// All buttons; no d-pads, pwr or sgn
#define	K_PADS			0xCFC0	// All d-pads
#define	K_LPAD			0x0F00	// Left d-pad only
#define	K_RPAD			0xC0C0	// Right d-pad only


#define __KEY_NONE		0x0000
#define __KEY_PRESSED	0x0001
#define __KEY_RELEASED	0x0010
#define __KEY_HOLD		0x0100


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * User's input
 *
 * @memberof	KeypadManager
 */
typedef struct UserInput
{
	/// Currently pressed key(s)
	uint16 allKeys;
	/// Currently pressed key(s)
	uint16 pressedKey;
	/// Released key(s)
	uint16 releasedKey;
	/// Held key(s)
	uint16 holdKey;
	/// How long the key(s) have been held (in game frames)
	uint32 holdKeyDuration;
	/// Previously pressed key(s)
	uint16 previousKey;
	/// Low power flag
	uint16 powerFlag;
} UserInput;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup hardware
singleton class KeypadManager : ListenerObject
{
	long accumulatedUserInput;
	// User's Input
	UserInput userInput;
	// User's Input to be registered
	UserInput userInputToRegister;
	// Enabled
	bool enabled;
	// Flag to prevent pressed and released keys from
	// being raised when holding buttons while changing 
	// game states
	bool reseted;

	/// @publicsection
	static KeypadManager getInstance();
	static void interruptHandler();
	static void printUserInput(const UserInput* userInput, int32 x, int32 y);

	void reset();
	void disable();
	void disableInterrupt();
	void enable();
	void enableInterrupt();
	void flush();
	uint16 getHoldKey();
	uint32 getHoldKeyDuration();
	uint16 getPressedKey();
	uint16 getPreviousKey();
	uint16 getReleasedKey();
	UserInput getUserInput();
	int32 isEnabled();
	UserInput captureUserInput();
	void registerInput(uint16 inputToRegister);
	long getAccumulatedUserInput();
}


#endif
