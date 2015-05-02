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

#include <CharSetManager.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define __CHAR_SEGMENT_SIZE		(512 / 32)

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define CharSetManager_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* charsets defined */														\
	VirtualList charSets[__CHAR_SEGMENTS];										\
																				\
	/* next offset to reclaim */												\
	u16 freedOffset[__CHAR_SEGMENTS];											\
																				\
	/* defragmentation flag */													\
	bool needsDefrag;															\

// define the CharSetManager
__CLASS_DEFINITION(CharSetManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void CharSetManager_constructor(CharSetManager this);
static CharSet CharSetManager_findCharSet(CharSetManager this, CharSetDefinition* charSetDefinition);
static CharSet CharSetManager_allocateCharSet(CharSetManager this, CharSetDefinition* charSetDefinition);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(CharSetManager);

// class's constructor
static void CharSetManager_constructor(CharSetManager this)
{
	__CONSTRUCT_BASE();

	int segment = 0;
	for(; segment < __CHAR_SEGMENTS; segment++)
	{
		this->freedOffset[segment] = 0;
		this->charSets[segment] = NULL;
	}
	
	this->needsDefrag = false;

	CharSetManager_reset(this);
}

// class's destructor
void CharSetManager_destructor(CharSetManager this)
{
	ASSERT(this, "CharSetManager::destructor: null this");

	int i = 0;
	for(; i < __CHAR_SEGMENTS; i++)
	{
		if(this->charSets[i])
		{
			VirtualNode node = VirtualList_begin(this->charSets[i]);
			
			for(; node; node = VirtualNode_getNext(node))
			{
				__DELETE(VirtualNode_getData(node));
			}
			
			__DELETE(this->charSets[i]);
			
			this->charSets[i] = NULL;
		}
	}

	// allow a new construct
	__SINGLETON_DESTROY;
}

// reset
void CharSetManager_reset(CharSetManager this)
{
	ASSERT(this, "CharSetManager::reset: null this");

	int segment = 0;
	for(; segment < __CHAR_SEGMENTS; segment++)
	{
		if(this->charSets[segment])
		{
			VirtualNode node = VirtualList_begin(this->charSets[segment]);
			
			for(; node; node = VirtualNode_getNext(node))
			{
				__DELETE(VirtualNode_getData(node));
			}
			
			__DELETE(this->charSets[segment]);
		}
		
		this->charSets[segment] = __NEW(VirtualList);
		this->freedOffset[segment] = 0;
	}
	
	this->needsDefrag = false;
}

// find a charset
static CharSet CharSetManager_findCharSet(CharSetManager this, CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "CharSetManager::findCharSet: null this");

	// try to find a charset with the same char definition
	int i = 0;
	for(; i < __CHAR_SEGMENTS; i++)
	{
		VirtualNode node = VirtualList_begin(this->charSets[i]);
		
		for(; node; node = VirtualNode_getNext(node))
		{
			CharSet charSet = __UPCAST(CharSet, VirtualNode_getData(node));
			
			if(CharSet_getCharSetDefinition(charSet) == charSetDefinition && CharSet_getAllocationType(charSet) == charSetDefinition->allocationType)
			{
				return charSet;
			}
		}
	}
		
	return NULL;
}

// get charset
CharSet CharSetManager_getCharSet(CharSetManager this, CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "CharSetManager::loadCharSet: null this");

	CharSet charSet = NULL;

	switch (charSetDefinition->allocationType)
	{
		case __ANIMATED_SINGLE:

			// ask for allocation
			charSet = CharSetManager_allocateCharSet(CharSetManager_getInstance(), charSetDefinition);

			break;

		case __ANIMATED_SHARED:
		case __ANIMATED_SHARED_COORDINATED:
		case __ANIMATED_MULTI:
		case __NOT_ANIMATED:

			// first try to find an already created charset
			charSet = CharSetManager_findCharSet(this, charSetDefinition);

			if (charSet)
			{
				CharSet_increaseUsageCount(charSet);
			}
			else
			{
				charSet = CharSetManager_allocateCharSet(CharSetManager_getInstance(), charSetDefinition);
			}

			break;

		default:

			ASSERT(false, "CharSet::write: with no allocation type");
			break;
	}
	
	return charSet;
}

