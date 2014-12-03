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

#include <ScrollBackground.h>

#include <CollisionManager.h>
#include <Optics.h>
#include <Shape.h>
#include <Prototypes.h>

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

// global
extern VBVec3D * _screenPosition;
extern MovementState* _screenMovementState;

// calculate the scroll's screen position
static void ScrollBackground_updateScrolling(ScrollBackground this);


enum ScrollSprites {
	kLeftSprite = 0,
	kRightSprite
};

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

	ASSERT(this, "ScrollBackground::constructor: null this");
	ASSERT(scrollBackgroundDefinition, "ScrollBackground::constructor: null definition");
	
	// construct base object
	__CONSTRUCT_BASE(Entity, __ARGUMENTS(scrollBackgroundDefinition, ID));
	
	
	ASSERT(this->sprites, "ScrollBackground::constructor: null sprite list");
	
	VirtualNode node = VirtualList_begin(this->sprites);
	int i = 0;

	for(; node && i <= kRightSprite ; node = VirtualNode_getNext(node), i++){
		
		this->scrollSprites[i] = VirtualNode_getData(node);
		
		ASSERT(__GET_CAST(Sprite, this->scrollSprites[i]), "ScrollBackground::constructor: no sprite added to list")
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void ScrollBackground_destructor(ScrollBackground this){
	
	ASSERT(this, "ScrollBackground::destructor: null this");

	// destroy the super object
	__DESTROY_BASE(Entity);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// transform class
void ScrollBackground_transform(ScrollBackground this, Transformation* environmentTransform){

	ASSERT(this, "ScrollBackground::transform: null this");

	// call base class's transform method
	Entity_transform((Entity)this, environmentTransform);

	if(_screenMovementState->x || _screenMovementState->y || this->invalidateGlobalPosition.x || this->invalidateGlobalPosition.y || this->invalidateGlobalPosition.z){
		
		ScrollBackground_updateScrolling(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculate the scroll's screen position
static void ScrollBackground_updateScrolling(ScrollBackground this){
	
	ASSERT(this, "ScrollBackground::updateScrolling: null this");

	// TODO: add proper comments

	DrawSpec drawSpec0 = {
			{0, 0, this->transform.globalPosition.z},
			{1, 1}
	};
	
	DrawSpec drawSpec1 = drawSpec0;

	VBVec3D position3D = {_screenPosition->x, this->transform.globalPosition.y, this->transform.globalPosition.z};
	
	// get the screen's position
	VBVec2D screenPosition; 

	int screens = 0;

	// axis to put one map o each side of it
	int axis = 0;
	int factor = 1;
	int displacement = 0;
	
	// project position to 2D
	Optics_projectTo2D(&screenPosition, &position3D);
	
	// get the number of "screens" from the beginnig of the world
	// to the actual screen's position
	screens = FIX19_13TOI(screenPosition.x) / __SCREEN_WIDTH;
	
	// check if the number of screens is divisible by 2
	//if(!(screens & 2)1 == 0 && screens != 0){
	displacement = FIX19_13TOI(screenPosition.x);
	
	if(screens){
		
		displacement -= ( screens - 1) * __SCREEN_WIDTH;

		if(!(screens & 1)){	
			
			// if so, 
			factor = 2;
		}
	}
	
	axis = __SCREEN_WIDTH * factor - displacement;
	
	if((unsigned)axis <= __SCREEN_WIDTH){
		
		drawSpec0.position.x = ITOFIX19_13(axis - __SCREEN_WIDTH);
		
		drawSpec1.position.x = ITOFIX19_13(axis);
	}
	else{
		
		if(axis < 0){
			
			drawSpec1.position.x = ITOFIX19_13(axis);
			
			drawSpec0.position.x = drawSpec1.position.x + ITOFIX19_13(__SCREEN_WIDTH);
		}
		else{
			
			drawSpec0.position.x = ITOFIX19_13(axis - __SCREEN_WIDTH - 1);
			
			drawSpec1.position.x = drawSpec0.position.x - ITOFIX19_13(__SCREEN_WIDTH - 1);
		}
	}
	
	// now move the drawspec in order to render the texture in the center
	drawSpec0.position.y = drawSpec1.position.y = screenPosition.y - ITOFIX19_13(Texture_getRows(Sprite_getTexture(this->scrollSprites[kLeftSprite])) << 2);
	drawSpec0.position.parallax = drawSpec1.position.parallax = Sprite_getDrawSpec(this->scrollSprites[kLeftSprite]).position.parallax;

	// set map's position
	Sprite_setDrawSpec(this->scrollSprites[kRightSprite], &drawSpec0);
	Sprite_setRenderFlag(this->scrollSprites[kRightSprite], __UPDATE_G);
	
	Sprite_setDrawSpec(this->scrollSprites[kLeftSprite], &drawSpec1);	
	Sprite_setRenderFlag(this->scrollSprites[kLeftSprite], __UPDATE_G);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// whether it is visible
int ScrollBackground_isVisible(ScrollBackground this, int pad){
	
	ASSERT(this, "ScrollBackground::isVisible: null this");

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if must update sprite's position
int ScrollBackground_updateSpritePosition(ScrollBackground this){

	ASSERT(this, "ScrollBackground::updateSpritePosition: null this");

	return false;
}
