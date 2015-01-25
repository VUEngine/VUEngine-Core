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

#include <Image.h>


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern BYTE VBJaEngineDefaultSplashScreenLogoTiles[];
extern BYTE VBJaEngineDefaultSplashScreenLogoMap[];
extern BYTE VBJaEngineDefaultSplashScreenLogoOutlineTiles[];
extern BYTE VBJaEngineDefaultSplashScreenLogoOutlineMap[];


//---------------------------------------------------------------------------------------------------------
// 												DEFINITIONS
//---------------------------------------------------------------------------------------------------------

TextureROMDef VBJAENGINE_LOGO_3D_TX =
{
    {
        // number of chars, depending on allocation type:
        // __ANIMATED: number of chars of a single animation frame (cols * rows of this texture)
        // __ANIMATED_SHARED: sum of chars of all animation frames
        // __NO_ANIMATED: number of chars of whole image
        52,

        // allocation type
        __NO_ANIMATED,

        // char definition
        VBJaEngineDefaultSplashScreenLogoTiles,
    },

    // bgmap definition
    VBJaEngineDefaultSplashScreenLogoMap,

    // cols (max 48)
    8,

    // rows (max 28)
    9,

    // number of frames
    1,

   // palette number
    0,
};

TextureROMDef VBJAENGINE_LOGO_OUTLINE_TX =
{
    {
        // number of chars, depending on allocation type:
        // __ANIMATED: number of chars of a single animation frame (cols * rows of this texture)
        // __ANIMATED_SHARED: sum of chars of all animation frames
        // __NO_ANIMATED: number of chars of whole image
        63,

        // allocation type
        __NO_ANIMATED,

        // char definition
        VBJaEngineDefaultSplashScreenLogoOutlineTiles,
    },

    // bgmap definition
    VBJaEngineDefaultSplashScreenLogoOutlineMap,

    // cols (max 48)
    19,

    // rows (max 28)
    9,

    // number of frames
    1,

    // palette number
    0,
};

SpriteROMDef VBJAENGINE_LOGO_3D_IM_SPRITE =
{
	// sprite's type
	__TYPE(Sprite),

	// texture definition
	(TextureDefinition*)&VBJAENGINE_LOGO_3D_TX,
	
	// bgmap mode (BGMAP, AFFINE or H-BIAS)
	WRLD_BGMAP,
	
	// display mode
	WRLD_ON,

	// parallax displacement
	0		
};

SpriteROMDef* VBJAENGINE_LOGO_3D_IM_SPRITES[] =
{
	&VBJAENGINE_LOGO_3D_IM_SPRITE,
	NULL
};

SpriteROMDef VBJAENGINE_LOGO_OUTLINE_IM_SPRITE =
{
	// sprite's type
	__TYPE(Sprite),

	// texture definition
	(TextureDefinition*)&VBJAENGINE_LOGO_OUTLINE_TX,

	// bgmap mode (BGMAP, AFFINE or H-BIAS)
	WRLD_BGMAP,

	// display mode
	WRLD_ON,

	// parallax displacement
	0		
};

SpriteROMDef* VBJAENGINE_LOGO_OUTLINE_IM_SPRITES[] =
{
	&VBJAENGINE_LOGO_OUTLINE_IM_SPRITE,
	NULL
};

ImageROMDef VBJAENGINE_LOGO_3D_IM =
{
	__TYPE(Image),
	VBJAENGINE_LOGO_3D_IM_SPRITES,
};

ImageROMDef VBJAENGINE_LOGO_OUTLINE_IM =
{
	__TYPE(Image),
	VBJAENGINE_LOGO_OUTLINE_IM_SPRITES,
};