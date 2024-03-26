/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapTextureManager.h>
#include <CharSet.h>
#include <DebugConfig.h>
#include <DebugUtilities.h>
#include <Mem.h>
#include <SpriteManager.h>
#include <VIPManager.h>

#include "BgmapTexture.h"


//---------------------------------------------------------------------------------------------------------
//												GLOBALS
//---------------------------------------------------------------------------------------------------------

static BgmapTextureManager _bgmapTextureManager = NULL;
static SpriteManager _spriteManager = NULL;

static const uint16 _emptyTextureRow[64] = {0};


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
void BgmapTexture::constructor(BgmapTextureSpec* bgmapTextureSpec, uint16 id)
{
	// construct base object
	Base::constructor((TextureSpec*)bgmapTextureSpec, id);

	this->segment = -1;
	this->remainingRowsToBeWritten = 0;

	this->horizontalFlip = this->textureSpec->horizontalFlip;
	this->verticalFlip = this->textureSpec->verticalFlip;

	this->xOffset = 0;
	this->yOffset = 0;

	if(NULL == _bgmapTextureManager)
	{
		_bgmapTextureManager = BgmapTextureManager::getInstance();
	}

	if(NULL == _spriteManager)
	{
		_spriteManager = SpriteManager::getInstance();
	}
}	

/**
 * Class destructor
 */
void BgmapTexture::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Write again the texture to DRAM
 */
void BgmapTexture::rewrite()
{
	Base::rewrite(this);

	this->remainingRowsToBeWritten = this->textureSpec->rows;
}

/**
 * Write the texture to DRAM
 */
bool BgmapTexture::write(int16 maximumTextureRowsToWrite)
{
	if(isDeleted(this->charSet))
	{
		// make sure to force full writing if no char set
		this->remainingRowsToBeWritten = this->textureSpec->rows;
	}
	
	uint8 status = this->status;

	if(!Base::write(this, maximumTextureRowsToWrite))
	{
		return false;
	}

	if(0 >= this->remainingRowsToBeWritten)
	{
		this->remainingRowsToBeWritten = this->textureSpec->rows;
	}

	uint16 charSetOffset = (uint16)CharSet::getOffset(this->charSet);
	
	if(BgmapTexture::isMultiframe(this))
	{
		BgmapTexture::writeAllFrames(this, maximumTextureRowsToWrite, this->xOffset, this->yOffset, charSetOffset);
	}
	else
	{
		BgmapTexture::writeFrame(this, maximumTextureRowsToWrite, kTexturePendingWriting < status && kTextureFrameChanged >= status, this->xOffset, this->yOffset, charSetOffset, 0);
	}

	if(kTexturePendingRewriting == status)
	{
		this->status = 0 >= this->remainingRowsToBeWritten ? kTextureWritten : kTexturePendingRewriting;
	}
	else
	{
		this->status = 0 >= this->remainingRowsToBeWritten ? kTextureWritten : kTexturePendingWriting;
	}

	return true;
}

/**
 * Write multi frame Texture to DRAM
 *
 * @private
 */
void BgmapTexture::writeAllFrames(int16 maximumTextureRowsToWrite, int16 xOffset, int16 yOffset, uint16 charSetOffset)
{
	if((0 > xOffset) | (0 > yOffset))
	{
		return;
	}

	bool isCharSetOptimized = CharSet::isOptimized(this->charSet);

	int16 currentXOffset = xOffset;
	int16 currentYOffset = yOffset;
	int16 charSetOffsetDelta = isCharSetOptimized ? 0 : this->textureSpec->cols * this->textureSpec->rows;

	this->mapDisplacement = 0;

	for(int16 frame = 0; frame < this->textureSpec->numberOfFrames; frame++)
	{
		this->remainingRowsToBeWritten = this->textureSpec->rows;

		BgmapTexture::writeFrame(this, maximumTextureRowsToWrite, true, currentXOffset, currentYOffset, charSetOffset, isCharSetOptimized ? frame : 0);

		charSetOffset += charSetOffsetDelta;

		currentXOffset += this->textureSpec->cols;

		if(64 <= currentXOffset + this->textureSpec->cols)
		{
			currentXOffset = xOffset;
			currentYOffset += this->textureSpec->rows;

			if(64 <= currentYOffset)
			{
				NM_ASSERT(false, "BgmapTexture::writeAllFrames: no more space");
				return;
			}
		}
	}
}

