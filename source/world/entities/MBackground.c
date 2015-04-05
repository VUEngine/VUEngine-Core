/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
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


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(MBackground, MBackgroundDefinition* mBackgroundDefinition, s16 id)
__CLASS_NEW_END(MBackground, mBackgroundDefinition, id);

// class's constructor
void MBackground_constructor(MBackground this, MBackgroundDefinition* mBackgroundDefinition, s16 id)
{
	ASSERT(this, "MBackground::constructor: null this");
	ASSERT(mBackgroundDefinition, "MBackground::constructor: null definition");
	ASSERT(mBackgroundDefinition->spritesDefinitions[0], "MBackground::constructor: null sprite definition");
	ASSERT(mBackgroundDefinition->spritesDefinitions[0]->textureDefinition, "MBackground::constructor: null texture definition");
	
	// construct base object
	__CONSTRUCT_BASE((EntityDefinition*)mBackgroundDefinition, id);

	this->mBackgroundDefinition = mBackgroundDefinition;
	
	this->size.x = __SCREEN_WIDTH;
	this->size.y = __SCREEN_HEIGHT;
	this->size.z = 1;
}

// class's destructor
void MBackground_destructor(MBackground this)
{
	ASSERT(this, "MBackground::destructor: null this");

	MBackgroundManager_removeMBackground(MBackgroundManager_getInstance(), this);

	// destroy the super object
	__DESTROY_BASE;
}

// initialize method
void MBackground_initialize(MBackground this)
{
	// first register with the manager so it handles the texture loading process
	MBackgroundManager_registerMBackground(MBackgroundManager_getInstance(), this, this->mBackgroundDefinition->spritesDefinitions[0]->textureDefinition);

	Entity_initialize(__UPCAST(Entity, this));

	ASSERT(this->sprites, "MBackground::constructor: null sprite list");
}

// retrieve texture
Texture MBackground_getTexture(MBackground this)
{
	ASSERT(this, "MBackground::getTexture: null this");

	return !this->sprites? NULL: Sprite_getTexture(__UPCAST(Sprite, VirtualList_front(this->sprites)));
}

int MBackground_isVisible(MBackground this, int pad)
{
	ASSERT(this, "MBackground::isVisible: null this");

	// TODO: add support for MSprites
	return Entity_isVisible(__UPCAST(Entity, this), pad);
}
