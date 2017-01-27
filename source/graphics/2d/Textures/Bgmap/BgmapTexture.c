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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapTexture.h>
#include <BgmapTextureManager.h>
#include <SpriteManager.h>
#include <HardwareManager.h>
#include <Mem.h>
#include <VIPManager.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------
// define the class
__CLASS_DEFINITION(BgmapTexture, Texture);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void BgmapTexture_constructor(BgmapTexture this, BgmapTextureDefinition* bgmapTextureDefinition, u16 id);
static void BgmapTexture_writeAnimatedSingle(BgmapTexture this);
static void BgmapTexture_writeAnimatedShared(BgmapTexture this);
static void BgmapTexture_writeAnimatedMulti(BgmapTexture this);
static void BgmapTexture_writeNotAnimated(BgmapTexture this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------


// always call these two macros next to each other
__CLASS_NEW_DEFINITION(BgmapTexture, BgmapTextureDefinition* bgmapTextureDefinition, u16 id)
__CLASS_NEW_END(BgmapTexture, bgmapTextureDefinition, id);

// class's constructor
static void BgmapTexture_constructor(BgmapTexture this, BgmapTextureDefinition* bgmapTextureDefinition, u16 id)
{
	// construct base object
	__CONSTRUCT_BASE(Texture, (TextureDefinition*)bgmapTextureDefinition, id);

	this->usageCount = 1;
	this->remainingRowsToBeWritten = 0;
}

// class's destructor
void BgmapTexture_destructor(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::destructor: null this");

	// make sure that I'm not destroyed again
	this->usageCount = -1;

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// write into memory the chars and this
void BgmapTexture_write(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::write: null this");

	Texture_write(__SAFE_CAST(Texture, this));

	if(!this->charSet)
	{
    	this->written = false;
	    return;
	}

	if(0 >= this->remainingRowsToBeWritten)
	{
		this->remainingRowsToBeWritten = this->textureDefinition->rows;
	}

	//determine the allocation type
	switch(this->textureDefinition->charSetDefinition->allocationType)
	{
		case __ANIMATED_SINGLE:

			// write the definition to graphic memory
			BgmapTexture_writeAnimatedSingle(this);
			break;

		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:

			BgmapTexture_writeAnimatedShared(this);
			break;

		case __ANIMATED_MULTI:

			// write the definition to graphic memory
			BgmapTexture_writeAnimatedMulti(this);
			break;

		case __NOT_ANIMATED:

			// write the definition to graphic memory
			BgmapTexture_writeNotAnimated(this);
			break;

		default:

			NM_ASSERT(false, "BgmapTexture::write: no allocation type");
	}

	this->written = 0 >= this->remainingRowsToBeWritten;
}

// write an animated map
static void BgmapTexture_writeAnimatedSingle(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::writeAnimatedSingle: null this");

	int bgmapSegment = BgmapTexture_getBgmapSegment(this);
	int palette = this->palette << 14;

	int charLocation = (int)CharSet_getOffset(this->charSet);

	int xOffset = (int)BgmapTextureManager_getXOffset(BgmapTextureManager_getInstance(), this->id);
	int yOffset = (int)BgmapTextureManager_getYOffset(BgmapTextureManager_getInstance(), this->id);

	if(0 > xOffset || 0 > yOffset)
	{
		return;
	}

	int counter = SpriteManager_getTexturesMaximumRowsToWrite(SpriteManager_getInstance());

	//put the map into memory calculating the number of char for each reference
	for(; counter && this->remainingRowsToBeWritten--; counter--)
	{
		Mem_add ((u8*)__BGMAP_SEGMENT(bgmapSegment) + ((xOffset + (yOffset << 6 ) + (this->remainingRowsToBeWritten << 6)) << 1),
				(const u8*)(this->textureDefinition->bgmapDefinition + (this->remainingRowsToBeWritten * (this->textureDefinition->cols) << 1)),
				this->textureDefinition->cols,
				(palette) | (charLocation));
	}
}

// write an animated map
static void BgmapTexture_writeAnimatedShared(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::writeAnimated: null this");

	int bgmapSegment = BgmapTexture_getBgmapSegment(this);
	int palette = this->palette << 14;

	int charLocation = (int)CharSet_getOffset(this->charSet);

	int xOffset = (int)BgmapTextureManager_getXOffset(BgmapTextureManager_getInstance(), this->id);
	int yOffset = (int)BgmapTextureManager_getYOffset(BgmapTextureManager_getInstance(), this->id);

	if(0 > xOffset || 0 > yOffset)
	{
		return;
	}

	int counter = SpriteManager_getTexturesMaximumRowsToWrite(SpriteManager_getInstance());

	//put the map into memory calculating the number of char for each reference
	for(; counter && this->remainingRowsToBeWritten--; counter--)
	{
		Mem_add ((u8*)__BGMAP_SEGMENT(bgmapSegment) + ((xOffset + (yOffset << 6 ) + (this->remainingRowsToBeWritten << 6)) << 1),
				(const u8*)(this->textureDefinition->bgmapDefinition + (this->remainingRowsToBeWritten * (this->textureDefinition->cols) << 1)),
				this->textureDefinition->cols,
				(palette) | (charLocation));
	}
}

// write an animated and shared map
static void BgmapTexture_writeAnimatedMulti(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::writeAnimatedShared: null this");

	int bgmapSegment = BgmapTexture_getBgmapSegment(this);
	int palette = this->palette << 14;

	// determine the number of frames the map had
	int area = (this->textureDefinition->cols * this->textureDefinition->rows);
	int charLocation = (int)CharSet_getOffset(this->charSet);
	int frames = CharSet_getNumberOfChars(this->charSet) / area;

	int xOffset = (int)BgmapTextureManager_getXOffset(BgmapTextureManager_getInstance(), this->id);
	int yOffset = (int)BgmapTextureManager_getYOffset(BgmapTextureManager_getInstance(), this->id);

	if(0 > xOffset || 0 > yOffset)
	{
		return;
	}

	int counter = SpriteManager_getTexturesMaximumRowsToWrite(SpriteManager_getInstance());

	//put the map into memory calculating the number of char for each reference
	for(; counter && this->remainingRowsToBeWritten--; counter--)
	{
		int j = 1;
		//write into the specified bgmap segment plus the offset defined in the this structure, the this definition
		//specifying the char displacement inside the char mem
		for(; j <= frames; j++)
		{
			Mem_add ((u8*)__BGMAP_SEGMENT(bgmapSegment) + ((xOffset + (this->textureDefinition->cols * (j - 1)) + (yOffset << 6) + (this->remainingRowsToBeWritten << 6)) << 1),
					(const u8*)(this->textureDefinition->bgmapDefinition + (this->remainingRowsToBeWritten * (this->textureDefinition->cols) << 1)),
					this->textureDefinition->cols,
					(palette) | (charLocation + area * (j - 1)));
		}
	}
}

// write an inanimated map
static void BgmapTexture_writeNotAnimated(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::writeNoAnimated: null this");

	int bgmapSegment = BgmapTexture_getBgmapSegment(this);
	int palette = this->palette << 14;

	int charLocation = (int)CharSet_getOffset(this->charSet);

	int xOffset = (int)BgmapTextureManager_getXOffset(BgmapTextureManager_getInstance(), this->id);
	int yOffset = (int)BgmapTextureManager_getYOffset(BgmapTextureManager_getInstance(), this->id);

	if(0 > xOffset || 0 > yOffset)
	{
		return;
	}

	int counter = SpriteManager_getTexturesMaximumRowsToWrite(SpriteManager_getInstance());

	//put the map into memory calculating the number of char for each reference
	for(; counter && this->remainingRowsToBeWritten--; counter--)
	{
		Mem_add ((u8*)__BGMAP_SEGMENT(bgmapSegment) + ((xOffset + (yOffset << 6 ) + (this->remainingRowsToBeWritten << 6)) << 1),
				(const u8*)(this->textureDefinition->bgmapDefinition + (this->remainingRowsToBeWritten * (this->textureDefinition->cols) << 1)),
				this->textureDefinition->cols,
				(palette) | (charLocation));
	}
}

// get texture's x offset within bgmap mem
s32 BgmapTexture_getXOffset(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::getXOffset: null this");

	return BgmapTextureManager_getXOffset(BgmapTextureManager_getInstance(), this->id);
}

// get texture's y offset within bgmap mem
s32 BgmapTexture_getYOffset(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::getYOffset: null this");

	return BgmapTextureManager_getYOffset(BgmapTextureManager_getInstance(), this->id);
}

//get texture's bgmap segment
u32 BgmapTexture_getBgmapSegment(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::getBgmapSegment: null this");

	return BgmapTextureManager_getBgmapSegment(BgmapTextureManager_getInstance(), this->id);
}

u8 BgmapTexture_getUsageCount(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::getUsageCount: null this");

	return this->usageCount;
}

void BgmapTexture_increaseUsageCount(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::increaseUsageCoung: null this");

	this->usageCount++;
}

bool BgmapTexture_decreaseUsageCount(BgmapTexture this)
{
	ASSERT(this, "BgmapTexture::decreaseUsageCoung: null this");

	return 0 >= --this->usageCount;
}
