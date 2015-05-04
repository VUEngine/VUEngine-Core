/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectSpriteContainerManager.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define ObjectSpriteContainerManager_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* object groups */															\
	ObjectSpriteContainer objectSpriteContainers[__TOTAL_OBJECT_SEGMENTS];							\

// define the ObjectSpriteContainer
__CLASS_DEFINITION(ObjectSpriteContainerManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ObjectSpriteContainer, u8 spt);

static void ObjectSpriteContainerManager_constructor(ObjectSpriteContainerManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------


// a singleton
__SINGLETON(ObjectSpriteContainerManager);

//class constructor
void ObjectSpriteContainerManager_constructor(ObjectSpriteContainerManager this)
{
	ASSERT(this, "ObjectSpriteContainerManager::constructor: null this");

	__CONSTRUCT_BASE();

	int i = 0;
	for(; i < __TOTAL_OBJECT_SEGMENTS; i++)
	{
		this->objectSpriteContainers[i] = NULL;
		VIP_REGS[SPT3 - i] = 0;
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
			this->objectSpriteContainers[i] = __NEW(ObjectSpriteContainer, i);
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
				if(abs(ObjectSpriteContainer_getPosition(this->objectSpriteContainers[i]).z - z) < abs(ObjectSpriteContainer_getPosition(suitableObjectSpriteContainer).z - z))
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


void ObjectSpriteContainerManager_setObjectSpriteContainersZPosition(ObjectSpriteContainerManager this, fix19_13 z[__TOTAL_OBJECT_SEGMENTS])
{
	ASSERT(this, "ObjectSpriteContainerManager::setObjectSpriteContainerZPosition: null this");
	
	fix19_13 previousZ = z[__TOTAL_OBJECT_SEGMENTS - 1];

	// must add them from SPT3 to SPT0
	// so each they start presorted in the WORLDS
	int i = __TOTAL_OBJECT_SEGMENTS;
	for(; i--; )
	{
		NM_ASSERT(z[i] <= previousZ, "ObjectSpriteContainerManager::setObjectSpriteContainerZPosition: wrong z");

		if(!this->objectSpriteContainers[i])
		{
			this->objectSpriteContainers[i] = __NEW(ObjectSpriteContainer, i);
		}

		VBVec2D position =
		{
				0, 0, z[i] + FTOFIX19_13(i * 0.1f), 0
		};

		ObjectSpriteContainer_setPosition(this->objectSpriteContainers[i], position);
		previousZ = z[i];
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