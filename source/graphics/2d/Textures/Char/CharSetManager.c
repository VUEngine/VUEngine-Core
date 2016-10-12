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
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define CharSetManager_ATTRIBUTES																		\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /* charsets defined */																			\
        VirtualList charSets;															                \
        /* next offset to reclaim */																	\
        u16 freedOffset;																                \

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

        if(CharSet_getCharSetDefinition(charSet)->charDefinition == charSetDefinition->charDefinition && CharSet_getAllocationType(charSet) == charSetDefinition->allocationType)
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

	// if there isn't enough memory trown an exception
	NM_ASSERT(false, "CharSetManager::allocateCharSet: char mem depleted");

	return NULL;
}

// defrag char memory
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
