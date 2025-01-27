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

#include <BgmapSprite.h>
#include <DebugConfig.h>
#include <Mem.h>
#include <ObjectSprite.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <Printer.h>
#include <Sprite.h>
#include <SpriteManager.h>
#include <Texture.h>
#include <Utilities.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VIPManager.h>

#include "ObjectSpriteContainer.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class ObjectSprite;
friend class Sprite;
friend class Texture;
friend class VirtualList;
friend class VirtualNode;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSpriteContainer::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor(NULL, NULL);

	this->hasTextures = false;
	this->head = __WORLD_ON | __WORLD_OBJECT | __WORLD_OVR;
	this->head &= ~__WORLD_END;
	this->spt = 0;
	this->firstObjectIndex = 0;
	this->lastObjectIndex = 0;
	this->transparency = __TRANSPARENCY_NONE;
	this->sortingSpriteNode = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSpriteContainer::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ClassPointer ObjectSpriteContainer::getBasicType()
{
	return typeofclass(BgmapSprite);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int16 ObjectSpriteContainer::doRender(int16 index)
{
	this->index = index;

	return index;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 ObjectSpriteContainer::getTotalPixels()
{
	if(__NO_RENDER_INDEX != this->index)
	{
		return (this->firstObjectIndex - this->lastObjectIndex) * 8 * 8;
	}

	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSpriteContainer::forceShow()
{
	Base::forceShow(this);
	this->show = __HIDE;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ObjectSpriteContainer::forceHide()
{
	this->show = __SHOW;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ObjectSpriteContainer::sortProgressively(bool complete)
{
	bool swapped = false;
	/*
	if(NULL == this->sortingSpriteNode)
	{
		this->sortingSpriteNode = this->objectSprites->head;

		if(NULL == this->sortingSpriteNode)
		{
			return false;
		}
	}

	bool swapped = false;

	do
	{
		swapped = false;

		for
		(
			VirtualNode node = complete ? this->objectSprites->head : this->sortingSpriteNode; 
			NULL != node && NULL != node->next; node = node->next
		)
		{
			VirtualNode nextNode = node->next;

			NM_ASSERT(!isDeleted(node->data), "ObjectSpriteContainer::sortProgressively: NULL node's data");
			ASSERT(__GET_CAST(Sprite, nextNode->data), "ObjectSpriteContainer::sortProgressively: node's data isn't a sprite");

			Sprite sprite = Sprite::safeCast(node->data);

			NM_ASSERT(!isDeleted(nextNode->data), "ObjectSpriteContainer::sortProgressively: NULL nextNode's data");
			ASSERT(__GET_CAST(Sprite, nextNode->data), "ObjectSpriteContainer::sortProgressively: NULL nextNode's data cast");

			Sprite nextSprite = Sprite::safeCast(nextNode->data);

			// Check if z positions are swapped
			if(nextSprite->position.z + nextSprite->displacement.z > sprite->position.z + sprite->displacement.z)
			{
				// Swap nodes' data
				node->data = nextSprite;
				nextNode->data = sprite;

				node = nextNode;

				swapped = true;
			}

			if(!complete)
			{
				break;
			}
		}
	}
	while(complete && swapped);

	if(!complete)
	{
		this->sortingSpriteNode = this->sortingSpriteNode->next;
	}
*/
	return swapped;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 ObjectSpriteContainer::getTotalUsedObjects()
{
	return this->firstObjectIndex - this->lastObjectIndex;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
