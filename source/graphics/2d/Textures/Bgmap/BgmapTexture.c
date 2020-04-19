/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
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
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapTexture.h>
#include <BgmapTextureManager.h>
#include <SpriteManager.h>
#include <HardwareManager.h>
#include <Mem.h>
#include <VIPManager.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//												GLOBALS
//---------------------------------------------------------------------------------------------------------

static BgmapTextureManager _bgmapTextureManager = NULL;
static SpriteManager _spriteManager = NULL;

static const u16 _emptyTextureRow[64] = {0};


//---------------------------------------------------------------------------------------------------------
//											CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 * @param bgmapTextureSpec		Texture spec
 * @param id							Identifier
 */
void BgmapTexture::constructor(BgmapTextureSpec* bgmapTextureSpec, u16 id)
{
	// construct base object
	Base::constructor((TextureSpec*)bgmapTextureSpec, id);

	this->segment = -1;
	this->usageCount = 1;
	this->remainingRowsToBeWritten = 0;

	if(!_bgmapTextureManager)
	{
		_bgmapTextureManager = BgmapTextureManager::getInstance();
	}

	if(!_spriteManager)
	{
		_spriteManager = SpriteManager::getInstance();
	}
}

/**
 * Class destructor
 */
void BgmapTexture::destructor()
{
	// make sure that I'm not destroyed again
	this->usageCount = 0;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Write again the texture to DRAM
 */
void BgmapTexture::rewrite()
{
	this->written = false;
	this->remainingRowsToBeWritten = this->textureSpec->rows;

	BgmapTexture::write(this);
}

/**
 * Write the texture to DRAM
 */
void BgmapTexture::write()
{
	if(!this->charSet)
	{
		// make sure to force full writing if no char set
		this->remainingRowsToBeWritten = this->textureSpec->rows;
	}

	Base::write(this);

	if(!this->charSet)
	{
		this->written = false;
		return;
	}

	if(0 >= this->remainingRowsToBeWritten)
	{
		this->remainingRowsToBeWritten = this->textureSpec->rows;
	}

	//determine the allocation type
	switch(this->textureSpec->charSetSpec->allocationType)
	{
		case __ANIMATED_SINGLE:
		case __ANIMATED_SINGLE_OPTIMIZED:
		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:
		case __NOT_ANIMATED:

			BgmapTexture::doWrite(this);
			break;

		case __ANIMATED_MULTI:

			// write the spec to graphic memory
			BgmapTexture::writeAnimatedMulti(this);
			break;

		default:

			NM_ASSERT(false, "BgmapTexture::write: no allocation type");
	}

	this->written = 0 >= this->remainingRowsToBeWritten;
}

/**
 * Write __ANIMATED_MULTI Texture to DRAM
 *
 * @private
 */
void BgmapTexture::writeAnimatedMulti()
{
	int xOffset = (int)BgmapTextureManager::getXOffset(_bgmapTextureManager, this->id);
	int yOffset = (int)BgmapTextureManager::getYOffset(_bgmapTextureManager, this->id);

	if((0 > xOffset) | (0 > yOffset))
	{
		return;
	}

	int bgmapSegment = this->segment;
	int offsetDisplacement = xOffset + (yOffset << 6);
	int palette = this->palette << 14;

	// determine the number of frames the map had
	int area = (this->textureSpec->cols * this->textureSpec->rows);
	int charLocation = (int)CharSet::getOffset(this->charSet);
	int frames = CharSet::getNumberOfChars(this->charSet) / area;
	u32 mapDisplacement = this->mapDisplacement;

	int counter = SpriteManager::getTexturesMaximumRowsToWrite(_spriteManager);

	bool disableCache = false;

	if(area / counter >= DRM_WRINTING_PASSES_TO_ENABLE_CACHE)
	{
		disableCache = true;
		CACHE_DISABLE;
		CACHE_CLEAR;
		CACHE_ENABLE;
	}

	// put the map into memory calculating the number of char for each reference
	for(; counter && this->remainingRowsToBeWritten--; counter--)
	{
		int j = 1;
		// write into the specified bgmap segment plus the offset defined in the this structure, the this spec
		// specifying the char displacement inside the char mem
		for(; j <= frames; j++)
		{
			Mem::addHWORD((HWORD*)__BGMAP_SEGMENT(bgmapSegment) + (offsetDisplacement + (this->textureSpec->cols * (j - 1)) + (this->remainingRowsToBeWritten << 6)),
				(const HWORD*)this->textureSpec->mapSpec + mapDisplacement + (this->remainingRowsToBeWritten * this->textureSpec->cols),
				this->textureSpec->cols,
				(palette) | (charLocation + area * (j - 1)));
		}
	}

	if(64 > this->textureSpec->rows && -1 == this->remainingRowsToBeWritten)
	{
		int j = 1;
		// write into the specified bgmap segment plus the offset defined in the this structure, the this spec
		// specifying the char displacement inside the char mem
		for(; j <= frames; j++)
		{
			Mem::addHWORD((HWORD*)__BGMAP_SEGMENT(bgmapSegment) + (offsetDisplacement + (this->textureSpec->cols * (j - 1)) + (this->textureSpec->rows << 6)),
				(const HWORD*)_emptyTextureRow,
				this->textureSpec->cols,
				0
			);
		}
	}

	if(disableCache)
	{
		CACHE_DISABLE;
		CACHE_CLEAR;
		CACHE_ENABLE;
	}
}

/**
 * Set Texture's frame
 *
 * @param frame	Texture's frame to display
 */
void BgmapTexture::setFrameAnimatedMulti(u16 frame __attribute__((unused)))
{
	BgmapTexture::fireEvent(this, kEventTextureSetFrame);
}

// TODO: inlining this causes trouble with ANIMATED_MULTI animations
static inline void BgmapTexture::addHWORD(HWORD* destination, const HWORD* source, u32 numberOfHWORDS, u32 offset, u16 flip, bool backward)
{
	s16 increment = backward ? -2 : 2;

	const HWORD* finalSource = source + numberOfHWORDS;

    asm("					\n\t"      \
		"jr end%=			\n\t"      \
		"loop%=:			\n\t"      \
		"ld.h 0[%1],r10		\n\t"      \
		"xor %4,r10			\n\t"      \
		"add %3,r10			\n\t"      \
		"st.h r10,0[%0]		\n\t"      \
		"add %5,%0			\n\t"      \
		"add 2,%1			\n\t"      \
		"end%=:				\n\t"      \
		"cmp %1,%2			\n\t"      \
		"bgt loop%=			\n\t"      \
    : // No Output
    : "r" (destination), "r" (source), "r" (finalSource), "r" (offset), "r" (flip), "r" (increment)
	: "r10" // regs used
    );
}

/**
 * Write Texture to DRAM
 *
 * @private
 */
void BgmapTexture::doWrite()
{
	int xOffset = (int)BgmapTextureManager::getXOffset(_bgmapTextureManager, this->id);
	int yOffset = (int)BgmapTextureManager::getYOffset(_bgmapTextureManager, this->id);

	if((0 > xOffset) | (0 > yOffset))
	{
		return;
	}

	int bgmapSegment = this->segment;
	int offsetDisplacement = xOffset + (yOffset << 6);
	u16 offset = (int)CharSet::getOffset(this->charSet) | (this->palette << 14);

	int counter = SpriteManager::getTexturesMaximumRowsToWrite(_spriteManager);
	u32 mapDisplacement = this->mapDisplacement;

	u32 numberOfHWORDS = this->textureSpec->cols;

	bool disableCache = false;
	u16 flip = ((this->textureSpec->horizontalFlip << 1) | this->textureSpec->verticalFlip) << 12;

	if(this->textureSpec->horizontalFlip)
	{
		if(this->textureSpec->verticalFlip)
		{
			if(numberOfHWORDS * counter >= DRM_WRINTING_PASSES_TO_ENABLE_CACHE)
			{
				disableCache = true;
				CACHE_DISABLE;
				CACHE_CLEAR;
				CACHE_ENABLE;
			}

			//put the map into memory calculating the number of char for each reference
			for(; counter && this->remainingRowsToBeWritten--; counter--)
			{
				BgmapTexture::addHWORD((HWORD*)__BGMAP_SEGMENT(bgmapSegment) + offsetDisplacement + ((this->remainingRowsToBeWritten) << 6) + numberOfHWORDS - 1,
						(const HWORD*)this->textureSpec->mapSpec + mapDisplacement + ((this->textureSpec->rows - this->remainingRowsToBeWritten) * this->textureSpec->cols),
						numberOfHWORDS,
						offset,
						flip,
						true);
			}
		}
		else
		{
			if(numberOfHWORDS * counter >= DRM_WRINTING_PASSES_TO_ENABLE_CACHE)
			{
				disableCache = true;
				CACHE_DISABLE;
				CACHE_CLEAR;
				CACHE_ENABLE;
			}

			//put the map into memory calculating the number of char for each reference
			for(; counter && this->remainingRowsToBeWritten--; counter--)
			{
				BgmapTexture::addHWORD((HWORD*)__BGMAP_SEGMENT(bgmapSegment) + offsetDisplacement + (this->remainingRowsToBeWritten << 6) + numberOfHWORDS - 1,
						(const HWORD*)this->textureSpec->mapSpec + mapDisplacement + ((this->remainingRowsToBeWritten) * this->textureSpec->cols),
						numberOfHWORDS,
						offset,
						flip,
						true);
			}
		}
	}
	else
	{
		if(this->textureSpec->verticalFlip)
		{
			if(numberOfHWORDS * counter >= DRM_WRINTING_PASSES_TO_ENABLE_CACHE)
			{
				disableCache = true;
				CACHE_DISABLE;
				CACHE_CLEAR;
				CACHE_ENABLE;
			}

			//put the map into memory calculating the number of char for each reference
			for(; counter && this->remainingRowsToBeWritten--; counter--)
			{
				BgmapTexture::addHWORD((HWORD*)__BGMAP_SEGMENT(bgmapSegment) + offsetDisplacement + (this->remainingRowsToBeWritten << 6),
						(const HWORD*)this->textureSpec->mapSpec + mapDisplacement + ((this->textureSpec->rows - this->remainingRowsToBeWritten - 1) * this->textureSpec->cols),
						numberOfHWORDS,
						offset,
						flip,
						false);
			}
		}
		else
		{
			if(numberOfHWORDS * counter >= DRM_WRINTING_PASSES_TO_ENABLE_CACHE)
			{
				disableCache = true;
				CACHE_DISABLE;
				CACHE_CLEAR;
				CACHE_ENABLE;
			}

			//put the map into memory calculating the number of char for each reference
			for(; counter && this->remainingRowsToBeWritten--; counter--)
			{
				BgmapTexture::addHWORD((HWORD*)__BGMAP_SEGMENT(bgmapSegment) + offsetDisplacement + (this->remainingRowsToBeWritten << 6),
						(const HWORD*)this->textureSpec->mapSpec + mapDisplacement + ((this->remainingRowsToBeWritten) * this->textureSpec->cols),
						numberOfHWORDS,
						offset,
						flip,
						false);
			}
		}
	}

	if(64 > this->textureSpec->rows && -1 == this->remainingRowsToBeWritten)
	{
		BgmapTexture::addHWORD((HWORD*)__BGMAP_SEGMENT(bgmapSegment) + offsetDisplacement + (this->textureSpec->rows << 6),
				(const HWORD*)_emptyTextureRow,
				numberOfHWORDS,
				0,
				false,
				false);
	}

	if(disableCache)
	{
		CACHE_DISABLE;
		CACHE_CLEAR;
		CACHE_ENABLE;
	}
}

/*
void BgmapTexture::writeAnimatedSingleOptimized()
{
	int bgmapSegment = this->segment;
	int palette = this->palette << 14;

	int charLocation = (int)CharSet::getOffset(this->charSet);

	if((0 > xOffset) | (0 > yOffset))
	{
		return;
	}

	int counter = SpriteManager::getTexturesMaximumRowsToWrite(SpriteManager::getInstance());

	//put the map into memory calculating the number of char for each reference
	for(; counter && this->remainingRowsToBeWritten--; counter--)
	{
		Mem::add ((u8*)__BGMAP_SEGMENT(bgmapSegment) + ((xOffset + (yOffset << 6 ) + (this->remainingRowsToBeWritten << 6)) << 1),
				(const u8*)(this->textureSpec->mapSpec + this->mapDisplacement + (this->remainingRowsToBeWritten * (this->textureSpec->cols) << 1)),
				this->textureSpec->cols << 1,
				(palette) | (charLocation));
	}
}
*/

/**
 * Retrieve the number of rows pending writing
 *
 * @return				Number of rows pending writing
 */
s8 BgmapTexture::getRemainingRowsToBeWritten()
{
	return this->remainingRowsToBeWritten;
}

/**
 * Retrieve the Texture's x offset within the BGMAP segment where it is allocated
 *
 * @return				Texture's x offset within BGMAP segment
 */
s16 BgmapTexture::getXOffset()
{
	return BgmapTextureManager::getXOffset(_bgmapTextureManager, this->id);
}

/**
 * Retrieve the Texture's y offset within the BGMAP segment where it is allocated
 *
 * @return				Texture's y offset within BGMAP segment
 */
s16 BgmapTexture::getYOffset()
{
	return BgmapTextureManager::getYOffset(_bgmapTextureManager, this->id);
}

/**
 * Set the Texture's BGMAP segment where it is allocated
 *
 * @param segment		Texture's BGMAP segment
 */
void BgmapTexture::setSegment(s8 segment)
{
	this->segment = segment;
}

/**
 * Retrieve the Texture's BGMAP segment where it is allocated
 *
 * @return				Texture's BGMAP segment
 */
s8 BgmapTexture::getSegment()
{
	return this->segment;
}

/**
 * Retrieve the count usage for this Texture
 *
 * @return				Texture's count usage
 */
u8 BgmapTexture::getUsageCount()
{
	return this->usageCount;
}

/**
 * Increase the count usage for this Texture
 */
void BgmapTexture::increaseUsageCount()
{
	this->usageCount++;
}

/**
 * Decrease the count usage for this Texture
 *
 * @return				True if count usage reached zero
 */
bool BgmapTexture::decreaseUsageCount()
{
	if(this->usageCount)
	{
		this->usageCount--;
	}

	return 0 == this->usageCount;
}

