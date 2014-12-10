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


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Texture.h>
#include <Optics.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

__CLASS_DEFINITION(Texture);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void Texture_constructor(Texture this, TextureDefinition* textureDefinition, u16 id);

// write an animated map
static void Texture_writeAnimated(Texture this);

// write an inanimated map
static void Texture_writeNoAnimated(Texture this);

// write an animated and shared map
static void Texture_writeAnimatedShared(Texture this);

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
__CLASS_NEW_DEFINITION(Texture, __PARAMETERS(TextureDefinition* textureDefinition, u16 id))
__CLASS_NEW_END(Texture, __ARGUMENTS(textureDefinition, id));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Texture_constructor(Texture this, TextureDefinition* textureDefinition, u16 id){

	// construct base object
	__CONSTRUCT_BASE(Object);

	// set id
	this->id = id;

	// save the bgmap definition's address
	this->textureDefinition = textureDefinition;	
	
	// if the char definition is NULL, it must be a text
	this->charGroup = __NEW(CharGroup, __ARGUMENTS((CharGroupDefinition*)&this->textureDefinition->charGroupDefinition, (Object)this));
	
	// set the pallet
	this->pallet = textureDefinition->pallet;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Texture_destructor(Texture this){
	
	ASSERT(this, "Texture::destructor: null this");

	Texture_freeCharMemory(this);
	
	// destroy the super object
	__DESTROY_BASE(Object);
}
//extern void addmem1 (u8* dest, const u8* src, u16 num, u16 offset);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write an animated map
static void Texture_writeAnimated(Texture this){

	ASSERT(this, "Texture::writeAnimated: null this");

	int bgmapSegment = Texture_getBgmapSegment(this);
	int pallet = Texture_getPallet(this) << 14;
	
	int charLocation = (CharGroup_getCharSet(this->charGroup) << 9) + CharGroup_getOffset(this->charGroup);
	int i = this->textureDefinition->rows;
	
	int xOffset = TextureManager_getXOffset(TextureManager_getInstance(), this->id);
	int yOffset = TextureManager_getYOffset(TextureManager_getInstance(), this->id);

	//put the map into memory calculating the number of char for each reference
	for(; i--;){

		Mem_add ((u8*)BGMap(bgmapSegment) + ((xOffset + (yOffset << 6 ) + (i << 6)) << 1), 
				(const u8*)(this->textureDefinition->bgmapDefinition + ( i * (this->textureDefinition->cols) << 1)), 
				this->textureDefinition->cols,
				(pallet) | (charLocation));
		
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write an inanimated map
static void Texture_writeNoAnimated(Texture this){

	ASSERT(this, "Texture::writeNoAnimated: null this");

	int bgmapSegment = Texture_getBgmapSegment(this);
	int pallet = Texture_getPallet(this) << 14;
	
	int charLocation = (CharGroup_getCharSet(this->charGroup) << 9) + CharGroup_getOffset(this->charGroup);
	int i = this->textureDefinition->rows;
	
	int xOffset = TextureManager_getXOffset(TextureManager_getInstance(), this->id);
	int yOffset = TextureManager_getYOffset(TextureManager_getInstance(), this->id);

	//put the map into memory calculating the number of char for each reference
	for(; i--;){
	
		//specifying the char displacement inside the char mem
		Mem_add ((u8*)BGMap(bgmapSegment) + ((xOffset + (yOffset << 6 ) + (i << 6)) << 1), 
				(const u8*)(this->textureDefinition->bgmapDefinition + ( i * (this->textureDefinition->cols) << 1)), 
				this->textureDefinition->cols,
				(pallet) | (charLocation));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write an animated and shared map
static void Texture_writeAnimatedShared(Texture this){

	ASSERT(this, "Texture::writeAnimatedShared: null this");

	int bgmapSegment = Texture_getBgmapSegment(this);
	int pallet = Texture_getPallet(this) << 14;
	
	// determine the number of frames the map had
	int area = (this->textureDefinition->cols * this->textureDefinition->rows);
	int charLocation = (CharGroup_getCharSet(this->charGroup) << 9) + CharGroup_getOffset(this->charGroup);
	int frames = CharGroup_getNumberOfChars(this->charGroup) / area;
	
	int i = this->textureDefinition->rows;
	
	int xOffset = TextureManager_getXOffset(TextureManager_getInstance(), this->id);
	int yOffset = TextureManager_getYOffset(TextureManager_getInstance(), this->id);

	//put the map into memory calculating the number of char for each reference
	for(; i--;){
	
		int j = 1;
		//write into the specified bgmap segment plus the offset defined in the this structure, the this definition
		//specifying the char displacement inside the char mem
		for(; j <= frames; j++){
			
			Mem_add ((u8*)BGMap(bgmapSegment) + ((xOffset + (this->textureDefinition->cols * (j - 1)) + (yOffset << 6) + (i << 6)) << 1),
					(const u8*)(this->textureDefinition->bgmapDefinition + ( i * (this->textureDefinition->cols) << 1)),
					this->textureDefinition->cols,
					(pallet) | (charLocation + area * (j - 1)));

		}
	}
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// free char memory
void Texture_freeCharMemory(Texture this){
	
	ASSERT(this, "Texture::freeCharMemory: null this");

	if(this->charGroup){
		
		//destroy the chargroup
		__DELETE(this->charGroup);
		
		this->charGroup = NULL;
	}	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write into memory the chars and this
void Texture_write(Texture this){

	ASSERT(this, "Texture::write: null this");

	if(!this->charGroup){
		
		// if the char definition is NULL, it must be a text
		this->charGroup = __NEW(CharGroup, __ARGUMENTS((CharGroupDefinition*)&this->textureDefinition->charGroupDefinition, (Object)this));
	}

	//write char group	
	CharGroup_write(this->charGroup);
	
	//determine the allocation type
	switch(CharGroup_getAllocationType(this->charGroup)){
	
		case __ANIMATED:
			
			// write the definition to graphic memory
			Texture_writeAnimated(this);
			
			break;
			
		case __ANIMATED_SHARED:
			
			// write the definition to graphic memory
			Texture_writeAnimatedShared(this);											
			
			break;
			
		case __NO_ANIMATED:
			
			// write the definition to graphic memory
			Texture_writeNoAnimated(this);					
			
			break;
		
		default:
			
			ASSERT(false, "Texture::write: no allocation type");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write into memory the chars and this
void Texture_rewrite(Texture this){

	ASSERT(this, "Texture::rewrite: null this");

	// determine the allocation type
	switch(CharGroup_getAllocationType(this->charGroup)){
	
		case __ANIMATED:		
			
			// put the definition in graphic memory
			Texture_writeAnimated(this);
			
			break;
			
		case __ANIMATED_SHARED:
			
			// put the definitionin graphic memory
			Texture_writeAnimatedShared(this);											

			break;
			
		case __NO_ANIMATED:
			// put the definition in graphic memory
			Texture_writeNoAnimated(this);
			
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this reallocate a write the bgmap definition in graphical memory
void Texture_resetMemoryState(Texture this){
	
	ASSERT(this, "Texture::resetMemoryState: null this");

	ASSERT(false, "Texture::resetMemoryState: null this");
	//fake char offset so it is allocated again
	CharGroup_setOffset(this->charGroup, 0xFF);
	
	// write it in graphical memory
	Texture_write(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write map in hbias mode
void Texture_writeHBiasMode(Texture this){

	ASSERT(this, "Texture::writeHBiasMode: null this");

	// TODO
	/*
	int i;	
	//put the this into memory calculatin the number of char for each reference
	for(i=0;i<this->textureDefinition->rows;i++){
		//write into the specified bgmap segment plus the offset defined in the this structure, the this definition
		//specifying the char displacement inside the char mem
		//addMem ((void*)BGTexture(this->bgmapSegment)+((this->xOffset+this->textureDefinition->cols/3+(this->yOffset<<6)+(i<<6))<<1), this->textureDefinition->bgmapDefinition+(i<<7), (this->textureDefinition->cols/3)*2,(this->pallet<<14)|((CharGroup_getCharSet(&this->charGroup)<<9)+CharGroup_getOffset(&this->charGroup)));
		addMem ((void*)BGTexture(this->bgmapSegment)+((this->xOffset+this->textureDefinition->cols/3+64* const this->yOffset+64*i)<<1), this->textureDefinition->bgmapDefinition+(i<<7), (this->textureDefinition->cols/3)*2,(this->pallet<<14)|((CharGroup_getCharSet(&this->charGroup)<<9)+CharGroup_getOffset(&this->charGroup)));
	}
	*/	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get texture's number of chars
int Texture_getNumberOfChars(Texture this){
	
	ASSERT(this, "Texture::getNumberOfChars: null this");
	ASSERT(this->charGroup, "Texture::getNumberOfChars: null chargroup");
	
	return CharGroup_getNumberOfChars(this->charGroup);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get texture's y offset within bgmap mem
u8 Texture_getYOffset(Texture this){
	
	ASSERT(this, "Texture::getYOffset: null this");

	return TextureManager_getYOffset(TextureManager_getInstance(), this->id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get texture's x offset within bgmap mem
u8 Texture_getXOffset(Texture this){
	
	ASSERT(this, "Texture::getXOffset: null this");

	return TextureManager_getXOffset(TextureManager_getInstance(), this->id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get texture's cols
u8 Texture_getTotalCols(Texture this){
	
	ASSERT(this, "Texture::getTotalCols: null this");
	
	// determine the allocation type
	switch(CharGroup_getAllocationType(this->charGroup)){
	
		case __ANIMATED:
			
			// just return the cols
			return this->textureDefinition->cols;
			break;
			
		case __ANIMATED_SHARED:
			
			// return the total number of chars
			return CharGroup_getNumberOfChars(this->charGroup) / this->textureDefinition->rows;
			break;
			
		case __NO_ANIMATED:
			
			// just return the cols
			return this->textureDefinition->cols;
			break;			
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//get texture's rows
u8 Texture_getTotalRows(Texture this){
	
	ASSERT(this, "Texture::getTotalRows: null this");

	return this->textureDefinition->rows;
	/*
	switch(CharGroup_getAllocationType(&this->charGroup)){
		case __ANIMATED:
			return this->textureDefinition->rows;
			break;
		case __ANIMATEDSHARED:
			return this->textureDefinition->rows*CharGroup_getNumberOfChars(&this->charGroup)/this->textureDefinition->cols;
			break;
		case __NOANIMATED:
			return this->textureDefinition->cols;
			break;			
	}
	return 0;
	*/
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//get texture's bgmap segment
u8 Texture_getBgmapSegment(Texture this){
	
	ASSERT(this, "Texture::getBgmapSegment: null this");

	return TextureManager_getBgmapSegment(TextureManager_getInstance(), this->id);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//get texture's chargroup
CharGroup Texture_getCharGroup(Texture this){
	
	ASSERT(this, "Texture::getCharGroup: null this");

	return this->charGroup;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//get texture's bgmap definition
BYTE* Texture_getBgmapDef(Texture this){
	
	ASSERT(this, "Texture::getBgmapDef: null this");

	return this->textureDefinition->bgmapDefinition;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set the pallet
void Texture_setPallet(Texture this, u8 pallet){
	
	ASSERT(this, "Texture::setPallet: null this");

	this->pallet = pallet;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
u8 Texture_getPallet(Texture this){
	
	ASSERT(this, "Texture::getPallet: null this");

	return this->pallet;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve texture's rows
u8 Texture_getRows(Texture this){
	
	ASSERT(this, "Texture::getRows: null this");
	//ASSERT(this->textureDefinition, "Texture::getRows: 0 rows");
	
	return this->textureDefinition->rows;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve texture's cols
u8 Texture_getCols(Texture this){
	
	ASSERT(this, "Texture::getCols: null this");

	return this->textureDefinition->cols;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve texture's id
u16 Texture_getId(Texture this){
	
	ASSERT(this, "Texture::getId: null this");

	return this->id;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process a telegram
int Texture_handleMessage(Texture this, Telegram telegram){
	
	ASSERT(this, "Texture::handleMessage: null this");
	switch(Telegram_getMessage(telegram)) {

		case kCharGroupRewritten:
			
			Texture_write(this);
			return true;
			break;
	}
	
	return false;
}
