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

#include <AnimatedSprite.h>
#include <string.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// define the AnimatedSprite
__CLASS_DEFINITION(AnimatedSprite);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

extern int strcmp(const char *, const char *);

// class's constructor
static void AnimatedSprite_constructor(AnimatedSprite this, Object owner, const SpriteDefinition* spriteDefinition);

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
__CLASS_NEW_DEFINITION(AnimatedSprite, __PARAMETERS(Object owner, const SpriteDefinition* spriteDefinition))
__CLASS_NEW_END(AnimatedSprite, __ARGUMENTS(owner, spriteDefinition));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void AnimatedSprite_constructor(AnimatedSprite this, Object owner, const SpriteDefinition* spriteDefinition){

	// construct base object
	__CONSTRUCT_BASE(Sprite, __ARGUMENTS(spriteDefinition));

	// since the offset will be moved during animation, must save it
	this->originalTextureXOffset = Texture_getXOffset(this->texture); 
	
	// set the owner
	this->owner = owner;
	
	// initialize frame tracking
	this->actualFrame = 0;	
	this->previousFrame = 0;
	
	// initialize frame head
	this->frameDelay = 0;
	this->frameDelayDelta = -1;
	
	// don't project position
	this->calculatePositionFlag = true;
	
	// intialize animation function
	this->animationFunction = NULL;
	
	// not playing anything yet
	this->playing = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//destructor
void AnimatedSprite_destructor(AnimatedSprite this){
	
	// first make sure the map's bgmap definition points to the
	// beginnig of the bgmap's definition in ROM, otherwhise
	// the BGTextureManager will not be able to remove it from its
	// records
	//Texture_setXOffset(this->texture, this->originalTextureXOffset);

	// destroy the super object
	__DESTROY_BASE(Sprite);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve frame allocation type
int AnimatedSprite_getType(AnimatedSprite this){
	
	return CharGroup_getAllocationType(Texture_getCharGroup(this->texture));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve frame's map
Texture AnimatedSprite_getTexture(AnimatedSprite this){
	
	return this->texture;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write char animation frame to char memory
void AnimatedSprite_writeAnimation(AnimatedSprite this){

	// write according to the allocation type
	switch(CharGroup_getAllocationType(Texture_getCharGroup(this->texture))){
	
		case __ANIMATED:
			{
				CharGroup auxCharGroup = NULL;
				
				BYTE* charDefinition = NULL;

				// get chargroup address
				auxCharGroup = Texture_getCharGroup(this->texture);
				
				// get chargroup's charset's definition
				charDefinition = CharGroup_getCharDefinition(auxCharGroup);
				
				// move chargroup's charset's definition to the next frame chars
				CharGroup_setCharDefinition(auxCharGroup, 
						charDefinition + Texture_getNumberOfChars(this->texture) * 
						(this->animationFunction->frames[this->actualFrame] << 4));
				
				//write charset
				CharGroup_write(Texture_getCharGroup(this->texture));
				
				//move back to base chargroup's charset's definition
				CharGroup_setCharDefinition(auxCharGroup, charDefinition);
				
				// must update the whole world layer in the next render cicle
				this->renderFlag = __UPDATEHEAD;				
			}
			
			break;
			
		case __ANIMATED_SHARED:
			
			// shared animations are updated writing the param table values
			this->texturePosition.x = this->originalTextureXOffset + Texture_getCols(this->texture)
										* this->animationFunction->frames[this->actualFrame];
			
			// must retrieve the owner scale first (to allow for flips)
			Sprite_invalidateParamTable((Sprite)this);

			break;				
			
		case __ANIMATED_SHARED_2:
			
			// TODO: for expansions
			
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// if true, the frame's screen position will be calculated in the
// next render cicle
void AnimatedSprite_setCalculatePositionFlag(AnimatedSprite this, int calculatePositionFlag){
	
	this->calculatePositionFlag = calculatePositionFlag;	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve actual frame index of animation
int AnimatedSprite_getActualAnimatedSprite(AnimatedSprite this){

	return this->actualFrame;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve previous frame index of animation
int AnimatedSprite_getPreviousAnimatedSprite(AnimatedSprite this){
	
	return this->previousFrame;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set actual frame of animation
void AnimatedSprite_setActualAnimatedSprite(AnimatedSprite this, int actualFrame){
	
	this->actualFrame = actualFrame;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set previos frame index of animation
void AnimatedSprite_setPreviousAnimatedSprite(AnimatedSprite this, int previousFrame){
	
	// TODO: this method should not exist
	this->previousFrame = previousFrame;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve frame cicle
int AnimatedSprite_getAnimatedSpriteCicle(AnimatedSprite this){
	
	return this->frameDelay;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set frame cicle 
void AnimatedSprite_setAnimatedSpriteCicle(AnimatedSprite this, int frameCicle){

	this->frameDelay = frameCicle;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve frame cicle delta
int AnimatedSprite_getAnimatedSpriteCicleDelta(AnimatedSprite this){
	
	return this->frameDelayDelta;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set frame cicle delta
void AnimatedSprite_setAnimatedSpriteCicleDelta(AnimatedSprite this, int frameCicleDelta){

	this->frameDelayDelta = frameCicleDelta;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// animate the frame
void AnimatedSprite_animate(AnimatedSprite this){
		
	// first check for a valid animation function
	if(!this->animationFunction){
		
		return;
	}
	
	// if the actual frame was set to -1
	// it means that a not loop animation has been completed
	if(-1 == this->actualFrame){
		
		return;
	}
	
	// show the next frame
	if(this->actualFrame >= this->animationFunction->numberOfFrames){
		
		// the last frame has been reached
		if(this->animationFunction->onAnimationComplete){
		
			// call notifying
			((void (*)(void* owner))this->animationFunction->onAnimationComplete)(this->owner);
		}

		// rewind to first frame
		this->actualFrame = 0;

		// if the animation is not a loop
		if(!this->animationFunction->loop){

			// not playing anymore
			this->playing = false;
		
			// invalidate animation
			this->actualFrame = -1;
					
			return;
		}
	}
	
	// if the frame has changed
	if(this->actualFrame != this->previousFrame){

		// write the new frame of animation
		AnimatedSprite_writeAnimation(this);
		
		// don't write animation each time, only when the animation
		// has changed
		this->previousFrame = this->actualFrame; 
	}
	
	this->frameDelay += this->frameDelayDelta;
	
	// reduce frame delay count	
	if(0 >= this->frameDelay){
		
		// incrase the frame to show
		this->previousFrame = this->actualFrame++;
		
		// reset frame delay
		this->frameDelay = this->animationFunction->delay;
		
		// if the delay is negative
		if(0 > this->frameDelay){	
						 
			// pick up a random delay
			this->frameDelay = Utilities_random(Utilities_randomSeed(), abs(this->frameDelay));
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// render frame
void AnimatedSprite_update(AnimatedSprite this){
	
	if(!Clock_isPaused(_inGameClock)){
	
		// first animate the frame
		AnimatedSprite_animate(this);
	}
}	

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve frame's map's height
int AnimatedSprite_getRows(AnimatedSprite this){
	
	return Texture_getRows(this->texture);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve frame's map's width
int AnimatedSprite_getCols(AnimatedSprite this){
	
	return Texture_getCols(this->texture);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// allocate a write in graphic memory again
void AnimatedSprite_resetMemoryState(AnimatedSprite this, int worldLayer){

	// make sure is is allocated and written
	Texture_resetMemoryState(this->texture);
	
	this->worldLayer= worldLayer;
	
	// recover frame's current frame animation
	AnimatedSprite_setPreviousAnimatedSprite(this, 255);
	
	// write the animation
	AnimatedSprite_writeAnimation(this);				
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// play animation
void AnimatedSprite_play(AnimatedSprite this, AnimationDescription* animationDescription, char* functionName){
	
	int i = 0;

	// search for the animation function
	for(; animationDescription->animationFunctions[i]; i++ ){
		
		// compare function's names
		if(!strcmp((const char *)functionName, (const char *)animationDescription->animationFunctions[i]->name)){
			
			// setup animation frame
			this->animationFunction = animationDescription->animationFunctions[i];
			
			// force frame writing in the next update
			this->previousFrame = -1;
			
			// reset frame to play
			this->actualFrame = 0;

			// set frame delay to 1 to force the writing of the first
			// animation frame in the next update
			this->frameDelay = 1;
			
			// it's playing now
			this->playing = true;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is play animation
int AnimatedSprite_isPlayingFunction(AnimatedSprite this, AnimationDescription* animationDescription, char* functionName){
	
	// compare function's names
	return !strcmp((const char *)functionName, (const char *)this->animationFunction->name);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// is playing animation
int AnimatedSprite_isPlaying(AnimatedSprite this){
	
	return this->playing;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write sprite to graphic memory
void AnimatedSprite_write(AnimatedSprite this){

	// write the texture
	Texture_write(this->texture);

	// save the original offset
	this->originalTextureXOffset = Texture_getXOffset(this->texture);
}