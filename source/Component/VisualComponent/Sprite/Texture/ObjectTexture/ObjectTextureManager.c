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

#include <ObjectTexture.h>
#include <Singleton.h>
#include <VirtualList.h>

#include "ObjectTextureManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class ObjectTexture;
friend class Texture;
friend class VirtualList;
friend class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void ObjectTextureManager::reset()
{
	VirtualList::deleteData(this->objectTextures);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void ObjectTextureManager::updateTextures(int16 maximumTextureRowsToWrite, bool defer)
{
	for(VirtualNode node = this->objectTextures->head; NULL != node; node = node->next)
	{
		Texture texture = Texture::safeCast(node->data);

		if(!texture->update)
		{
			continue;
		}

		texture->update = Texture::update(texture, maximumTextureRowsToWrite);

		if(!texture->update && defer)
		{
			break;
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure ObjectTexture ObjectTextureManager::getTexture(ObjectTextureSpec* objectTextureSpec)
{
	NM_ASSERT(NULL != objectTextureSpec, "ObjectTextureManager::getTexture: NULL objectTextureSpec");

	if(NULL == objectTextureSpec)
	{
		return NULL;
	}

	static uint16 textureNextId = 0;

	ObjectTexture objectTexture = new ObjectTexture(objectTextureSpec, textureNextId++);

	VirtualList::pushBack(this->objectTextures, objectTexture);

	ObjectTexture::prepare(objectTexture);

	return objectTexture;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void ObjectTextureManager::releaseTexture(ObjectTexture objectTexture)
{
	NM_ASSERT(!isDeleted(objectTexture), "ObjectTextureManager::releaseTexture: trying to release an invalid objectTexture");

	VirtualList::removeData(this->objectTextures, objectTexture);

	delete objectTexture;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectTextureManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();
	
	this->objectTextures = new VirtualList();

	ObjectTextureManager::reset(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectTextureManager::destructor()
{
	if(!isDeleted(this->objectTextures))
	{
		VirtualList::deleteData(this->objectTextures);
		delete this->objectTextures;
		this->objectTextures = NULL;
	}


	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
