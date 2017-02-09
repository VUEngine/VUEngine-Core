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

#include <CharSetManager.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <Mem.h>
#include <Printing.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define CharSetManager_ATTRIBUTES																		\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* charsets defined */																			\
		VirtualList charSets;																			\
		/* next offset to reclaim */																	\
		u16 freedOffset;																				\

/**
 * @class 	CharSetManager
 * @extends Object
 */
__CLASS_DEFINITION(CharSetManager, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void CharSetManager_constructor(CharSetManager this);
static CharSet CharSetManager_findCharSet(CharSetManager this, CharSetDefinition* charSetDefinition);
static CharSet CharSetManager_allocateCharSet(CharSetManager this, CharSetDefinition* charSetDefinition);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(CharSetManager);

// class's constructor
static void __attribute__ ((noinline)) CharSetManager_constructor(CharSetManager this)
{
	__CONSTRUCT_BASE(Object);

	this->charSets = __NEW(VirtualList);
	this->freedOffset = 1;
}

// class's destructor
void CharSetManager_destructor(CharSetManager this)
{
	ASSERT(this, "CharSetManager::destructor: null this");

	CharSetManager_reset(this);

	__DELETE(this->charSets);
	this->charSets = NULL;

	// allow a new construct
	__SINGLETON_DESTROY;
}

// reset
void CharSetManager_reset(CharSetManager this)
{
	ASSERT(this, "CharSetManager::reset: null this");

	if(this->charSets)
	{
		VirtualNode node = this->charSets->head;

		for(; node; node = node->next)
		{
			__DELETE(node->data);
		}

		VirtualList_clear(this->charSets);
	}

	this->freedOffset = 1;
}

// find a charset
static CharSet CharSetManager_findCharSet(CharSetManager this, CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "CharSetManager::findCharSet: null this");

	// try to find a charset with the same char definition
	VirtualNode node = this->charSets->head;

	for(; node; node = node->next)
	{
		CharSet charSet = __SAFE_CAST(CharSet, node->data);

		if(charSet && CharSet_getCharSetDefinition(charSet)->charDefinition == charSetDefinition->charDefinition && CharSet_getAllocationType(charSet) == charSetDefinition->allocationType)
		{
			return charSet;
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
				charSet = CharSetManager_allocateCharSet(this, charSetDefinition);
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
	ASSERT(this, "CharSetManager::releaseCharSet: null this");

	if(CharSet_decreaseUsageCount(charSet))
	{
		u32 offset = CharSet_getOffset(charSet);

		if(1 == this->freedOffset || offset < this->freedOffset)
		{
			this->freedOffset = offset;
		}

		VirtualList_removeElement(this->charSets, charSet);

		__DELETE(charSet);
	}
}

// allocate a char definition within char graphic memory
static CharSet CharSetManager_allocateCharSet(CharSetManager this, CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "CharSetManager::allocateCharSet: null this");
	ASSERT(this->charSets, "CharSetManager::allocateCharSet: null this");
	ASSERT(charSetDefinition->numberOfChars > 0, "CharSetManager::allocateCharSet: number of chars < 0");

	u16 offset = 1;

	if(this->charSets->head)
	{
		CharSet lastCharSet = __SAFE_CAST(CharSet, VirtualList_back(this->charSets));
		offset += CharSet_getOffset(lastCharSet) + CharSet_getNumberOfChars(lastCharSet) + __CHAR_ROOM;
	}

	if((unsigned)offset + charSetDefinition->numberOfChars < __CHAR_MEMORY_TOTAL_CHARS)
	{
		CharSet charSet = __NEW(CharSet, charSetDefinition, offset);

		CharSet_write(charSet);

		VirtualList_pushBack(this->charSets, charSet);

		return charSet;
	}

	// if there isn't enough memory thrown an exception
	NM_ASSERT(false, "CharSetManager::allocateCharSet: char mem depleted");

	return NULL;
}

// defragment char memory
void CharSetManager_defragment(CharSetManager this)
{
	ASSERT(this, "CharSetManager::defragment: null this");

	while(1 < this->freedOffset)
	{
		CharSetManager_defragmentProgressively(this);
	}
}

// defragment progressively char memory
void CharSetManager_defragmentProgressively(CharSetManager this)
{
	ASSERT(this, "CharSetManager::defragmentProgressively: null this");

	if(this->freedOffset)
	{
		VirtualNode node = this->charSets->head;

		for(; node; node = node->next)
		{
			CharSet charSet = __SAFE_CAST(CharSet, node->data);

			if(this->freedOffset == CharSet_getOffset(charSet))
			{
				this->freedOffset = 1;
				return;
			}

			if(this->freedOffset < CharSet_getOffset(charSet))
			{
				Mem_copy((u8*)__CHAR_SPACE_BASE_ADDRESS + (((u32)CharSet_getOffset(charSet)) << 4), (u8*)(0), (u32)(CharSet_getNumberOfChars(charSet) + __CHAR_ROOM) << 4);

				CharSet_setOffset(charSet, this->freedOffset);
				//write to char memory
				CharSet_rewrite(charSet);
				this->freedOffset += CharSet_getNumberOfChars(charSet) + __CHAR_ROOM;
				return;
			}
		}

		this->freedOffset = 1;
	}
}

int CharSetManager_getTotalUsedChars(CharSetManager this)
{
	ASSERT(this, "CharSetManager::getTotalFreeChars: null this");
	ASSERT(this->charSets, "CharSetManager::getTotalFreeChars: null charSets list");

	CharSet lastCharSet = VirtualList_back(this->charSets);
	return (int)CharSet_getOffset(lastCharSet) + CharSet_getNumberOfChars(lastCharSet) + __CHAR_ROOM;
}

int CharSetManager_getTotalFreeChars(CharSetManager this)
{
	ASSERT(this, "CharSetManager::getTotalFreeChars: null this");

	return __CHAR_MEMORY_TOTAL_CHARS - CharSetManager_getTotalUsedChars(this);
}

int CharSetManager_getTotalCharSets(CharSetManager this)
{
	ASSERT(this, "CharSetManager::getTotalCharSets: null this");

	return VirtualList_getSize(this->charSets);
}

// print class's attributes's states
void CharSetManager_print(CharSetManager this, int x, int y)
{
	ASSERT(this, "CharSetManager::print: null this");

	Printing_text(Printing_getInstance(), "CHAR MEMORY'S USAGE", x, y++, NULL);

	Printing_text(Printing_getInstance(), "Total CharSets: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->charSets), x + 18, y, NULL);
	Printing_text(Printing_getInstance(), "Total used chars: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), CharSetManager_getTotalUsedChars(this), x + 18, y, NULL);
	Printing_text(Printing_getInstance(), "Total free chars: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), CharSetManager_getTotalFreeChars(this), x + 18, y, NULL);
}
