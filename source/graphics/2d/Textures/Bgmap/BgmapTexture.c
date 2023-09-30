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

#include <BgmapTexture.h>

#include <BgmapTextureManager.h>
#include <CharSet.h>
#include <Mem.h>
#include <SpriteManager.h>
#include <VIPManager.h>

#include <debugConfig.h>
#include <debugUtilities.h>


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
	
	uint8 allocationType = __NO_ALLOCATION_TYPE;

	if(!isDeleted(this->charSet))
	{
		allocationType = CharSet::getAllocationType(this->charSet);
	}
	else if(NULL != this->textureSpec->charSetSpec)
	{
		allocationType = this->textureSpec->charSetSpec->allocationType;
	}
	else
	{
		return false;
	}

	//determine the allocation type
	switch(allocationType)
	{
		case __NOT_ANIMATED:

			BgmapTexture::doWrite(this, maximumTextureRowsToWrite, false);
			break;

		case __ANIMATED_MULTI:

			// write the spec to graphic memory
			BgmapTexture::writeAnimatedMulti(this, maximumTextureRowsToWrite);
			break;
/*
		case __ANIMATED_SINGLE:
		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:
		case __ANIMATED_SINGLE_OPTIMIZED:
		case __ANIMATED_SHARED_OPTIMIZED:
		case __ANIMATED_SHARED_COORDINATED_OPTIMIZED:
*/
		default:

			BgmapTexture::doWrite(this, maximumTextureRowsToWrite, kTexturePendingWriting < status && kTextureFrameChanged >= status);
			break;
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
 * Write __ANIMATED_MULTI Texture to DRAM
 *
 * @private
 */
void BgmapTexture::writeAnimatedMulti(int16 maximumTextureRowsToWrite)
{
	int32 xOffset = (int32)BgmapTextureManager::getXOffset(_bgmapTextureManager, this->id);
	int32 yOffset = (int32)BgmapTextureManager::getYOffset(_bgmapTextureManager, this->id);

	if((0 > xOffset) | (0 > yOffset))
	{
		return;
	}

	int32 bgmapSegment = this->segment;
	int32 offsetDisplacement = xOffset + (yOffset << 6);
	int32 palette = this->palette << 14;

	// determine the number of frames the map had
	int32 area = (this->textureSpec->cols * this->textureSpec->rows);
	int32 charLocation = (int32)CharSet::getOffset(this->charSet);
	int32 frames = CharSet::getNumberOfChars(this->charSet) / area;
	uint32 mapDisplacement = this->mapDisplacement;

	int32 counter = maximumTextureRowsToWrite;

	// put the map into memory calculating the number of char for each reference
	for(; 0 != counter && this->remainingRowsToBeWritten--; counter--)
	{
		int32 j = 1;
		// write into the specified bgmap segment plus the offset defined in the this structure, the this spec
		// specifying the char displacement inside the char mem
		for(; j <= frames; j++)
		{
			Mem::addHWORD((HWORD*)__BGMAP_SEGMENT(bgmapSegment) + (offsetDisplacement + (this->textureSpec->cols * (j - 1)) + (this->remainingRowsToBeWritten << 6)),
				(const HWORD*)this->textureSpec->map + mapDisplacement + (this->remainingRowsToBeWritten * this->textureSpec->cols),
				this->textureSpec->cols,
				(palette) | (charLocation + area * (j - 1)));

#ifdef __SHOW_SPRITES_PROFILING
				extern int32 _writtenTextureTiles;
				_writtenTextureTiles += this->textureSpec->cols;
#endif
		}
	}

	if(this->textureSpec->padding.rows && -1 == this->remainingRowsToBeWritten)
	{
		int32 j = 1;
		// write into the specified bgmap segment plus the offset defined in the this structure, the this spec
		// specifying the char displacement inside the char mem
		for(; j <= frames; j++)
		{
			Mem::addHWORD((HWORD*)__BGMAP_SEGMENT(bgmapSegment) + (offsetDisplacement + (this->textureSpec->cols * (j - 1)) + (this->textureSpec->rows << 6)),
				(const HWORD*)_emptyTextureRow,
				this->textureSpec->cols,
				0
			);

#ifdef __SHOW_SPRITES_PROFILING
			extern int32 _writtenTextureTiles;
			_writtenTextureTiles += this->textureSpec->cols;
#endif
		}
	}
}

/**
 * Set Texture's frame
 *
 * @param frame	Texture's frame to display
 */
void BgmapTexture::setFrameAnimatedMulti(uint16 frame __attribute__((unused)))
{
	BgmapTexture::fireEvent(this, kEventTextureSetFrame);
	NM_ASSERT(!isDeleted(this), "BgmapTexture::setFrameAnimatedMulti: deleted this during kEventTextureSetFrame");
}

// TODO: inlining this causes trouble with ANIMATED_MULTI animations
static inline void BgmapTexture::addHWORD(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS, uint32 offset, uint16 flip, int16 increment)
{
#ifdef __SHOW_SPRITES_PROFILING
	extern int32 _writtenTextureTiles;
	_writtenTextureTiles += numberOfHWORDS;
#endif

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
	: "r10", "sp", "lp" // regs used
    );
}

