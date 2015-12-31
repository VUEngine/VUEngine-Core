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

#include <MBackground.h>
#include <CollisionManager.h>
#include <Optics.h>
#include <Shape.h>
#include <Prototypes.h>
#include <Game.h>
#include <MBackgroundManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the MBackground
__CLASS_DEFINITION(MBackground, Entity);

__CLASS_FRIEND_DEFINITION(VirtualNode);


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
	ASSERT(mBackgroundDefinition->spritesDefinitions[0]->textureDefinition, "MBackground::constructor: null texture definition");
	
	// construct base object
	__CONSTRUCT_BASE((EntityDefinition*)mBackgroundDefinition, id, name);

	this->mBackgroundDefinition = mBackgroundDefinition;
}

// class's destructor
void MBackground_destructor(MBackground this)
{
	ASSERT(this, "MBackground::destructor: null this");

	VirtualNode node = VirtualList_begin(this->sprites);
	
	for(; node; node = node->next)
	{
		MBackgroundManager_removeTexture(MBackgroundManager_getInstance(), Sprite_getTexture(__SAFE_CAST(Sprite, node->data)));
	}

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// initialize method
void MBackground_initialize(MBackground this)
{
	ASSERT(this->mBackgroundDefinition->spritesDefinitions[0], "MBackground::initialize: null sprite list");

	// first register with the manager so it handles the texture loading process
	if(this->mBackgroundDefinition->spritesDefinitions[0])
	{
		int i = 0;
		
		for(; this->mBackgroundDefinition->spritesDefinitions[i]; i++)
		{	
			MBackgroundManager_registerTexture(MBackgroundManager_getInstance(), this->mBackgroundDefinition->spritesDefinitions[i]->textureDefinition);
		}
	}
	
	Entity_initialize(__SAFE_CAST(Entity, this));
}


void MBackground_suspend(MBackground this)
{
	ASSERT(this, "MBackground::suspend: null this");

	VirtualNode node = VirtualList_begin(this->sprites);
	
	for(; node; node = node->next)
	{
		MBackgroundManager_removeTexture(MBackgroundManager_getInstance(), Sprite_getTexture(__SAFE_CAST(Sprite, node->data)));
	}

	Entity_suspend(__SAFE_CAST(Entity, this));
}

void MBackground_resume(MBackground this)
{
	ASSERT(this, "MBackground::resume: null this");

	// first register with the manager so it handles the texture loading process
	if(this->mBackgroundDefinition->spritesDefinitions[0])
	{
		int i = 0;
		
		for(; this->mBackgroundDefinition->spritesDefinitions[i]; i++)
		{	
			MBackgroundManager_registerTexture(MBackgroundManager_getInstance(), this->mBackgroundDefinition->spritesDefinitions[i]->textureDefinition);
		}
	}

	Entity_resume(__SAFE_CAST(Entity, this));
}

int MBackground_isVisible(MBackground this, int pad)
{
	ASSERT(this, "MBackground::isVisible: null this");

	// TODO: add support for MBgmapSprites
	return Entity_isVisible(__SAFE_CAST(Entity, this), pad);
}
