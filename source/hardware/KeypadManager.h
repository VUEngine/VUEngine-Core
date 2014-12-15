/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef KEY_PAD_MANAGER_H_
#define KEY_PAD_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>

//---------------------------------------------------------------------------------------------------------
// 											MACROS
//---------------------------------------------------------------------------------------------------------

/* Hardware reg SCR definitions */
#define	S_INTDIS	0x80 	// Disable Interrups
#define	S_SW		0x20 	// Software Reading
#define	S_SWCK		0x10 	// Software Clock, Interrupt?
#define	S_HW		0x04 	// Hardware Reading
#define	S_STAT		0x02 	// Hardware Reading Status
#define	S_HWDIS		0x01	// Disable Hardware Reading

/* Keypad Definitions */
#define	K_ANY	0xFFFC		// All keys, without pwr & sgn
#define	K_BTNS	0x303C		// All buttons; no d-pads, pwr or sgn
#define	K_PADS	0xCFC0		// All d-pads
#define	K_LPAD	0x0F00		// Left d-pad only
#define	K_RPAD	0xC0C0		// Right d-pad only
#define	K_PWR	0x0001		// Low Battery
#define	K_SGN	0x0002		// Signature; 1 = Standard Pad
#define	K_A		0x0004		// A Button
#define	K_B		0x0008		// B Button
#define	K_RT	0x0010		// R Trigger
#define	K_LT	0x0020		// L Trigger
#define	K_RU	0x0040		// Right Pad, Up
#define	K_RR	0x0080		// Right Pad, Right
#define	K_LR	0x0100		// Left Pad,  Right
#define	K_LL	0x0200		// Left Pad,  Left
#define	K_LD	0x0400		// Left Pad,  Down
#define	K_LU	0x0800		// Left Pad,  Up
#define	K_STA	0x1000		// Start Button
#define	K_SEL	0x2000		// Select Button
#define	K_RL	0x4000		// Right Pad, Left
#define	K_RD	0x8000		// Right Pad, Down

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/* Defines as a pointer to a structure that
 * is not defined here and so is not accessible to the outside world
 */
// declare the virtual methods
#define KeypadManager_METHODS													\
		Object_METHODS															\

// declare the virtual methods which are redefined
#define KeypadManager_SET_VTABLE(ClassName)										\
		Object_SET_VTABLE(ClassName)											\

__CLASS(KeypadManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

// it is a singleton!
KeypadManager KeypadManager_getInstance();

// class's destructor
void KeypadManager_destructor(KeypadManager this);

// enable keypad reads
void KeypadManager_enable(KeypadManager this);

// disable keypad reads
void KeypadManager_disable(KeypadManager this);

// read keypad
u16 KeypadManager_read(KeypadManager this);

// get pressed key
u16 KeypadManager_getPressedKey(KeypadManager this);

// get released key
u16 KeypadManager_getReleasedKey(KeypadManager this);

// get hold key
u16 KeypadManager_getHoldKey(KeypadManager this);

// get previous key
u16 KeypadManager_getPreviousKey(KeypadManager this);

#endif /*KEY_PAD_MANAGER_H_*/
