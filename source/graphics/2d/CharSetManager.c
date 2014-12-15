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

#define __CHAR_ROOM		0

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define CharSetManager_ATTRIBUTES												\
																				\
	/* super's attributes */													\
	Object_ATTRIBUTES;															\
																				\
	/* 4 segments, each one with 512 bits  of mask */							\
	u32 segment[__CHAR_SEGMENTS][__CHAR_SEGMENT_SIZE];							\
																				\
	/* chargroups defined */													\
	BYTE *charDefinition[__CHAR_SEGMENTS * __CHAR_GRP_PER_SEG];					\
																				\
	/* set whether a definition can be dropped or not */						\
	u8 charDefUsage[__CHAR_SEGMENTS * __CHAR_GRP_PER_SEG];						\
																				\
	/* registered char groups */												\
	VirtualList charGroups	;													\
																				\
	/* register every offset */													\
	u16 offset[__CHAR_SEGMENTS * __CHAR_GRP_PER_SEG];							\
																				\
	/* defragmentation flag */													\
	u8 needsDefrag;																\

// define the CharSetManager
__CLASS_DEFINITION(CharSetManager);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void CharSetManager_constructor(CharSetManager this);

// look for an already writen char group
static int CharSetManager_searchCharDefinition(CharSetManager this, CharGroup charGroup);

// record an allocated char defintion
static void CharSetManager_setCharDefinition(CharSetManager this, BYTE *charDefinition, u16 offset);

// find a hole long enough to fit the number of chars
static u16 CharSetManager_getNextFreeOffset(CharSetManager this, int charSeg, u16 numberOfChars);

// free char graphic memory
static void CharSetManager_deallocate(CharSetManager this, CharGroup charGroup);

// free char graphic memory
static void CharSetManager_markFreedChars(CharSetManager this, int charSet, u16 offset, u16 numberOfChars);

//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(CharSetManager);


// class's constructor
static void CharSetManager_constructor(CharSetManager this)
{
	__CONSTRUCT_BASE(Object);

	this->charGroups = NULL;
	this->needsDefrag = false;

	CharSetManager_reset(this);
}

