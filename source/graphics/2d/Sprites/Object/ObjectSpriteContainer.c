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

#include <ObjectSpriteContainer.h>
#include <Mem.h>
#include <ObjectTexture.h>
#include <Optics.h>
#include <Camera.h>
#include <SpriteManager.h>
#include <VIPManager.h>
#include <Utilities.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Sprite;
friend class Texture;
friend class ObjectSprite;
friend class VirtualNode;
friend class VirtualList;


static int32 _spt;
static int16 _objectIndex;
static int16 _previousObjectIndex;
static uint16 _vipRegistersCache[__TOTAL_OBJECT_SEGMENTS];

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 */
void ObjectSpriteContainer::constructor()
{
	Base::constructor(NULL, NULL);

	this->head = __WORLD_ON | __WORLD_OBJECT | __WORLD_OVR;
	this->head &= ~__WORLD_END;
	this->spt = 0;
	this->firstObjectIndex = 0;
	this->lastObjectIndex = 0;
	this->objectSprites = new VirtualList();
	this->hidden = false;
	this->visible = true;
	this->transparent = __TRANSPARENCY_NONE;
	this->positioned = true;
	this->lockSpritesLists = false;
	this->hideSprites = false;

	ObjectSpriteContainer::registerWithManager(this);
}

/**
 * Class destructor
 */
