/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <CharGroup.h>
#include <CharSetManager.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define NUM_CHARS_MASK		(u32)0x000001FF
#define OFFSET_MASK			(u32)0x0003FE00
#define ALLOC_TYPE_MASK		(u32)0x003C0000
#define CHARSET_MASK		(u32)0x00C00000


#define __CH_NOT_ALLOCATED		0x200


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

__CLASS_DEFINITION(CharGroup);



/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//class's constructor
static void CharGroup_constructor(CharGroup this, CharGroupDefinition* charGroupDefinition);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// always call these to macros next to each other
__CLASS_NEW_DEFINITION(CharGroup, __PARAMETERS(CharGroupDefinition* charGroupDefinition))
__CLASS_NEW_END(CharGroup, __ARGUMENTS(charGroupDefinition))

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void CharGroup_constructor(CharGroup this, CharGroupDefinition* charGroupDefinition){
	
	__CONSTRUCT_BASE(Object);	

	// save definition
	this->charDefinition = charGroupDefinition->charDefinition;
	
	// set number of chars
	CharGroup_setNumberOfChars(this, charGroupDefinition->numberOfChars);	
	
	// set the offset
	this->offset = __CH_NOT_ALLOCATED;

	// set the allocation type
	this->allocationType = charGroupDefinition->allocationType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void CharGroup_destructor(CharGroup this){
	
	ASSERT(this, "CharGroup::destructor: null this");

	// first check if the chargroup's definition is valid
	if(this->charDefinition){	

		//free char graphic memory
		CharSetManager_free(CharSetManager_getInstance(), this);
	}
	
	// free processor memory
	__DESTROY_BASE(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve chargroup's allocation type
int CharGroup_getAllocationType(CharGroup this){
	
	ASSERT(this, "CharGroup::getAllocationType: null this");

	return this->allocationType;	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve chargroup's offset within char segment
int CharGroup_getOffset(CharGroup this){
	
	ASSERT(this, "CharGroup::getOffset: null this");

	return this->offset;	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set chargroup's offset within the char segment
void CharGroup_setOffset(CharGroup this, int offset){
	
	ASSERT(this, "CharGroup::setOffset: null this");

	this->offset = offset;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get chargroup's char definition	
BYTE* CharGroup_getCharDefinition(CharGroup this){
	
	ASSERT(this, "CharGroup::getCharDefinition: null this");

	return this->charDefinition;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set chargroup's char definition
void CharGroup_setCharDefinition(CharGroup this, void *charDefinition){
	
	ASSERT(this, "CharGroup::setCharDefinition: null this");

	this->charDefinition = charDefinition;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set chargroup's number of chars
void CharGroup_setNumberOfChars(CharGroup this, int numberOfChars){
	
	ASSERT(this, "CharGroup::setNumberOfChars: null this");

	this->numberOfChars = numberOfChars;	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve chargrop's number of chars
int CharGroup_getNumberOfChars(CharGroup this){
	
	ASSERT(this, "CharGroup::getNumberOfChars: null this");

	return this->numberOfChars;	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get chargroup's segment 
int CharGroup_getCharSet(CharGroup this){
	
	ASSERT(this, "CharGroup::getCharSet: null this");

	return this->charset;	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set chargroup's char segment
void CharGroup_setCharSet(CharGroup this, int charSet){
		
	ASSERT(this, "CharGroup::setCharSet: null this");

	this->charset = charSet;	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//copy a chargroup
void CharGroup_copy(CharGroup this,CharGroup source){
	
	ASSERT(this, "CharGroup::destructor: null this");

	// copy the definition
	this->charDefinition = source->charDefinition;
	
	// copy the configuration
	this->charset = source->charset;
	this->offset = source->offset;
	this->allocationType = source->allocationType;
	this->numberOfChars = source->numberOfChars;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write char on memory	
void CharGroup_write(CharGroup this){

	ASSERT(this, "CharGroup::write: null this");

	// determine allocation type
	switch(this->allocationType){
	
		case __ANIMATED:
					
			//if not allocated
			if(__CH_NOT_ALLOCATED == (int)this->offset){
				
				// ask for allocation
				CharSetManager_allocate(CharSetManager_getInstance(), this);				
			}	

			//write to char memory
			Mem_copy((u8*)CharSegs(this->charset) + (this->offset << 4), (u8*)this->charDefinition, (int)this->numberOfChars << 4);

			break;
			
		case __ANIMATED_SHARED:
		case __NO_ANIMATED:

			//if not allocated
			if(__CH_NOT_ALLOCATED == (int)this->offset){
				
				// ask for allocation
				if(CharSetManager_allocateShared(CharSetManager_getInstance(), this)){
			
					//write to char memory
					Mem_copy((u8*)CharSegs(this->charset)  + ( this->offset << 4), (u8*)this->charDefinition, (int)this->numberOfChars << 4);
				}
			}	
			break;
			
		default:

			ASSERT(false, "CharGroup::copy: with no allocation type");
	}
}
