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

#include <CharSetManager.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define CharSetManager_ATTRIBUTES							\
															\
	/* super's attributes */								\
	Object_ATTRIBUTES;										\
															\
	/* 4 segments, each one with 512 bits  of mask */		\
	u32 segment[CHAR_SEGMENTS][CHAR_SEGMENT_SIZE];			\
															\
	/* chargroups defined */								\
	BYTE *charDefinition[CHAR_SEGMENTS * CHAR_GRP_PER_SEG];	\
															\
	/* set whether a definition can be dropped or not */	\
	int charDefUsage[CHAR_SEGMENTS * CHAR_GRP_PER_SEG];		\
															\
	/* register every offset */								\
	int offset[CHAR_SEGMENTS * CHAR_GRP_PER_SEG];

// define the CharSetManager
__CLASS_DEFINITION(CharSetManager);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void CharSetManager_constructor(CharSetManager this);

// look for an already writen char group
static int CharSetManager_searchCharDefinition(CharSetManager this, CharGroup charGroup);

// record an allocated char defintion
static void CharSetManager_setCharDefinition(CharSetManager  this, BYTE *charDefinition, int offset);
 
// free char graphic memory
static void CharSetManager_deallocate(CharSetManager this, CharGroup charGroup);

 
 
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
__SINGLETON(CharSetManager);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void CharSetManager_constructor(CharSetManager this){
	
	__CONSTRUCT_BASE(Object);
	
	CharSetManager_reset(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void CharSetManager_destructor(CharSetManager this){

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset
void CharSetManager_reset(CharSetManager this){	

	int i = 0;
	int j = 0;
	// clear each segment's ofssets
	for(; i < CHAR_SEGMENTS; i++){
		
		for(j = 0; j < CHAR_SEGMENT_SIZE; j++){
			
			this->segment[i][j] = 0;			
		}
	}	
	
	// clear all definitions and usage
	for( i = 0; i < CHAR_SEGMENTS * CHAR_GRP_PER_SEG; i++){
		
		this->charDefinition[i] = NULL;
		this->charDefUsage[i] = 0;
		this->offset[i] = 0;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// record an allocated char defintion
static void CharSetManager_setCharDefinition(CharSetManager this, BYTE *charDefinition, int offset){
	
	int i = 0;
	
	// search where to register the char definition 
	for(; i < CHAR_SEGMENTS * CHAR_GRP_PER_SEG && this->charDefinition[i]; i++);
		
	ASSERT(i < CHAR_SEGMENTS * CHAR_GRP_PER_SEG, "CharSetManager: no char definitions slots left");
			
	// save char definition
	this->charDefinition[i] = charDefinition;
	
	// set usage value of char defintion
	this->charDefUsage[i] = 1;
	
	// save char definition offset
	this->offset[i] = offset;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// release char graphic memory
void CharSetManager_free(CharSetManager this, CharGroup charGroup){
	
	// retrieve index of char's defintion
	int i = CharSetManager_searchCharDefinition(this,charGroup);
	
	
	// if char found
	if(i >= 0){
		
		//decrease char definition usage		
		this->charDefUsage[i]--;
		
		// just make sure it is not going in a loop
		if(0xFE < this->charDefUsage[i]){
			this->charDefUsage[i] = 0;
		}
		
		// if no other object uses the char defintion
		if(!this->charDefUsage[i]){
			
			// deallocate space
			CharSetManager_deallocate(this, charGroup);
			
			// free char definition record
			this->charDefinition[i] = NULL;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set number of chars used in a given segment
void CharSetManager_setChars(CharSetManager this, int charSet, int numberOfChars){

	// set the number of chars of the given charset
	this->segment[charSet][0] = numberOfChars;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// print class's attributes's states
void CharSetManager_print(CharSetManager this, int charSet, int numberOfChars, int x){
	
	int i=0;
	for (i=0;i<16;i++){
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// look for an already writen char group
static int CharSetManager_searchCharDefinition(CharSetManager this, CharGroup charGroup){
	
	int i = 0;
	//
	BYTE* charDef = CharGroup_getCharDefinition(charGroup);
	
	ASSERT(charDef, "CharSetManager: null chardef in chargroup");

	for (; i < CHAR_SEGMENTS * CHAR_GRP_PER_SEG; i++){
		
		// if char's definition matches
		if(this->charDefinition[i] == charDef){
			
			// return the index
			return i;
		}
	}
	
	// otherwise return an invalid index
	return -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// if char if part of a background or oder object whose frame doesn't change
int CharSetManager_allocateShared(CharSetManager this, CharGroup charGroup){
	
	// get the index if the chargroup is already defined
	int i = CharSetManager_searchCharDefinition(this, charGroup);

	// verify that the char's definition is not already defined		
	if(0 <= i){

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// allocate a char defintion within char graphic memory
void CharSetManager_allocate(CharSetManager this,CharGroup charGroup){
	
	int i = 0;
	int j = 0;
	int auxJ = 0;
	int hole=0;
	unsigned int index = 0;
	int offset = 0;
	int numberOfChars = CharGroup_getNumberOfChars(charGroup);
	unsigned int block = 0;
	int currentChar = 0;	
	int counter;
	
	ASSERT(numberOfChars > 0, "CharSetManager: number of chars < 0");
	
	// if char is defined as part of an animation frame allocate 
	// space for it
	CACHE_ENABLE;
	for(; i < CHAR_SEGMENTS ; i++){
		
		// each segment has 512 chars slots so, 16 ints are 512 bits
		for( j = 0; j < 16; j++){
			
			// see if there is a 0 in the block
			block = this->segment[i][j] ^ 0xFFFFFFFF;
			
			// if there is at least a 1 in the block			
			if(block){		
				
				// set index 1000 0000 0000 0000 2b
				index = 0x80000000;
				
				// while there is at least a 1 in the block
				while(index){
					
					// in-block offset
					if(block & index){
						
						// increase the hole
						hole++;
						
						// control if the first free char is the last one from a block
						// if hole fits numberOfChars plus one free space at the begining and end
						if(hole >= numberOfChars + 2){
						
							// determine offset within the segment
							offset = currentChar - numberOfChars;
							
							// determine the index to mark the offset
							auxJ = offset>>5;
							
							// set chargroup's offset
							CharGroup_setOffset(charGroup, offset);
		
							// record char defintion
							CharSetManager_setCharDefinition(this, CharGroup_getCharDefinition(charGroup), offset);
							
							// calculate the total slots
							counter = offset - (auxJ << 5);
							
							// initilize mask
							index = 0x80000000;

							while(counter--){
								// fill the mask acording to number of slots
								index >>= 1;
							}
							
							// mark segmant mask's used slots
							while(numberOfChars--){		
								
								this->segment[i][auxJ] |= index;
								
								index >>= 1;
								
								// reset the mask and increase the segment number
								if(!index){
									
									index = 0x80000000;
									
									auxJ++;
								}
							}

							// set chargroup's segment
							CharGroup_setCharSet(charGroup, i);

							CACHE_DISABLE;
							
							// stop processing
							return;					
						}
					}
					else{
						// otherwise clear hole must be cleared
						hole = 0;					
					}
					
					// shift the block to the right
					index >>= 1;
					
					// increase the in-block offset
					currentChar++;
				}
			}
			else{
				
				// move current block, 32 slots ahead
				currentChar += 32;
			}
		}
		
		// reset the current char within the block
		currentChar = 0;
		
		// reset hole
		hole = 0;
	}
	CACHE_DISABLE;
	
	// if there isn't enough memory trown an exception
	ASSERT(false, CHMEM_MEM_ERR);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// free char graphic memory
static void CharSetManager_deallocate(CharSetManager this, CharGroup charGroup){
		
	// get chargroup's offset
	int offset = CharGroup_getOffset(charGroup);
	
	// get chargroup's number of chars
	int numberOfChars = CharGroup_getNumberOfChars(charGroup);
	
	// get chargroup's segment
	int charSet = CharGroup_getCharSet(charGroup);
	
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
	while(counter--){
		
		// fill mask
		index >>= 1;
	}
	
	// inverse the mask
	index ^= 0xFFFFFFFF;
	
	// clear freeded slots within the segment
	CACHE_ENABLE;
	while(numberOfChars--){
		
		this->segment[charSet][j] &= index;
		
		index >>= 1;
		
		index |= 0x80000000;
		
		if(index == 0xFFFFFFFF){
			
			index = 0x7FFFFFFF;
			
			j++;
		}
	}
	CACHE_DISABLE;
}