void ObjectSpriteContainer::destructor()
{
	SpriteManager::unregisterSprite(SpriteManager::getInstance(), Sprite::safeCast(this), false);

	ASSERT(this->objectSprites, "ObjectSpriteContainer::destructor: null objectSprites");

	VirtualList objectSprites = new VirtualList();
	VirtualList::copy(objectSprites, this->objectSprites);

	for(VirtualNode node = objectSprites->head; node; node = node->next)
	{
		delete node->data;
	}

	delete objectSprites;
	delete this->objectSprites;
	this->objectSprites = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Register
 *
 */
void ObjectSpriteContainer::registerWithManager()
{
	this->registered = SpriteManager::registerSprite(SpriteManager::getInstance(), Sprite::safeCast(this), false);
}

/**
 * Add an ObjectSprite to this container
 *
 * @param objectSprite		Sprite to add
 */
bool ObjectSpriteContainer::registerSprite(ObjectSprite objectSprite)
{
	ASSERT(objectSprite, "ObjectSpriteContainer::registerSprite: null objectSprite");

	NM_ASSERT(!VirtualList::find(this->objectSprites, objectSprite), "ObjectSpriteContainer::registerSprite: already registered");

	if(!isDeleted(objectSprite))
	{
		this->lockSpritesLists = true;

		VirtualList::pushBack(this->objectSprites, objectSprite);

		this->lockSpritesLists = false;

		return true;
	}

	NM_ASSERT(objectSprite, "ObjectSpriteContainer::registerSprite: null objectSprite");
	return false;
}

/**
 * Remove a previously registered ObjectSprite
 *
 * @param objectSprite		Sprite to remove
 */
void ObjectSpriteContainer::unregisterSprite(ObjectSprite objectSprite)
{
	ASSERT(objectSprite, "ObjectSpriteContainer::unregisterSprite: null objectSprite");
	NM_ASSERT(VirtualList::find(this->objectSprites, objectSprite), "ObjectSpriteContainer::unregisterSprite: null found");

	this->lockSpritesLists = true;

	// remove the objectSprite to prevent rendering afterwards
	VirtualList::removeElement(this->objectSprites, objectSprite);

	this->lockSpritesLists = false;
}

/**
 * Set 2D position
 *
 * @param position		New 2D position
 */
void ObjectSpriteContainer::setPosition(const PixelVector* position)
{
	if(this->objectSprites)
	{
		for(VirtualNode node = this->objectSprites->head; node; node = node->next)
		{
			ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

			ObjectSprite::setPosition(objectSprite, &objectSprite->position);
		}
	}

	this->position.z = position->z;
}

/**
 * Sort the object sprites within this container according to their z coordinates
 *
 * @private
 */
void ObjectSpriteContainer::sortProgressively()
{
	if(this->lockSpritesLists)
	{
		return;
	}

	for(VirtualNode node = this->objectSprites->tail; node && node->previous; node = node->previous)
	{
		VirtualNode previousNode = node->previous;

		NM_ASSERT(!isDeleted(node->data), "ObjectSpriteContainer::sortProgressively: NULL node's data");
		NM_ASSERT(__GET_CAST(Sprite, node->data), "ObjectSpriteContainer::sortProgressively: NULL node's data cast");

		Sprite sprite = Sprite::safeCast(node->data);

		NM_ASSERT(!isDeleted(previousNode->data), "ObjectSpriteContainer::sortProgressively: NULL previousNode's data");
		NM_ASSERT(__GET_CAST(Sprite, previousNode->data), "ObjectSpriteContainer::sortProgressively: NULL previousNode's data cast");

		Sprite nextSprite = Sprite::safeCast(previousNode->data);

		// check if z positions are swapped
		if(nextSprite->position.z + nextSprite->displacement.z < sprite->position.z + sprite->displacement.z)
		{
			// swap nodes' data
			VirtualNode::swapData(node, previousNode);

			sprite->renderFlag = nextSprite->renderFlag = true;
		}
	}
}

#ifdef __TOOLS
void ObjectSpriteContainer::hideSprites(ObjectSprite spareSprite)
{
	ObjectSpriteContainer::hideForDebug(this);

	for(VirtualNode node = this->objectSprites->head; node; node = node->next)
	{
		ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

		if(objectSprite == spareSprite)
		{
			ObjectSprite::showForDebug(objectSprite);
			ObjectSpriteContainer::showForDebug(this);
			continue;
		}

		ObjectSprite::hideForDebug(objectSprite);
	}
}

/**
 * Show all WORLD layers
 */
void ObjectSpriteContainer::showSprites(ObjectSprite spareSprite)
{
	ObjectSpriteContainer::showForDebug(this);

	for(VirtualNode node = this->objectSprites->head; node; node = node->next)
	{
		ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

		if(objectSprite == spareSprite)
		{
			ObjectSprite::hideForDebug(objectSprite);
			continue;
		}

		ObjectSprite::showForDebug(objectSprite);
	}
}
#endif

void ObjectSpriteContainer::showForDebug()
{
	Base::showForDebug(this);
	this->hidden = false;
	this->positioned = true;
	this->hideSprites = false;
}

void ObjectSpriteContainer::hideForDebug()
{
	this->hidden = false;
	this->positioned = true;
	this->hideSprites = true;
}

/**
 * Write WORLD data to DRAM
 *
 * @param evenFrame
 */
int16 ObjectSpriteContainer::doRender(int16 index __attribute__((unused)), bool evenFrame __attribute__((unused)))
{
	// Setup spt
	this->spt = _spt;

	this->firstObjectIndex = _objectIndex;

	if(!this->hideSprites)
	{
		for(VirtualNode node = this->objectSprites->head; node && 0 < _objectIndex; node = node->next)
		{
			NM_ASSERT(!isDeleted(node->data), "ObjectSpriteContainer::doRender: NULL node's data");

			ObjectSprite objectSprite = ObjectSprite::safeCast(node->data);

			// Saves on method calls quite a bit when there are lots of
			// sprites. Don't remove.
			if(objectSprite->hidden || !objectSprite->positioned)
			{
				continue;
			}

			if(objectSprite->transparent & evenFrame)
			{
				objectSprite->index = __NO_RENDER_INDEX;
				continue;
			}

			if(0 > _objectIndex - objectSprite->totalObjects)
			{
				objectSprite->index = __NO_RENDER_INDEX;
				break;
			}

			// Do not change the order of this condition, objectSprite->totalObjects may be modified during rendering
			// but calling ObjectSprite::getTotalObjects is too costly
			if(ObjectSprite::render(objectSprite, _objectIndex - (objectSprite->totalObjects - 1), evenFrame) == _objectIndex - (objectSprite->totalObjects - 1))
			{
				_objectIndex -= objectSprite->totalObjects;
			}
		}
	}

	bool renderedObjectSprites = true;

	if(this->firstObjectIndex == _objectIndex)
	{
		_objectAttributesCache[_objectIndex].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
		_objectIndex--;
		renderedObjectSprites = false;
	}
	else
	{
		_worldAttributesCache[index].head = this->head;

		// Make sure that the rest of spt segments only run up to the last
		// used object index
		for(int32 i = _spt--; i--;)
		{
			_vipRegistersCache[i] = _objectIndex;
		}
	}

	this->lastObjectIndex = _objectIndex;

	this->renderFlag = true;

	return !renderedObjectSprites ? __NO_RENDER_INDEX : index;
}

/**
 * Retrieve the number of used OBJECTs within the segment assigned to this container
 *
 * @return 		Number of used OBJECTs
 */
int32 ObjectSpriteContainer::getTotalUsedObjects()
{
	return this->firstObjectIndex - this->lastObjectIndex;
}

/**
 * Retrieve the index of the first OBJECT within the segment assigned to this container
 *
 * @return 		Index of the first OBJECT
 */
int32 ObjectSpriteContainer::getFirstObjectIndex()
{
	return this->firstObjectIndex;
}

/**
 * Retrieve the index of the last OBJECT within the segment assigned to this container
 *
 * @return 		Index of the last OBJECT
 */
int32 ObjectSpriteContainer::getLastObjectIndex()
{
	return this->firstObjectIndex + this->lastObjectIndex;
}

/**
 * Print the container's status
 *
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 */
void ObjectSpriteContainer::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "SPRITE ", x, y++, NULL);
	Printing::text(Printing::getInstance(), "Index: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), SpriteManager::getSpritePosition(SpriteManager::getInstance(), Sprite::safeCast(this)), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Class: ", x, ++y, NULL);
	Printing::text(Printing::getInstance(), __GET_CLASS_NAME_UNSAFE(this), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Head:                         ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), Sprite::getWorldHead(this), x + 18, y, 8, NULL);
	Printing::text(Printing::getInstance(), "Mode:", x, ++y, NULL);
	Printing::text(Printing::getInstance(), "OBJECT   ", x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Segment:                ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->spt, x + 18, y++, NULL);
	Printing::text(Printing::getInstance(), "SPT value:                ", x, y, NULL);
	Printing::int32(Printing::getInstance(), _vipRegisters[__SPT0 + this->spt], x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "HEAD:                   ", x, ++y, NULL);
	Printing::hex(Printing::getInstance(), _worldAttributesBaseAddress[this->index].head, x + 18, y, 4, NULL);
	Printing::text(Printing::getInstance(), "Total OBJs:            ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->firstObjectIndex - this->lastObjectIndex, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "OBJ index range:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->lastObjectIndex, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "-", x  + 18 + Utilities::intLength(this->firstObjectIndex), y, NULL);
	Printing::int32(Printing::getInstance(), this->firstObjectIndex, x  + 18 + Utilities::intLength(ObjectSpriteContainer::getFirstObjectIndex(this)) + 1, y, NULL);
	Printing::text(Printing::getInstance(), "Z Position: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), this->position.z, x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Pixels: ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), ObjectSpriteContainer::getTotalPixels(this), x + 18, y, NULL);
}

