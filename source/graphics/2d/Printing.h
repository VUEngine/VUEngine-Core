/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

#ifndef PRINTING_H_
#define	PRINTING_H_


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <CharSetManager.h>
#include <TextureManager.h>
#include <Globals.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// print VIP register's state
void Printing_vipRegisters(int x,int y);

// print an int
void Printing_int(int value,int x,int y);

// print in hex
void Printing_hex(WORD value,int x,int y);

// retrieve number of digits in a number 
int Utilities_intLength(int value);

// print text
void Printing_text(char *string,int x,int y);

// print a float
void Printing_float(float value,int x,int y);

extern Optical 		*_optical;
extern const u16 ASCII_CH[];
extern VBVec3D				*_screenPosition;

int vbjDigitCount(int value);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											PRINTING HELPER FUNCTIONS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//render general print output layer
void Printing_render(int textLayer);

/* ---------------------------------------------------------------------------------------------------------*/
//setup the bgmap and char memory with printing data
void Printing_writeAscii();

//show debug info and hung up there
void Printing_debug(u32 x);


#endif 