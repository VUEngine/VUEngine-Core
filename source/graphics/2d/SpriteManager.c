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

#include <SpriteManager.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define SpriteManager_ATTRIBUTES				\
												\
	/* super's attributes */					\
	Object_ATTRIBUTES;							\
												\
	/* list of sprites to render */				\
	Sprite sprites[__OBJECTLISTTAM];			\
												\
	/* next world layer	*/						\
	int freeLayer;


__CLASS_DEFINITION(SpriteManager);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//class's constructor
static void SpriteManager_constructor(SpriteManager this);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S ATTRIBUTES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __TOTAL_LAYERS	31

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

__SINGLETON(SpriteManager);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void SpriteManager_constructor(SpriteManager this){

	// construct base object
	__CONSTRUCT_BASE(Object);

	SpriteManager_reset(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void SpriteManager_destructor(SpriteManager this){
	
	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset
void SpriteManager_reset(SpriteManager this){

	int i = 0;
	
	for ( i = 0; i < __OBJECTLISTTAM; i++){
		
		this->sprites[i] = NULL;
	}
	
	this->freeLayer = __TOTAL_LAYERS;
}	

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if any entity must be assigned another world layer
void SpriteManager_checkLayers(SpriteManager this){

	int i = 0;

	DrawSpec drawSpec = Sprite_getDrawSpec(this->sprites[0]);

	CACHE_ENABLE;
	for(;i < __OBJECTLISTTAM - 1 &&  this->sprites[i + 1]; i++){

		DrawSpec nextDrawSpec = Sprite_getDrawSpec(this->sprites[i + 1]);

		// check if z positions are swaped
		if(nextDrawSpec.position.parallax > drawSpec.position.parallax){
			
			// get each entity's layer
			int worldLayer1 = Sprite_getWorldLayer(this->sprites[i]);
			int worldLayer2 = Sprite_getWorldLayer(this->sprites[i + 1]);
			
			// swap array entries
			Sprite auxSprite = this->sprites[i];			
			this->sprites[i] = this->sprites[i + 1];			
			this->sprites[i + 1] = auxSprite;

			// swap layers
			Sprite_setWorldLayer(this->sprites[i], worldLayer1);
			Sprite_setWorldLayer(this->sprites[i + 1], worldLayer2);

			break;
		}
		
		drawSpec = nextDrawSpec;
	}
	CACHE_DISABLE;
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteManager_addSprite(SpriteManager this, Sprite sprite){
	
	int i = 0;
	
	// find the last render object's index
	for(; this->sprites[i] && i < __OBJECTLISTTAM; i++);
	
	if(i < __OBJECTLISTTAM){
		
		// set entity into slot
		this->sprites[i] = sprite;

		// set layer
		Sprite_setWorldLayer(this->sprites[i], __TOTAL_LAYERS - i);
	}

	// reasign layers
	SpriteManager_assignLayers(this);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteManager_removeSprite(SpriteManager this, Sprite sprite){
	
	int i = 0;
	
	CACHE_ENABLE;
	
	// search for the entity to remove
	for(; this->sprites[i] != sprite && i < __OBJECTLISTTAM; i++);
	
	// if found
	if(i < __OBJECTLISTTAM){
		
		int j = i;
		// must render the whole entities after the entity to be removed twice
		// to avoid flickering
		for(; this->sprites[j + 1] && j < __OBJECTLISTTAM - 1; j++){
			
			this->sprites[j] = this->sprites[j + 1];
			
			Sprite_setWorldLayer(this->sprites[j], 1);
		}
		
		// remove object from list
		this->sprites[j] = NULL;
	}
	
	CACHE_DISABLE;

	// reassign layers
	SpriteManager_assignLayers(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteManager_assignLayers(SpriteManager this){
	
	int i = 0;
	
	this->freeLayer = __TOTAL_LAYERS;
	
	for(i = 0; this->sprites[i] && i < __OBJECTLISTTAM; i++){
		
		// change layers
		Sprite_setWorldLayer(this->sprites[i], this->freeLayer--);
	}	

	ASSERT(this->freeLayer > 0, SpriteManager: worldLayers depleted);
	
	//update printing world layer for non textboxs (use mainly for debug)	
	vbjRenderOutputText(this->freeLayer, TextureManager_getFreeBgmap(TextureManager_getInstance()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// render sprites
void SpriteManager_render(SpriteManager this){

	int i = 0;
	
	for(i = 0; this->sprites[i] && i < __OBJECTLISTTAM; i++){
		
		//render sprite	
		Sprite_render((Sprite)this->sprites[i]);
	}	
	
	if(0 == i){
		
		//update printing world layer for non textboxs (use mainly for debug)	
		vbjRenderOutputText(this->freeLayer, TextureManager_getFreeBgmap(TextureManager_getInstance()));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve free layer
int SpriteManager_getFreeLayer(SpriteManager this){
	
	return this->freeLayer;
}
