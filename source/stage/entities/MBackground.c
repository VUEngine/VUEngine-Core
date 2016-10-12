/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
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
__CLASS_NEW_DEFINITION(MBackground, MBackgroundDefinition* mBackgroundDefinition, s16 id, const char* const name)
__CLASS_NEW_END(MBackground, mBackgroundDefinition, id, name);

// class's constructor
void MBackground_constructor(MBackground this, MBackgroundDefinition* mBackgroundDefinition, s16 id, const char* const name)
{
	ASSERT(this, "MBackground::constructor: null this");
	ASSERT(mBackgroundDefinition, "MBackground::constructor: null definition");
	ASSERT(mBackgroundDefinition->spritesDefinitions[0], "MBackground::constructor: null sprite definition");

	// construct base object
	__CONSTRUCT_BASE(Entity, (EntityDefinition*)mBackgroundDefinition, id, name);

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
