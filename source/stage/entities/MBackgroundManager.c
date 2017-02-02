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

#include <MBackgroundManager.h>
#include <BgmapTextureManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define MBackgroundManager_ATTRIBUTES																	\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* textureRegistries */																			\
		VirtualList textureRegistries;																	\

__CLASS_DEFINITION(MBackgroundManager, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


typedef struct TextureRegistry
{
	Texture texture;
	u8 cols;
	u8 rows;
	u8 free;
} TextureRegistry;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void MBackgroundManager_constructor(MBackgroundManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
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

		if(textureRegistry->free)
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
		selectedTextureRegistry->free = false;
		Texture_setDefinition(selectedTextureRegistry->texture, textureDefinition);
		Texture_setPalette(selectedTextureRegistry->texture, textureDefinition->palette);
		__VIRTUAL_CALL(Texture, rewrite, selectedTextureRegistry->texture);
	}
	else
	{
		// texture not found, must load it
		selectedTextureRegistry = __NEW_BASIC(TextureRegistry);
		selectedTextureRegistry->free = false;
		selectedTextureRegistry->texture = __SAFE_CAST(Texture, BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), textureDefinition));
		selectedTextureRegistry->cols = textureDefinition->cols;
		selectedTextureRegistry->rows = textureDefinition->rows;
		VirtualList_pushBack(this->textureRegistries, selectedTextureRegistry);
	}

	ASSERT(selectedTextureRegistry, "MBackgroundManager::registerTexture: null selectedTextureRegistry");
	ASSERT(selectedTextureRegistry->texture, "MBackgroundManager::registerTexture: null selectedTextureRegistry");

	return selectedTextureRegistry? selectedTextureRegistry->texture : NULL;
}

// remove background
void MBackgroundManager_removeTexture(MBackgroundManager this __attribute__ ((unused)), Texture texture)
{
	ASSERT(this, "MBackgroundManager::removeTexture: null this");
	ASSERT(texture, "MBackgroundManager::removeTexture: null texture");

	VirtualNode node = this->textureRegistries->head;

	for(; node; node = node->next)
	{
		TextureRegistry* textureRegistry = (TextureRegistry*)node->data;

		if(texture == textureRegistry->texture)
		{
			textureRegistry->free = true;
			break;
		}
	}

	ASSERT(node, "MBackgroundManager::removeTexture: texture not found");
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
