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

#include <Sprite.h>
#include <Game.h>
#include <SpriteManager.h>
#include <Optics.h>
#include <ParamTableManager.h>
#include <HardwareManager.h>
#include <Screen.h>

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

// define the Sprite
__CLASS_DEFINITION(Sprite);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define FIX19_13_05F 0x00001000

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
__CLASS_NEW_DEFINITION(Sprite, __PARAMETERS(const SpriteDefinition* spriteDefinition))
__CLASS_NEW_END(Sprite, __ARGUMENTS(spriteDefinition));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's conctructor
void Sprite_constructor(Sprite this, const SpriteDefinition* spriteDefinition){

	__CONSTRUCT_BASE(Object);
	
	// create the texture			 
	this->texture = TextureManager_get(TextureManager_getInstance(), spriteDefinition->textureDefinition);

	ASSERT(this->texture, "Sprite::constructor: texture no allocated");
	
	// set texture position
	this->texturePosition.x = Texture_getXOffset(this->texture);
	this->texturePosition.y = Texture_getYOffset(this->texture);
	
	// clear position
	this->drawSpec.position.x = 0;
	this->drawSpec.position.y = 0;
	this->drawSpec.position.z = 0;
	this->drawSpec.position.parallax = 0;
	
	this->drawSpec.scale.x = ITOFIX7_9(1);
	this->drawSpec.scale.y = ITOFIX7_9(1);
	
	this->parallaxDisplacement = spriteDefinition->parallaxDisplacement;

	this->param = 0;
		
	//this->head = spriteDefinition->display | WRLD_BGMAP;	
	//set world layer's head acording to map's render mode
	switch(spriteDefinition->bgmapMode){
	
		case WRLD_BGMAP:
			
			//set map head
			this->head = spriteDefinition->display | WRLD_BGMAP;	
			
			break;
			
		case WRLD_AFFINE:
	
			//set map head			
			this->head = spriteDefinition->display | WRLD_AFFINE | WRLD_OVR;

			//allocate param table space			
			ParamTableManager_allocate(ParamTableManager_getInstance(), this);
			
			break;
			
		case WRLD_HBIAS:
			
			//set map head
			this->head = spriteDefinition->display | WRLD_HBIAS | WRLD_OVR;
			
			break;
	}

	// set the default layer
	this->worldLayer = 0;
	
	// set the render flag
	this->renderFlag = 0;
		
	// register with sprite manager
	SpriteManager_addSprite(SpriteManager_getInstance(), this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Sprite_destructor(Sprite this){

	ASSERT(this, "Sprite::destructor: null this");

	Sprite_hide(this);
	
	//if affine or bgmap
	if(WRLD_AFFINE & this->head){

		//free param table space
		ParamTableManager_free(ParamTableManager_getInstance(), this);
	}	

	// remove from sprite manager
	SpriteManager_removeSprite(SpriteManager_getInstance(), this);

	// free the texture
	TextureManager_free(TextureManager_getInstance(), this->texture);
	
	this->texture = NULL;

	// destroy the super object
	__DESTROY_BASE(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
Scale Sprite_getScale(Sprite this){
	
	ASSERT(this, "Sprite::getScale: null this");

	//  return the scale
	return this->drawSpec.scale;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set the direction
void Sprite_setDirection(Sprite this, int axis, int direction){
	
	ASSERT(this, "Sprite::setDirection: null this");

	switch(axis){
	
		case __XAXIS:
			
			this->drawSpec.scale.x = FIX7_9_MULT(abs(this->drawSpec.scale.x), ITOFIX7_9(direction));
			
			break;
			
		case __YAXIS:
					
			this->drawSpec.scale.y = FIX7_9_MULT(abs(this->drawSpec.scale.y), ITOFIX7_9(direction));
			break;
	}
	
	// scale the texture in the next render cycle
	Sprite_invalidateParamTable(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculate zoom scaling factor
void Sprite_calculateScale(Sprite this, fix19_13 z){
	
	ASSERT(this, "Sprite::calculateScale: null this");

	Optical optical = Game_getOptical(Game_getInstance());
	fix7_9 ratio = FIX19_13TOFIX7_9(ITOFIX19_13(1) - 
			       FIX19_13_DIV(z , optical.maximunViewDistance));

	// TODO: remove the * and the branch
	this->drawSpec.scale.x = ratio * (this->drawSpec.scale.x < 0? -1: 1);
	this->drawSpec.scale.y = ratio;
	
	if(WRLD_AFFINE == Sprite_getMode(this)){

		this->halfWidth = ITOFIX19_13((int)Texture_getCols(this->texture) << 2);
		this->halfHeight = ITOFIX19_13((int)Texture_getRows(this->texture) << 2);
	}
	else {
		
		this->halfWidth = FIX19_13_DIV(ITOFIX19_13((int)Texture_getCols(this->texture) << 2), (FIX7_9TOFIX19_13(this->drawSpec.scale.x)));
		this->halfHeight = FIX19_13_DIV(ITOFIX19_13((int)Texture_getRows(this->texture) << 2), (FIX7_9TOFIX19_13(this->drawSpec.scale.y)));
	}

	Sprite_invalidateParamTable(this);
}

void Sprite_roundDrawSpec(Sprite this){
	
	ASSERT(this, "Sprite::roundDrawSpec: null this");

	this->drawSpec.position.x &= 0xFFFFE000;
	this->drawSpec.position.y &= 0xFFFFE000;
	this->drawSpec.position.z &= 0xFFFFE000;	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set sprite's position
void Sprite_setPosition(Sprite this, const VBVec3D* const position){

	ASSERT(this, "Sprite::setPosition: null this");

	// normalize the position to screen coordinates
	VBVec3D position3D = Optics_normalizePosition(position);

	ASSERT(this->texture, "Sprite::setPosition: null texture");

	position3D.x -= this->halfWidth;
	position3D.y -= this->halfHeight;

	fix19_13 previousZPosition = this->drawSpec.position.z;
	
	// project position to 2D space
	Optics_projectTo2D(&this->drawSpec.position, &position3D);
	
	if(previousZPosition != this->drawSpec.position.z) {
		
		this->drawSpec.position.z = position->z;
		
		// calculate sprite's parallax
		Sprite_calculateParallax(this, this->drawSpec.position.z);

		//SpriteManager_spriteChangedPosition(SpriteManager_getInstance());
	}

	this->renderFlag |= __UPDATE_G;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculate the parallax
void Sprite_calculateParallax(Sprite this, fix19_13 z){
	
	ASSERT(this, "Sprite::calculateParallax: null this");

	this->drawSpec.position.z = z;
	this->drawSpec.position.parallax = Optics_calculateParallax(this->drawSpec.position.x, z);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve the texture
Texture Sprite_getTexture(Sprite this){
	
	ASSERT(this, "Sprite::getTexture: null this");

	return this->texture;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve drawspec
DrawSpec Sprite_getDrawSpec(Sprite this){
	
	ASSERT(this, "Sprite::getDrawSpec: null this");

	return this->drawSpec;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set to true to allow render
void Sprite_setRenderFlag(Sprite this, u8 renderFlag){
	
	ASSERT(this, "Sprite::setRenderFlag: null this");

	// do not override the whole world entry, or will be updated in the
	// next render
	if(__UPDATE_HEAD != this->renderFlag || !renderFlag) {
		
		this->renderFlag = renderFlag;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// show
void Sprite_show(Sprite this){
	
	this->renderFlag = __UPDATE_HEAD;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// hide
void Sprite_hide(Sprite this){
	
    WORLD_SIZE(this->worldLayer, 0, 0);
//	WORLD_GSET(this->worldLayer, __SCREEN_WIDTH, 0, __SCREEN_HEIGHT);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update sprite
void Sprite_update(Sprite this){

	ASSERT(this, "Sprite::update: null this");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// render a world layer with the map's information
void Sprite_render(Sprite this){

	ASSERT(this, "Sprite::render: null this");

	//if render flag is set
	if(this->renderFlag){
		
		DrawSpec drawSpec = this->drawSpec;

		if(__UPDATE_HEAD == this->renderFlag){
			
			//create an independant of software variable to point XPSTTS register
			unsigned int volatile *xpstts =	(unsigned int *)&VIP_REGS[XPSTTS];

			//wait for screen to idle	
			while (*xpstts & XPBSYR);

			// write the head
			WORLD_HEAD(this->worldLayer, this->head | Texture_getBgmapSegment(this->texture));

			WORLD_MSET(this->worldLayer, (this->texturePosition.x << 3), 0, this->texturePosition.y << 3);
		}

		//set the world screen position
		if(this->renderFlag & __UPDATE_G ){

			WORLD_GSET(this->worldLayer, FIX19_13TOI(drawSpec.position.x + FIX19_13_05F), drawSpec.position.parallax + this->parallaxDisplacement, FIX19_13TOI(drawSpec.position.y + FIX19_13_05F));
		}
		
		if(this->renderFlag & __UPDATE_SIZE){

			//set the world size according to the zoom
			if(WRLD_AFFINE & this->head){

				// now scale the texture
				Sprite_scale(this);
				
				WORLD_SIZE(this->worldLayer, 
						((int)Texture_getCols(this->texture)<< 3) * FIX7_9TOF(abs(drawSpec.scale.x)) - 1,						
						((int)Texture_getRows(this->texture)<< 3) * FIX7_9TOF(abs(drawSpec.scale.y));)	
				
				WORLD_PARAM(this->worldLayer, PARAM(this->param));				
			}
			else{
				
				WORLD_SIZE(this->worldLayer, (Texture_getCols(this->texture) << 3), (Texture_getRows(this->texture) << 3));
			}
		}
		
		// make sure to not render again
		this->renderFlag = false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get map's param table address
u32 Sprite_getParam(Sprite this){
	
	ASSERT(this, "Sprite::getParam: null this");

	return this->param;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set map's param table address
void Sprite_setParam(Sprite this, u32 param){

	ASSERT(this, "Sprite::setParam: null this");

	this->param = param;
	
	// set flag to rewrite texture's param table
	Sprite_invalidateParamTable(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set map's world layer
void Sprite_setWorldLayer(Sprite this, u8 worldLayer){

	ASSERT(this, "Sprite::setWorldLayer: null this");

	if(this->worldLayer != worldLayer && 0 <= worldLayer){
	
		this->worldLayer = worldLayer;
	
		Sprite_show(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//get map's world layer
u8 Sprite_getWorldLayer(Sprite this){
		
	ASSERT(this, "Sprite::getWorldLayer: null this");

	return this->worldLayer;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get sprite's render head
u16 Sprite_getHead(Sprite this){
	
	ASSERT(this, "Sprite::getHead: null this");

	return this->head;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get map's render mode
u16 Sprite_getMode(Sprite this){
	
	ASSERT(this, "Sprite::getMode: null this");

	return this->head & 0x3000;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// force refresh param table in the next render
void Sprite_invalidateParamTable(Sprite this){
	
	ASSERT(this, "Sprite::invalidateParamTable: null this");

	this->renderFlag |= __UPDATE_SIZE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this reallocate a write the bgmap definition in graphical memory
void Sprite_resetMemoryState(Sprite this){
	
	ASSERT(this, "Sprite::resetMemoryState: null this");

	//if affine or hbias mode, allocate inside paramtable		
	if(WRLD_AFFINE == Sprite_getMode(this)){
		
		ParamTableManager_allocate(ParamTableManager_getInstance(), this);
	}
	
	//allow to render
	//this->renderFlag = __UPDATE_HEAD;
	
	// write it in graphical memory
	Texture_resetMemoryState(this->texture);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set drawspec
void Sprite_setDrawSpec(Sprite this, const DrawSpec* const drawSpec){

	ASSERT(this, "Sprite::setDrawSpec: null this");

	this->drawSpec = *drawSpec;
	
}
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 										MAP FXs
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

/*
 * Affine FX
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sprite_noAFX(Sprite this, int direction){
	
	ASSERT(this, "Sprite::noAFX: null this");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sprite_scale(Sprite this){
	
	ASSERT(this, "Sprite::scale: null this");

	//put the map into memory calculating the number of char for each reference
	if(this->param){
		
		int cols = (int)Texture_getCols(this->texture) << 2;
		int rows = (int)Texture_getRows(this->texture) << 2;

		Affine_scale(this->param, this->drawSpec.scale.x, this->drawSpec.scale.y, 
				   (this->texturePosition.x << 3) + cols,
				   (this->texturePosition.y << 3) + rows,
				   cols, rows);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sprite_rotate(Sprite this, int angle){

	ASSERT(this, "Sprite::rotate: null this");

	// TODO
	if(this->param){
		
		int cols = Texture_getCols(this->texture) << 2;
		int rows = Texture_getRows(this->texture) << 2;

		Affine_rotateZ(this->param, this->drawSpec.scale.x, this->drawSpec.scale.y, 
				   (this->texturePosition.x << 3) + cols,
				   (this->texturePosition.y << 3) + rows,
				   cols, rows, angle);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TODO
	/*
	static int alpha=0;
	if(this->updateParamTable==true){
		affineRotateY(this->param,alpha,this->scale.x,this->scale.y, 
		(this->xOffset<<3)+(this->cols<<2), (this->yOffset<<3)+(this->rows<<2), 
		(this->cols<<2),(this->rows<<2));
		if(alpha++ >125){
			alpha=125;
					
		}
		// put down the flag
		Sprite_setUpdateParamTableFlag(this, false);
		
	}
	*/
	//delay(5);

/*
 * H-Bias FX
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sprite_squezeXHFX(Sprite this){

	ASSERT(this, "Sprite::squezeXHFX: null this");
	
	// TODO
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sprite_fireHFX(Sprite this){

	ASSERT(this, "Sprite::fireHFX: null this");

	// TODO
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sprite_waveHFX(Sprite this){
	ASSERT(this, "Sprite::waveHFX: null this");

	// TODO
}