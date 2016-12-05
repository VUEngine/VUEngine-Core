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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern BYTE VBJaEFontTiles[];


//---------------------------------------------------------------------------------------------------------
// 												DEFINITIONS
//---------------------------------------------------------------------------------------------------------

CharSetROMDef VBJAENGINE_FONT_CH =
{
    // number of chars
    256,

    // allocation type
    __NOT_ANIMATED,

    // char definition
    VBJaEFontTiles,
};

FontROMDef VBJAENGINE_FONT =
{
    // font charset definition pointer
    (CharSetDefinition*)&VBJAENGINE_FONT_CH,

    // character number at which the font starts, allows you to skip the control characters for example
	0,

	// size of a single character (in chars) ({width, height})
	{1, 1},

	// font's name
    "VBJaEngineFont",
};

