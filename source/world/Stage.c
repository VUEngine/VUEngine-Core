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

#undef __STREAMING_AMPLITUDE	
#undef __ENTITY_LOAD_PAD 			
#undef __ENTITY_UNLOAD_PAD 		
#define __ENTITY_LOAD_PAD 			20
#define __ENTITY_UNLOAD_PAD 		30

#define __STREAMING_AMPLITUDE	5
#define __STREAM_CYCLE	(__TARGET_FPS)	
#define __STREAM_UNLOAD_CYCLE	(0)	
#define __STREAM_LOAD_CYCLE_1	__STREAM_CYCLE / 3	
#define __STREAM_LOAD_CYCLE_2	(__STREAM_CYCLE / 3) * 2	

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

//class's constructor
static void Stage_constructor(Stage this);

// update world's entities state
static void Stage_setEntityState(Stage this, int ID, int inGameState);

// get entity's state
static inline int Stage_getEntityState(Stage this, int ID);

// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, int loadOnlyInRangeEntities, int loadProgressively, int disableInterrupts);

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
	
	this->streamingAmplitude = __STREAMING_AMPLITUDE;
	this->streamingLeftHead = 0;
	this->streamingRightHead = 0;
	this->streamingHeadDisplacement = 1;
	
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
	Stage_loadEntities(this, loadOnlyInRangeEntities, false, false);

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
		Entity entity = Entity_load(entityDefinition, inGameIndex, extraInfo);
		
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
		
		// set spatial position
		__VIRTUAL_CALL(void, Entity, setLocalPosition, entity, __ARGUMENTS(position));

		// apply transformations
		__VIRTUAL_CALL(void, Container, initialTransform, (Container)entity, __ARGUMENTS(&environmentTransform));

		return entity;
	}
	
	return NULL;
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
// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, int loadOnlyInRangeEntities, int loadProgressively, int disableInterrupts){

	ASSERT(this, "Stage::loadEntities: null this");

	StageDefinition* world = this->stageDefinition;
	
	int i = loadProgressively? 0 < this->streamingHeadDisplacement? this->streamingRightHead: this->streamingLeftHead: 0;
	int counter = 0;
	int lastLoadedIndex = -1;
	int skippedEntity = false;

	for(; (!loadProgressively || counter < this->streamingAmplitude) && 0 <= i && i < __ENTITIES_PER_STAGE && world->entities[i].entity; i += loadProgressively? this->streamingHeadDisplacement: 1,  counter += loadProgressively? 1: 0 ){
		
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
				
				if(!skippedEntity) {

					lastLoadedIndex = i;
				}

				if(loadProgressively) {

					break;
				}
			}
			else {
				
				skippedEntity = true;
			}
		}
	}
	
	if(0 <= lastLoadedIndex) {

		if(0 < this->streamingHeadDisplacement) {
			
			this->streamingRightHead = lastLoadedIndex;
		}
		else {
			
			this->streamingLeftHead = lastLoadedIndex;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// unload non visible entities
static void Stage_unloadOutOfRangeEntities(Stage this, int unloadProgressively){

	ASSERT(this, "Stage::unloadOutOfRangeEntities: null this");

	if(!this->children) {
		
		return;
	}
	// need a temporal list to remove and delete entities
	VirtualList removedEntities = __NEW(VirtualList);
	VirtualNode node = 0 <= this->streamingHeadDisplacement? VirtualList_begin(this->children): VirtualList_end(this->children);

//	int counter = 0;
	CACHE_ENABLE;
	// check which actors must be unloaded
	for(; node; node = 0 <= this->streamingHeadDisplacement? VirtualNode_getNext(node): VirtualNode_getPrevious(node)){
//	for(; node && counter < this->streamingAmplitude; counter++){
		
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
	
	if(0 > this->streamingHeadDisplacement) {
		
		this->streamingRightHead -= VirtualList_getSize(removedEntities);
	}
	else {
		
		this->streamingLeftHead += VirtualList_getSize(removedEntities);
	}
	
	// now remove and delete entities
	for(node = VirtualList_begin(removedEntities); node; node = VirtualNode_getNext(node)){
	
		// get next entity
		Entity entity = (Entity)VirtualNode_getData(node);

		VirtualList_removeElement(this->children, entity);
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
/*
	Printing_text("NS", 20, 9);
	Printing_text("*", 19, 9);
	Printing_text(" ", 19, 10);
	Printing_text(" ", 19, 11);
*/
	// if the screen is moving
	//if(*((u8*)_screenMovementState)){

		static int load = __STREAM_CYCLE;

		if(!--load){

			/*
			Printing_text("Ul", 20, 10);
			Printing_text(" ", 19, 9);
			Printing_text("*", 19, 10);
			Printing_text(" ", 19, 11);
*/
			// unload not visible objects
			Stage_unloadOutOfRangeEntities(this, false);
			
			load = __STREAM_CYCLE;
		}
		else if (__STREAM_LOAD_CYCLE_1 == load || __STREAM_LOAD_CYCLE_2 == load) {

			VBVec3D lastScreenDisplacement = Screen_getLastDisplacement(Screen_getInstance());
			
			this->streamingHeadDisplacement = 0 <= lastScreenDisplacement.x? 1: -1;
			/*
			Printing_text("LD", 20, 11);
			Printing_text(" ", 19, 9);
			Printing_text(" ", 19, 10);
			Printing_text("*", 19, 11);
*/
			//Printing_text("                      ", 1, 10);
			//Printing_int(this->streamingLeftHead, 1, 10);
			//Printing_int(this->streamingRightHead, 10, 10);
			
			// load visible objects	
			Stage_loadEntities(this, true, false, true);
		}	
	//}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stream entities according to screen's position
void Stage_streamAll(Stage this) {
	
	Stage_unloadOutOfRangeEntities(this, false);
	Stage_loadEntities(this, true, false, true);
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