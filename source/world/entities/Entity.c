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
void Entity_constructor(Entity this, int ID){

	// this is an abstract class so must initialize the vtable here
	// since this class does not have an allocator
	__SET_CLASS(Entity);	

	// construct base Container
	__CONSTRUCT_BASE(Container, __ARGUMENTS(ID));

	/* the sprite must be initializated in the derivated class */
	this->sprite = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Entity_destructor(Entity this){

	if(this->sprite){
	
		// delete the sprite
		__DELETE(this->sprite);
	}
	
	// destroy the super Container
	__DESTROY_BASE(Container);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process extra info in intialization
void Entity_setExtraInfo(Entity this, void* extraInfo){
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// render class
void Entity_render(Entity this, Transformation environmentTransform){

	int updateSpritePosition = __VIRTUAL_CALL(int, Entity, updateSpritePosition, this);
	int updateSpriteScale = __VIRTUAL_CALL(int, Entity, updateSpriteScale, this);

	// call base class's render method
	Container_render((Container)this, environmentTransform);

	if(this->sprite){
		
		// update scale if needed
		if(updateSpriteScale){
	
			// calculate the scale	
			Sprite_calculateScale(this->sprite, this->transform.globalPosition.z);
			
			// scale the sprite
			Sprite_scale(this->sprite);			

			// calculate sprite's parallax
			Sprite_calculateParallax(this->sprite, this->transform.globalPosition.z);
		}
		
		//if screen is moving
		if(updateSpritePosition){
					
			//update sprite's 2D position 
			Sprite_setPosition(this->sprite, &this->transform.globalPosition);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve class's scale
Scale Entity_getScale(Entity this){
	
	ASSERT(this->sprite);
	
	return Sprite_getScale(this->sprite);
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
Sprite Entity_getSprite(Entity this){

	ASSERT(this->sprite);
	
	return this->sprite;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process a telegram
int Entity_handleMessage(Entity this, Telegram telegram){
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get width
int Entity_getWidth(Entity this){

	Texture texture = Sprite_getTexture(this->sprite);
	
	ASSERT(this->sprite);
	ASSERT(texture);
	
	// must calculate based on the scale because not affine Container must be enlarged
	return vbjCalculateRealSize(Texture_getCols(texture) << 3, Sprite_getMode(this->sprite), abs(Sprite_getScale(this->sprite).x));
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get height
int Entity_getHeight(Entity this){

	Texture texture = Sprite_getTexture(this->sprite);
	
	ASSERT(this->sprite);
	ASSERT(texture);
	
	// must calculate based on the scale because not affine object must be enlarged
	return vbjCalculateRealSize(Texture_getRows(texture) << 3, Sprite_getMode(this->sprite), abs(Sprite_getScale(this->sprite).y));
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// whether it is visible
int Entity_isVisible(Entity this, int pad){
	
	ASSERT(this->sprite);

	return vbjIsVisible(this->transform.globalPosition,
			Entity_getWidth(this),
			Entity_getHeight(this),
			Sprite_getDrawSpec(this->sprite).position.parallax,
			pad);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// determine if the entity is outside the game
int Entity_isOutsideGame(Entity this){
	
	ASSERT(this->sprite);
	
	return vbjOutsideGame(this->transform.globalPosition, 
			Entity_getWidth(this), 
			Entity_getHeight(this));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve state when unloading the entity 
int Entity_getInGameState(Entity this){

	return kUnloaded;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create an entity in gameengine's memory
Entity Entity_load(EntityDefinition* entityDefinition, VBVec3D* position, int ID, void* extraInfo){
	
	ASSERT((int)entityDefinition->allocator, No allocator define);
	
	ASSERT(entityDefinition, Entity_load: NULL definition);
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
