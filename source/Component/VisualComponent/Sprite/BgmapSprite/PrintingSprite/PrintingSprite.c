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

	this->hasTextures = false;
	this->transformation = NULL;

	if(NULL == this->texture)
	{
		PrintingSprite::loadTexture(this, typeofclass(BgmapTexture), false);		
	}

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
	worldPointer->gx = this->position.x;
	worldPointer->gp = this->position.parallax;
	worldPointer->gy = this->position.y;
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
	this->bgmapTextureSource.mx = 0;
	this->bgmapTextureSource.my = 0;
	this->bgmapTextureSource.mp = 0;	

	this->halfWidth = __HALF_SCREEN_WIDTH;
	this->halfHeight = __HALF_SCREEN_HEIGHT;

	if(!isDeleted(this->texture))
	{
		PrintingSprite::setMValues(this, 0, 0, 0);
		PrintingSprite::setSize(this, this->texture->textureSpec->cols << 3,this->texture->textureSpec->rows << 3);
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

void PrintingSprite::setMValues(int16 mx, int16 my, int16 mp)
{
	if(!isDeleted(this->texture))
	{
		this->bgmapTextureSource.mx = BgmapTexture::getXOffset(this->texture) << 3;
		this->bgmapTextureSource.my = BgmapTexture::getYOffset(this->texture) << 3;
		this->bgmapTextureSource.mp = __PRINTING_BGMAP_PARALLAX_OFFSET;

		this->bgmapTextureSource.mx += mx;
		this->bgmapTextureSource.my += my;
		this->bgmapTextureSource.mp += mp;
	}
	else
	{
		this->bgmapTextureSource.mx = mx;
		this->bgmapTextureSource.my = my;
		this->bgmapTextureSource.mp = mp;
	}

	this->rendered = false;
}

void PrintingSprite::setSize(uint16 width, uint16 height)
{
	this->halfWidth = width >> 1;
	this->halfHeight = height >> 1;

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