// TODO: inlining this causes trouble with ANIMATED_MULTI animations
static inline void BgmapTexture::addHWORDCompressed(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS, uint32 offset, uint16 flip, int16 increment)
{
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
	: "r10", "sp", "lp" // regs used
    );
}

/**
 * Write Texture to DRAM
 *
 * @private
 */
void BgmapTexture::doWrite(int16 maximumTextureRowsToWrite, bool forceFullRewrite)
{
	int32 xOffset = (int32)BgmapTextureManager::getXOffset(_bgmapTextureManager, this->id);
	int32 yOffset = (int32)BgmapTextureManager::getYOffset(_bgmapTextureManager, this->id);

	if((0 > xOffset) | (0 > yOffset))
	{
		return;
	}

	int32 bgmapSegment = this->segment;
	HWORD* offsetDisplacement = (HWORD*)__BGMAP_SEGMENT(bgmapSegment) + xOffset + (yOffset << 6);
	const HWORD* mapDisplacement = (HWORD*)this->textureSpec->map + this->mapDisplacement;
	uint16 offset = (int32)CharSet::getOffset(this->charSet) | (this->palette << 14);
	int32 counter = forceFullRewrite ? -1 : maximumTextureRowsToWrite;
	uint16 flip = ((this->horizontalFlip << 1) | this->verticalFlip) << 12;
	int8 remainingRowsToBeWritten = this->remainingRowsToBeWritten;
	int16 cols = this->textureSpec->cols;
	int16 rows = this->textureSpec->rows;

	// This micromanagemt actually works. Don't remove
	CACHE_DISABLE;
	CACHE_CLEAR;
	CACHE_ENABLE;

	if(this->horizontalFlip)
	{
		if(this->verticalFlip)
		{
			//put the map into memory calculating the number of char for each reference
			for(; counter && remainingRowsToBeWritten--; counter--)
			{
				BgmapTexture::addHWORD((HWORD*)offsetDisplacement + ((remainingRowsToBeWritten) << 6) + cols - 1,
						mapDisplacement + ((rows - remainingRowsToBeWritten - 1) * cols),
						cols,
						offset,
						flip,
						-2);
			}
		}
		else
		{
			//put the map into memory calculating the number of char for each reference
			for(; counter && remainingRowsToBeWritten--; counter--)
			{
				BgmapTexture::addHWORD((HWORD*)offsetDisplacement + (remainingRowsToBeWritten << 6) + cols - 1,
						mapDisplacement + ((remainingRowsToBeWritten) * cols),
						cols,
						offset,
						flip,
						-2);
			}
		}
	}
	else
	{
		if(this->verticalFlip)
		{
			//put the map into memory calculating the number of char for each reference
			for(; counter && remainingRowsToBeWritten--; counter--)
			{
				BgmapTexture::addHWORD((HWORD*)offsetDisplacement + (remainingRowsToBeWritten << 6),
						mapDisplacement + ((rows - remainingRowsToBeWritten - 1) * cols),
						cols,
						offset,
						flip,
						2);
			}
		}
		else
		{
			//put the map into memory calculating the number of char for each reference
			for(; counter && remainingRowsToBeWritten--; counter--)
			{
				BgmapTexture::addHWORD((HWORD*)offsetDisplacement + (remainingRowsToBeWritten << 6),
						mapDisplacement + ((remainingRowsToBeWritten) * cols),
						cols,
						offset,
						flip,
						2);
			}
		}
	}

	this->remainingRowsToBeWritten = remainingRowsToBeWritten;

	if(0 < this->textureSpec->padding.rows && -1 == this->remainingRowsToBeWritten)
	{
		BgmapTexture::addHWORD((HWORD*)offsetDisplacement + (rows << 6),
				(const HWORD*)_emptyTextureRow,
				cols,
				0,
				false,
				2);
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
 * Retrieve the Texture's x offset within the BGMAP segment where it is allocated
 *
 * @return				Texture's x offset within BGMAP segment
 */
int16 BgmapTexture::getXOffset()
{
	return BgmapTextureManager::getXOffset(_bgmapTextureManager, this->id);
}

/**
 * Retrieve the Texture's y offset within the BGMAP segment where it is allocated
 *
 * @return				Texture's y offset within BGMAP segment
 */
int16 BgmapTexture::getYOffset()
{
	return BgmapTextureManager::getYOffset(_bgmapTextureManager, this->id);
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
