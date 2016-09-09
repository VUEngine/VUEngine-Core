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

#include <CharSetManager.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <Mem.h>
#include <Printing.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define __CHAR_SEGMENT_SIZE		(512 / 32)


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define CharSetManager_ATTRIBUTES																		\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* charsets defined */																			\
        VirtualList charSets[__CHAR_SEGMENTS];															\
        /* next offset to reclaim */																	\
        u16 freedOffset[__CHAR_SEGMENTS];																\

// define the CharSetManager
__CLASS_DEFINITION(CharSetManager, Object);

__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


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
static void __attribute__ ((noinline)) CharSetManager_constructor(CharSetManager this)
{
	__CONSTRUCT_BASE(Object);

	int segment = 0;
	for(; segment < __CHAR_SEGMENTS; segment++)
	{
		this->freedOffset[segment] = 1;
		this->charSets[segment] = NULL;
	}

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
			VirtualNode node = this->charSets[i]->head;

			for(; node; node = node->next)
			{
				__DELETE(node->data);
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
			VirtualNode node = this->charSets[segment]->head;

			for(; node; node = node->next)
			{
				__DELETE(node->data);
			}

			__DELETE(this->charSets[segment]);
		}

		this->charSets[segment] = __NEW(VirtualList);
		this->freedOffset[segment] = 1;
	}
}

// find a charset
static CharSet CharSetManager_findCharSet(CharSetManager this, CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "CharSetManager::findCharSet: null this");

	// try to find a charset with the same char definition
	int i = 0;
	for(; i < __CHAR_SEGMENTS; i++)
	{
		VirtualNode node = this->charSets[i]->head;

		for(; node; node = node->next)
		{
			CharSet charSet = __SAFE_CAST(CharSet, node->data);

			if(CharSet_getCharSetDefinition(charSet)->charDefinition == charSetDefinition->charDefinition && CharSet_getAllocationType(charSet) == charSetDefinition->allocationType)
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

	switch(charSetDefinition->allocationType)
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

			if(charSet)
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
		u32 segment = CharSet_getSegment(charSet);
		u32 offset = CharSet_getOffset(charSet);

		if(1 == this->freedOffset[segment] || offset < this->freedOffset[segment])
		{
			this->freedOffset[segment] = offset;
		}

		VirtualList_removeElement(this->charSets[segment], charSet);

		__DELETE(charSet);
	}
}

// allocate a char definition within char graphic memory
static CharSet CharSetManager_allocateCharSet(CharSetManager this, CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "CharSetManager::allocateCharSet: null this");

	ASSERT(charSetDefinition->numberOfChars > 0, "CharSetManager::allocateCharSet: number of chars < 0");

	int segment = 0;

	for(; segment < __CHAR_SEGMENTS ; segment++)
	{
		ASSERT(this->charSets[segment], "CharSetManager::getNextFreeOffset: null this");

		u16 offset = 1;

		if(this->charSets[segment]->head)
		{
			CharSet lastCharSet = __SAFE_CAST(CharSet, VirtualList_back(this->charSets[segment]));
			offset += CharSet_getOffset(lastCharSet) + CharSet_getNumberOfChars(lastCharSet) + __CHAR_ROOM;
		}

		if((unsigned)offset + charSetDefinition->numberOfChars < __CHAR_SEGMENT_TOTAL_CHARS)
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

	int segment = 0;
	for(; segment < __CHAR_SEGMENTS ; segment++)
	{
		if(this->freedOffset[segment])
		{
			VirtualNode node = this->charSets[segment]->head;

			for(; node; node = node->next)
			{
				CharSet charSet = __SAFE_CAST(CharSet, node->data);

				if(this->freedOffset[segment] == CharSet_getOffset(charSet))
				{
					this->freedOffset[segment] = 1;
					return;
				}

				if(this->freedOffset[segment] < CharSet_getOffset(charSet))
				{
					Mem_copy((u32*)__CHAR_SEGMENT((u32)CharSet_getSegment(charSet)) + (((u32)CharSet_getOffset(charSet)) << 4), (u32*)(0), (u32)(CharSet_getNumberOfChars(charSet) + __CHAR_ROOM) << 4);

					CharSet_setOffset(charSet, this->freedOffset[segment]);
					//write to char memory
					CharSet_rewrite(charSet);
					this->freedOffset[segment] += CharSet_getNumberOfChars(charSet) + __CHAR_ROOM;
					return;
				}
			}

			this->freedOffset[segment] = 1;
		}
	}
}

int CharSetManager_getTotalUsedChars(CharSetManager this, int segment)
{
	ASSERT(this, "CharSetManager::getTotalFreeChars: null this");
	ASSERT((unsigned)segment < __CHAR_SEGMENTS, "CharSetManager::getTotalUsedChars: invalid segment");

	if(this->charSets[segment]->head)
	{
		CharSet lastCharSet = VirtualList_back(this->charSets[segment]);
		return (int)CharSet_getOffset(lastCharSet) + CharSet_getNumberOfChars(lastCharSet) + __CHAR_ROOM;
	}

	return 0;
}

int CharSetManager_getTotalFreeChars(CharSetManager this, int segment)
{
	ASSERT(this, "CharSetManager::getTotalFreeChars: null this");
	ASSERT((unsigned)segment < __CHAR_SEGMENTS, "CharSetManager::getTotalUsedChars: invalid segment");

	return __CHAR_SEGMENT_TOTAL_CHARS - CharSetManager_getTotalUsedChars(this, segment);
}

int CharSetManager_getTotalCharSets(CharSetManager this, int segment)
{
	ASSERT(this, "CharSetManager::getTotalCharSets: null this");
	ASSERT((unsigned)segment < __CHAR_SEGMENTS, "CharSetManager::getTotalUsedChars: invalid segment");

	return VirtualList_getSize(this->charSets[segment]);
}

// print class's attributes's states
void CharSetManager_print(CharSetManager this, int x, int y)
{
	ASSERT(this, "CharSetManager::print: null this");

	Printing_text(Printing_getInstance(), "CHAR MEMORY'S USAGE", x, y++, NULL);

	int totalCharSets = 0;
	int totalUsedChars = 0;
	int totalFreeChars = 0;
	int segment = 0;

	for(; segment < __CHAR_SEGMENTS; segment++)
	{
		totalCharSets += VirtualList_getSize(this->charSets[segment]);
		totalUsedChars += CharSetManager_getTotalUsedChars(this, segment);
		totalFreeChars += CharSetManager_getTotalFreeChars(this, segment);
	}

	Printing_text(Printing_getInstance(), "Total CharSets: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), totalCharSets, x + 18, y, NULL);
	Printing_text(Printing_getInstance(), "Total used chars: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), totalUsedChars, x + 18, y, NULL);
	Printing_text(Printing_getInstance(), "Total free chars: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), totalFreeChars, x + 18, y, NULL);
}
