/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <BgmapTextureManager.h>
#include <DebugConfig.h>
#include <Optics.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VIPManager.h>

#include "PrintingSprite.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void PrintingSprite::constructor(GameObject owner, const PrintingSpriteSpec* printingSpriteSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, &printingSpriteSpec->bgmapSpriteSpec);

	this->hasTextures = false;

	PrintingSprite::reset(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void PrintingSprite::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 PrintingSprite::doRender(int16 index)
{
	WorldAttributes* worldPointer = &_worldAttributesCache[index];

	worldPointer->mx = this->bgmapTextureSource.mx;
	worldPointer->mp = this->bgmapTextureSource.mp;
	worldPointer->my = this->bgmapTextureSource.my;
	worldPointer->gx = this->position.x;
	worldPointer->gp = this->position.parallax;
	worldPointer->gy = this->position.y;
	worldPointer->w = this->halfWidth << 1;
	worldPointer->h = this->halfHeight << 1;
	worldPointer->head = __WORLD_ON | __WORLD_BGMAP | __WORLD_OVR | this->printingBgmapSegment;

	return index;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void PrintingSprite::reset()
{
	this->position.x = 0;
	this->position.y = 0;
	this->position.parallax = 0;

	this->bgmapTextureSource.mx = __PRINTING_BGMAP_X_OFFSET;
	this->bgmapTextureSource.my = __PRINTING_BGMAP_Y_OFFSET;
	this->bgmapTextureSource.mp = __PRINTING_BGMAP_PARALLAX_OFFSET;

	this->halfWidth = __SCREEN_WIDTH >> 1;
	this->halfHeight = __SCREEN_HEIGHT >> 1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void PrintingSprite::setPrintingBgmapSegment(int8 printingBgmapSegment)
{
	if((unsigned)printingBgmapSegment < __MAX_NUMBER_OF_BGMAPS_SEGMENTS)
	{
		this->printingBgmapSegment = printingBgmapSegment;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void PrintingSprite::setGValues(int16 gx, int16 gy, int16 gp)
{
	this->position.x = gx;
	this->position.y = gy;
	this->position.parallax = gp;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void PrintingSprite::setMValues(int16 mx, int16 my, int16 mp)
{
	this->bgmapTextureSource.mx = mx;
	this->bgmapTextureSource.my = my;
	this->bgmapTextureSource.mp = mp;
}

void PrintingSprite::setSize(uint16 width, uint16 height)
{
	this->halfWidth = width >> 1;
	this->halfHeight = height >> 1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

