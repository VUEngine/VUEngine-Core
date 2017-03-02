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

#include <Object.h>
#include <RecyclableImage.h>
#include <Optics.h>
#include <Shape.h>
#include <Prototypes.h>
#include <Game.h>
#include <RecyclableBgmapTextureManager.h>
#include <BgmapSprite.h>
#include <MBgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	RecyclableImage
 * @extends Entity
 * @ingroup stage-entities
 */
__CLASS_DEFINITION(RecyclableImage, Entity);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void RecyclableImage_registerTextures(RecyclableImage this);
static void RecyclableImage_releaseTextures(RecyclableImage this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(RecyclableImage, RecyclableImageDefinition* RecyclableImageDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(RecyclableImage, RecyclableImageDefinition, id, internalId, name);

// class's constructor
void RecyclableImage_constructor(RecyclableImage this, RecyclableImageDefinition* RecyclableImageDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "RecyclableImage::constructor: null this");
	ASSERT(RecyclableImageDefinition, "RecyclableImage::constructor: null definition");
	ASSERT(RecyclableImageDefinition->spritesDefinitions[0], "RecyclableImage::constructor: null sprite definition");

	// construct base object
	__CONSTRUCT_BASE(Entity, (EntityDefinition*)RecyclableImageDefinition, id, internalId, name);

	this->RecyclableImageDefinition = RecyclableImageDefinition;

	RecyclableImage_registerTextures(this);
}

// class's destructor
void RecyclableImage_destructor(RecyclableImage this)
{
	ASSERT(this, "RecyclableImage::destructor: null this");

	RecyclableImage_releaseTextures(this);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// set definition
void RecyclableImage_setDefinition(RecyclableImage this, RecyclableImageDefinition* RecyclableImageDefinition)
{
	ASSERT(this, "RecyclableImage::setDefinition: null this");
	ASSERT(RecyclableImageDefinition, "RecyclableImage::setDefinition: null definition");

	// save definition
	this->RecyclableImageDefinition = RecyclableImageDefinition;

	Entity_setDefinition(__SAFE_CAST(Entity, this), (EntityDefinition*)RecyclableImageDefinition);
}

void RecyclableImage_suspend(RecyclableImage this)
{
	ASSERT(this, "RecyclableImage::suspend: null this");

	RecyclableImage_releaseTextures(this);

	Entity_suspend(__SAFE_CAST(Entity, this));
}

void RecyclableImage_resume(RecyclableImage this)
{
	ASSERT(this, "RecyclableImage::resume: null this");

	// first register with the manager so it handles the texture loading process
	RecyclableImage_registerTextures(this);

	Entity_resume(__SAFE_CAST(Entity, this));
}

void RecyclableImage_releaseGraphics(RecyclableImage this)
{
	ASSERT(this, "RecyclableImage::releaseGraphics: null this");

	RecyclableImage_releaseTextures(this);

	Entity_releaseGraphics(__SAFE_CAST(Entity, this));
}

static void RecyclableImage_registerTextures(RecyclableImage this)
{
	ASSERT(this, "RecyclableImage::registerTextures: null this");

	if(this->RecyclableImageDefinition->spritesDefinitions[0])
	{
		int i = 0;

		for(; this->RecyclableImageDefinition->spritesDefinitions[i]; i++)
		{
			if(__TYPE(BgmapSprite) == __ALLOCATOR_TYPE(this->RecyclableImageDefinition->spritesDefinitions[i]->allocator))
			{
				RecyclableBgmapTextureManager_registerTexture(RecyclableBgmapTextureManager_getInstance(), this->RecyclableImageDefinition->spritesDefinitions[i]->textureDefinition);
			}
			else if(__TYPE(MBgmapSprite) == __ALLOCATOR_TYPE(this->RecyclableImageDefinition->spritesDefinitions[i]->allocator))
			{
				int j = 0;

				for(; ((MBgmapSpriteDefinition*)this->RecyclableImageDefinition->spritesDefinitions[i])->textureDefinitions[j]; j++)
				{
					RecyclableBgmapTextureManager_registerTexture(RecyclableBgmapTextureManager_getInstance(), ((MBgmapSpriteDefinition*)this->RecyclableImageDefinition->spritesDefinitions[i])->textureDefinitions[j]);
				}
			}
		}
	}
}

static void RecyclableImage_releaseTextures(RecyclableImage this)
{
	// speed up my destruction by deleting my sprites
	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

		for(; node; node = node->next)
		{
			Texture texture = Sprite_getTexture(__SAFE_CAST(Sprite, node->data));
			RecyclableBgmapTextureManager_removeTexture(RecyclableBgmapTextureManager_getInstance(), texture);
		}
	}
}
