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

#include <CharSet.h>
#include <CharSetManager.h>
#include <MessageDispatcher.h>


//---------------------------------------------------------------------------------------------------------
// 											 CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------

#define NUM_CHARS_MASK		(u32)0x000001FF
#define OFFSET_MASK			(u32)0x0003FE00
#define ALLOC_TYPE_MASK		(u32)0x003C0000
#define CHARSET_MASK		(u32)0x00C00000

#define __CH_NOT_ALLOCATED	0x200


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(CharSet, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

//class's constructor
static void CharSet_constructor(CharSet this, CharSetDefinition* charSetDefinition, Object owner);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(CharSet, CharSetDefinition* charSetDefinition, Object owner)
__CLASS_NEW_END(CharSet, charSetDefinition, owner)

// class's constructor
static void CharSet_constructor(CharSet this, CharSetDefinition* charSetDefinition, Object owner)
{
	__CONSTRUCT_BASE();

	// save definition
	this->charDefinition = charSetDefinition->charDefinition;
	this->owner = owner;

	// set number of chars
	CharSet_setNumberOfChars(this, charSetDefinition->numberOfChars);

	// set the offset
	this->offset = __CH_NOT_ALLOCATED;

	// set the allocation type
	this->allocationType = charSetDefinition->allocationType;

	this->charDefinitionDisplacement = 0;
}

// class's destructor
void CharSet_destructor(CharSet this)
{
	ASSERT(this, "CharSet::destructor: null this");

	// first check if the charset's definition is valid
	if (this->charDefinition)
	{
		//free char graphic memory
		CharSetManager_free(CharSetManager_getInstance(), this);
	}

	// free processor memory
	__DESTROY_BASE;
}

// retrieve charset's allocation type
int CharSet_getAllocationType(CharSet this)
{
	ASSERT(this, "CharSet::getAllocationType: null this");

	return this->allocationType;
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
BYTE* CharSet_getCharDefinition(CharSet this)
{
	ASSERT(this, "CharSet::getCharDefinition: null this");

	return this->charDefinition;
}

// set charset's char definition
void CharSet_setCharDefinition(CharSet this, void *charDefinition)
{
	ASSERT(this, "CharSet::setCharDefinition: null this");

	this->charDefinition = charDefinition;
}

// set charset's number of chars
void CharSet_setNumberOfChars(CharSet this, int numberOfChars)
{
	ASSERT(this, "CharSet::setNumberOfChars: null this");

	this->numberOfChars = numberOfChars;
}

// retrieve chargrop's number of chars
int CharSet_getNumberOfChars(CharSet this)
{
	ASSERT(this, "CharSet::getNumberOfChars: null this");

	return this->numberOfChars;
}

// get charset's segment
int CharSet_getSegment(CharSet this)
{
	ASSERT(this, "CharSet::getCharSet: null this");

	return this->segment;
}

// set charset's char segment
void CharSet_setSegment(CharSet this, int segment)
{
	ASSERT(this, "CharSet::setCharSet: null this");

	this->segment = segment;
}

//copy a charset
void CharSet_copy(CharSet this,CharSet source)
{
	ASSERT(this, "CharSet::copy: null this");

	// copy the definition
	this->charDefinition = source->charDefinition;

	// copy the configuration
	this->segment = source->segment;
	this->offset = source->offset;
	this->allocationType = source->allocationType;
	this->numberOfChars = source->numberOfChars;
}

// write char on memory
void CharSet_write(CharSet this)
{
	ASSERT(this, "CharSet::write: null this");

	// determine allocation type
	switch (this->allocationType)
	{
		case __ANIMATED:

			//if not allocated
			if (__CH_NOT_ALLOCATED == (int)this->offset)
			{
				// ask for allocation
				CharSetManager_allocate(CharSetManager_getInstance(), this);
			}

			//write to char memory
			Mem_copy((u8*)CharSegs(this->segment) + (this->offset << 4), (u8*)(this->charDefinition + this->charDefinitionDisplacement), (int)(this->numberOfChars + 1) << 4);

			break;

		case __ANIMATED_SHARED:
		case __NO_ANIMATED:

			//if not allocated
			if (__CH_NOT_ALLOCATED == (int)this->offset)
			{
				// ask for allocation
				if (CharSetManager_allocateShared(CharSetManager_getInstance(), this))
				{
					//write to char memory
					Mem_copy((u8*)CharSegs(this->segment)  + (this->offset << 4), (u8*)this->charDefinition, (int)(this->numberOfChars + 1) << 4);
				}
			}
			else
			{
				//write to char memory
				Mem_copy((u8*)CharSegs(this->segment)  + (this->offset << 4), (u8*)this->charDefinition, (int)(this->numberOfChars + 1) << 4);
			}
			break;

		default:

			ASSERT(false, "CharSet::write: with no allocation type");
	}
}

// rewrite char on memory
void CharSet_rewrite(CharSet this)
{
	ASSERT(this, "CharSet::rewrite: null this");

	VPUManager_waitForFrame(VPUManager_getInstance());

	// write again
	CharSet_write(this);

	// inform my owner
	MessageDispatcher_dispatchMessage(0, (Object)this, this->owner, kCharSetRewritten, NULL);
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

	if(newChar && charToReplace < this->numberOfChars)
	{
		Mem_copy((u8*)CharSegs(this->segment) + (this->offset << 4) + (charToReplace << 4), newChar, (int)(1 << 4));
	}
}

// write to char memory
void CharSet_putPixel(CharSet this, u16 charToReplace, Point* charSetPixel, BYTE newPixelColor)
{
	ASSERT(this, "CharSet::putPixel: null this");

	if(charSetPixel && charToReplace < this->numberOfChars && (unsigned)charSetPixel->x < 8 && (unsigned)charSetPixel->y < 8)
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

		Mem_copy(auxChar, (u8*)CharSegs(this->segment) + (this->offset << 4) + (charToReplace << 4), (int)(1 << 4));

		u16 displacement = (charSetPixel->y << 1) + (charSetPixel->x >> 2);
		u16 pixelToReplaceDisplacement = (charSetPixel->x % 4) << 1;
		auxChar[displacement] &= (~(0x03 << pixelToReplaceDisplacement) | ((u16)newPixelColor << pixelToReplaceDisplacement));
		Mem_copy((u8*)CharSegs(this->segment) + (this->offset << 4) + (charToReplace << 4), auxChar, (int)(1 << 4));
	}
}