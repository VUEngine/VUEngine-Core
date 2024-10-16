/*
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

#include <PrintingSprite.h>

#include <BgmapTextureManager.h>
#include <Optics.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VIPManager.h>

#include <DebugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof						PrintingSprite
 * @public
 *
 * @param bgmapSpriteSpec		Sprite spec
 * @param owner						Owner
 */
void PrintingSprite::constructor(SpatialObject owner, const PrintingSpriteSpec* printingSpriteSpec)
{
	Base::constructor(owner, &printingSpriteSpec->bgmapSpriteSpec);

	PrintingSprite::reset(this);
}

/**
 * Class denstructor
 *
 * @memberof			PrintingSprite
 * @public
 */
void PrintingSprite::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void PrintingSprite::setPrintingBgmapSegment(int8 printingBgmapSegment)
{
	if((unsigned)printingBgmapSegment < __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		this->printingBgmapSegment = printingBgmapSegment;
	}
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		PrintingSprite
 * @public
 *
 * @param evenFrame
 */
int16 PrintingSprite::doRender(int16 index)
{
	WorldAttributes* worldPointer = &_worldAttributesCache[index];

	worldPointer->mx = this->bgmapTextureSource.mx;
	worldPointer->mp = this->bgmapTextureSource.mp;
	worldPointer->my = this->bgmapTextureSource.my;
	worldPointer->gx = this->position.x;
	worldPointer->gp = this->position.parallax;
	worldPointer->gy = this->position.y;
	worldPointer->w = this->w;
	worldPointer->h = this->h;
	worldPointer->head = __WORLD_ON | __WORLD_BGMAP | __WORLD_OVR | this->printingBgmapSegment;

	return index;
}

void PrintingSprite::reset()
{
	this->position.x = 0;
	this->position.y = 0;
	this->position.parallax = 0;

	this->bgmapTextureSource.mx = __PRINTING_BGMAP_X_OFFSET;
	this->bgmapTextureSource.my = __PRINTING_BGMAP_Y_OFFSET;
	this->bgmapTextureSource.mp = __PRINTING_BGMAP_PARALLAX_OFFSET;

	this->w = __SCREEN_WIDTH - 1;
	this->h = __SCREEN_HEIGHT - 1;
}

void PrintingSprite::setGValues(int16 gx, int16 gy, int16 gp)
{
	this->position.x = gx;
	this->position.y = gy;
	this->position.parallax = gp;
}

void PrintingSprite::setMValues(int16 mx, int16 my, int16 mp)
{
	this->bgmapTextureSource.mx = mx;
	this->bgmapTextureSource.my = my;
	this->bgmapTextureSource.mp = mp;
}

void PrintingSprite::setSize(uint16 w, uint16 h)
{
	this->w = w;
	this->h = h;
}

int16 PrintingSprite::getGX()
{
	return this->position.x;
}

int16 PrintingSprite::getGY()
{
	return this->position.y;
}

int16 PrintingSprite::getGP()
{
	return this->position.parallax;
}