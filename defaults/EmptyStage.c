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

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Stage.h>


//---------------------------------------------------------------------------------------------------------
// 												ASSETS
// ---------------------------------------------------------------------------------------------------------

PositionedEntityROMDef EMPTY_ST_ENTITIES[] =
{
	{NULL, {0,0,0}, NULL, NULL, NULL},
};

PositionedEntityROMDef EMPTY_ST_UI_ENTITIES[] =
{
	{NULL, {0,0,0}, NULL, NULL, NULL}
};

TextureROMDef* EMPTY_ST_TEXTURES[] =
{
	NULL
};


//---------------------------------------------------------------------------------------------------------
// 											STAGE DEFINITION
//---------------------------------------------------------------------------------------------------------

StageROMDef EMPTY_ST =
{
    // size
    {
        // x
        __SCREEN_WIDTH,
        // y
        __SCREEN_HEIGHT,
        // z
        0,
    },

    // gravity
    {
	    ITOFIX19_13(0),
	    ITOFIX19_13(0),
	    ITOFIX19_13(0)
    },

    // friction
    ITOFIX19_13(0),

	// OBJs segments z coordinates (SPT0 to SPT3)
    {
    	ITOFIX19_13(0),
		ITOFIX19_13(0),
		ITOFIX19_13(0),
		ITOFIX19_13(0)
    },

    //initial screen position
    {
        // x
        ITOFIX19_13(0),
        // y
        ITOFIX19_13(0),
        // z
        ITOFIX19_13(0)
    },

    //textures to preload
    (TextureDefinition**)EMPTY_ST_TEXTURES,

    //UI
    {
        EMPTY_ST_UI_ENTITIES,
        __TYPE(UI),
    },

    //entities
    EMPTY_ST_ENTITIES,

    //background music
    NULL,

    //identifier
    "",

    //name
    "",
};