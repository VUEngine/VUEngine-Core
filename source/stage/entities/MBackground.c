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

#include <Object.h>
#include <MBackground.h>
#include <Optics.h>
#include <Shape.h>
#include <Prototypes.h>
#include <Game.h>
#include <MBackgroundManager.h>
#include <BgmapSprite.h>
#include <MBgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the MBackground
__CLASS_DEFINITION(MBackground, Entity);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void MBackground_registerTextures(MBackground this);
static void MBackground_releaseTextures(MBackground this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(MBackground, MBackgroundDefinition* mBackgroundDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(MBackground, mBackgroundDefinition, id, internalId, name);

// class's constructor
void MBackground_constructor(MBackground this, MBackgroundDefinition* mBackgroundDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "MBackground::constructor: null this");
	ASSERT(mBackgroundDefinition, "MBackground::constructor: null definition");
	ASSERT(mBackgroundDefinition->spritesDefinitions[0], "MBackground::constructor: null sprite definition");

	// construct base object
	__CONSTRUCT_BASE(Entity, (EntityDefinition*)mBackgroundDefinition, id, internalId, name);

	this->mBackgroundDefinition = mBackgroundDefinition;

	MBackground_registerTextures(this);
}

// class's destructor
void MBackground_destructor(MBackground this)
{
	ASSERT(this, "MBackground::destructor: null this");

    MBackground_releaseTextures(this);

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// set definition
void MBackground_setDefinition(MBackground this, MBackgroundDefinition* mBackgroundDefinition)
{
	ASSERT(this, "MBackground::setDefinition: null this");
	ASSERT(mBackgroundDefinition, "MBackground::setDefinition: null definition");

	// save definition
	this->mBackgroundDefinition = mBackgroundDefinition;

	Entity_setDefinition(__SAFE_CAST(Entity, this), (EntityDefinition*)mBackgroundDefinition);
}

void MBackground_suspend(MBackground this)
{
	ASSERT(this, "MBackground::suspend: null this");

    MBackground_releaseTextures(this);

	Entity_suspend(__SAFE_CAST(Entity, this));
}

void MBackground_resume(MBackground this)
{
	ASSERT(this, "MBackground::resume: null this");

	// first register with the manager so it handles the texture loading process
	MBackground_registerTextures(this);

	Entity_resume(__SAFE_CAST(Entity, this));
}

static void MBackground_registerTextures(MBackground this)
{
	ASSERT(this, "MBackground::registerTextures: null this");

	if(this->mBackgroundDefinition->spritesDefinitions[0])
	{
		int i = 0;

		for(; this->mBackgroundDefinition->spritesDefinitions[i]; i++)
		{
			if(__TYPE(BgmapSprite) == __ALLOCATOR_TYPE(this->mBackgroundDefinition->spritesDefinitions[i]->allocator))
			{
			    MBackgroundManager_registerTexture(MBackgroundManager_getInstance(), this->mBackgroundDefinition->spritesDefinitions[i]->textureDefinition);
			}
			else if(__TYPE(MBgmapSprite) == __ALLOCATOR_TYPE(this->mBackgroundDefinition->spritesDefinitions[i]->allocator))
			{
				int j = 0;

				for(; ((MBgmapSpriteDefinition*)this->mBackgroundDefinition->spritesDefinitions[i])->textureDefinitions[j]; j++)
				{
					MBackgroundManager_registerTexture(MBackgroundManager_getInstance(), ((MBgmapSpriteDefinition*)this->mBackgroundDefinition->spritesDefinitions[i])->textureDefinitions[j]);
				}
			}
		}
	}
}

static void MBackground_releaseTextures(MBackground this)
{
    // speed up my destruction by deleting my sprites
	if(this->sprites)
	{
		VirtualNode node = this->sprites->head;

        for(; node; node = node->next)
        {
            Texture texture = Sprite_getTexture(__SAFE_CAST(Sprite, node->data));
            __DELETE(node->data);
            MBackgroundManager_removeTexture(MBackgroundManager_getInstance(), texture);
        }

		// delete the sprites
		__DELETE(this->sprites);

		this->sprites = NULL;
	}
}
