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
	/* registerd background */													\
	VirtualList mBackgrounds;													\
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

	this->mBackgrounds = __NEW(VirtualList);
	this->textures = __NEW(VirtualList);
}

// class destructor
void MBackgroundManager_destructor(MBackgroundManager this)
{
	ASSERT(this, "MBackgroundManager::destructor: null this");

	if(this->mBackgrounds) 
	{
		__DELETE(this->mBackgrounds);
		
		this->mBackgrounds = NULL;
	}

	if(this->textures) 
	{
		__DELETE(this->textures);
		
		this->textures = NULL;
	}

	// allow a new construct
	__SINGLETON_DESTROY;
}

// register coin
void MBackgroundManager_registerMBackground(MBackgroundManager this, MBackground mBackground, TextureDefinition* textureDefinition)
{
	ASSERT(this, "MBackgroundManager::registerMBackground: null this");
	ASSERT(textureDefinition, "MBackgroundManager::registerMBackground: null texture definition");
	
	if(!VirtualList_find(this->mBackgrounds, mBackground))
	{
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
			Texture_releaseCharSet(texture);
			Texture_rewrite(texture);
		}

		VirtualList_pushBack(this->mBackgrounds, mBackground);
	}
}

// remove background
void MBackgroundManager_removeMBackground(MBackgroundManager this, MBackground mBackground)
{
	ASSERT(this, "MBackgroundManager::removeMBackground: null this");
	ASSERT(VirtualList_find(this->mBackgrounds, mBackground), "MBackgroundManager::removeMBackground: mBackground not found");
	
	Texture texture = MBackground_getTexture(mBackground);
	ASSERT(VirtualList_find(this->textures, texture), "MBackgroundManager::removeMBackground: texture not found");
	
	if(texture)
	{
		Texture_setDefinition(texture, NULL);
	}
	
	VirtualList_removeElement(this->mBackgrounds, mBackground);
}

// remove texture
void MBackgroundManager_reset(MBackgroundManager this)
{
	ASSERT(this, "MBackgroundManager::reset: null this");

	VirtualList_clear(this->textures);
}