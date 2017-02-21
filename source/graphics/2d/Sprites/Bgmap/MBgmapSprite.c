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
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MBgmapSprite.h>
#include <SpriteManager.h>
#include <BgmapTextureManager.h>
#include <Optics.h>
#include <Screen.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define __ACCOUNT_FOR_BGMAP_PLACEMENT		1
#define __GX_LIMIT							511
//#define __GY_LIMIT						511


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	MBgmapSprite
 * @extends BgmapSprite
 * @ingroup graphics-2d-sprites-bgmap
 * @brief	Sprite which holds a texture and a drawing specification.
 */
__CLASS_DEFINITION(MBgmapSprite, BgmapSprite);
__CLASS_FRIEND_DEFINITION(Texture);
__CLASS_FRIEND_DEFINITION(BgmapTexture);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
extern const VBVec3D* _screenPosition;
extern Optical* _optical;

static void MBgmapSprite_releaseTextures(MBgmapSprite this);
static void MBgmapSprite_loadTextures(MBgmapSprite this);
static void MBgmapSprite_loadTexture(MBgmapSprite this, TextureDefinition* textureDefinition);
static void MBgmapSprite_calculateSize(MBgmapSprite this);
//static void MBgmapSprite_calculateSizeMultiplier(MBgmapSprite this);
//static void MBgmapSprite_calculateSize(MBgmapSprite this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(MBgmapSprite, const MBgmapSpriteDefinition* mBgmapSpriteDefinition, Object owner)
__CLASS_NEW_END(MBgmapSprite, mBgmapSpriteDefinition, owner);

/**
 * Class constructor
 *
 * @memberof							MBgmapSprite
 * @public
 *
 * @param this							Function scope
 * @param mBgmapSpriteDefinition		Definition to use
 * @param owner							Sprite's owner
 */
void MBgmapSprite_constructor(MBgmapSprite this, const MBgmapSpriteDefinition* mBgmapSpriteDefinition, Object owner)
{
	__CONSTRUCT_BASE(BgmapSprite, &mBgmapSpriteDefinition->bgmapSpriteDefinition, owner);

	this->mBgmapSpriteDefinition = mBgmapSpriteDefinition;

	ASSERT(!this->texture, "MBgmapSprite::constructor: texture already loaded");
	this->textures = NULL;
	MBgmapSprite_loadTextures(this);
	MBgmapSprite_calculateSize(this);

	// TODO: check if this is really needed
/*
	// assume looping
	this->size.x = 0;
	this->size.y = 0;

	this->sizeMultiplier.x = 1;
	this->sizeMultiplier.y = 1;

	MBgmapSprite_calculateSize(this);
*/
}

/**
 * Class destructor
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param this		Function scope
 */
void MBgmapSprite_destructor(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::destructor: null this");

	MBgmapSprite_releaseTextures(this);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

/**
 * Release the loaded textures
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param this		Function scope
 */
static void MBgmapSprite_releaseTextures(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::releaseTextures: null this");

	if(this->textures)
	{
		VirtualNode node = this->textures->head;

		for(; node; node = node->next)
		{
			// free the texture
			BgmapTextureManager_releaseTexture(BgmapTextureManager_getInstance(), __SAFE_CAST(BgmapTexture, node->data));
		}

		__DELETE(this->textures);
		this->textures = NULL;
		this->texture = NULL;
	}
}

/**
 * Load textures from definition
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param this		Function scope
 */
static void MBgmapSprite_loadTextures(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::loadTextures: null this");

	if(this->mBgmapSpriteDefinition)
	{
		MBgmapSprite_releaseTextures(this);
		this->textures = __NEW(VirtualList);

		int i = 0;

		for(; this->mBgmapSpriteDefinition->textureDefinitions[i]; i++)
		{
			MBgmapSprite_loadTexture(this, this->mBgmapSpriteDefinition->textureDefinitions[i]);
		}

		this->texture = __SAFE_CAST(Texture, VirtualList_front(this->textures));
		ASSERT(this->texture, "MBgmapSprite::loadTextures: null texture");

		this->textureXOffset = BgmapTexture_getXOffset(__SAFE_CAST(BgmapTexture, this->texture)) << 3;
		this->textureYOffset = BgmapTexture_getYOffset(__SAFE_CAST(BgmapTexture, this->texture)) << 3;
	}
}

/**
 * Load a texture
 *
 * @memberof					MBgmapSprite
 * @public
 *
 * @param this					Function scope
 * @param textureDefinition		TextureDefinition to use
 */
static void MBgmapSprite_loadTexture(MBgmapSprite this, TextureDefinition* textureDefinition)
{
	ASSERT(this, "MBgmapSprite::loadTexture: null this");

	ASSERT(textureDefinition, "MBgmapSprite::loadTexture: null textureDefinition");

	BgmapTexture bgmapTexture = BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), textureDefinition);

	ASSERT(bgmapTexture, "MBgmapSprite::loadTexture: texture not loaded");
	ASSERT(this->textures, "MBgmapSprite::loadTexture: null textures list");

	VirtualList_pushBack(this->textures, bgmapTexture);
}

