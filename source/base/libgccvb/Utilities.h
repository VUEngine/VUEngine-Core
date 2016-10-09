/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_


//---------------------------------------------------------------------------------------------------------
// 											 INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 											 PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// macros for bitmask operations
#define SET_BIT(var,bit) 	(var |= (0x01 << bit))
#define CLEAR_BIT(var,bit) 	(var &= (~(0x01 << bit)))
#define TOGGLE_BIT(var,bit) (var ^= (0x01 << bit))
#define GET_BIT(var,bit) 	(0x01 & (var >> bit))


//---------------------------------------------------------------------------------------------------------
// 											DECLARATIONS
//---------------------------------------------------------------------------------------------------------

void Utilities_setClock(Clock clock);
long Utilities_randomSeed();
int Utilities_random(long seed, int randnums);
char* Utilities_itoa(u32 num, u32 base, u32 digits);
int Utilities_equalSign(int a, int b);
int Utilities_getDigitCount(int value);
int Utilities_intLength(int value);


#endif
