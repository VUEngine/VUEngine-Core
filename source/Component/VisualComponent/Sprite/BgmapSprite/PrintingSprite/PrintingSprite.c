/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <BgmapTexture.h>
#include <DebugConfig.h>
#include <Mem.h>
#include <Optics.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include "PrintingSprite.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Texture;
friend class BgmapTexture;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void PrintingSprite::constructor(Entity owner, const PrintingSpriteSpec* printingSpriteSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, &printingSpriteSpec->bgmapSpriteSpec);

	this->checkIfWithinScreenSpace = false;

	if(!isDeleted(this->texture))
	{
		Texture::write(this->texture, -1);
	}

	PrintingSprite::reset(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void PrintingSprite::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 PrintingSprite::doRender(int16 index)
{
	WorldAttributes* worldPointer = &_worldAttributesCache[index];

	worldPointer->mx = this->bgmapTextureSource.mx;
	worldPointer->mp = this->bgmapTextureSource.mp;
	worldPointer->my = this->bgmapTextureSource.my;
	worldPointer->gx = this->position.x - this->halfWidth;
	worldPointer->gp = this->position.parallax;
	worldPointer->gy = this->position.y - this->halfHeight;
	worldPointer->w = this->halfWidth << 1;
	worldPointer->h = this->halfHeight << 1;
	worldPointer->head = __WORLD_ON | __WORLD_BGMAP | __WORLD_OVR |  (BgmapTexture::safeCast(this->texture))->segment;

	return index;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void PrintingSprite::reset()
{
	this->position.x = 0;
	this->position.y = 0;
	this->position.parallax = 0;

	NM_ASSERT(!isDeleted(this->texture), "PrintingSprite::reset: no texture");

	if(!isDeleted(this->texture))
	{
		this->bgmapTextureSource.mx = BgmapTexture::getXOffset(this->texture) << 3;
		this->bgmapTextureSource.my = BgmapTexture::getYOffset(this->texture) << 3;
		this->bgmapTextureSource.mp = __PRINTING_BGMAP_PARALLAX_OFFSET;
		this->halfWidth = this->texture->textureSpec->cols << 2;
		this->halfHeight = this->texture->textureSpec->rows << 2;
	}
	else
	{
		this->bgmapTextureSource.mx = __PRINTING_BGMAP_X_OFFSET;
		this->bgmapTextureSource.my = __PRINTING_BGMAP_Y_OFFSET;
		this->bgmapTextureSource.mp = __PRINTING_BGMAP_PARALLAX_OFFSET;	

		this->halfWidth = __HALF_SCREEN_WIDTH;
		this->halfHeight = __HALF_SCREEN_HEIGHT;
	}

	if(__HALF_SCREEN_WIDTH < this->halfWidth)
	{
		this->halfWidth = __HALF_SCREEN_WIDTH;
	}

	if(__HALF_SCREEN_HEIGHT < this->halfHeight)
	{
		this->halfHeight = __HALF_SCREEN_HEIGHT;
	}

	this->rendered = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void PrintingSprite::clear()
{
	if(!isDeleted(this->texture))
	{
		Texture::write(this->texture, -1);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void PrintingSprite::setGValues(int16 gx, int16 gy, int16 gp)
{
	this->position.x = gx;
	this->position.y = gy;
	this->position.parallax = gp;

	this->rendered = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void PrintingSprite::setMValues(int16 mx, int16 my, int16 mp)
{
	this->bgmapTextureSource.mx = mx;
	this->bgmapTextureSource.my = my;
	this->bgmapTextureSource.mp = mp;

	this->rendered = false;
}

void PrintingSprite::setSize(uint16 width, uint16 height)
{
	this->halfWidth = width >> 1;
	this->halfHeight = height >> 1;

	this->rendered = false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
