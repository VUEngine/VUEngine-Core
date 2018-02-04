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

#include <ObjectSpriteContainerManager.h>
#include <VIPManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define ObjectSpriteContainerManager_ATTRIBUTES															\
		Object_ATTRIBUTES																				\
		/**
		 * @var ObjectSpriteContainer 	objectSpriteContainers
		 * @brief						object groups
		 * @memberof					ObjectSpriteContainerManager
		 */																								\
		ObjectSpriteContainer objectSpriteContainers[__TOTAL_OBJECT_SEGMENTS];							\

/**
 * @class 	ObjectSpriteContainerManager
 * @extends Object
 * @ingroup graphics-2d-sprites-object
 */
__CLASS_DEFINITION(ObjectSpriteContainerManager, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ObjectSpriteContainer, int spt, int totalObjects, int firstObjectIndex);

static void ObjectSpriteContainerManager_constructor(ObjectSpriteContainerManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			ObjectSpriteContainerManager_getInstance()
 * @memberof	ObjectSpriteContainerManager
 * @public
 *
 * @return		ObjectSpriteContainerManager instance
 */
__SINGLETON(ObjectSpriteContainerManager);

/**
 * Class constructor
 *
 * @memberof		ObjectSpriteContainerManager
 * @public
 *
 * @param this		Function scope
 */
void __attribute__ ((noinline)) ObjectSpriteContainerManager_constructor(ObjectSpriteContainerManager this)
{
	ASSERT(this, "ObjectSpriteContainerManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	int i = 0;
	for(; i < __TOTAL_OBJECT_SEGMENTS; i++)
	{
		this->objectSpriteContainers[i] = NULL;
		_vipRegisters[__SPT3 - i] = 0;
	}
}

/**
 * Class destructor
 *
 * @memberof		ObjectSpriteContainerManager
 * @public
 *
 * @param this		Function scope
 */
void ObjectSpriteContainerManager_destructor(ObjectSpriteContainerManager this)
{
	ASSERT(this, "ObjectSpriteContainerManager::destructor: null this");

	ObjectSpriteContainerManager_reset(this);

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Reset manager's status
 *
 * @memberof		ObjectSpriteContainerManager
 * @public
 *
 * @param this		Function scope
 */
void ObjectSpriteContainerManager_reset(ObjectSpriteContainerManager this)
{
	ASSERT(this, "ObjectSpriteContainerManager::reset: null this");

	int i = 0;
	for(; i < __TOTAL_OBJECT_SEGMENTS; i++)
	{
		this->objectSpriteContainers[i] = NULL;
	}

	// clean OBJ memory
	for(i = 0; i < __AVAILABLE_CHAR_OBJECTS; i++)
	{
		_objectAttributesBaseAddress[(i << 2) + 0] = 0;
		_objectAttributesBaseAddress[(i << 2) + 1] = 0;
		_objectAttributesBaseAddress[(i << 2) + 2] = 0;
		_objectAttributesBaseAddress[(i << 2) + 3] = 0;
	}

}

/**
 * Retrieve an ObjectSpriteContainer capable of allocating the given number of OBJECTs and close to the given z coordinate
 *
 * @memberof					ObjectSpriteContainerManager
 * @public
 *
 * @param this					Function scope
 * @param numberOfObjects		Number of OBJECTs required
 * @param z						Z coordinate
 *
 * @return 						ObjectSpriteContainer instance
 */
ObjectSpriteContainer ObjectSpriteContainerManager_getObjectSpriteContainer(ObjectSpriteContainerManager this, int numberOfObjects, fix10_6 z)
{
	ASSERT(this, "ObjectSpriteContainerManager::getObjectSpriteContainer: null this");

	// check if there is need to build the containers
	int i = __TOTAL_OBJECT_SEGMENTS;
	for(; i--; )
	{
		if(!this->objectSpriteContainers[i])
		{
			this->objectSpriteContainers[i] = __NEW(ObjectSpriteContainer, i, __AVAILABLE_CHAR_OBJECTS >> 2, __AVAILABLE_CHAR_OBJECTS - (__TOTAL_OBJECT_SEGMENTS - (i + 1)) * (__TOTAL_OBJECT_SEGMENTS >> 2));
		}
	}

	// search for the container with the closes z position and room to
	// hold the sprite
	ObjectSpriteContainer suitableObjectSpriteContainer = NULL;

	for(i = __TOTAL_OBJECT_SEGMENTS; i--; )
	{
		if(this->objectSpriteContainers[i] && ObjectSpriteContainer_hasRoomFor(this->objectSpriteContainers[i], numberOfObjects))
		{
			if(!suitableObjectSpriteContainer)
			{
				suitableObjectSpriteContainer = this->objectSpriteContainers[i];
			}
			else
			{
				if(__ABS(__FIX10_6_TO_I(ObjectSpriteContainer_getPosition(this->objectSpriteContainers[i]).z - z)) < __ABS(__FIX10_6_TO_I(ObjectSpriteContainer_getPosition(suitableObjectSpriteContainer).z - z)))
				{
					suitableObjectSpriteContainer = this->objectSpriteContainers[i];
				}
			}
		}
	}

	NM_ASSERT(suitableObjectSpriteContainer, "ObjectSpriteContainerManager::getObjectSpriteContainer: no ObjectSpriteContainers available");

	return suitableObjectSpriteContainer;
}

/**
 * Retrieve the ObjectSpriteContainer for the given segment
 *
 * @memberof			ObjectSpriteContainerManager
 * @public
 *
 * @param this			Function scope
 * @param segment		Spt segment
 *
 * @return 				ObjectSpriteContainer instance
 */
ObjectSpriteContainer ObjectSpriteContainerManager_getObjectSpriteContainerBySegment(ObjectSpriteContainerManager this, int segment)
{
	ASSERT(this, "ObjectSpriteContainerManager::getObjectSpriteContainerBySegment: null this");
	ASSERT((unsigned)segment < __TOTAL_OBJECT_SEGMENTS, "ObjectSpriteContainerManager::getObjectSpriteContainerBySegment: invalid segment");

	return (unsigned)segment < __TOTAL_OBJECT_SEGMENTS? this->objectSpriteContainers[segment]: NULL;
}

/**
 * Setup the object sprite containers
 *
 * @memberof			ObjectSpriteContainerManager
 * @public
 *
 * @param this			Function scope
 * @param size			Array with the number of OBJECTs per container
 * @param z				Z coordinate of each container
 */
const ObjectSpriteContainer* ObjectSpriteContainerManager_setupObjectSpriteContainers(ObjectSpriteContainerManager this, s16 size[__TOTAL_OBJECT_SEGMENTS], s16 z[__TOTAL_OBJECT_SEGMENTS])
{
	ASSERT(this, "ObjectSpriteContainerManager::setupObjectSpriteContainers: null this");

	int availableObjects = __AVAILABLE_CHAR_OBJECTS;
#ifndef __RELEASE
	s16 previousZ = z[__TOTAL_OBJECT_SEGMENTS - 1];
#endif

	// must add them from __SPT3 to __SPT0
	// so each they start presorted in the WORLDS
	int i = __TOTAL_OBJECT_SEGMENTS;
	for(; i--; )
	{
		NM_ASSERT(z[i] <= previousZ, "ObjectSpriteContainerManager::setupObjectSpriteContainers: wrong z");

		if(0 <= size[i] && !this->objectSpriteContainers[i])
		{
			availableObjects -= size[i];
			NM_ASSERT(0 <= availableObjects, "ObjectSpriteContainerManager::setupObjectSpriteContainers: OBJs depleted");
			this->objectSpriteContainers[i] = __NEW(ObjectSpriteContainer, i, size[i], availableObjects);
		}

		if(this->objectSpriteContainers[i])
		{
			PixelVector position =
			{
					0, 0, __PIXELS_TO_METERS(z[i] + i), 0
			};

			ObjectSpriteContainer_setPosition(this->objectSpriteContainers[i], &position);

#ifndef __RELEASE
			previousZ = z[i];
#endif
		}
	}

	return this->objectSpriteContainers;
}

/**
 * Set the z position of the ObjectSpriteContainer in the given segment
 *
 * @memberof		ObjectSpriteContainerManager
 * @public
 *
 * @param this		Function scope
 * @param spt		Spt segment of the ObjectSpriteContainer to modify
 * @param z			New z coordinate
 */
void ObjectSpriteContainerManager_setZPosition(ObjectSpriteContainerManager this, int spt, fix10_6 z)
{
	ASSERT(this, "ObjectSpriteContainerManager::position: null this");

	ASSERT(spt < __TOTAL_OBJECT_SEGMENTS, "ObjectSpriteContainerManager::position: invalid spt");

	if(spt < __TOTAL_OBJECT_SEGMENTS)
	{
		PixelVector position =
		{
				0, 0, z, 0
		};

		ObjectSpriteContainer_setPosition(this->objectSpriteContainers[spt], &position);
	}
}

/**
 * Print the manager's status
 *
 * @memberof		ObjectSpriteContainerManager
 * @public
 *
 * @param this		Function scope
 * @param x			Camera x coordinate
 * @param y			Camera y coordinate
 */
void ObjectSpriteContainerManager_print(ObjectSpriteContainerManager this, int x, int y)
{
	ASSERT(this, "ObjectSpriteContainerManager::print: null this");

	Printing_text(Printing_getInstance(), "OBJECTS' USAGE", x, y++, NULL);
	int totalUsedObjects = 0;
	int i = 0;
	for(; i < __TOTAL_OBJECT_SEGMENTS; i++)
	{
		totalUsedObjects += ObjectSpriteContainer_getTotalUsedObjects(this->objectSpriteContainers[i]);
	}

	Printing_text(Printing_getInstance(), "Total used objects: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), totalUsedObjects, x + 20, y, NULL);
}
