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

#include <MBackgroundManager.h>
#include <BgmapTextureManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define MBackgroundManager_ATTRIBUTES																	\
																										\
	/* super's attributes */																			\
	Object_ATTRIBUTES;																					\
																										\
	/* textureRegistries */																				\
	VirtualList textureRegistries;																		\

__CLASS_DEFINITION(MBackgroundManager, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


typedef struct TextureRegistry
{
	Texture texture;
	u8 cols;
	u8 rows;
} TextureRegistry;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void MBackgroundManager_constructor(MBackgroundManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(MBackgroundManager);

//class constructor
static void MBackgroundManager_constructor(MBackgroundManager this)
{
	__CONSTRUCT_BASE(Object);

	this->textureRegistries = __NEW(VirtualList);
}

// class destructor
void MBackgroundManager_destructor(MBackgroundManager this)
{
	ASSERT(this, "MBackgroundManager::destructor: null this");
	ASSERT(this->textureRegistries, "MBackgroundManager::destructor: null textureRegistries");

	VirtualNode node = this->textureRegistries->head;

	for(; node; node = node->next)
	{
		__DELETE_BASIC(node->next);
	}

	__DELETE(this->textureRegistries);

	this->textureRegistries = NULL;

	// allow a new construct
	__SINGLETON_DESTROY;
}

// register coin
Texture MBackgroundManager_registerTexture(MBackgroundManager this, TextureDefinition* textureDefinition)
{
	ASSERT(this, "MBackgroundManager::registerTexture: null this");
	ASSERT(textureDefinition, "MBackgroundManager::registerTexture: null texture definition");

	VirtualNode node = this->textureRegistries->head;

	TextureRegistry* selectedTextureRegistry = NULL;

	for(; node; node = node->next)
	{
		TextureRegistry* textureRegistry = (TextureRegistry*)node->data;

		if(!Texture_getCharSet(textureRegistry->texture))
		{
			if(textureDefinition->cols <= textureRegistry->cols &&
				textureDefinition->rows <= textureRegistry->rows
			)
			{
				if(!selectedTextureRegistry)
				{
					selectedTextureRegistry = textureRegistry;
				}
				else if(textureDefinition->cols == textureRegistry->cols && textureDefinition->rows == textureRegistry->rows)
				{
					selectedTextureRegistry = textureRegistry;
					break;
				}
				else if(textureRegistry->cols <= selectedTextureRegistry->cols && textureRegistry->rows <= selectedTextureRegistry->rows)
				{
					selectedTextureRegistry = textureRegistry;
				}
			}
		}
		else if(Texture_getTextureDefinition(textureRegistry->texture) == textureDefinition)
		{
			return textureRegistry->texture;
		}
	}

	if(selectedTextureRegistry)
	{
		// free texture found, so replace it
		Texture_setDefinition(selectedTextureRegistry->texture, textureDefinition);
		Texture_setPalette(selectedTextureRegistry->texture, textureDefinition->palette);
		Texture_rewrite(selectedTextureRegistry->texture);
	}
	else
	{
		// texture not found, must load it
		selectedTextureRegistry = __NEW_BASIC(TextureRegistry);
		selectedTextureRegistry->texture = __SAFE_CAST(Texture, BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), textureDefinition));
		selectedTextureRegistry->cols = textureDefinition->cols;
		selectedTextureRegistry->rows = textureDefinition->rows;
		VirtualList_pushBack(this->textureRegistries, selectedTextureRegistry);
	}

	return selectedTextureRegistry? selectedTextureRegistry->texture : NULL;
}

// remove background
void MBackgroundManager_removeTexture(MBackgroundManager this, Texture texture)
{
	ASSERT(this, "MBackgroundManager::removeTexture: null this");

#ifdef __DEBUG
	VirtualNode node = this->textureRegistries->head;

	for(; node; node = node->next)
	{
		TextureRegistry* textureRegistry = (TextureRegistry*)node->data;

		if(texture == textureRegistry->texture)
		{
			break;
		}
	}

	ASSERT(node, "MBackgroundManager::removeTexture: texture not found");
#endif

	if(texture)
	{
		Texture_releaseCharSet(texture);
	}
}

// remove texture
void MBackgroundManager_reset(MBackgroundManager this)
{
	ASSERT(this, "MBackgroundManager::reset: null this");

	VirtualNode node = this->textureRegistries->head;

	for(; node; node = node->next)
	{
		TextureRegistry* textureRegistry = (TextureRegistry*)node->data;

		// textures could be deleted externally
		if(*(u32*)textureRegistry->texture)
		{
			ASSERT(textureRegistry, "MBackgroundManager::reset: null textureRegistry");
			ASSERT(textureRegistry->texture, "MBackgroundManager::reset: null texture");
			ASSERT(__SAFE_CAST(BgmapTexture, textureRegistry->texture), "MBackgroundManager::reset: no BgmapTexture");

			BgmapTextureManager_releaseTexture(BgmapTextureManager_getInstance(), __SAFE_CAST(BgmapTexture, textureRegistry->texture));
		}

		__DELETE_BASIC(textureRegistry);
	}

	VirtualList_clear(this->textureRegistries);
}
