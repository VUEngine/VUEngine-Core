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

#include <Entity.h>
#include <Optics.h>
#include <AnimatedSprite.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// define the Entity
__CLASS_DEFINITION(Entity);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


// add sprite
static void Entity_addSprites(Entity this, const SpriteDefinition* spritesDefinitions, int numberOfSprites);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
void Entity_constructor(Entity this, EntityDefinition* entityDefinition, int ID){

	// this is an abstract class so must initialize the vtable here
	// since this class does not have an allocator
	__SET_CLASS(Entity);	

	// construct base Container
	__CONSTRUCT_BASE(Container, __ARGUMENTS(ID));

	/* the sprite must be initializated in the derivated class */
	this->sprites = NULL;
	
	// initialize sprites
	if (entityDefinition) {
	
		Entity_addSprites(this, entityDefinition->spritesDefinitions, entityDefinition->numberOfSprites);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Entity_destructor(Entity this){

	if(this->sprites){
	
		VirtualNode node = VirtualList_begin(this->sprites);
	
		// move each child to a temporary list
		for(; node ; node = VirtualNode_getNext(node)){
			
			Sprite sprite = (Sprite)VirtualNode_getData(node);
			
			__DELETE(sprite);
		}	

		// delete the sprites
		__DELETE(this->sprites);
		
		this->sprites = NULL;
	}
	
	// destroy the super Container
	__DESTROY_BASE(Container);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process extra info in intialization
void Entity_setExtraInfo(Entity this, void* extraInfo){
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add sprite
static void Entity_addSprites(Entity this, const SpriteDefinition* spritesDefinitions, int numberOfSprites){

	if (spritesDefinitions) {
		
		int i = 0;
		
		//go through n sprites in entity's definition
		for(; i < numberOfSprites && i < __MAX_SPRITES_PER_ENTITY; i++){
	
			Entity_addSprite(this, &spritesDefinitions[i]);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add sprite
void Entity_addSprite(Entity this, const SpriteDefinition* spriteDefinition){

	Sprite sprite = NULL;
	
	switch(spriteDefinition->textureDefinition->charGroupDefinition.allocationType){
	
		case __ANIMATED:
		case __ANIMATED_SHARED:	

			// create the animated sprite
			sprite = (Sprite)__NEW(AnimatedSprite, __ARGUMENTS((void*)this, spriteDefinition));	
			
			break;
			
		default:
			
			// create the sprite
			sprite = __NEW(Sprite, __ARGUMENTS(spriteDefinition));
			
			break;
	}
	
	if(sprite) {
		
		if (!this->sprites) {
			
			this->sprites = __NEW(VirtualList);
		}
		
		VirtualList_pushBack(this->sprites, (void*)sprite);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// transform class
void Entity_transform(Entity this, Transformation* environmentTransform){

	int updateSpritePosition = __VIRTUAL_CALL(int, Entity, updateSpritePosition, this);
	int updateSpriteScale = __VIRTUAL_CALL(int, Entity, updateSpriteScale, this);

	// call base class's transform method
	Container_transform((Container)this, environmentTransform);

	if(this->sprites){

		VirtualNode node = VirtualList_begin(this->sprites);

		// move each child to a temporary list
		for(; node ; node = VirtualNode_getNext(node)){
			
			Sprite sprite = (Sprite)VirtualNode_getData(node);

			// calculate sprite's parallax
			Sprite_calculateParallax(sprite, this->transform.globalPosition.z);

			// update scale if needed
			if(updateSpriteScale){
		
				// calculate the scale	
				Sprite_calculateScale(sprite, this->transform.globalPosition.z);
				
				// scale the sprite
				Sprite_scale(sprite);			
	
				// calculate sprite's parallax
				Sprite_calculateParallax(sprite, this->transform.globalPosition.z);
			}
			
			//if screen is moving
			if(updateSpritePosition){
						
				//update sprite's 2D position 
				Sprite_setPosition(sprite, &this->transform.globalPosition);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve class's scale
Scale Entity_getScale(Entity this){
	
	return Sprite_getScale((Sprite)VirtualNode_getData(VirtualList_begin(this->sprites)));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve position
VBVec3D Entity_getPosition(Entity this){

	return this->transform.globalPosition;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve local position
VBVec3D Entity_getLocalPosition(Entity this){

	return this->transform.localPosition;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve sprite
VirtualList Entity_getSprites(Entity this){

	return this->sprites;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process a telegram
int Entity_handleMessage(Entity this, Telegram telegram){
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get width
int Entity_getWidth(Entity this){

	Sprite sprite = (Sprite)VirtualNode_getData(VirtualList_begin(this->sprites));
	Texture texture = Sprite_getTexture(sprite);
	
	// must calculate based on the scale because not affine Container must be enlarged
	return Optics_calculateRealSize(Texture_getCols(texture) << 3, Sprite_getMode(sprite), abs(Sprite_getScale(sprite).x));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get height
int Entity_getHeight(Entity this){

	Sprite sprite = (Sprite)VirtualNode_getData(VirtualList_begin(this->sprites));

	Texture texture = Sprite_getTexture(sprite);
	
	// must calculate based on the scale because not affine object must be enlarged
	return Optics_calculateRealSize(Texture_getRows(texture) << 3, Sprite_getMode(sprite), abs(Sprite_getScale(sprite).y));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get deep
int Entity_getDeep(Entity this){

	// must calculate based on the scale because not affine object must be enlarged
	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// whether it is visible
int Entity_isVisible(Entity this, int pad){
	
	Sprite sprite = (Sprite)VirtualNode_getData(VirtualList_begin(this->sprites));

	return Optics_isVisible(this->transform.globalPosition,
			Entity_getWidth(this),
			Entity_getHeight(this),
			Sprite_getDrawSpec(sprite).position.parallax,
			pad);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// determine if the entity is outside the game
int Entity_isOutsideGame(Entity this){
	
	return Optics_isOutsidePlayableArea(this->transform.globalPosition, 
			Entity_getWidth(this), 
			Entity_getHeight(this));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve state when unloading the entity 
int Entity_getInGameState(Entity this){

	return __UNLOADED;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create an entity in gameengine's memory
Entity Entity_load(EntityDefinition* entityDefinition, VBVec3D* position, int ID, void* extraInfo){
	
	ASSERT(entityDefinition, "Entity_load: NULL definition");
	ASSERT((int)entityDefinition->allocator, "Entity: no allocator defined");
	{
		// call the appropiate allocator to support inheritance!
		Entity entity = (Entity)((Entity (*)(EntityDefinition*, ...)) entityDefinition->allocator)(0, entityDefinition, ID);

		// setup entity if allocated and constructed
		if(entity){
	
			// set spatial position
			__VIRTUAL_CALL(void, Entity, setLocalPosition, entity, __ARGUMENTS(*position));
			 
			// process extra info
			if(extraInfo){
				
				__VIRTUAL_CALL(void, Entity, setExtraInfo, entity, __ARGUMENTS(extraInfo));
			}
			
			return entity;
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if must update sprite's position
int Entity_updateSpritePosition(Entity this){

	return (*((int*)_screenMovementState) || this->invalidateGlobalPosition);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if must update sprite's scale
int Entity_updateSpriteScale(Entity this){

	return (_screenMovementState->z || this->invalidateGlobalPosition);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set the direction
void Entity_setSpritesDirection(Entity this, int axis, int direction){
	
	if(this->sprites){

		VirtualNode node = VirtualList_begin(this->sprites);

		// move each child to a temporary list
		for(; node ; node = VirtualNode_getNext(node)){
			
			Sprite sprite = (Sprite)VirtualNode_getData(node);

			if (sprite) {
				
				Sprite_setDirection(sprite, axis, direction);
			}
		}
	}
}

