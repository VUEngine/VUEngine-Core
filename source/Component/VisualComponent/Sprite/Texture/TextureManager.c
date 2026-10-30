/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Singleton.h>
#include <Texture.h>
#include <TileSet.h>
#include <VirtualList.h>
#include <VirtualNode.h>

#include <TextureManager.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Texture;
friend class TileSet;
friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static VirtualList _textureManagers = NULL;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TextureManager::registerManager(TextureManager textureManager)
{
	if(isDeleted(textureManager))
	{
		return;
	}

	if(NULL == _textureManagers)
	{
		_textureManagers = new VirtualList();
	}
	
	if(!VirtualList::find(_textureManagers, textureManager))
	{
		VirtualList::pushBack(_textureManagers, textureManager);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TextureManager::resetManager()
{
	if(isDeleted(_textureManagers))
	{
		return;
	}

	for(VirtualNode node = _textureManagers->head; NULL != node; node = node->next)
	{
		TextureManager textureManager = TextureManager::safeCast(node->data);

		if(isDeleted(textureManager))
		{
			continue;
		}
		
		TextureManager::reset(textureManager);
	} 
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Texture TextureManager::getTexture(ClassPointer textureClass, const TextureSpec* textureSpec)
{
	if(NULL == textureSpec)
	{
		return NULL;
	}

	if(isDeleted(_textureManagers))
	{
		return NULL;
	}

	for(VirtualNode node = _textureManagers->head; NULL != node; node = node->next)
	{
		TextureManager textureManager = TextureManager::safeCast(node->data);

		if(isDeleted(textureManager))
		{
			continue;
		}
		
		if(TextureManager::getTextureClass(textureManager) == textureClass)
		{		
			Texture texture = TextureManager::get(textureManager, textureSpec);

			if(!isDeleted(texture))
			{
				return texture;
			}
		}
	} 

	return NULL;	
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TextureManager::releaseTexture(Texture texture)
{
	if(isDeleted(texture))
	{
		return;
	}
	
	TextureManager textureManager = Texture::getTextureManager(texture);

	if(isDeleted(textureManager))
	{
		return;
	}

	TextureManager::release(textureManager, texture);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TextureManager::updateTextures(int16 maximumTextureRowsToWrite, bool defer)
{
	if(isDeleted(_textureManagers))
	{
		return;
	}

	for(VirtualNode node = _textureManagers->head; NULL != node; node = node->next)
	{
		TextureManager textureManager = TextureManager::safeCast(node->data);

		if(isDeleted(textureManager))
		{
			continue;
		}
		
		for(VirtualNode textureNode = textureManager->textures->head; NULL != textureNode; textureNode = textureNode->next)
		{
			Texture texture = Texture::safeCast(textureNode->data);
			
			if(kTextureInvalid == texture->status)
			{
				continue;
			}

			if(NULL != texture->tileSet && kTexturePendingWriting < texture->status)
			{
				texture->status = texture->generation != texture->tileSet->generation? kTexturePendingRewriting : texture->status;
			}

			if(kTextureWritten == texture->status)
			{
				continue;
			}

			if(kTextureWritten != Texture::update(texture, maximumTextureRowsToWrite) && defer)
			{
				break;
			}
		}
	} 
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TextureManager::loadTextures(const TextureSpec** textureSpecs)
{
	if(isDeleted(_textureManagers))
	{
		return;
	}

	for(VirtualNode node = _textureManagers->head; NULL != node; node = node->next)
	{
		TextureManager textureManager = TextureManager::safeCast(node->data);

		if(isDeleted(textureManager))
		{
			continue;
		}
		
		TextureManager::load(textureManager, textureSpecs);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void TextureManager::clearGraphicMemory()
{
	if(isDeleted(_textureManagers))
	{
		return;
	}

	for(VirtualNode node = _textureManagers->head; NULL != node; node = node->next)
	{
		TextureManager textureManager = TextureManager::safeCast(node->data);

		if(isDeleted(textureManager))
		{
			continue;
		}
		
		TextureManager::clear(textureManager);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TextureManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->textures = new VirtualList();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void TextureManager::destructor()
{
	if(!isDeleted(this->textures))
	{
		VirtualList::deleteData(this->textures);
		delete this->textures;
		this->textures = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
