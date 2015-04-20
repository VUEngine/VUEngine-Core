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

#include <OMegaSpriteManager.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __TOTAL_OBJECT_GROUPS 	4

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define OMegaSpriteManager_ATTRIBUTES														\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* object groups */															\
	OMegaSprite oMegaSprites[__TOTAL_OBJECT_GROUPS];							\

// define the OMegaSprite
__CLASS_DEFINITION(OMegaSpriteManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(OMegaSprite, u8 spt);


static void OMegaSpriteManager_constructor(OMegaSpriteManager this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------


// a singleton
__SINGLETON(OMegaSpriteManager);

//class constructor
void OMegaSpriteManager_constructor(OMegaSpriteManager this)
{
	__CONSTRUCT_BASE();

	OMegaSpriteManager_reset(this);
}

// class destructor
void OMegaSpriteManager_destructor(OMegaSpriteManager this)
{
	ASSERT(this, "OMegaSpriteManager::destructor: null this");

	OMegaSpriteManager_reset(this);
	
	// allow a new construct
	__SINGLETON_DESTROY;
}

// reset
void OMegaSpriteManager_reset(OMegaSpriteManager this)
{
	ASSERT(this, "OMegaSpriteManager::reset: null this");
	
	int i = 0;
	for(; i < __TOTAL_OBJECT_GROUPS; i++)
	{
		this->oMegaSprites[i] = NULL;
	}
}

// reset
void OMegaSpriteManager_destroyOMegaSprites(OMegaSpriteManager this)
{
	ASSERT(this, "OMegaSpriteManager::reset: null this");
	
	int i = 0;
	for(; i < __TOTAL_OBJECT_GROUPS; i++)
	{
		if(this->oMegaSprites[i])
		{
			__DELETE(this->oMegaSprites[i]);
		}
		
		this->oMegaSprites[i] = NULL;
	}
}

// reset
OMegaSprite OMegaSpriteManager_getOMegaSprite(OMegaSpriteManager this, int numberOfObjects)
{
	ASSERT(this, "OMegaSpriteManager::getOMegaSprite: null this");
	
	int i = 0;
	for(; i < __TOTAL_OBJECT_GROUPS; i++)
	{
		if(!this->oMegaSprites[i])
		{
			this->oMegaSprites[i] = __NEW(OMegaSprite, SPT3 - i);
			
			return this->oMegaSprites[i];
		}
		
		if(OMegaSprite_hasRoomFor(this->oMegaSprites[i], numberOfObjects))
		{
			return this->oMegaSprites[i];
		}		
	}
	
	NM_ASSERT(this, "OMegaSpriteManager::getOMegaSprite: no OMegaSprites available");

	return NULL;
}