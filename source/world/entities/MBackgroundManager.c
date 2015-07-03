/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MBackgroundManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define MBackgroundManager_ATTRIBUTES											\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* textures */																\
	VirtualList textures;														\

__CLASS_DEFINITION(MBackgroundManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class constructor 
static void MBackgroundManager_constructor(MBackgroundManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(MBackgroundManager);

//class constructor
static void MBackgroundManager_constructor(MBackgroundManager this)
{
	__CONSTRUCT_BASE();

	this->textures = __NEW(VirtualList);
}

// class destructor
void MBackgroundManager_destructor(MBackgroundManager this)
{
	ASSERT(this, "MBackgroundManager::destructor: null this");

	if(this->textures) 
	{
		__DELETE(this->textures);
		
		this->textures = NULL;
	}

	// allow a new construct
	__SINGLETON_DESTROY;
}

// register coin
void MBackgroundManager_registerTexture(MBackgroundManager this, TextureDefinition* textureDefinition)
{
	ASSERT(this, "MBackgroundManager::registerTexture: null this");
	ASSERT(textureDefinition, "MBackgroundManager::registerTexture: null texture definition");
	
	VirtualNode node = VirtualList_begin(this->textures);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		if(!Texture_getDefinition(__GET_CAST(Texture, VirtualNode_getData(node))))
		{
			break;
		}
	}
	
	if(!node)
	{
		// texture not found, must load it
		BgmapTexture bgmapTexture = BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), textureDefinition);
		VirtualList_pushBack(this->textures, bgmapTexture);
	}
	else 
	{
		// free texture found, so replace it
		Texture texture = __GET_CAST(Texture, VirtualNode_getData(node));
		Texture_setDefinition(texture, textureDefinition);
		Texture_rewrite(texture);
	}
}

// remove background
void MBackgroundManager_removeTexture(MBackgroundManager this, Texture texture)
{
	ASSERT(this, "MBackgroundManager::removeTexture: null this");
	
	ASSERT(VirtualList_find(this->textures, texture), "MBackgroundManager::removeTexture: texture not found");
	
	if(texture && VirtualList_find(this->textures, texture))
	{
		Texture_setDefinition(texture, NULL);
		Texture_releaseCharSet(texture);
	}
}

// remove texture
void MBackgroundManager_reset(MBackgroundManager this)
{
	ASSERT(this, "MBackgroundManager::reset: null this");

	VirtualList_clear(this->textures);
}