// TODO: inlining this causes trouble with ANIMATED_MULTI animations
static inline void BgmapTexture::addHWORD(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS, uint32 offset, uint16 flip, int16 increment)
{
#ifdef __SHOW_SPRITES_PROFILING
	extern int32 _writtenTextureTiles;
	_writtenTextureTiles += numberOfHWORDS;
#endif

	const HWORD* finalSource = source + numberOfHWORDS;

    asm
	(
		"jr		end%=			\n\t"      \
		"loop%=:				\n\t"      \
		"ld.h	0[%1], r10		\n\t"      \
		"xor	%4, r10			\n\t"      \
		"add	%3, r10			\n\t"      \
		"st.h	r10, 0[%0]		\n\t"      \
		"add	%5, %0			\n\t"      \
		"add	2, %1			\n\t"      \
		"end%=:					\n\t"      \
		"cmp	%1, %2			\n\t"      \
		"bgt	loop%=			\n\t"
		: /*"+r" (destination), "+r" (source)*/ // <- causes the graphics not to be writen
		:  "r" (destination), "r" (source), "r" (finalSource), "r" (offset), "r" (flip), "r" (increment)
		: "r10"
    );
}

// TODO: inlining this causes trouble with ANIMATED_MULTI animations
static inline void BgmapTexture::addHWORDCompressed(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS, uint32 offset, uint16 flip, int16 increment)
{
	const HWORD* finalSource = source + numberOfHWORDS;

    asm
	(
		"jr 	end%=			\n\t"      \
		"loop%=:				\n\t"      \
		"ld.h 	0[%1], r10		\n\t"      \
		"xor	%4, r10			\n\t"      \
		"add	%3, r10			\n\t"      \
		"st.h	r10, 0[%0]		\n\t"      \
		"add	%5, %0			\n\t"      \
		"add	2, %1			\n\t"      \
		"end%=:					\n\t"      \
		"cmp	%1, %2			\n\t"      \
		"bgt	loop%=			\n\t"
		: /*"+r" (destination), "+r" (source)*/ // <- causes the graphics not to be writen
		: "r" (destination), "r" (source), "r" (finalSource), "r" (offset), "r" (flip), "r" (increment)
		: "r10" // regs used
    );
}

/**
 * Write Texture to DRAM
 *
 * @private
 */
void BgmapTexture::writeFrame(int16 maximumTextureRowsToWrite, bool forceFullRewrite, int16 xOffset, int16 yOffset, uint16 charSetOffset, uint16 frame)
{
	if((0 > xOffset) || (0 > yOffset))
	{
		return;
	}

	int16 cols = this->textureSpec->cols;
	int16 rows = this->textureSpec->rows;
	HWORD* offsetDisplacement = (HWORD*)__BGMAP_SEGMENT(this->segment) + xOffset + (yOffset << 6);
	const HWORD* mapDisplacement = (HWORD*)this->textureSpec->map + this->mapDisplacement + cols * rows * frame;
	int32 counter = forceFullRewrite ? -1 : maximumTextureRowsToWrite;
	uint16 flip = ((this->horizontalFlip << 1) | this->verticalFlip) << 12;
	int8 remainingRowsToBeWritten = this->remainingRowsToBeWritten;
	uint16 offset = charSetOffset | (this->palette << 14);

	if(forceFullRewrite)
	{
		CACHE_RESET;
	}

	if(this->horizontalFlip)
	{
		if(this->verticalFlip)
		{
			//put the map into memory calculating the number of char for each reference
			for(; 0 != counter && 0 != remainingRowsToBeWritten--; counter--)
			{
				BgmapTexture::addHWORD
				(
					(HWORD*)offsetDisplacement + ((remainingRowsToBeWritten) << 6) + cols - 1,
					mapDisplacement + ((rows - remainingRowsToBeWritten - 1) * cols),
					cols,
					offset,
					flip,
					-2
				);
			}
		}
		else
		{
			//put the map into memory calculating the number of char for each reference
			for(; 0 != counter && 0 != remainingRowsToBeWritten--; counter--)
			{
				BgmapTexture::addHWORD
				(
					(HWORD*)offsetDisplacement + (remainingRowsToBeWritten << 6) + cols - 1,
					mapDisplacement + ((remainingRowsToBeWritten) * cols),
					cols,
					offset,
					flip,
					-2
				);
			}
		}
	}
	else
	{
		if(this->verticalFlip)
		{
			//put the map into memory calculating the number of char for each reference
			for(; 0 != counter && 0 != remainingRowsToBeWritten--; counter--)
			{
				BgmapTexture::addHWORD
				(
					(HWORD*)offsetDisplacement + (remainingRowsToBeWritten << 6),
					mapDisplacement + ((rows - remainingRowsToBeWritten - 1) * cols),
					cols,
					offset,
					flip,
					2
				);
			}
		}
		else
		{
			//put the map into memory calculating the number of char for each reference
			for(; 0 != counter && 0 != remainingRowsToBeWritten--; counter--)
			{
				BgmapTexture::addHWORD
				(
					(HWORD*)offsetDisplacement + (remainingRowsToBeWritten << 6),
					mapDisplacement + ((remainingRowsToBeWritten) * cols),
					cols,
					offset,
					flip,
					2
				);
			}
		}
	}

	this->remainingRowsToBeWritten = remainingRowsToBeWritten;

	if(0 < this->textureSpec->padding.rows && -1 == this->remainingRowsToBeWritten)
	{
		BgmapTexture::addHWORD
		(
			(HWORD*)offsetDisplacement + (rows << 6),
			(const HWORD*)_emptyTextureRow,
			cols,
			0,
			false,
			2
		);
	}
}