int32 ObjectSpriteContainer::getTotalPixels()
{
	if(__NO_RENDER_INDEX != this->index)
	{
		return (this->firstObjectIndex - this->lastObjectIndex) * 8 * 8;
	}

	return 0;
}


/**
 * Set Sprite's render mode
 *
 * @param display	Which displays to show on
 * @param mode		WORLD layer's head mode
 */
void ObjectSpriteContainer::setMode(uint16 display __attribute__ ((unused)), uint16 mode __attribute__ ((unused)))
{}

/**
 * Write textures
 *
 * @return			true it all textures are written
 */
bool ObjectSpriteContainer::writeTextures()
{
	if(!isDeleted(this->objectSprites))
	{
		for(VirtualNode node = this->objectSprites->head; node; node = node->next)
		{
			ObjectSprite::writeTextures(ObjectSprite::safeCast(node->data));
		}
	}

	return true;
}

static void ObjectSpriteContainer::reset()
{
	for(int32 i = __AVAILABLE_CHAR_OBJECTS - 1; 0 <= i; i--)
	{
		_objectAttributesCache[i].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
	}

	_spt = __TOTAL_OBJECT_SEGMENTS - 1;
	_objectIndex = __AVAILABLE_CHAR_OBJECTS - 1;

	for(int32 i = __TOTAL_OBJECT_SEGMENTS; i--;)
	{
		_vipRegistersCache[i] = _objectIndex;
	}
}

static void ObjectSpriteContainer::prepareForRendering()
{
	_spt = __TOTAL_OBJECT_SEGMENTS - 1;
	_objectIndex = __AVAILABLE_CHAR_OBJECTS - 1;

	for(int32 i = __TOTAL_OBJECT_SEGMENTS; i--;)
	{
		_vipRegistersCache[i] = _objectIndex;
	}
}

static void ObjectSpriteContainer::finishRendering()
{
	// clear OBJ memory
	for(int32 i = _objectIndex; _previousObjectIndex <= i; i--)
	{
		_objectAttributesCache[i].head = __OBJECT_SPRITE_CHAR_HIDE_MASK;
	}

	_previousObjectIndex = _objectIndex;
}

static void ObjectSpriteContainer::writeDRAM()
{
	for(int32 i = __TOTAL_OBJECT_SEGMENTS; i--;)
	{
		_vipRegisters[__SPT0 + i] = _vipRegistersCache[i] - _objectIndex;
	}

	Mem::copyWORD((WORD*)(_objectAttributesBaseAddress), (WORD*)(_objectAttributesCache + _objectIndex), sizeof(ObjectAttributes) * (__AVAILABLE_CHAR_OBJECTS - _objectIndex) >> 2);

#ifdef __SHOW_SPRITES_PROFILING
	extern int32 _writtenObjectTiles;
	_writtenObjectTiles = __AVAILABLE_CHAR_OBJECTS - _objectIndex;
#endif
}