/**
 * Calculate 2D position
 *
 * @memberof			MBgmapSprite
 * @public
 *
 * @param this			Function scope
 * @param position		3D position
 */
void MBgmapSprite_position(MBgmapSprite this, const VBVec3D* position)
{
	ASSERT(this, "MBgmapSprite::position: null this");

	VBVec3D position3D = *position;

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	VBVec2D position2D;

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, position2D);

	position2D.x -= this->halfWidth;
	position2D.y -= this->halfHeight;

	MBgmapSprite_setPosition(this, &position2D);
}

/**
 * Set 2D position
 *
 * @memberof			BgmapSprite
 * @public
 *
 * @param this			Function scope
 * @param position		New 2D position
 */
void MBgmapSprite_setPosition(MBgmapSprite this, const VBVec2D* position)
{
	ASSERT(this, "MBgmapSprite::setPosition: null this");

	if(this->mBgmapSpriteDefinition->xLoop)
	{
		this->drawSpec.position.x = 0;
		this->drawSpec.textureSource.mx = -FIX19_13TOI(position->x + __0_5F_FIX19_13);
	}
	else
	{
		this->drawSpec.textureSource.mx = this->textureXOffset;
		this->drawSpec.position.x = position->x;

		if(0 > position->x + this->displacement.x)
		{
			this->drawSpec.textureSource.mx -= FIX19_13TOI(position->x + this->displacement.x + __0_5F_FIX19_13);
		}
	}

	if(this->mBgmapSpriteDefinition->yLoop)
	{
		this->drawSpec.position.y = 0;
		this->drawSpec.textureSource.my = -FIX19_13TOI(position->y + __0_5F_FIX19_13);
	}
	else
	{
		this->drawSpec.textureSource.my = this->textureYOffset;
		this->drawSpec.position.y = position->y;

		if(0 > position->y + this->displacement.y)
		{
			this->drawSpec.textureSource.my -= FIX19_13TOI(position->y + this->displacement.y + __0_5F_FIX19_13);
		}
	}

	fix19_13 previousZPosition = this->drawSpec.position.z;
	this->drawSpec.position.z = position->z;

	if(previousZPosition != this->drawSpec.position.z)
	{
		// calculate sprite's parallax
		__VIRTUAL_CALL(Sprite, calculateParallax, __SAFE_CAST(Sprite, this), this->drawSpec.position.z);
	}

	if(!this->worldLayer)
	{
		// register with sprite manager
		SpriteManager_registerSprite(SpriteManager_getInstance(), __SAFE_CAST(Sprite, this));
	}
}

/**
 * Add displacement to position
 *
 * @memberof				MBgmapSprite
 * @public
 *
 * @param this				Function scope
 * @param displacement		2D position displacement
 */
