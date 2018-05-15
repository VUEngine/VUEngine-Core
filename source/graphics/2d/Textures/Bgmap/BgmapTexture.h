/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef BGMAP_TEXTURE_H_
#define BGMAP_TEXTURE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Texture.h>
#include <CharSet.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

typedef const TextureDefinition BgmapTextureDefinition;
typedef const BgmapTextureDefinition BgmapTextureROMDef;


class BgmapTexture : Texture
{
	/**
	* @var s8 		segment
	* @brief		Segment
	* @memberof	BgmapTexture
	*/
	s8 segment;
	/**
	* @var u8 		usageCount
	* @brief		How many textures are using me
	* @memberof	BgmapTexture
	*/
	u8 usageCount;
	/**
	* @var s8 		remainingRowsToBeWritten
	* @brief		Remaining rows to be written
	* @memberof	BgmapTexture
	*/
	s8 remainingRowsToBeWritten;

	void constructor(BgmapTexture this, BgmapTextureDefinition* bgmapTextureDefinition, u16 id);
	s8 getRemainingRowsToBeWritten(BgmapTexture this);
	s16 getXOffset(BgmapTexture this);
	s16 getYOffset(BgmapTexture this);
	s8 getSegment(BgmapTexture this);
	void setSegment(BgmapTexture this, s8 segment);
	u8 getUsageCount(BgmapTexture this);
	void increaseUsageCount(BgmapTexture this);
	bool decreaseUsageCount(BgmapTexture this);
	override void write(BgmapTexture this);
	override void rewrite(BgmapTexture this);
}


#endif
