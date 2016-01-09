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

#include <CharSet.h>
#include <CharSetManager.h>
#include <MessageDispatcher.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(CharSet, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals

static void CharSet_constructor(CharSet this, CharSetDefinition* charSetDefinition, u8 segment, u16 offset);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(CharSet, CharSetDefinition* charSetDefinition, u8 segment, u16 offset)
__CLASS_NEW_END(CharSet, charSetDefinition, segment, offset)

// class's constructor
static void CharSet_constructor(CharSet this, CharSetDefinition* charSetDefinition, u8 segment, u16 offset)
{
	__CONSTRUCT_BASE();

	// save definition
	this->charSetDefinition = charSetDefinition;
	this->charDefinitionDisplacement = 0;

	// set the offset
	this->offset = offset;
	this->segment = segment;
	this->usageCount = 1;
}

// class's destructor
void CharSet_destructor(CharSet this)
{
	ASSERT(this, "CharSet::destructor: null this");

	// make sure that I'm not destroyed again
	this->usageCount = 0xFF;

	// free processor memory
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

void CharSet_increaseUsageCount(CharSet this)
{
	ASSERT(this, "CharSet::increaseUsageCoung: null this");
	ASSERT(255 > (int)this->usageCount, "CharSet::increaseUsageCoung: null this");

	this->usageCount++;
}

bool CharSet_decreaseUsageCount(CharSet this)
{
	ASSERT(this, "CharSet::getAllocationType: null this");

	return 0 == --this->usageCount;
}


// retrieve charset's allocation type
int CharSet_getAllocationType(CharSet this)
{
	ASSERT(this, "CharSet::getAllocationType: null this");

	return this->charSetDefinition->allocationType;
}

// retrieve charset's offset within char segment
u16 CharSet_getOffset(CharSet this)
{
	ASSERT(this, "CharSet::getOffset: null this");

	return this->offset;
}

// set charset's offset within the char segment
void CharSet_setOffset(CharSet this, u16 offset)
{
	ASSERT(this, "CharSet::setOffset: null this");
	ASSERT(0 <= offset, "CharSet::setOffset: offset less than 0");

	this->offset = offset;
}

// get charset's char definition
CharSetDefinition* CharSet_getCharSetDefinition(CharSet this)
{
	ASSERT(this, "CharSet::getCharDefinition: null this");

	return this->charSetDefinition;
}

// set charset's char definition
void CharSet_setCharSetDefinition(CharSet this, CharSetDefinition* charSetDefinition)
{
	ASSERT(this, "CharSet::setCharDefinition: null this");

	this->charSetDefinition = charSetDefinition;
}

// retrieve chargrop's number of chars
u16 CharSet_getNumberOfChars(CharSet this)
{
	ASSERT(this, "CharSet::getNumberOfChars: null this");

	return this->charSetDefinition->numberOfChars;
}

// get charset's segment
u8 CharSet_getSegment(CharSet this)
{
	ASSERT(this, "CharSet::getSegment: null this");

	return this->segment;
}

// write char on memory
void CharSet_write(CharSet this)
{
	ASSERT(this, "CharSet::write: null this");

	//write to char memory
	Mem_copy((u8*)CharSegs((u32)this->segment) + (((u32)this->offset) << 4), (u8*)(this->charSetDefinition->charDefinition + this->charDefinitionDisplacement), (u32)(this->charSetDefinition->numberOfChars + __CHAR_ROOM) << 4);
}

// rewrite char on memory
void CharSet_rewrite(CharSet this)
{
	ASSERT(this, "CharSet::rewrite: null this");

	// write again
	CharSet_write(this);

	// propagate event
	Object_fireEvent(__SAFE_CAST(Object, this), __EVENT_CHARSET_REWRITTEN);
}

// set charDefinitionDisplacement
void CharSet_setCharDefinitionDisplacement(CharSet this, u16 charDefinitionDisplacement)
{
	ASSERT(this, "CharSet::setCharDefinitionDisplacement: null this");

	this->charDefinitionDisplacement = charDefinitionDisplacement;
}

// write to char memory
void CharSet_putChar(CharSet this, u16 charToReplace, BYTE* newChar)
{
	ASSERT(this, "CharSet::putChar: null this");

	if(newChar && charToReplace < this->charSetDefinition->numberOfChars + __CHAR_ROOM)
	{
		Mem_copy((u8*)CharSegs((u32)this->segment) + (((u32)this->offset) << 4) + (charToReplace << 4), newChar, (int)(1 << 4));
	}
}

// write to char memory
void CharSet_putPixel(CharSet this, u16 charToReplace, Point* charSetPixel, BYTE newPixelColor)
{
	ASSERT(this, "CharSet::putPixel: null this");

	if(charSetPixel && charToReplace < this->charSetDefinition->numberOfChars + __CHAR_ROOM && (unsigned)charSetPixel->x < 8 && (unsigned)charSetPixel->y < 8)
	{
		static BYTE auxChar[] = 
		{
			0x00, 0x00,
			0x00, 0x00,
			0x00, 0x00,  
			0x00, 0x00,
			0x00, 0x00,
			0x00, 0x00,
			0x00, 0x00,
			0x00, 0x00,
		};

		Mem_copy(auxChar, (u8*)CharSegs((u32)this->segment) + (((u32)this->offset) << 4) + (charToReplace << 4), (int)(1 << 4));

		u16 displacement = (charSetPixel->y << 1) + (charSetPixel->x >> 2);
		u16 pixelToReplaceDisplacement = (charSetPixel->x % 4) << 1;
		auxChar[displacement] &= (~(0x03 << pixelToReplaceDisplacement) | ((u16)newPixelColor << pixelToReplaceDisplacement));
		Mem_copy((u8*)CharSegs((u32)this->segment) + (((u32)this->offset) << 4) + (charToReplace << 4), auxChar, (int)(1 << 4));
	}
}