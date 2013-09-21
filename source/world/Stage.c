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

#include <Stage.h>
#include <Globals.h>
#include <Optics.h>
#include <SoundManager.h>
#include <Screen.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#define __STAGE_BITS_PER_ENTITY		2
#define __ENTITIESINSTAGE (__ENTITIESPERWORLD / (sizeof(WORD) << 3) * __STAGE_BITS_PER_ENTITY)

WORD entityStateRegister[__ENTITIESINSTAGE];
int stateRegisterInUse = false;

// define the Stage
__CLASS_DEFINITION(Stage);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//class's constructor
static void Stage_constructor(Stage this);

// update world's entities state
static void Stage_setEntityState(int ID, int inGameState);

// get entity's state
static inline int Stage_getEntityState(int ID);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S ATTRIBUTES
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
__CLASS_NEW_DEFINITION(Stage)
__CLASS_NEW_END(Stage);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Stage_constructor(Stage this){

	// construct base object
	__CONSTRUCT_BASE(Container, __ARGUMENTS(-1));
	
	this->stageDefinition = NULL;
	
	// by default, don't save entities states
	this->saveEntityStates = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Stage_destructor(Stage this){
	
	// relieve the entity's state register
	if(this->saveEntityStates){
		
		stateRegisterInUse = false;
	}
	
	// destroy the super object
	__DESTROY_BASE(Container);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update world's entities state
static void Stage_setEntityState(int ID, int inGameState){
	
	int bitIndex =  ID * __STAGE_BITS_PER_ENTITY;
	
	// arrayIndex = bitIndex / (sizeof(WORD) << 3);
	int arrayIndex = bitIndex >> (sizeof(WORD) + 1);
	
	// displacement =  bitIndex % (sizeof(WORD) << 3);
	int displacement =  bitIndex & ((sizeof(WORD) << 3) - 1);

	WORD mask = entityStateRegister[arrayIndex] & vbRotate(0xFFFFFFFF << __STAGE_BITS_PER_ENTITY, displacement, __ROT_LEFT);
	
	// make sure we are unloading an entity loaded by 
	// me and not but the programmer
	if(0 > ID){
		
		return;
	}
	
	mask |=  ((WORD)inGameState << displacement);

	entityStateRegister[arrayIndex] = mask;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get entity's state
static inline int Stage_getEntityState(int ID){
	
	int bitIndex =  ID * __STAGE_BITS_PER_ENTITY;
	
	// arrayIndex = bitIndex / (sizeof(WORD) << 3);
	int arrayIndex = bitIndex >> (sizeof(WORD) + 1);
	
	// displacement =  bitIndex % (sizeof(WORD) << 3);
	int displacement =  bitIndex & ((sizeof(WORD) << 3) - 1);

	return (int)(entityStateRegister[arrayIndex] >> displacement) & 0x03;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// place holder for objects designed around OBJECTS in the VB hardware
void Stage_setupObjActor(int *actor,int x,int y, int z){

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// load stage's entites
void Stage_load(Stage this, StageDefinition* stageDefinition){
	
	int i = 0;

	// stop all sounds
	SoundManager_stopAllSound(SoundManager_getInstance());
	
	// set world's definition
	this->stageDefinition = stageDefinition;
		
	// this->number = worldNumber;
	// set world's limits
	GameWorld_setSize(GameWorld_getInstance(), stageDefinition->size);
	
	// set screen's position
	Screen_setPosition(Screen_getInstance(), stageDefinition->screenPosition);
	
	//load Stage's bgm
	//this->bgm = (u16 (*)[6])stageDefinition->bgm;
	
	// load entities
	for(i = 0; stageDefinition->entities[i].entity; i++){
		
		// test if the position is inside the game
		EntityDefinition* entityDefinition = stageDefinition->entities[i].entity;

		VBVec3D position = {
				ITOFIX19_13(stageDefinition->entities[i].position.x),
				ITOFIX19_13(stageDefinition->entities[i].position.y),
				ITOFIX19_13(stageDefinition->entities[i].position.z)
		};

		// if entity is visible
		if(vbjIsVisible(position, 
				entityDefinition->spriteDefinition.textureDefinition->cols << 3, 
				entityDefinition->spriteDefinition.textureDefinition->rows << 3,
				0,
				__ENTITYLOADPAD
				)){

			//load actor in world			
			Stage_addEntity(this, stageDefinition->entities[i].entity, &position, i, stageDefinition->entities[i].extraInfo);
			
			// save entity state if needed
			if(this->saveEntityStates){
				
				Stage_setEntityState(i, kLoaded);
			}
		}		
		else{
			
			if(this->saveEntityStates){
				
				//otherwise set object as alive but not loaded yet
				Stage_setEntityState(i, kLoaded);
			}
		}
	}
	//load background music
	//SoundManager_loadBGM(SoundManager_getInstance(),(u16 (*)[6])this->bgm);
	
	//setup the column table
	vbSetColTable();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// determine if a point is visible
inline static int Stage_inLoadRange(Stage this, VBVec3D* const position, int width, int height){
	
	int xLowLimit = -(width) ;
	int xHighLimit = __SCREENWIDTH + (width);

	int yLowLimit = -(height) ;
	int yHighLimit = __SCREENHEIGHT + (height);

	VBVec2D position2D;
	
	VBVec3D position3D = vbjNormalizePosition(position);
	
	//project the position to 2d space
	vbjProjectTo2D(&position2D, &position3D);

	//(x >= min && x < max) can be transformed into (unsigned)(x-min) < (max-min)
	// check x visibility
	if(!((unsigned)(position2D.x - xLowLimit) < (position2D.x - xHighLimit))){		
		
		// check y visibility
		if(!((unsigned)(position2D.y - yLowLimit) < (position2D.y - yHighLimit))){
		
			// check z visibility
			//if(position3D.z >= _screenPosition->z && position3D.z < _screenPosition->z + ITOFIX19_13(this->size.z)){
				
				return false;
			//}
		}	
	}
	
	xLowLimit -= __ENTITYLOADPAD;
	xHighLimit += __ENTITYLOADPAD;
	yLowLimit -= __ENTITYLOADPAD;
	yHighLimit += __ENTITYLOADPAD;	
	
	// check x visibility
	if((unsigned)(position2D.x - xLowLimit) < (position2D.x - xHighLimit)){
		
		// check y visibility
		if((unsigned)(position2D.y - yLowLimit) < (position2D.y - yHighLimit)){
		
			// check z visibility
			//if(position3D.z >= _screenPosition->z && position3D.z < _screenPosition->z + ITOFIX19_13(this->size.z)){
				
				return true;
			//}
		}	
	}	
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// load entities on demand (if they aren't loaded and are visible)
void Stage_loadEntities(Stage this){

	StageDefinition* world = this->stageDefinition;
	
	int i = 0;

	// TODO: don't check every entity in the stage's definition
	// must implement an algorithm which only check a portion of it

	//go through n entities in Stage	
	for(; i < __ENTITIESPERWORLD && world->entities[i].entity; i++){
		
		//if entity isn't loaded and haven't been killed
		int inGameState = Stage_getEntityState(i);

		if(kLoaded != inGameState){
		//if(kLoaded != inGameState && kDead != inGameState){
			
			// test if the position is inside the game
			EntityDefinition* entityDefinition = world->entities[i].entity;
			
			VBVec3D position = {
					ITOFIX19_13(world->entities[i].position.x),
					ITOFIX19_13(world->entities[i].position.y),
					ITOFIX19_13(world->entities[i].position.z)
			};
			
			// if entity in load range
			if(Stage_inLoadRange(this, &position, 
					entityDefinition->spriteDefinition.textureDefinition->cols << 2, 
					entityDefinition->spriteDefinition.textureDefinition->rows << 2)){
				
				Stage_addEntity(this, entityDefinition, &position, i, world->entities[i].extraInfo);
				
				if(kUnloaded == inGameState){
					 
					inGameState = kLoaded;
				}
				
				//set actor state as loaded
				Stage_setEntityState(i, inGameState);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add entity to the stage
Entity Stage_addEntity(Stage this, EntityDefinition* entityDefinition, VBVec3D* position, int inGameIndex, void *extraInfo){

	if(entityDefinition)
	{
		Entity entity = Entity_load(entityDefinition, position, inGameIndex, extraInfo);
		
		// create the entity and add it to the world
		Container_addChild((Container)this, (Container)entity);
		
		return entity;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// unload non visible entities
void Stage_unloadEntities(Stage this){

	// need a temporal list to remove and delete entities
	VirtualList removedEntities = __NEW(VirtualList);
	VirtualNode node = VirtualList_begin(this->children);

	CACHE_ENABLE;
	// check which actors must be unloaded
	for(; node; node = VirtualNode_getNext(node)){
		
		// get next entity
		Entity entity = (Entity)VirtualNode_getData(node);
		
		//if the entity isn't visible inside the view field, unload it
		if(!__VIRTUAL_CALL(int, Entity, isVisible, entity, __ARGUMENTS(__ENTITYLOADPAD))){		

			int inGameState = __VIRTUAL_CALL(int, Entity, getInGameState, entity);

			// if state is loaded... just set it as unloaded
			if(kLoaded == inGameState){
				
				inGameState = kUnloaded;
			}
			
			// update in game state
			Stage_setEntityState(Container_getID((Container)entity), inGameState);
			
			// register entity to remove
			VirtualList_pushBack(removedEntities, (const BYTE* const )entity);
		}
	}
	
	// now remove and delete entities
	for(node = VirtualList_begin(removedEntities); node; node = VirtualNode_getNext(node)){
	
		// get next entity
		Entity entity = (Entity)VirtualNode_getData(node);

		// destroy it
		__DELETE(entity);		
	}
	
	// destroy the temporal list
	__DELETE(removedEntities);
	
	CACHE_DISABLE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set save entity's states flag
int Stage_saveEntityStates(Stage this){
	
	ASSERT(!stateRegisterInUse, Stage: state register already in use);
	
	// only save states if register not in use yet
	if(!stateRegisterInUse){

		int i = 0;
		// initialize world's actor states
		for (i = 0; i < __ENTITIESINSTAGE; i++){
			
			Stage_setEntityState(i, kUnloaded);
		}
		
		// set flags
		stateRegisterInUse = this->saveEntityStates = true;
		
		return true;
	}
	
	return false;
}