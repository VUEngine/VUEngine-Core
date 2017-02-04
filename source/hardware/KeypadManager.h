/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef KEY_PAD_MANAGER_H_
#define KEY_PAD_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
//											MACROS
//---------------------------------------------------------------------------------------------------------

// hardware reg __SCR definitions
#define	__S_INTDIS	0x80 	// Disable Interrupts
#define	__S_SW		0x20 	// Software Reading
#define	__S_SWCK	0x10 	// Software Clock, Interrupt?
#define	__S_HW		0x04 	// Hardware Reading
#define	__S_STAT	0x02 	// Hardware Reading Status
#define	__S_HWDIS	0x01	// Disable Hardware Reading

// keypad definitions
#define	K_PWR	0x0001		// Low Battery
#define	K_SGN	0x0002		// Signature; 1 = Standard Pad
#define	K_A		0x0004		// A Button
#define	K_B		0x0008		// B Button
#define	K_RT	0x0010		// R Trigger
#define	K_LT	0x0020		// L Trigger
#define	K_RU	0x0040		// Right Pad, Up
#define	K_RR	0x0080		// Right Pad, Right
#define	K_LR	0x0100		// Left Pad, Right
#define	K_LL	0x0200		// Left Pad, Left
#define	K_LD	0x0400		// Left Pad, Down
#define	K_LU	0x0800		// Left Pad, Up
#define	K_STA	0x1000		// Start Button
#define	K_SEL	0x2000		// Select Button
#define	K_RL	0x4000		// Right Pad, Left
#define	K_RD	0x8000		// Right Pad, Down
#define	K_ANY	0xFFFC		// All keys, without pwr & sgn
#define	K_BTNS	0x303C		// All buttons; no d-pads, pwr or sgn
#define	K_PADS	0xCFC0		// All d-pads
#define	K_LPAD	0x0F00		// Left d-pad only
#define	K_RPAD	0xC0C0		// Right d-pad only


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// defines as a pointer to a structure that's not defined here and so is not accessible to the outside world

// declare the virtual methods
#define KeypadManager_METHODS(ClassName)																\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define KeypadManager_SET_VTABLE(ClassName)																\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(KeypadManager);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

KeypadManager KeypadManager_getInstance();

void KeypadManager_destructor(KeypadManager this);

void KeypadManager_clear(KeypadManager this);
void KeypadManager_disable(KeypadManager this);
void KeypadManager_disableInterrupt(KeypadManager this);
void KeypadManager_enable(KeypadManager this);
void KeypadManager_enableInterrupt(KeypadManager this);
void KeypadManager_flush(KeypadManager this);
u32 KeypadManager_getHoldKey(KeypadManager this);
u32 KeypadManager_getPressedKey(KeypadManager this);
u32 KeypadManager_getPreviousKey(KeypadManager this);
u32 KeypadManager_getReleasedKey(KeypadManager this);
int KeypadManager_isEnabled(KeypadManager this);
void KeypadManager_read(KeypadManager this);


#endif
