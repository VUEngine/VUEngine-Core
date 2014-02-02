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
#include <HardwareManager.h>
#include <SpriteManager.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

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
static void Stage_setEntityState(Stage this, int ID, int inGameState);

// get entity's state
static inline int Stage_getEntityState(Stage this, int ID);

// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, int loadOnlyInRangeEntities, int loadAllEntitiesInRange);

// unload non visible entities
static void Stage_unloadOutOfRangeEntities(Stage this, int unloadProgressively);


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
	
	int i = 0;
	
	for(i = 0; i < __ENTITIES_IN_STAGE; i++){
		
		this->entityStateRegister[i] = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Stage_destructor(Stage this){
	
	// destroy the super object
	__DESTROY_BASE(Container);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update world's entities state
static void Stage_setEntityState(Stage this, int ID, int inGameState){
	
	int bitIndex =  ID * __STAGE_BITS_PER_ENTITY;
	
	// arrayIndex = bitIndex / (sizeof(WORD) << 3);
	int arrayIndex = bitIndex >> (sizeof(WORD) + 1);
	
	// displacement =  bitIndex % (sizeof(WORD) << 3);
	int displacement =  bitIndex & ((sizeof(WORD) << 3) - 1);

	WORD mask = this->entityStateRegister[arrayIndex] & Utilities_rotateBits(0xFFFFFFFF << __STAGE_BITS_PER_ENTITY, displacement, __ROT_LEFT);
	
	// make sure we are unloading an entity loaded by 
	// me and not but the programmer
	if(0 > ID){
		
		return;
	}
	
	mask |=  ((WORD)inGameState << displacement);

	this->entityStateRegister[arrayIndex] = mask;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get entity's state
static inline int Stage_getEntityState(Stage this, int ID){
	
	int bitIndex =  ID * __STAGE_BITS_PER_ENTITY;
	
	// arrayIndex = bitIndex / (sizeof(WORD) << 3);
	int arrayIndex = bitIndex >> (sizeof(WORD) + 1);
	
	// displacement =  bitIndex % (sizeof(WORD) << 3);
	int displacement =  bitIndex & ((sizeof(WORD) << 3) - 1);

	return (int)(this->entityStateRegister[arrayIndex] >> displacement) & 0x03;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// place holder for objects designed around OBJECTS in the VB hardware
void Stage_setupObjActor(int *actor,int x,int y, int z){

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// determine if a point is visible
inline static int Stage_inLoadRange(Stage this, VBVec3D* const position, int width, int height){
	
	int xLowLimit = -(width) ;
	int xHighLimit = __SCREENWIDTH + (width);

	int yLowLimit = -(height) ;
	int yHighLimit = __SCREENHEIGHT + (height);

	VBVec2D position2D;
	
	VBVec3D position3D = Optics_normalizePosition(position);
	
	//project the position to 2d space
	Optics_projectTo2D(&position2D, &position3D);

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
// load stage's entites
void Stage_load(Stage this, StageDefinition* stageDefinition, int loadOnlyInRangeEntities){
	
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
	Stage_loadEntities(this, loadOnlyInRangeEntities, true);

	//load background music
	//SoundManager_loadBGM(SoundManager_getInstance(),(u16 (*)[6])this->bgm);
	SoundManager_loadBGM(SoundManager_getInstance(), (u16 (*)[6])stageDefinition->bgm);
	
	//setup the column table
	HardwareManager_setupColumnTable(HardwareManager_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add entity to the stage
Entity Stage_addEntity(Stage this, EntityDefinition* entityDefinition, VBVec3D* position, int inGameIndex, void *extraInfo){

	if(entityDefinition)
	{
		Entity entity = Entity_load(entityDefinition, position, inGameIndex, extraInfo);
		
		// create the entity and add it to the world
		Container_addChild((Container)this, (Container)entity);
		
		// static to avoid call to _memcpy
		static Transformation environmentTransform = {
				// local position
				{0, 0, 0},
				// global position
				{0, 0, 0},
				// scale
				{1, 1},
				// rotation
				{0, 0, 0}			
		};
		
		__VIRTUAL_CALL(void, Container, transform, (Container)entity, __ARGUMENTS(&environmentTransform));

		return entity;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, int loadOnlyInRangeEntities, int loadAllEntitiesInRange){

	StageDefinition* world = this->stageDefinition;
	
	int i = 0;

	// TODO: don't check every entity in the stage's definition
	// must implement an algorithm which only check a portion of it
	for(; i < __ENTITIESPERWORLD && world->entities[i].entity; i++){
		
		//if entity isn't loaded and haven't been killed
		int inGameState = Stage_getEntityState(this, i);

		if(!(__LOADED & inGameState)){
		//if(kLoaded != inGameState && kDead != inGameState){
			
			// test if the position is inside the game
			EntityDefinition* entityDefinition = world->entities[i].entity;
			
			VBVec3D position = {
					ITOFIX19_13(world->entities[i].position.x),
					ITOFIX19_13(world->entities[i].position.y),
					ITOFIX19_13(world->entities[i].position.z)
			};
			
			// if entity in load range
			if(!loadOnlyInRangeEntities || Stage_inLoadRange(this, &position, 
					entityDefinition->spritesDefinitions[0].textureDefinition->cols << 2, 
					entityDefinition->spritesDefinitions[0].textureDefinition->rows << 2)){

				VPUManager_waitForFrame(VPUManager_getInstance());

				Stage_addEntity(this, entityDefinition, &position, i, world->entities[i].extraInfo);
				
				if(!(__LOADED & inGameState)){
					 
					inGameState |= __LOADED;
				}
				
				//set actor state as loaded
				Stage_setEntityState(this, i, inGameState);
				
				if(loadOnlyInRangeEntities && !loadAllEntitiesInRange) {

					break;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// unload non visible entities
static void Stage_unloadOutOfRangeEntities(Stage this, int unloadProgressively){

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
			if(__LOADED & inGameState){
				
				inGameState &= ~__LOADED;
			}
			
			// update in game state
			Stage_setEntityState(this, Container_getID((Container)entity), inGameState);
			
			// register entity to remove
			VirtualList_pushBack(removedEntities, (const BYTE* const )entity);
			
			VPUManager_waitForFrame(VPUManager_getInstance());

			if(unloadProgressively) {
				
				break;
			}
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
// stream entities according to screen's position
void Stage_stream(Stage this){
	
	// if the screen is moving
	if(*((u8*)_screenMovementState)){

		static int turn = __RENDER_FPS >> 1;
			
		if(!--turn){

			// wait for frame before rendering
			//VPUManager_waitForFrame(VPUManager_getInstance());

			// unload not visible objects
			Stage_unloadOutOfRangeEntities(this, true);	
			
			// enable interrupts
			VPUManager_displayOn(VPUManager_getInstance());
			
			turn = __RENDER_FPS >> 1;
		}
		else if (((__RENDER_FPS >> 1) >> 1) == turn) {

			// wait for frame before rendering
			//VPUManager_waitForFrame(VPUManager_getInstance());

			// load visible objects	
			Stage_loadEntities(this, true, true);

			// enable interrupts
			VPUManager_displayOn(VPUManager_getInstance());

		}		
		
	}
}
