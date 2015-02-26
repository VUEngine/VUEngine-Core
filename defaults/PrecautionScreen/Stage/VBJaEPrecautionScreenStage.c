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

extern EntityDefinition VBJAENGINE_DEFAULT_PRECAUTION_SCREEN_JAPANESE_IM;


//---------------------------------------------------------------------------------------------------------
// 												 ASSETS
//---------------------------------------------------------------------------------------------------------

PositionedEntityROMDef VBJAENGINE_DEFAULT_PRECAUTION_SCREEN_ST_ENTITIES[] =
{
	{&VBJAENGINE_DEFAULT_PRECAUTION_SCREEN_JAPANESE_IM, {188, 148, 0}, NULL, NULL, NULL},
	{NULL,{0,0,0}, NULL, NULL, NULL},
};


//---------------------------------------------------------------------------------------------------------
// 											STAGE DEFINITION
//---------------------------------------------------------------------------------------------------------

StageROMDef VBJAENGINE_DEFAULT_PRECAUTION_SCREEN_ST =
{
    // size
    {
        // x
        __SCREEN_WIDTH,
        // y
        __SCREEN_HEIGHT,
        // z
        1
    },

    //initial screen position
    {
        // x
        ITOFIX19_13(0),
        // y
        ITOFIX19_13(0),
        //z
        ITOFIX19_13(__ZZERO)
    },

    //textures
    NULL,

    //UI entities
    {
        NULL,
        NULL
    },

    //entities
    VBJAENGINE_DEFAULT_PRECAUTION_SCREEN_ST_ENTITIES,

    //background music
    NULL,

    //identifier
    "",

    //name
    "",
};