void MBgmapSprite_addDisplacement(MBgmapSprite this, const VBVec2D* displacement)
{
	ASSERT(this, "MBgmapSprite::addDisplacement: null this");

	if(this->mBgmapSpriteDefinition->xLoop)
	{
		this->drawSpec.position.x = 0;
		this->drawSpec.textureSource.mx -= FIX19_13TOI(displacement->x + __0_5F_FIX19_13);
	}
	else
	{
		this->drawSpec.textureSource.mx = this->textureXOffset;
		this->drawSpec.position.x += displacement->x;

		if(0 > this->drawSpec.position.x + this->displacement.x)
		{
			this->drawSpec.textureSource.mx -= FIX19_13TOI(this->drawSpec.position.x + this->displacement.x + __0_5F_FIX19_13);
		}
	}

	if(this->mBgmapSpriteDefinition->yLoop)
	{
		this->drawSpec.position.y = 0;
		this->drawSpec.textureSource.my -= FIX19_13TOI(displacement->y + __0_5F_FIX19_13);
	}
	else
	{
		this->drawSpec.textureSource.my = this->textureYOffset;
		this->drawSpec.position.y += displacement->y;

		if(0 > this->drawSpec.position.y + this->displacement.y)
		{
			this->drawSpec.textureSource.my -= FIX19_13TOI(this->drawSpec.position.y + this->displacement.y + __0_5F_FIX19_13);
		}
	}

	this->drawSpec.position.z += displacement->z;
	this->drawSpec.position.parallax += displacement->parallax;
}

/**
 * Write WORLD data to DRAM
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param this		Function scope
 */
void MBgmapSprite_render(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::render: null this");

	// if render flag is set
	if(this->texture && this->worldLayer)
	{
		static WorldAttributes* worldPointer = NULL;
		worldPointer = &_worldAttributesBaseAddress[this->worldLayer];

		// TODO: check if required, causes that the sprite is turned off
		// when changing the texture definition
/*
		if(!this->texture->written)
		{
			worldPointer->head = 0x0000;
			return;
		}
*/

		// set the head
		worldPointer->head = this->head | (__SAFE_CAST(BgmapTexture, this->texture))->segment;

		// get coordinates
		int gx = FIX19_13TOI(this->drawSpec.position.x + this->displacement.x + __0_5F_FIX19_13);
		int gy = FIX19_13TOI(this->drawSpec.position.y + this->displacement.y + __0_5F_FIX19_13);

		// get sprite's size
		worldPointer->gx = gx > __SCREEN_WIDTH? __SCREEN_WIDTH : gx < 0? 0: gx;
		worldPointer->gy = gy > __SCREEN_HEIGHT? __SCREEN_HEIGHT : gy < 0? 0: gy;
		worldPointer->gp = this->drawSpec.position.parallax + FIX19_13TOI((this->displacement.z + this->displacement.p) & 0xFFFFE000);

		worldPointer->mx = this->drawSpec.textureSource.mx;
		worldPointer->my = this->drawSpec.textureSource.my;
		worldPointer->mp = this->drawSpec.textureSource.mp;

		// set the world size
		if(!this->mBgmapSpriteDefinition->xLoop)
		{
			int w = (((int)Texture_getCols(this->texture))<< 3) - 1 - (worldPointer->mx - this->textureXOffset);
			worldPointer->w = w + worldPointer->gx > __SCREEN_WIDTH? __SCREEN_WIDTH - worldPointer->gx: 0 > w? 0: w;

			if(!worldPointer->w)
			{
				worldPointer->head = __WORLD_OFF;
#ifdef __PROFILE_GAME
				worldPointer->w = 0;
				worldPointer->h = 0;
#endif
				return;
			}
		}
		else
		{
			worldPointer->gx -= (this->drawSpec.position.parallax);
			worldPointer->w = __SCREEN_WIDTH + (this->drawSpec.position.parallax << 1);
		}

		if(!this->mBgmapSpriteDefinition->yLoop)
		{
			int h = (((int)Texture_getRows(this->texture))<< 3) - 1 - (worldPointer->my - this->textureYOffset);
			worldPointer->h = h + worldPointer->gy > __SCREEN_HEIGHT? __SCREEN_HEIGHT - worldPointer->gy: 0 > h? 0: h;

			if(!worldPointer->h)
			{
				worldPointer->head = __WORLD_OFF;
#ifdef __PROFILE_GAME
				worldPointer->w = 0;
				worldPointer->h = 0;
#endif
				return;
			}
		}
		else
		{
			worldPointer->h = __SCREEN_HEIGHT;
		}
	}
}

/**
 * Retrieve position
 *
 * @memberof		MBgmapSprite
 * @public
 *
 * @param this		Function scope
 *
 * @return			2D position
 */
