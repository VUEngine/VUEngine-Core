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
	/* charsets defined */														\
	BYTE *charDefinition[__CHAR_SEGMENTS * __CHAR_GRP_PER_SEG];					\
																				\
	/* set whether a definition can be dropped or not */						\
	s8 charDefUsage[__CHAR_SEGMENTS * __CHAR_GRP_PER_SEG];						\
																				\
	/* registered char groups */												\
	VirtualList charSets;														\
																				\
	/* register every offset */													\
	u16 offset[__CHAR_SEGMENTS * __CHAR_GRP_PER_SEG];							\
																				\
	/* defragmentation flag */													\
	bool needsDefrag;															\

// define the CharSetManager
__CLASS_DEFINITION(CharSetManager, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void CharSetManager_constructor(CharSetManager this);
static int CharSetManager_searchCharDefinition(CharSetManager this, CharSet charSet);
static void CharSetManager_setCharDefinition(CharSetManager this, BYTE *charDefinition, int segment, u16 offset);
static u16 CharSetManager_getNextFreeOffset(CharSetManager this, int charSeg, u16 numberOfChars);
static void CharSetManager_deallocate(CharSetManager this, CharSet charSet);
static void CharSetManager_markFreedChars(CharSetManager this, int segment, u16 offset, u16 numberOfChars);
static void CharSetManager_markUsedChars(CharSetManager this, int segment, int offset, int numberOfChars);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(CharSetManager);

// class's constructor
static void CharSetManager_constructor(CharSetManager this)
{
	__CONSTRUCT_BASE();

	this->charSets = NULL;
	this->needsDefrag = false;

	CharSetManager_reset(this);
}

// class's destructor
void CharSetManager_destructor(CharSetManager this)
{
	ASSERT(this, "CharSetManager::destructor: null this");

	__DELETE(this->charSets);

	// allow a new construct
	__SINGLETON_DESTROY;
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

	if (this->charSets)
	{
		__DELETE(this->charSets);
	}

	this->charSets = __NEW(VirtualList);
	this->needsDefrag = false;
}

// record an allocated char defintion
static void CharSetManager_setCharDefinition(CharSetManager this, BYTE *charDefinition, int segment, u16 offset)
{
	ASSERT(this, "CharSetManager::setCharDefinition: null this");

	int i = __CHAR_GRP_PER_SEG * segment;

	// search where to register the char definition
	for (; i < __CHAR_GRP_PER_SEG * (segment + 1) && this->charDefinition[i]; i++);

	NM_ASSERT(i < __CHAR_GRP_PER_SEG * (segment + 1), "CharSetManager::setCharDefinition: no char definitions slots left");

	// save char definition
	this->charDefinition[i] = charDefinition;

	// set usage value of char defintion
	this->charDefUsage[i] = 1;

	// save char definition offset
	this->offset[i] = offset;
}

// release char graphic memory
void CharSetManager_free(CharSetManager this, CharSet charSet)
{
	ASSERT(this, "CharSetManager::free: null this");

	// retrieve index of char's defintion
	int i = CharSetManager_searchCharDefinition(this, charSet);

	// remove from char set list
	VirtualList_removeElement(this->charSets, charSet);

	// if char definition found
	if (0 <= i)
	{
		NM_ASSERT(0 < this->charDefUsage[i], "CharSetManager::free: deallocating unused char");
		
		this->charDefUsage[i]--;

		// if no other object uses the char defintion
		if (!this->charDefUsage[i])
		{
			// deallocate space
			CharSetManager_deallocate(this, charSet);

			// free char definition record
			this->charDefinition[i] = NULL;
			this->offset[i] = 0;
			this->needsDefrag = true;
		}
	}
}

// set number of chars used in a given segment
void CharSetManager_setChars(CharSetManager this, int segment, int numberOfChars)
{
	ASSERT(this, "CharSetManager::setChars: null this");

	// set the number of chars of the given charset
	this->segment[segment][0] = numberOfChars;
}

// print class's attributes's states
void CharSetManager_print(CharSetManager this, int x, int y)
{
	ASSERT(this, "CharSetManager::print: null this");

	Printing_text(Printing_getInstance(), "CHARACTER MEMORY", x, y++, NULL);
	Printing_text(Printing_getInstance(), "CharSets: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->charSets), x + 12, y++, NULL);
	y++;

	int segment = 0;
	int i = 0;
	for (; segment < 4; segment++)
	{
		Printing_text(Printing_getInstance(), "CharSeg", x, y, NULL);
		Printing_int(Printing_getInstance(), segment, x + 8, y, NULL);
		for (i = 0; i < __CHAR_GRP_PER_SEG && (y + i + 1) < 28; i++)
		{
			Printing_hex(Printing_getInstance(), this->segment[segment][i], x, y + i + 1, NULL);
		}

		x += 12;
	}
}

// look for an already writen char group
static int CharSetManager_searchCharDefinition(CharSetManager this, CharSet charSet)
{
	ASSERT(this, "CharSetManager::searchCharDefinition: null this");

	int i = 0;
	
	BYTE* charDef = CharSet_getCharDefinition(charSet);

	ASSERT(charDef, "CharSetManager::searchCharDefinition: null chardef in charset");

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
int CharSetManager_allocateShared(CharSetManager this, CharSet charSet)
{
	ASSERT(this, "CharSetManager::allocateShared: null this");

	// get the index if the charset is already defined
	int i = CharSetManager_searchCharDefinition(this, charSet);

	// register charSet
	VirtualList_pushBack(this->charSets, charSet);

	// verify that the char's definition is not already defined
	if (0 <= i)
	{
		BYTE* charDef = CharSet_getCharDefinition(charSet);
		VirtualNode node = VirtualList_begin(this->charSets);
		
		for (; node && charDef != CharSet_getCharDefinition(__UPCAST(CharSet, VirtualNode_getData(node))); node = VirtualNode_getNext(node));

		NM_ASSERT(node, "CharSetManager::allocateShared: lists unsynchronized");

		CharSet_setSegment(charSet, CharSet_getSegment(__UPCAST(CharSet, VirtualNode_getData(node))));

		// make charset point to it
		CharSet_setOffset(charSet, this->offset[i]);

		// increase char usage
		this->charDefUsage[i]++;

		return false;
	}

	// if not, then allocate
	CharSetManager_allocate(this, charSet);

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
static void CharSetManager_markUsedChars(CharSetManager this, int segment, int offset, int numberOfChars)
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
		this->segment[segment][auxJ] |= index;

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
void CharSetManager_allocate(CharSetManager this, CharSet charSet)
{
	ASSERT(this, "CharSetManager::allocate: null this");

	int numberOfChars = CharSet_getNumberOfChars(charSet);

	ASSERT(numberOfChars > 0, "CharSetManager::allocate: number of chars < 0");

	int i = 0;

	// if char is defined as part of an animation frame allocate
	// space for it
	for (; i < __CHAR_SEGMENTS ; i++)
	{
		u16 offset = CharSetManager_getNextFreeOffset(this, i, numberOfChars);

		if (0 < offset)
		{
			// set charset's offset
			CharSet_setOffset(charSet, offset);

			// record char defintion
			CharSetManager_setCharDefinition(this, CharSet_getCharDefinition(charSet), i, offset);

			// set charset's segment
			CharSet_setSegment(charSet, i);

			// register the used chars
			CharSetManager_markUsedChars(this, i, offset, numberOfChars);

			// stop processing
			return;
		}
	}

	// if there isn't enough memory trown an exception
	NM_ASSERT(false, "CharSetManager::allocate: char mem depleted");
}

// free char graphic memory
static void CharSetManager_deallocate(CharSetManager this, CharSet charSet)
{
	ASSERT(this, "CharSetManager::deallocate: null this");

	CharSetManager_markFreedChars(this, CharSet_getSegment(charSet), CharSet_getOffset(charSet), CharSet_getNumberOfChars(charSet) + 1);
}

// free char graphic memory
static void CharSetManager_markFreedChars(CharSetManager this, int segment, u16 offset, u16 numberOfChars)
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
		this->segment[segment][j] &= index;

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

	int segment = 0;
	for (; segment < __CHAR_SEGMENTS ; segment++)
	{
		u16 freeOffset = CharSetManager_getNextFreeOffset(this, segment, 1);

		if (0 < freeOffset)
		{
			VirtualNode charSetNode = VirtualList_begin(this->charSets);

			int lowestOffset = 10000;
			CharSet charSetToRewrite = NULL;

			for (; charSetNode; charSetNode = VirtualNode_getNext(charSetNode))
			{
				CharSet charSet = __UPCAST(CharSet, VirtualNode_getData(charSetNode));

				if (CharSet_getSegment(charSet) != segment)
				{
					continue;
				}

				ASSERT(charSet, "CharSetManager::defragmentProgressively: null charSet");

				int offset = CharSet_getOffset(charSet);

				if (offset > freeOffset)
				{
					if (offset < lowestOffset)
					{
						lowestOffset = offset;
						charSetToRewrite = charSet;
					}
				}
			}

			if (charSetToRewrite)
			{
				int previousOffset = CharSet_getOffset(charSetToRewrite);
				CharSet_setOffset(charSetToRewrite, freeOffset);

				// register the used chars
				CharSetManager_markUsedChars(this, CharSet_getSegment(charSetToRewrite), freeOffset, CharSet_getNumberOfChars(charSetToRewrite));
				CharSetManager_markFreedChars(this, CharSet_getSegment(charSetToRewrite), freeOffset + CharSet_getNumberOfChars(charSetToRewrite), previousOffset - freeOffset);

				BYTE* charDefinition = CharSet_getCharDefinition(charSetToRewrite);

				int i = 0;
				for (; i < __CHAR_SEGMENTS * __CHAR_GRP_PER_SEG; i++)
				{
					if (charDefinition == this->charDefinition[i])
					{
						this->offset[i] = CharSet_getOffset(charSetToRewrite);
						ASSERT(0 <= this->offset[segment], "CharSetManager::defragmentProgressively: offset less than 0")
						break;
					}
				}

				CharSet_rewrite(charSetToRewrite);
				return;
			}
		}
	}

	this->needsDefrag = false;
}