// release char graphic memory
void CharSetManager_releaseCharSet(CharSetManager this, CharSet charSet)
{
	ASSERT(this, "CharSetManager::free: null this");

	if(CharSet_decreaseUsageCount(charSet))
	{
		u8 segment = CharSet_getSegment(charSet);
		u16 offset = CharSet_getOffset(charSet);
		
		if(!this->freedOffset[segment] || offset < this->freedOffset[segment])
		{
			this->freedOffset[segment] = offset;
		}

		VirtualList_removeElement(this->charSets[segment], charSet);
	
		__DELETE(charSet);

		this->needsDefrag = true;
	}
}

// allocate a char defintion within char graphic memory
static CharSet CharSetManager_allocateCharSet(CharSetManager this, CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "CharSetManager::allocateCharSet: null this");

	ASSERT(charSetDefinition->numberOfChars > 0, "CharSetManager::allocateCharSet: number of chars < 0");

	int segment = 0;

	for (; segment < __CHAR_SEGMENTS ; segment++)
	{
		ASSERT(this->charSets[segment], "CharSetManager::getNextFreeOffset: null this");

		u16 offset = 1;

		if(VirtualList_begin(this->charSets[segment]))
		{
			CharSet lastCharSet = __UPCAST(CharSet, VirtualList_back(this->charSets[segment]));
			offset += CharSet_getOffset(lastCharSet) + CharSet_getNumberOfChars(lastCharSet) + __CHAR_ROOM;
		}
		
		if ((unsigned)offset + charSetDefinition->numberOfChars < __CHAR_SEGMENT_TOTAL_CHARS)
		{
			CharSet charSet = __NEW(CharSet, charSetDefinition, segment, offset);

			CharSet_write(charSet);
			
			VirtualList_pushBack(this->charSets[segment], charSet);

			return charSet;
		}
	}

	// if there isn't enough memory trown an exception
	NM_ASSERT(false, "CharSetManager::allocateCharSet: char mem depleted");
	
	return NULL;
}

// defrag char memory
void CharSetManager_defragmentProgressively(CharSetManager this)
{
	ASSERT(this, "CharSetManager::defragmentProgressively: null this");

	if (!this->needsDefrag)
	{
		return;
	}

	int segment = 0;
	for (; segment < __CHAR_SEGMENTS ; segment++)
	{
		if (this->freedOffset[segment])
		{
			VirtualNode node = VirtualList_begin(this->charSets[segment]);
			
			for(; node; node = VirtualNode_getNext(node))
			{
				CharSet charSet = __UPCAST(CharSet, VirtualNode_getData(node));

				if(this->freedOffset[segment] < CharSet_getOffset(charSet))
				{
					CharSet_setOffset(charSet, this->freedOffset[segment]);
					CharSet_rewrite(charSet);
					this->freedOffset[segment] += CharSet_getNumberOfChars(charSet) + __CHAR_ROOM;
					return;	
				}
			}
			
			this->freedOffset[segment] = 0;
		}
	}

	this->needsDefrag = false;
}

// print class's attributes's states
void CharSetManager_print(CharSetManager this, int x, int y)
{
	ASSERT(this, "CharSetManager::print: null this");

	Printing_text(Printing_getInstance(), "CHARACTER MEMORY", x, y++, NULL);
	Printing_text(Printing_getInstance(), "CharSets: ", x, ++y, NULL);

	int segment = 0;

	for(; segment < __CHAR_SEGMENTS; segment++)
	{
		Printing_int(Printing_getInstance(), VirtualList_getSize(this->charSets[segment]), x + 12, y++, NULL);
	}
	
	y++;
		
/*	int i = 0;
	for (segment = 0; segment < 4; segment++)
	{
		Printing_text(Printing_getInstance(), "CharSeg", x, y, NULL);
		Printing_int(Printing_getInstance(), segment, x + 8, y, NULL);
		for (i = 0; i < __CHAR_SETS_PER_SEGMENT && (y + i + 1) < 28; i++)
		{
			Printing_hex(Printing_getInstance(), this->segment[segment][i], x, y + i + 1, NULL);
		}

		x += 12;
	}
	*/
}