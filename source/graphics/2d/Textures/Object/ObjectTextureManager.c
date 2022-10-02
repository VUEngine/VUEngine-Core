/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectTextureManager.h>
#include <HardwareManager.h>
#include <ParamTableManager.h>
#include <VIPManager.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S DECLARATIONS
//---------------------------------------------------------------------------------------------------------

friend class Texture;
friend class VirtualList;
friend class VirtualNode;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			ObjectTextureManager::getInstance()
 * @public
 * @return		ObjectTextureManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void ObjectTextureManager::constructor()
{
	Base::constructor();

	this->objectTextures = new VirtualList();
}

/**
 * Class destructor
 */
void ObjectTextureManager::destructor()
{
	ObjectTextureManager::reset(this);

	delete this->objectTextures;
	this->objectTextures = NULL;

	// allow a new construct
	Base::destructor();
}

/**
 * Reset manager's state
 */
void ObjectTextureManager::reset()
{
	VirtualList::deleteData(this->objectTextures);
}

/**
 * Retrieve a Texture
 *
 * @private
 * @param objectTextureSpec		Texture spec to find o allocate a Texture
 * @param owber					Sprite owner
 * @return 								Allocated Texture
 */
ObjectTexture ObjectTextureManager::getTexture(ObjectTextureSpec* objectTextureSpec)
{
	NM_ASSERT(NULL != objectTextureSpec, "ObjectTextureManager::getTexture: NULL objectTextureSpec");

	if(NULL == objectTextureSpec)
	{
		return NULL;
	}

	static uint16 textureNextId = 0;

	ObjectTexture objectTexture = new ObjectTexture(objectTextureSpec, textureNextId++);

	VirtualList::pushBack(this->objectTextures, objectTexture);

	return objectTexture;
}

/**
 * Release a previously allocated Texture
 *
 * @param objectTexture		Texture to release
 */
void ObjectTextureManager::releaseTexture(ObjectTexture objectTexture)
{
	NM_ASSERT(!isDeleted(objectTexture), "ObjectTextureManager::releaseTexture: trying to release an invalid objectTexture");

	VirtualList::removeElement(this->objectTextures, objectTexture);

	delete objectTexture;
}

/**
 * Update textures
 *
 * @public
 */
void ObjectTextureManager::updateTextures()
{
	for(VirtualNode node = this->objectTextures->head; NULL != node; node = node->next)
	{
		Texture texture = Texture::safeCast(node->data);

		if(texture->update)
		{
			texture->update = !Texture::update(texture);
		}
	}
}
