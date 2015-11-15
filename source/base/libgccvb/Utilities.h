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
// 											DECLARATIONS
//---------------------------------------------------------------------------------------------------------

long Utilities_randomSeed();
int Utilities_random(long seed, int randnums);
char* Utilities_itoa(u32 num, u8 base, u8 digits);
WORD Utilities_rotateBits(WORD invalue, int places, int direction);
int Utilities_equalSign(int a, int b);
int Utilities_getDigitCount(int value);
int Utilities_intLength(int value);


#endif