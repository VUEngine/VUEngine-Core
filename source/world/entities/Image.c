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

#include <Image.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// define the Image
__CLASS_DEFINITION(Image);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */




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
__CLASS_NEW_DEFINITION(Image, __PARAMETERS(ImageDefinition* imageDefinition, int inGameIndex))
__CLASS_NEW_END(Image, __ARGUMENTS(imageDefinition, inGameIndex));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
void Image_constructor(Image this, ImageDefinition* imageDefinition, int inGameIndex){

	__CONSTRUCT_BASE(Entity, __ARGUMENTS(inGameIndex));

	// create the sprite
	this->sprite = __NEW(Sprite, __ARGUMENTS(&imageDefinition->spriteDefinition));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Image_destructor(Image this){
	
	// destroy the super object
	__DESTROY_BASE(Entity);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set class's position
void Image_setLocalPosition(Image this, VBVec3D position){
	
	// set the position
	Container_setLocalPosition((Container)this, position);
	/*
	// calculate the scale	
	Sprite_calculateScale(this->sprite, this->position.z);

	// set sprite's position
	Sprite_setPosition(this->sprite, &this->position);

	// calculate sprite's parallax
	Sprite_calculateParallax(this->sprite, this->position.z);

	// scale the sprite
	Sprite_scale(this->sprite);
	*/
}