VBVec2D MBgmapSprite_getPosition(MBgmapSprite this)
{
	ASSERT(this, "BgmapSprite::getPosition: null this");

	return this->drawSpec.position;
}

/**
 * Resize
 *
 * @memberof			MBgmapSprite
 * @public
 *
 * @param this			Function scope
 * @param scale			Scale to apply
 * @param z				Z coordinate to base on the size calculation
 */
void MBgmapSprite_resize(MBgmapSprite this, Scale scale, fix19_13 z)
{
	ASSERT(this, "MBgmapSprite::resize: null this");

	BgmapSprite_resize(__SAFE_CAST(BgmapSprite, this), scale, z);

	MBgmapSprite_calculateSize(this);
}

/**
 * Calculate Sprite's size
 *
 * @memberof			MBgmapSprite
 * @public
 *
 * @param this			Function scope
 */
static void MBgmapSprite_calculateSize(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::calculateSize: null this");

	VirtualNode node = this->textures->head;

	int cols = 0;
	int rows = 0;

	for(; node; node = node->next)
	{
		// free the texture
		int textureCols = Texture_getCols(__SAFE_CAST(Texture, node->data));
		int textureRows = Texture_getRows(__SAFE_CAST(Texture, node->data));

		if(cols < textureCols)
		{
			cols = textureCols;
		}

		if(rows < textureRows)
		{
			rows = textureRows;
		}
	}

	this->halfWidth = ITOFIX19_13(cols << 2);
	this->halfHeight = ITOFIX19_13(rows << 2);
}

/*
// calculate total sprite's size
static void MBgmapSprite_calculateSize(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::calculateSize: null this");

	MBgmapSprite_calculateSizeMultiplier(this);

	Texture texture = __SAFE_CAST(Texture, VirtualList_front(this->textures));

	if(!this->mBgmapSpriteDefinition->xLoop)
	{
		this->size.x = this->sizeMultiplier.x << 9;
	}

	if(!this->mBgmapSpriteDefinition->yLoop)
	{
		this->size.y = ((texture ? Texture_getRows(texture) : 64) << 3) * this->sizeMultiplier.x;
	}
}


// calculate the size multiplier
static void MBgmapSprite_calculateSizeMultiplier(MBgmapSprite this)
{
	ASSERT(this, "MBgmapSprite::calculateSizeMultiplier: null this");

	switch(this->mBgmapSpriteDefinition->scValue)
	{
		case __WORLD_1x1:

			this->sizeMultiplier.x = 1;
			this->sizeMultiplier.y = 1;
			break;

		case __WORLD_1x2:

			this->sizeMultiplier.x = 1;
			this->sizeMultiplier.y = 2;
			break;

		case __WORLD_1x4:

			this->sizeMultiplier.x = 1;
			this->sizeMultiplier.y = 4;
			break;

		case __WORLD_1x8:

			this->sizeMultiplier.x = 1;
			this->sizeMultiplier.y = 8;
			break;

		case __WORLD_2x1:

			this->sizeMultiplier.x = 2;
			this->sizeMultiplier.y = 1;
			break;

		case __WORLD_2x2:

			this->sizeMultiplier.x = 2;
			this->sizeMultiplier.y = 2;
			break;

		case __WORLD_2x4:

			this->sizeMultiplier.x = 2;
			this->sizeMultiplier.y = 4;
			break;

		case __WORLD_4x1:

			this->sizeMultiplier.x = 4;
			this->sizeMultiplier.y = 1;
			break;

		case __WORLD_4x2:

			this->sizeMultiplier.x = 4;
			this->sizeMultiplier.y = 2;
			break;

		case __WORLD_8x1:

			this->sizeMultiplier.x = 8;
			this->sizeMultiplier.y = 1;
			break;
	}
}
*/

/**
 * Set Sprite's render mode
 *
 * @memberof	MBgmapSprite
 * @public
 *
 * @param this	Function scope
 */
void MBgmapSprite_setMode(MBgmapSprite this __attribute__ ((unused)), u16 display __attribute__ ((unused)), u16 mode __attribute__ ((unused)))
{
	ASSERT(this, "MBgmapSprite::setMode: null this");
}
