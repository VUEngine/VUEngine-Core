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

#include <RecyclableBgmapTextureManager.h>
#include <BgmapTextureManager.h>
#include <Printing.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define RecyclableBgmapTextureManager_ATTRIBUTES																	\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* textureRegistries */																			\
		VirtualList textureRegistries;																	\

/**
 * @class 	RecyclableBgmapTextureManager
 * @extends Object
 * @ingroup graphics-2d-textures-bgmap
 */
__CLASS_DEFINITION(RecyclableBgmapTextureManager, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);

/**
 * Texture Registry
 *
 * @memberof RecyclableBgmapTextureManager
 */
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

static void RecyclableBgmapTextureManager_constructor(RecyclableBgmapTextureManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			RecyclableBgmapTextureManager_getInstance()
 * @memberof	RecyclableBgmapTextureManager
 * @public
 *
 * @return		RecyclableBgmapTextureManager instance
 */
__SINGLETON(RecyclableBgmapTextureManager);

/**
 * Class constructor
 *
 * @memberof			RecyclableBgmapTextureManager
 * @private
 *
 * @param this			Function scope
 */
static void RecyclableBgmapTextureManager_constructor(RecyclableBgmapTextureManager this)
{
	__CONSTRUCT_BASE(Object);

	this->textureRegistries = __NEW(VirtualList);
}

/**
 * Class destructor
 *
 * @memberof			RecyclableBgmapTextureManager
 * @public
 *
 * @param this			Function scope
 */
void RecyclableBgmapTextureManager_destructor(RecyclableBgmapTextureManager this)
{
	ASSERT(this, "RecyclableBgmapTextureManager::destructor: null this");
	ASSERT(this->textureRegistries, "RecyclableBgmapTextureManager::destructor: null textureRegistries");

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

/**
 * Register a Texture to be recycled
 *
 * @memberof					RecyclableBgmapTextureManager
 * @public
 *
 * @param this					Function scope
 * @param textureDefinition		The definition of the Texture
 *
 * @return						The recyclable Texture
 */
Texture RecyclableBgmapTextureManager_registerTexture(RecyclableBgmapTextureManager this, TextureDefinition* textureDefinition)
{
	ASSERT(this, "RecyclableBgmapTextureManager::registerTexture: null this");
	ASSERT(textureDefinition, "RecyclableBgmapTextureManager::registerTexture: null texture definition");

	VirtualNode node = this->textureRegistries->head;

	TextureRegistry* selectedTextureRegistry = NULL;

	for(; node; node = node->next)
	{
		TextureRegistry* textureRegistry = (TextureRegistry*)node->data;

		if(Texture_getTextureDefinition(textureRegistry->texture) == textureDefinition)
		{
			BgmapTexture_increaseUsageCount(__SAFE_CAST(BgmapTexture, textureRegistry->texture));
			selectedTextureRegistry = textureRegistry;
			break;
		}
	}

	if(!selectedTextureRegistry)
	{
		for(node = this->textureRegistries->head; node; node = node->next)
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
		}
	}

	if(selectedTextureRegistry)
	{
		// free texture found, so replace it
		selectedTextureRegistry->free = false;

		if(0 >= BgmapTexture_getUsageCount(__SAFE_CAST(BgmapTexture, selectedTextureRegistry->texture)))
		{
			Texture_setDefinition(selectedTextureRegistry->texture, textureDefinition);
			Texture_setPalette(selectedTextureRegistry->texture, textureDefinition->palette);
			__VIRTUAL_CALL(Texture, rewrite, selectedTextureRegistry->texture);
		}
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

	ASSERT(selectedTextureRegistry, "RecyclableBgmapTextureManager::registerTexture: null selectedTextureRegistry");
	ASSERT(selectedTextureRegistry->texture, "RecyclableBgmapTextureManager::registerTexture: null selectedTextureRegistry");

	return selectedTextureRegistry? selectedTextureRegistry->texture : NULL;
}

/**
 * Unregister a recyclable Texture
 *
 * @memberof					RecyclableBgmapTextureManager
 * @public
 *
 * @param this					Function scope
 * @param texture				The recyclable Texture to unregister
 */
void RecyclableBgmapTextureManager_removeTexture(RecyclableBgmapTextureManager this, Texture texture)
{
	ASSERT(this, "RecyclableBgmapTextureManager::removeTexture: null this");
	ASSERT(texture, "RecyclableBgmapTextureManager::removeTexture: null texture");

	BgmapTextureManager_releaseTexture(BgmapTextureManager_getInstance(), texture);

	VirtualNode node = this->textureRegistries->head;

	Printing_text(Printing_getInstance(), "   ", 1, 25, NULL);
	Printing_int(Printing_getInstance(), BgmapTexture_getUsageCount(__SAFE_CAST(BgmapTexture, texture)), 1, 25, NULL);

	for(; node; node = node->next)
	{
		TextureRegistry* textureRegistry = (TextureRegistry*)node->data;

		if(texture == textureRegistry->texture && 0 >= BgmapTexture_getUsageCount(__SAFE_CAST(BgmapTexture, textureRegistry->texture)))
		{
			textureRegistry->free = true;
			break;
		}
	}

	ASSERT(node, "RecyclableBgmapTextureManager::removeTexture: texture not found");
}

/**
 * Reset manager's state
 *
 * @memberof					RecyclableBgmapTextureManager
 * @public
 *
 * @param this					Function scope
 */
void RecyclableBgmapTextureManager_reset(RecyclableBgmapTextureManager this)
{
	ASSERT(this, "RecyclableBgmapTextureManager::reset: null this");

	VirtualNode node = this->textureRegistries->head;

	for(; node; node = node->next)
	{
		TextureRegistry* textureRegistry = (TextureRegistry*)node->data;

		// textures could be deleted externally
		if(__IS_OBJECT_ALIVE(textureRegistry->texture))
		{
			ASSERT(textureRegistry, "RecyclableBgmapTextureManager::reset: null textureRegistry");
			ASSERT(textureRegistry->texture, "RecyclableBgmapTextureManager::reset: null texture");
			ASSERT(__SAFE_CAST(BgmapTexture, textureRegistry->texture), "RecyclableBgmapTextureManager::reset: no BgmapTexture");

			BgmapTextureManager_releaseTexture(BgmapTextureManager_getInstance(), __SAFE_CAST(BgmapTexture, textureRegistry->texture));
		}

		__DELETE_BASIC(textureRegistry);
	}

	VirtualList_clear(this->textureRegistries);
}

/**
 * Print manager's state
 *
 * @memberof					RecyclableBgmapTextureManager
 * @public
 *
 * @param this					Function scope
 * @param x						Screen's x coordinate
 * @param y						Screen's y coordinate
 */
void RecyclableBgmapTextureManager_print(RecyclableBgmapTextureManager this, int x, int y)
{
	Printing_text(Printing_getInstance(), "RecyclableBgmapTextureManager's status", x, y++, NULL);
	y++;
	Printing_text(Printing_getInstance(), "Texture entries: ", x, y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->textureRegistries), x + 17, y++, NULL);
	Printing_text(Printing_getInstance(), "Free entries: ", x, y++, NULL);
	y++;

	Printing_text(Printing_getInstance(), "#  Tex.Def. Free", x, y++, NULL);

	int i = 0;
	int j = 0;
	int counter = 0;
	int freeEntries = 0;

	VirtualNode node = this->textureRegistries->head;

	for(; node; node = node->next)
	{
		TextureRegistry* textureRegistry = (TextureRegistry*)node->data;

		freeEntries += textureRegistry->free? 1 : 0;

		Printing_int(Printing_getInstance(), ++counter, x, y + i, NULL);
		Printing_hex(Printing_getInstance(), (int)Texture_getTextureDefinition(textureRegistry->texture), x + j + 3, y + i, 8, NULL);
		Printing_text(Printing_getInstance(), textureRegistry->free? __CHAR_CHECKBOX_CHECKED : __CHAR_CHECKBOX_UNCHECKED, x + j + 12, y + i, NULL);

		if(++i + y > __SCREEN_HEIGHT / 8)
		{
			i = 0;
			j += 14;

			if(j + x > __SCREEN_WIDTH / 8)
			{
				i = 0;
				j = 0;
			}
		}
	}

	Printing_int(Printing_getInstance(), freeEntries, x + 17, y - 3, NULL);
}