/*
void BgmapTexture::writeAnimatedSingleOptimized()
{
	int32 bgmapSegment = this->segment;
	int32 palette = this->palette << 14;

	int32 charLocation = (int32)CharSet::getOffset(this->charSet);

	if((0 > xOffset) | (0 > yOffset))
	{
		return;
	}

	int32 counter = SpriteManager::getTexturesMaximumRowsToWrite(SpriteManager::getInstance());

	//put the map into memory calculating the number of char for each reference
	for(; counter && this->remainingRowsToBeWritten--; counter--)
	{
		Mem::add ((uint8*)__BGMAP_SEGMENT(bgmapSegment) + ((xOffset + (yOffset << 6 ) + (this->remainingRowsToBeWritten << 6)) << 1),
				(const uint8*)(this->textureSpec->map + this->mapDisplacement + (this->remainingRowsToBeWritten * (this->textureSpec->cols) << 1)),
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
int8 BgmapTexture::getRemainingRowsToBeWritten()
{
	return this->remainingRowsToBeWritten;
}

/**
 * Set the Texture's x and y offsets within the BGMAP segment where it is allocated
 *
 * @return				Texture's x offset within BGMAP segment
 */
void BgmapTexture::setOffsets(int16 xOffset, int16 yOffset)
{
	this->xOffset = xOffset;
	this->yOffset = yOffset;
}

/**
 * Retrieve the Texture's x offset within the BGMAP segment where it is allocated
 *
 * @return				Texture's x offset within BGMAP segment
 */
int16 BgmapTexture::getXOffset()
{
	return this->xOffset;
}

/**
 * Retrieve the Texture's y offset within the BGMAP segment where it is allocated
 *
 * @return				Texture's y offset within BGMAP segment
 */
int16 BgmapTexture::getYOffset()
{
	return this->yOffset;
}

/**
 * Set the Texture's BGMAP segment where it is allocated
 *
 * @param segment		Texture's BGMAP segment
 */
void BgmapTexture::setSegment(int8 segment)
{
	this->segment = segment;
}

/**
 * Retrieve the Texture's BGMAP segment where it is allocated
 *
 * @return				Texture's BGMAP segment
 */
int8 BgmapTexture::getSegment()
{
	return this->segment;
}

void BgmapTexture::setHorizontalFlip(bool value)
{	
	// TODO: this is a hack, positioned entities should have a complete transformation
	// and the flip flags should be removed from the texture spec
	if(this->textureSpec->horizontalFlip)
	{
		value = !value;
	}

	if(this->horizontalFlip != value)
	{
		BgmapTexture::rewrite(this);
	}

	this->horizontalFlip = value;
}

void BgmapTexture::setVerticalFlip(bool value)
{
	// TODO: this is a hack, positioned entities should have a complete transformation
	// and the flip flags should be removed from the texture spec
	if(this->textureSpec->verticalFlip)
	{
		value = !value;
	}

	if(this->verticalFlip != value)
	{
		BgmapTexture::rewrite(this);
	}

	this->verticalFlip = value;
}
