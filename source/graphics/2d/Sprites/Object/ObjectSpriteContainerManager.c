/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectSpriteContainerManager.h>
#include <VIPManager.h>
#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define ObjectSpriteContainerManager_ATTRIBUTES															\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* object groups */																				\
        ObjectSpriteContainer objectSpriteContainers[__TOTAL_OBJECT_SEGMENTS];							\

__CLASS_DEFINITION(ObjectSpriteContainerManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ObjectSpriteContainer, int spt, int totalObjects, int firstObjectIndex);

static void ObjectSpriteContainerManager_constructor(ObjectSpriteContainerManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// a singleton
__SINGLETON(ObjectSpriteContainerManager);

//class constructor
void __attribute__ ((noinline)) ObjectSpriteContainerManager_constructor(ObjectSpriteContainerManager this)
{
	ASSERT(this, "ObjectSpriteContainerManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

	int i = 0;
	for(; i < __TOTAL_OBJECT_SEGMENTS; i++)
	{
		this->objectSpriteContainers[i] = NULL;
		VIP_REGS[__SPT3 - i] = 0;
	}
}

// class destructor
void ObjectSpriteContainerManager_destructor(ObjectSpriteContainerManager this)
{
	ASSERT(this, "ObjectSpriteContainerManager::destructor: null this");

	ObjectSpriteContainerManager_reset(this);

	// allow a new construct
	__SINGLETON_DESTROY;
}

// reset
void ObjectSpriteContainerManager_reset(ObjectSpriteContainerManager this)
{
	ASSERT(this, "ObjectSpriteContainerManager::reset: null this");

	int i = 0;
	for(; i < __TOTAL_OBJECT_SEGMENTS; i++)
	{
		if(this->objectSpriteContainers[i])
		{
			__DELETE(this->objectSpriteContainers[i]);
		}

		this->objectSpriteContainers[i] = NULL;
	}

	// clean OBJ memory
	for(i = 0; i < __AVAILABLE_CHAR_OBJECTS; i++)
	{
		_objecAttributesBaseAddress[(i << 2) + 0] = 0;
		_objecAttributesBaseAddress[(i << 2) + 1] = 0;
		_objecAttributesBaseAddress[(i << 2) + 2] = 0;
		_objecAttributesBaseAddress[(i << 2) + 3] = 0;
	}

}

// retrieve a mega sprite
ObjectSpriteContainer ObjectSpriteContainerManager_getObjectSpriteContainer(ObjectSpriteContainerManager this, int numberOfObjects, fix19_13 z)
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
		if(ObjectSpriteContainer_hasRoomFor(this->objectSpriteContainers[i], numberOfObjects))
		{
			if(!suitableObjectSpriteContainer)
			{
				suitableObjectSpriteContainer = this->objectSpriteContainers[i];
			}
			else
			{
				if(ITOFIX19_13(1) == z || abs(ObjectSpriteContainer_getPosition(this->objectSpriteContainers[i]).z - z) < abs(ObjectSpriteContainer_getPosition(suitableObjectSpriteContainer).z - z))
				{
					suitableObjectSpriteContainer = this->objectSpriteContainers[i];
				}
			}
		}
	}

	NM_ASSERT(suitableObjectSpriteContainer, "ObjectSpriteContainerManager::getObjectSpriteContainer: no ObjectSpriteContainers available");

	return suitableObjectSpriteContainer;
}

ObjectSpriteContainer ObjectSpriteContainerManager_getObjectSpriteContainerBySegment(ObjectSpriteContainerManager this, int segment)
{
	ASSERT(this, "ObjectSpriteContainerManager::getObjectSpriteContainerBySegment: null this");
	ASSERT((unsigned)segment < __TOTAL_OBJECT_SEGMENTS, "ObjectSpriteContainerManager::getObjectSpriteContainerBySegment: invalid segment");

	return (unsigned)segment < __TOTAL_OBJECT_SEGMENTS? this->objectSpriteContainers[segment]: NULL;
}

void ObjectSpriteContainerManager_setupObjectSpriteContainers(ObjectSpriteContainerManager this, fix19_13 size[__TOTAL_OBJECT_SEGMENTS], fix19_13 z[__TOTAL_OBJECT_SEGMENTS])
{
	ASSERT(this, "ObjectSpriteContainerManager::setupObjectSpriteContainers: null this");

	fix19_13 availableObjects = __AVAILABLE_CHAR_OBJECTS;
	fix19_13 previousZ = z[__TOTAL_OBJECT_SEGMENTS - 1];

	// must add them from __SPT3 to __SPT0
	// so each they start presorted in the WORLDS
	int i = __TOTAL_OBJECT_SEGMENTS;
	for(; i--; )
	{
		NM_ASSERT(z[i] <= previousZ, "ObjectSpriteContainerManager::setupObjectSpriteContainers: wrong z");

		if(!this->objectSpriteContainers[i])
		{
			availableObjects -= size[i];
			NM_ASSERT(0 <= availableObjects, "ObjectSpriteContainerManager::setupObjectSpriteContainers: OBJs depleted");
			this->objectSpriteContainers[i] = __NEW(ObjectSpriteContainer, i, size[i], availableObjects);
		}

		VBVec2D position =
		{
				0, 0, z[i] + FTOFIX19_13(i * 0.1f), 0
		};

		ObjectSpriteContainer_setPosition(this->objectSpriteContainers[i], &position);
		previousZ = z[i];
	}
}

void ObjectSpriteContainerManager_setZPosition(ObjectSpriteContainerManager this, int spt, fix19_13 z)
{
	ASSERT(this, "ObjectSpriteContainerManager::position: null this");

	ASSERT(spt < __TOTAL_OBJECT_SEGMENTS, "ObjectSpriteContainerManager::position: invalid spt");

	if(spt < __TOTAL_OBJECT_SEGMENTS)
	{
		VBVec2D position =
		{
				0, 0, z, 0
		};

		ObjectSpriteContainer_setPosition(this->objectSpriteContainers[spt], &position);
	}
}

// print status
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
