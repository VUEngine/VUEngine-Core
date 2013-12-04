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

#include <CollisionManager.h>
#include <Optics.h>
#include <Shape.h>

#include <ScrollBackground.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// define the ScrollBackground
__CLASS_DEFINITION(ScrollBackground);


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
__CLASS_NEW_DEFINITION(ScrollBackground, __PARAMETERS(ScrollBackgroundDefinition* backgroundDefinition, int ID))
__CLASS_NEW_END(ScrollBackground, __ARGUMENTS(backgroundDefinition, ID));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
void ScrollBackground_constructor(ScrollBackground this, ScrollBackgroundDefinition* scrollBackgroundDefinition, int ID){

	ASSERT(scrollBackgroundDefinition, ScrollBackground: NULL definition);
	
	// construct base object
	__CONSTRUCT_BASE(Entity, __ARGUMENTS(scrollBackgroundDefinition, ID));
	
	ScrollBackground_setScroll(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void ScrollBackground_destructor(ScrollBackground this){
	
	// destroy the super object
	__DESTROY_BASE(Entity);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// whether it is visible
int ScrollBackground_isVisible(ScrollBackground this, int pad){
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// positione scroll image childs
void ScrollBackground_setScroll(ScrollBackground this){
/*
	if(this->image0 && this->image1){

		VBVec3D position0 = {ITOFIX19_13(-384/2) , 0, 0};
		VBVec3D position1 = {ITOFIX19_13(384/2) , 0, 0};
		
		Image_setLocalPosition(this->image0, position0);
		
		Image_setLocalPosition(this->image1, position1);
	}
	*/
}