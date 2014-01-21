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

#include <TextureManager.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											 CLASS'S MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


/*------------------------------ERROR-MESSAGES------------------------------*/
						   /*--------------------Max lenght-----------------*/
				
#define BGMEM_MEM_ERR 		"BgmapMem: bgmap segments depleted"				
#define BGMEM_MC_ERR 		"BgmapMem: Mem depleted (MapCharacter)"
#define BGMEM_TX_ERR 		"BgmapMem: Mem depleted (TextBox)"
#define BGMEM_BG_ERR 		"BgmapMem: Mem depleted (Background)"
#define BGMEM_0CH_ERR		"BgmapMem: map has 0 chars"
#define BGMEM_NOALLOC_ERR	"BgmapMem: bgmap definition not found"

enum OffsetIndex{
	
	kXOffset = 0,
	kYOffset,
	kBgmapSegment
};

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define TextureManager_ATTRIBUTES										\
																		\
	/* super's attributes */											\
	Object_ATTRIBUTES;													\
																		\
	/* number of chars ocuppied */										\
	int numberOfChars[NUM_BGMAPS];										\
																		\
	/* current x offset to set the next bgmap */						\
	int xOffset[NUM_BGMAPS][NUM_MAPS_PER_SEG];							\
																		\
	/* current y offset to set the next bgmap */						\
	int yOffset[NUM_BGMAPS][NUM_MAPS_PER_SEG];							\
																		\
	/* 12 segments, 28 maps, 2 indexes (x,y) and bgmap segment */ 		\
	int offset[NUM_BGMAPS * NUM_MAPS_PER_SEG][3];						\
																		\
	/* next free bgmap used for text printing */						\
	int freeBgmap;														\
																		\
	/* the textures allocated */										\
	Texture texture[NUM_BGMAPS * NUM_MAPS_PER_SEG];						\
																		\
	/* texture usage count */											\
	int textureUsageCount[NUM_BGMAPS * NUM_MAPS_PER_SEG];				\

	

// define the TextureManager
__CLASS_DEFINITION(TextureManager);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void TextureManager_constructor(TextureManager this);

// allocate texture in bgmpa graphic memory
static int TextureManager_allocate(TextureManager this, Texture texture);

// retrieve a texture previuosly loaded
static Texture TextureManager_findTexture(TextureManager this, TextureDefinition* textureDefinition);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

