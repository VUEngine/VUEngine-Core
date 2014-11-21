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
#include <Texture.h>


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

#undef __ENTITY_LOAD_PAD
#define __ENTITY_LOAD_PAD 20
#define __ENTITY_UNLOAD_PAD 30

//class's constructor
static void Stage_constructor(Stage this);

// update world's entities state
static void Stage_setEntityState(Stage this, int ID, int inGameState);

// get entity's state
static inline int Stage_getEntityState(Stage this, int ID);

// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, int loadOnlyInRangeEntities, int loadAllEntitiesInRange, int disableInterrupts);

// preload textures
static void Stage_loadTextures(Stage this);

// unload non visible entities
static void Stage_unloadOutOfRangeEntities(Stage this, int unloadProgressively);

// load and retrieve a texture (for internal usage: use TextureManager_get)
Texture TextureManager_loadTexture(TextureManager this, TextureDefinition* textureDefinition, int isPreload);

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

	ASSERT(this, "Stage::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Container, __ARGUMENTS(-1));
	
	this->stageDefinition = NULL;
	
	this->flushCharGroups = true;
	
	int i = 0;
	
	for(i = 0; i < __ENTITIES_IN_STAGE; i++){
		
		this->entityStateRegister[i] = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Stage_destructor(Stage this){
	
	ASSERT(this, "Stage::destructor: null this");

	// destroy the super object
	__DESTROY_BASE(Container);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// update world's entities state
static void Stage_setEntityState(Stage this, int ID, int inGameState){
	
	ASSERT(this, "Stage::setEntityState: null this");

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
	
	ASSERT(this, "Stage::getEntityState: null this");

	int bitIndex =  ID * __STAGE_BITS_PER_ENTITY;
	
	// arrayIndex = bitIndex / (sizeof(WORD) << 3);
	int arrayIndex = bitIndex >> (sizeof(WORD) + 1);
	
	// displacement =  bitIndex % (sizeof(WORD) << 3);
	int displacement =  bitIndex & ((sizeof(WORD) << 3) - 1);

	return (int)(this->entityStateRegister[arrayIndex] >> displacement) & 0x03;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// place holder for objects designed around OBJECTS in the VB hardware
void Stage_setupObjActor(Stage this, int *actor,int x,int y, int z){

	// TODO
	ASSERT(this, "Stage::setupObjActor: null this");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// determine if a point is visible
inline static int Stage_inLoadRange(Stage this, VBVec3D* const position, int width, int height){
	
	ASSERT(this, "Stage::inLoadRange: null this");

	fix19_13 xLowLimit = ITOFIX19_13(-(width));
	fix19_13 xHighLimit = ITOFIX19_13(__SCREEN_WIDTH + (width));

	fix19_13 yLowLimit = ITOFIX19_13(-(height));
	fix19_13 yHighLimit = ITOFIX19_13(__SCREEN_HEIGHT + (height));

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
	
	xLowLimit -= ITOFIX19_13(__ENTITY_LOAD_PAD);
	xHighLimit += ITOFIX19_13(__ENTITY_LOAD_PAD);
	yLowLimit -= ITOFIX19_13(__ENTITY_LOAD_PAD);
	yHighLimit += ITOFIX19_13(__ENTITY_LOAD_PAD);	
	
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
	
	ASSERT(this, "Stage::load: null this");

	// stop all sounds
	SoundManager_stopAllSound(SoundManager_getInstance());
	
	// set world's definition
	this->stageDefinition = stageDefinition;
		
	// this->number = worldNumber;
	// set world's limits
	GameWorld_setSize(GameWorld_getInstance(), stageDefinition->size);
	
	// set screen's position
	Screen_setPosition(Screen_getInstance(), stageDefinition->screenPosition);

	// preload textures
	Stage_loadTextures(this);
	
	//load Stage's bgm
	//this->bgm = (u16 (*)[6])stageDefinition->bgm;

	// load entities
	Stage_loadEntities(this, loadOnlyInRangeEntities, true, false);

	//load background music
	//SoundManager_loadBGM(SoundManager_getInstance(),(u16 (*)[6])this->bgm);
	SoundManager_loadBGM(SoundManager_getInstance(), (u16 (*)[6])stageDefinition->bgm);
	
	//setup the column table
	HardwareManager_setupColumnTable(HardwareManager_getInstance());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add entity to the stage
Entity Stage_addEntity(Stage this, EntityDefinition* entityDefinition, VBVec3D* position, int inGameIndex, void *extraInfo){

	ASSERT(this, "Stage::addEntity: null this");

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
		
		__VIRTUAL_CALL(void, Container, initialTransform, (Container)entity, __ARGUMENTS(&environmentTransform));

		return entity;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, int loadOnlyInRangeEntities, int loadAllEntitiesInRange, int disableInterrupts){

	ASSERT(this, "Stage::loadEntities: null this");

	StageDefinition* world = this->stageDefinition;
	
	int i = 0;

	// TODO: don't check every entity in the stage's definition
	// must implement an algorithm which only check a portion of it
	for(; i < __ENTITIES_PER_STAGE && world->entities[i].entity; i++){
		
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

				if(disableInterrupts) {
					
					VPUManager_disableInterrupt(VPUManager_getInstance());
				}
				
				Stage_addEntity(this, entityDefinition, &position, i, world->entities[i].extraInfo);

				if(disableInterrupts) {
					
					VPUManager_enableInterrupt(VPUManager_getInstance());
				}

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
// preload textures
static void Stage_loadTextures(Stage this) {

	ASSERT(this, "Entity::loadTextures: null this");

	StageDefinition* world = this->stageDefinition;
	
	int i = 0;

	for(; i < __MAX_TEXTURES_PER_STAGE && world->textures[i]; i++){

		TextureManager_loadTexture(TextureManager_getInstance(), world->textures[i], this->flushCharGroups);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// unload non visible entities
static void Stage_unloadOutOfRangeEntities(Stage this, int unloadProgressively){

	ASSERT(this, "Stage::unloadOutOfRangeEntities: null this");
	ASSERT(this->children, "Stage::unloadOutOfRangeEntities: null children");

	// need a temporal list to remove and delete entities
	VirtualList removedEntities = __NEW(VirtualList);
	VirtualNode node = VirtualList_begin(this->children);

	CACHE_ENABLE;
	// check which actors must be unloaded
	for(; node; node = VirtualNode_getNext(node)){
		
		// get next entity
		Entity entity = (Entity)VirtualNode_getData(node);
		
		//if the entity isn't visible inside the view field, unload it
		if(!__VIRTUAL_CALL(int, Entity, isVisible, entity, __ARGUMENTS(__ENTITY_UNLOAD_PAD))){		

			int inGameState = __VIRTUAL_CALL(int, Entity, getInGameState, entity);

			// if state is loaded... just set it as unloaded
			if(__LOADED & inGameState){
				
				inGameState &= ~__LOADED;
			}
			
			// update in game state
			Stage_setEntityState(this, Container_getID((Container)entity), inGameState);
			
			// register entity to remove
			VirtualList_pushBack(removedEntities, (const BYTE* const )entity);
			
			if(unloadProgressively) {
				
				break;
			}
		}
	}
	
	VPUManager_disableInterrupt(VPUManager_getInstance());

	// now remove and delete entities
	for(node = VirtualList_begin(removedEntities); node; node = VirtualNode_getNext(node)){
	
		// get next entity
		Entity entity = (Entity)VirtualNode_getData(node);

		VPUManager_disableInterrupt(VPUManager_getInstance());

		// destroy it
		__DELETE(entity);		
		
	}

	VPUManager_enableInterrupt(VPUManager_getInstance());
	
	// destroy the temporal list
	__DELETE(removedEntities);
	
	CACHE_DISABLE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stream entities according to screen's position
void Stage_stream(Stage this){
	
	ASSERT(this, "Stage::stream: null this");

	// if the screen is moving
	if(*((u8*)_screenMovementState)){

		//static int load = 2;
		static int load = __LOGIC_FPS >> 1;
			
		if(!--load){

			// unload not visible objects
			Stage_unloadOutOfRangeEntities(this, false);
			
			load = __LOGIC_FPS >> 1;
		}
		else if (((__LOGIC_FPS >> 1) >> 1) == load) {
		//else if (1 == load) {

			// load visible objects	
			Stage_loadEntities(this, true, true, true);
		}	
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// if set to false, the char set memory is flushed when
// a char defintion is no longer used
// only useful to false when preloading textures
// otherwise it doesn't have any effect add flushing is the default 
// behvior
void Stage_setFlushCharGroups(Stage this, int flushCharGroups){
	
	this->flushCharGroups = flushCharGroups;

	/*
	if(!flushCharGroups) {
		
		if(this->stageDefinition->textures[0]) {

			this->flushCharGroups = flushCharGroups;
		}
	}
	else {

		this->flushCharGroups = flushCharGroups;
	}
	*/
}