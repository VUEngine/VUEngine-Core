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
#include <VirtualList.h>

#include "ObjectTextureManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class ObjectTexture;
friend class Texture;
friend class VirtualList;
friend class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ObjectTextureManager::reset()
{}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static ObjectTexture ObjectTextureManager::getTexture(ObjectTextureSpec* objectTextureSpec)
{
	NM_ASSERT(NULL != objectTextureSpec, "ObjectTextureManager::getTexture: NULL objectTextureSpec");

	if(NULL == objectTextureSpec)
	{
		return NULL;
	}

	static uint16 textureNextId = 0;

	ObjectTexture objectTexture = new ObjectTexture(objectTextureSpec, textureNextId++);

	ObjectTexture::prepare(objectTexture);

	return objectTexture;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void ObjectTextureManager::releaseTexture(ObjectTexture objectTexture)
{
	NM_ASSERT(!isDeleted(objectTexture), "ObjectTextureManager::releaseTexture: trying to release an invalid objectTexture");

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
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectTextureManager::destructor()
{
	ObjectTextureManager::reset(this);

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