__SINGLETON(TextureManager);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void TextureManager_constructor(TextureManager this){
	
	__CONSTRUCT_BASE(Object);
	
	TextureManager_reset(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void TextureManager_destructor(TextureManager this){

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset
void TextureManager_reset(TextureManager this) {
	
	int i = 0;
	int j = 0;

	// clear each bgmap segment usage
	for(; i < NUM_BGMAPS; i++){
		
		this->numberOfChars[i] = 0;
		
		// clear the offsets
		for(j=0;j<NUM_MAPS_PER_SEG;j++){
			
			this->xOffset[i][j] = 0;
			this->yOffset[i][j] = 0;
		}
	}
	
	for(i = 0; i < NUM_BGMAPS * NUM_MAPS_PER_SEG; i++){
		
		this->offset[i][kXOffset] = 0;
		this->offset[i][kYOffset] = 0;
		this->offset[i][kBgmapSegment] = 0;
		this->texture[i] = NULL;
		this->textureUsageCount[i] = 0;
	}
	
	this->freeBgmap = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// allocate texture in bgmap graphic memory
static int TextureManager_allocate(TextureManager this, Texture texture){
	
	int i = 0;
	int j = 0;	
	int aux = 0;
	//int bgmapMask = 0x0001;
	
	int rows = Texture_getRows(texture) + 1;
	int cols = Texture_getTotalCols(texture) + 1;
	
	int area = rows * cols;
	
	//if texture already defined, don't allocate
	CACHE_ENABLE;	
	if(Texture_getNumberOfChars(texture)){
		
		for(i = 0; i < NUM_BGMAPS; i++){
			//if there is space in the segment memory
			// there are 4096 chars in each bgmap segment
			if((4096 - this->numberOfChars[i]) >= area ){
				
				//check if there is space within the segment
				// we check the next so don't go to the last element
				for(j = 0; j < NUM_MAPS_PER_SEG - 1; j++){

					//determine the y offset inside the bgmap segment
					if(!this->yOffset[i][j + 1]){
						
						aux = 64;
					}
					else{
						
						aux = this->yOffset[i][j + 1];
					}
					
					//determine if there is still mem space (columns) in the current y offset
					if(rows < aux - this->yOffset[i][j] || (!this->yOffset[i][j+1])){
						
						if(rows < 64 - this->yOffset[i][j]){
						
							if(cols < 64 - this->xOffset[i][j]){
								
								int id = Texture_getId(texture);
								
								//registry bgmap definition
								this->offset[id][kXOffset] = this->xOffset[i][j];
								this->offset[id][kYOffset] = this->yOffset[i][j];
								this->offset[id][kBgmapSegment] = i;
								
								//increment the x offset
								this->xOffset[i][j] += cols;
								
								//if the number of rows of the bgmap definition is greater than the
								//next y offset defined, increase the next y offset
								if(this->yOffset[i][j+1] - this->yOffset[i][j] < rows){
									
									this->yOffset[i][j+1] = this->yOffset[i][j] + rows ;
								}
								else{
									
									// there must be at least 2 columns free
									if(this->xOffset[i][j] >= 62){
										
										//this->yOffset[i][j+1] = this->yOffset[i][j] + rows ;										
									}								
								}

								
								//update the number of chars defined inside the bgmap segment
								this->numberOfChars[i] += area;
								
								if(i + 1 > this->freeBgmap){
									
									this->freeBgmap = i + 1;
								}
								
								CACHE_DISABLE;
								
								//if there is a free bgmap segment
								return true;
							}
						}
						else{
							
							break;
						}
					}	
					else{
						
						if(rows > 64 - this->yOffset[i][j]){
							
							break;
						}
					}
				}
			}
		}

		//throw an exception if there is no enough space to allocate the bgmap definition
		ASSERT(false, BGMEM_MEM_ERR);
		
	}
	CACHE_DISABLE;

	//through exception if texture has 0 chars
	ASSERT(false, BGMEM_0CH_ERR);
	
	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve free bgmap segment number
int TextureManager_getFreeBgmap(TextureManager this){
	
	return this->freeBgmap;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// allocate bgmap text boxes
// this bgmap segment is handled as one only bgmap defined inside so, only
// TextureManager.xOffset[textbgmap][0] is used
void TextureManager_allocateText(TextureManager this, Texture texture){
	
	int xDisplacement = 0;
	int yDisplacement = 0;
	
	int length = Texture_getCols(texture);
	
	//if there is space in the first row
	//calculate y displacement
	//offset/64->chars per row inside a bgmap
	yDisplacement = (this->xOffset[this->freeBgmap][0] + length) >> 6;
	
	//move to the next row
	if(this->xOffset[this->freeBgmap][0] < 64 * yDisplacement){
		this->xOffset[this->freeBgmap][0] = 64 * yDisplacement;
	}
	
	//offset%/64->chars per row inside a bgmap
	xDisplacement = (this->xOffset[this->freeBgmap][0]) % 64;
	
	//let free the upper chars to debug and quick text printing
	//Texture_setYOffset(texture, 28 + yDisplacement);
	
	//calculate x displacement
	//Texture_setXOffset(texture, xDisplacement);
	
	//set next ofsset entry to modify within the free bgmap segment
	this->xOffset[this->freeBgmap][0] += length;

	//Texture_setBgmapSegment(texture, this->freeBgmap);

	//if there are no more rows in the segment... thrown and exception
	ASSERT(this->xOffset[this->freeBgmap][0] < 4096, BGMEM_TX_ERR);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// deallocate texture from bgmap graphic memory
void TextureManager_free(TextureManager this, Texture texture){
	
	// if no one is using the texture anymore
	if(!(--this->textureUsageCount[Texture_getId(texture)])){
		
		// free char memory
		Texture_freeCharMemory(texture);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve a texture previuosly loaded
static Texture TextureManager_findTexture(TextureManager this, TextureDefinition* textureDefinition){

	int i = 0;
	
	// try to find a texture with the same bgmap definition
	for(; i < NUM_BGMAPS * NUM_MAPS_PER_SEG; i++){
		
		if(this->texture[i] && Texture_getBgmapDef(this->texture[i]) == textureDefinition->bgmapDefinition){
			
			// return if found
			return this->texture[i];
		}
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// load a texture
static Texture TextureManager_loadTexture(TextureManager this, TextureDefinition* textureDefinition){
	
	int i = 0;
	
	// find and empty slot
	for(; i < NUM_BGMAPS * NUM_MAPS_PER_SEG; i++){
		
		if(!this->texture[i]){
			
			// create new texture and register it
			this->texture[i] = __NEW(Texture, __ARGUMENTS(textureDefinition, i));
			
			//if not, then allocate
			TextureManager_allocate(this, this->texture[i]);
			
			// write texture to graphic memory
			Texture_write(this->texture[i]);
			
			// set texture usage
			this->textureUsageCount[i] = 1;
			
			return this->texture[i];
		}
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// load and retrieve a texture
Texture TextureManager_get(TextureManager this, TextureDefinition* textureDefinition){
	
	Texture texture = NULL;
	
	//determine the allocation type
	switch(textureDefinition->charGroupDefinition.allocationType){
	
		case __ANIMATED:
	
			// load a new texture
			texture = TextureManager_loadTexture(this, textureDefinition);

			ASSERT(texture, "Texture: (animated) texture no allocated");

			break;
			
		case __ANIMATED_SHARED:
		case __NO_ANIMATED:
			
			// first try to find an already created texture
			texture = TextureManager_findTexture(this, textureDefinition);

			// if coudn't find the texture
			if(!texture){
			
				// load it
				texture = TextureManager_loadTexture(this, textureDefinition);
			}
			else{

				// if no using texture yet
				if(!this->textureUsageCount[Texture_getId(texture)]){
				
					// write texture to graphic memory
					Texture_write(texture);
				}
			}
			ASSERT(texture, "Texture: (shared) texture no allocated");

			// increase texture usage count
			this->textureUsageCount[Texture_getId(texture)]++;

			break;
	}

	return texture;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve x offset
int TextureManager_getXOffset(TextureManager this, int id){
	
	return this->offset[id][kXOffset];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve y offset
int TextureManager_getYOffset(TextureManager this, int id){
	
	return this->offset[id][kYOffset];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve bgmap segment
int TextureManager_getBgmapSegment(TextureManager this, int id){
	
	return this->offset[id][kBgmapSegment];
}

void TextureManager_debug(TextureManager this){
	
	int j = 0;
	//int bgmap = 0;
	for(j = 0; j < NUM_MAPS_PER_SEG; j++){
		
		//vbjPrintInt(this->yOffset[bgmap][j], 0, j);
		//vbjPrintInt(this->xOffset[bgmap][j], 10, j);
	}
}
