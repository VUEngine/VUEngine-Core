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
	this->transparency = __TRANSPARENCY_NONE;
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
	return typeofclass(Sprite);
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
	return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