// class's destructor
void CharSetManager_destructor(CharSetManager this)
{
	ASSERT(this, "CharSetManager::destructor: null this");

	__DELETE(this->charGroups);

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

// reset
void CharSetManager_reset(CharSetManager this)
{
	ASSERT(this, "CharSetManager::reset: null this");

	int i = 0;
	int j = 0;

	// clear each segment's ofssets
	for (; i < __CHAR_SEGMENTS; i++)
	{
		for (j = 0; j < __CHAR_SEGMENT_SIZE; j++)
		{
			this->segment[i][j] = 0;
		}
	}

	// clear all definitions and usage
	for ( i = 0; i < __CHAR_SEGMENTS * __CHAR_GRP_PER_SEG; i++)
	{
		this->charDefinition[i] = NULL;
		this->charDefUsage[i] = 0;
		this->offset[i] = 0;
	}

	if (this->charGroups)
	{
		__DELETE(this->charGroups);
	}

	this->charGroups = __NEW(VirtualList);
	this->needsDefrag = false;
}

// record an allocated char defintion
static void CharSetManager_setCharDefinition(CharSetManager this, BYTE *charDefinition, u16 offset)
{
	ASSERT(this, "CharSetManager::setCharDefinition: null this");

	int i = 0;

	// search where to register the char definition
	for (; i < __CHAR_SEGMENTS * __CHAR_GRP_PER_SEG && this->charDefinition[i]; i++);

	ASSERT(i < __CHAR_SEGMENTS * __CHAR_GRP_PER_SEG, "CharSetManager::setCharDefinition: no char definitions slots left");

	// save char definition
	this->charDefinition[i] = charDefinition;

	// set usage value of char defintion
	this->charDefUsage[i] = 1;

	// save char definition offset
	this->offset[i] = offset;
}

// release char graphic memory
void CharSetManager_free(CharSetManager this, CharGroup charGroup)
{
	ASSERT(this, "CharSetManager::free: null this");

	// retrieve index of char's defintion
	int i = CharSetManager_searchCharDefinition(this, charGroup);

	// if char found
	if (i >= 0)
	{
		//decrease char definition usage
		this->charDefUsage[i]--;

		// just make sure it is not going in a loop
		if (0xFE < this->charDefUsage[i])
		{
			this->charDefUsage[i] = 0;
		}

		// if no other object uses the char defintion
		if (!this->charDefUsage[i])
		{
			// deallocate space
			CharSetManager_deallocate(this, charGroup);

			// free char definition record
			this->charDefinition[i] = NULL;

			this->needsDefrag = true;
		}
	}
}

// set number of chars used in a given segment
void CharSetManager_setChars(CharSetManager this, int charSet, int numberOfChars)
{
	ASSERT(this, "CharSetManager::setChars: null this");

	// set the number of chars of the given charset
	this->segment[charSet][0] = numberOfChars;
}

// print class's attributes's states
void CharSetManager_print(CharSetManager this, int x, int y)
{
	ASSERT(this, "CharSetManager::print: null this");

	Printing_text("CHAR MEMORY'S USAGE", x, y++);
	Printing_text("CharGroups: ", x, ++y);
	Printing_int(VirtualList_getSize(this->charGroups), x + 12, y++);
	y++;

	int charSet = 0;
	int i = 0;
	for (; charSet < 4; charSet++)
	{
		Printing_text("CharSeg", x, y);
		Printing_int(charSet, x + 8, y);
		for (i = 0; i < __CHAR_GRP_PER_SEG && (y + i + 1) < 28; i++)
		{
			Printing_hex(this->segment[charSet][i], x, y + i + 1);
		}

		x += 12;
	}
}

// look for an already writen char group
static int CharSetManager_searchCharDefinition(CharSetManager this, CharGroup charGroup)
{
	ASSERT(this, "CharSetManager::searchCharDefinition: null this");

	int i = 0;
	//
	BYTE* charDef = CharGroup_getCharDefinition(charGroup);

	ASSERT(charDef, "CharSetManager::searchCharDefinition: null chardef in chargroup");

	for (; i < __CHAR_SEGMENTS * __CHAR_GRP_PER_SEG; i++)
	{
		// if char's definition matches
		if (this->charDefinition[i] == charDef)
		{
			// return the index
			return i;
		}
	}

	// otherwise return an invalid index
	return -1;
}

// if char if part of a background or oder object whose frame doesn't change
int CharSetManager_allocateShared(CharSetManager this, CharGroup charGroup)
{
	ASSERT(this, "CharSetManager::allocateShared: null this");

	// get the index if the chargroup is already defined
	int i = CharSetManager_searchCharDefinition(this, charGroup);

	// verify that the char's definition is not already defined
	if (0 <= i)
	{
		// make chargroup point to it
		CharGroup_setOffset(charGroup, this->offset[i]);

		// increase char usage
		this->charDefUsage[i]++;

		return false;
	}

	// if not, then allocate
	CharSetManager_allocate(this, charGroup);

	return true;
}

// find a hole long enough to fit the number of chars
static u16 CharSetManager_getNextFreeOffset(CharSetManager this, int charSeg, u16 numberOfChars)
{
	ASSERT(this, "CharSetManager::getNextFreeOffset: null this");

	int i = charSeg;
	int j = 0;
	int hole = 0;
	unsigned int index = 0;
	unsigned int block = 0;
	// the 0 char is always used to represent black
	int currentChar = 1;

	ASSERT(numberOfChars > 0, "CharSetManager::allocate: number of chars < 0");

	// if char is defined as part of an animation frame allocate
	// space for it
	CACHE_ENABLE;

	// each segment has 512 chars slots so, 16 ints are 512 bits
	for (j = 0; j < 16; j++)
	{
		// see if there is a 0 in the block
		block = this->segment[i][j] ^ 0xFFFFFFFF;

		// if there is at least a 1 in the block
		if (block)
		{
			// set index 1000 0000 0000 0000 2b
			index = 0x80000000;

			// while there is at least a 1 in the block
			while (index)
			{
				// in-block offset
				if (block & index)
				{
					// increase the hole
					hole++;

					// control if the first free char is the last one from a block
					// if hole fits numberOfChars plus one free space at the begining and end
					if (hole >= numberOfChars + 1)
	                {
						// stop processing
						return currentChar - numberOfChars;
					}
				}
				else
				{
					// otherwise clear hole must be cleared
					hole = 0;
				}

				// shift the block to the right
				index >>= 1;

				// increase the in-block offset
				currentChar++;
			}
		}
		else
		{
			// move current block, 32 slots ahead
			currentChar += 32;
		}
	}

	return 0;
}

// register the used chars
void CharSetManager_markUsedChars(CharSetManager this, int charSeg, int offset, int numberOfChars)
{
	ASSERT(this, "CharSetManager::markUsedChars: null this");

	int auxJ = 0;
	unsigned int index = 0;
	int counter;

	// determine the index to mark the offset
	auxJ = offset >> 5;

	// calculate the total slots
	counter = offset - (auxJ << 5);

	// initilize mask
	index = 0x80000000;

	while (counter--)
	{
		// fill the mask acording to number of slots
		index >>= 1;
	}

	// mark segmant mask's used slots
	while (numberOfChars--)
	{
		this->segment[charSeg][auxJ] |= index;

		index >>= 1;

		// reset the mask and increase the segment number
		if (!index)
		{
			index = 0x80000000;

			auxJ++;
		}
	}
}

// allocate a char defintion within char graphic memory
void CharSetManager_allocate(CharSetManager this, CharGroup charGroup)
{
	ASSERT(this, "CharSetManager::allocate: null this");

	int numberOfChars = CharGroup_getNumberOfChars(charGroup);

	ASSERT(numberOfChars > 0, "CharSetManager::allocate: number of chars < 0");

	int i = 0;

	// if char is defined as part of an animation frame allocate
	// space for it
	CACHE_ENABLE;
	for (; i < __CHAR_SEGMENTS ; i++)
	{
		u16 offset = CharSetManager_getNextFreeOffset(this, i, numberOfChars);

		if (0 < offset)
		{
			// set chargroup's offset
			CharGroup_setOffset(charGroup, offset);

			// record char defintion
			CharSetManager_setCharDefinition(this, CharGroup_getCharDefinition(charGroup), offset);

			// set chargroup's segment
			CharGroup_setCharSet(charGroup, i);

			// register the used chars
			CharSetManager_markUsedChars(this, i, offset, numberOfChars);

			// register charGroup
			VirtualList_pushBack(this->charGroups, charGroup);

			CACHE_DISABLE;

			// stop processing
			return;
		}
	}
	CACHE_DISABLE;

	// if there isn't enough memory trown an exception
	ASSERT(false, "CharSetManager::allocate: char mem depleted");
}

// free char graphic memory
static void CharSetManager_deallocate(CharSetManager this, CharGroup charGroup)
{
	ASSERT(this, "CharSetManager::deallocate: null this");

	CharSetManager_markFreedChars(this, CharGroup_getCharSet(charGroup), CharGroup_getOffset(charGroup), CharGroup_getNumberOfChars(charGroup) + 1);

	VirtualList_removeElement(this->charGroups, charGroup);
}

// free char graphic memory
static void CharSetManager_markFreedChars(CharSetManager this, int charSet, u16 offset, u16 numberOfChars)
{
	ASSERT(this, "CharSetManager::markFreedChars: null this");

	// counter of chars
	int counter = 0;

	// initialize mask
	u32 index = 0x80000000;

	// calculate block index
	int j = 0;
	j = offset >> 5;

	// calculate number of slots
	counter = offset - (j << 5);

	// while there are chars
	while (counter--)
	{
		// fill mask
		index >>= 1;
	}

	// inverse the mask
	index ^= 0xFFFFFFFF;

	// clear freeded slots within the segment
	CACHE_ENABLE;
	while (numberOfChars--)
	{
		this->segment[charSet][j] &= index;

		index >>= 1;

		index |= 0x80000000;

		if (index == 0xFFFFFFFF)
		{
			index = 0x7FFFFFFF;

			j++;
		}
	}
	CACHE_DISABLE;
}

// defrag char memory
void CharSetManager_defragmentProgressively(CharSetManager this)
{
	ASSERT(this, "CharSetManager::defragmentProgressively: null this");

	if (!this->needsDefrag)
	{
		return;
	}

	int charSet = 0;
	for (; charSet < __CHAR_SEGMENTS ; charSet++)
	{
		u16 freeOffset = CharSetManager_getNextFreeOffset(this, charSet, 1);

		if (0 < freeOffset)
		{
			VirtualNode charGroupNode = VirtualList_begin(this->charGroups);

			int lowestOffset = 10000;
			CharGroup charGroupToRewrite = NULL;

			for (; charGroupNode; charGroupNode = VirtualNode_getNext(charGroupNode))
			{
				CharGroup charGroup = (CharGroup)VirtualNode_getData(charGroupNode);

				if (CharGroup_getCharSet(charGroup) != charSet)
				{
					continue;
				}

				ASSERT(charGroup, "CharSetManager::defragmentProgressively: null charGroup");

				int offset = CharGroup_getOffset(charGroup);

				if (offset > freeOffset)
				{
					if (offset < lowestOffset)
					{
						lowestOffset = offset;
						charGroupToRewrite = charGroup;
					}
				}
			}

			if (charGroupToRewrite)
			{
				int previousOffset = CharGroup_getOffset(charGroupToRewrite);
				CharGroup_setOffset(charGroupToRewrite, freeOffset);

				// register the used chars
				CharSetManager_markUsedChars(this, CharGroup_getCharSet(charGroupToRewrite), freeOffset, CharGroup_getNumberOfChars(charGroupToRewrite));
				CharSetManager_markFreedChars(this, CharGroup_getCharSet(charGroupToRewrite), freeOffset + CharGroup_getNumberOfChars(charGroupToRewrite), previousOffset - freeOffset);

				BYTE* charDefinition = CharGroup_getCharDefinition(charGroupToRewrite);

				int i = 0;
				for (; i < __CHAR_SEGMENTS * __CHAR_GRP_PER_SEG; i++)
				{
					if (charDefinition == this->charDefinition[i])
					{
						this->offset[i] = CharGroup_getOffset(charGroupToRewrite);
						ASSERT(0 <= this->offset[charSet], "CharSetManager::defragmentProgressively: offset less than 0")
						break;
					}
				}

				CharGroup_rewrite(charGroupToRewrite);
				return;
			}
		}
	}

	this->needsDefrag = false;
}