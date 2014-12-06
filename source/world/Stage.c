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

#include <Stage.h>
#include <Optics.h>
#include <SoundManager.h>
#include <Screen.h>
#include <HardwareManager.h>
#include <SpriteManager.h>
#include <Texture.h>

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												MACROS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#undef __STREAMING_AMPLITUDE	
#undef __ENTITY_LOAD_PAD 			
#undef __ENTITY_UNLOAD_PAD 		
#define __ENTITY_LOAD_PAD 			30
#define __ENTITY_UNLOAD_PAD 		40

#define __STREAMING_AMPLITUDE	5
#define __STREAM_CYCLE	(__TARGET_FPS)	
#define __STREAM_UNLOAD_CYCLE	(0)	
#define __STREAM_LOAD_CYCLE_1	__STREAM_CYCLE / 3	
#define __STREAM_LOAD_CYCLE_2	(__STREAM_CYCLE / 3) * 2	

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

typedef struct StageEntityDescription {
	
	PositionedEntity* positionedEntity;
	s16 ID;
	
}StageEntityDescription;

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

//class's constructor
static void Stage_constructor(Stage this);

// setup ui
static void Stage_setupUI(Stage this);

// register an entity in the streaming list
static void Stage_registerEntity(Stage this, PositionedEntity* positionedEntity);

// register the stage's definition entities in the streaming list
static void Stage_registerEntities(Stage this);

// delete removed entities
static void Stage_processRemovedEntities(Stage this);

// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, int loadOnlyInRangeEntities, int loadProgressively);

// preload textures
static void Stage_loadTextures(Stage this);

// unload non visible entities
static void Stage_unloadOutOfRangeEntities(Stage this, int unloadProgressively);

// load and retrieve a texture (for internal usage: use TextureManager_get)
Texture TextureManager_loadTexture(TextureManager this, TextureDefinition* textureDefinition, int isPreload);

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
	
	this->stageEntities = NULL;
	this->removedEntities = __NEW(VirtualList);

	this->ui = NULL;
	this->stageDefinition = NULL;
	
	this->flushCharGroups = true;
	
	this->streamingAmplitude = __STREAMING_AMPLITUDE;
	this->streamingLeftHead = NULL;
	this->streamingRightHead = NULL;
	this->streamingHeadDisplacement = 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Stage_destructor(Stage this){
	
	ASSERT(this, "Stage::destructor: null this");

	if(this->ui){
		
		__DELETE(this->ui);
		this->ui = NULL;
	}

	if(this->stageEntities){
		
		VirtualNode node = VirtualList_begin(this->stageEntities);
		
		for(; node; node = VirtualNode_getNext(node)){
			
			__DELETE_BASIC(VirtualNode_getData(node));
		}

		__DELETE(this->stageEntities);
		
		this->stageEntities = NULL;
	}
	
	if(this->removedEntities){
			
		VirtualNode node = VirtualList_begin(this->removedEntities);
		
		for(; node; node = VirtualNode_getNext(node)){
			
			__DELETE_BASIC(VirtualNode_getData(node));
		}

		__DELETE(this->removedEntities);
		
		this->removedEntities = NULL;
	}
	
	// destroy the super object
	__DESTROY_BASE(Container);
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
		
	// set world's limits
	Screen_setStageSize(Screen_getInstance(), stageDefinition->size);
	
	// set screen's position
	Screen_setPosition(Screen_getInstance(), stageDefinition->screenPosition);

	// preload textures
	Stage_loadTextures(this);
	
	//load Stage's bgm
	//this->bgm = (u16 (*)[6])stageDefinition->bgm;

	// register all the entities in the stage's definition
	Stage_registerEntities(this);
	
	// load entities
	Stage_loadEntities(this, loadOnlyInRangeEntities, false);

	// setup ui
	Stage_setupUI(this);
	
	//load background music
	//SoundManager_loadBGM(SoundManager_getInstance(),(u16 (*)[6])this->bgm);
	SoundManager_loadBGM(SoundManager_getInstance(), (u16 (*)[6])stageDefinition->bgm);
	
	//setup the column table
	HardwareManager_setupColumnTable(HardwareManager_getInstance());
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup ui
static void Stage_setupUI(Stage this){

	ASSERT(this, "Stage::setupUI: null this");
	ASSERT(!this->ui, "Stage::setupUI: UI already exists");

	if(this->ui){
		
		__DELETE(this->ui);
		this->ui = NULL;
	}
	
	if(this->stageDefinition->uiDefinition.allocator) {

		// call the appropiate allocator to support inheritance!
		this->ui = (UI)((UI (*)(UIDefinition*, ...)) this->stageDefinition->uiDefinition.allocator)(0, &this->stageDefinition->uiDefinition);
		ASSERT(this->ui, "Stage::setupUI: null ui");
		
		// setup ui if allocated and constructed
		if(this->ui){
			
			VBVec3D position = {
					
					ITOFIX19_13(0),
					ITOFIX19_13(0),
					ITOFIX19_13(0)
			};
		
			Transformation environmentTransform = {
					// local position
					{0, 0, 0},
					// global position
					{0, 0, 0},
					// scale
					{1, 1},
					// rotation
					{0, 0, 0}	
			};

			Container_setLocalPosition((Container)this->ui, position);

			__VIRTUAL_CALL(void, Container, initialTransform, (Container)this->ui, __ARGUMENTS(&environmentTransform));
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add entity to the stage
Entity Stage_addEntity(Stage this, EntityDefinition* entityDefinition, VBVec3D* position, void *extraInfo, int permanent){

	ASSERT(this, "Stage::addEntity: null this");

	static s16 ID = 0;

	if(entityDefinition){
		
		Entity entity = Entity_load(entityDefinition, ID++, extraInfo);
		
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

		if(permanent) {

			// TODO
		}
		return entity;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// add entity to the stage
void Stage_removeEntity(Stage this, Entity entity, int permanent){
	
	ASSERT(this, "Stage::removeEntity: null this");
	ASSERT(entity, "Stage::removeEntity: null entity");

	if(!entity){
		
		return;
	}
	
	// hide until effectively deleted
	Entity_hide(entity);

	VirtualList_pushBack(this->removedEntities, entity);

	VirtualNode node = VirtualList_begin(this->stageEntities);
	
	s16 ID = Container_getId((Container)entity);

	for(; node; node = VirtualNode_getNext(node)){

		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if(stageEntityDescription->ID == ID) {
			
			stageEntityDescription->ID = -1;
			break;
		}
	}
	
	if(permanent) {

		ASSERT(entity, "Stage::removeEntity: null node");

		if(this->streamingLeftHead == node) {
			
			this->streamingLeftHead = VirtualNode_getNext(this->streamingLeftHead);
		}

		if(this->streamingRightHead == node) {
			
			this->streamingRightHead = VirtualNode_getPrevious(this->streamingRightHead);
		}
		
		VirtualList_removeElement(this->stageEntities, VirtualNode_getData(node));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// preload textures
static void Stage_loadTextures(Stage this) {

	ASSERT(this, "Entity::loadTextures: null this");

	int i = 0;

	for(; this->stageDefinition->textures[i]; i++){

		TextureManager_loadTexture(TextureManager_getInstance(), this->stageDefinition->textures[i], this->flushCharGroups);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// register an entity in the streaming list
static void Stage_registerEntity(Stage this, PositionedEntity* positionedEntity) {
	
	ASSERT(this, "Stage::registerEntities: null this");

	StageEntityDescription* stageEntityDescription = __NEW_BASIC(StageEntityDescription);

	stageEntityDescription->ID = -1;
	stageEntityDescription->positionedEntity = positionedEntity;
	VirtualList_pushBack(this->stageEntities, stageEntityDescription);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// register the stage's definition entities in the streaming list
static void Stage_registerEntities(Stage this) {
	
	ASSERT(this, "Stage::registerEntities: null this");

	if(this->stageEntities) {
		
		__DELETE(this->stageEntities);
		this->stageEntities = NULL;
	}
	
	if(!this->stageEntities) {
		
		this->stageEntities = __NEW(VirtualList);
	}
	
	int i = 0;
	for(;this->stageDefinition->entities[i].entityDefinition; i++){
		
		Stage_registerEntity(this, &this->stageDefinition->entities[i]);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, int loadOnlyInRangeEntities, int loadProgressively){

	ASSERT(this, "Stage::loadEntities: null this");

	int counter = 0;
	VirtualNode lastLoadedNode = NULL;
	int skippedEntity = false;
	
	if(!this->streamingLeftHead) {
		
		this->streamingLeftHead = VirtualList_begin(this->stageEntities); 
	}

	if(!this->streamingRightHead) {
		
		this->streamingRightHead = this->streamingLeftHead; 
	}
	
	VirtualNode node = 0 < this->streamingHeadDisplacement? this->streamingRightHead: this->streamingLeftHead;

	for(; (!loadProgressively || counter < this->streamingAmplitude) && node; 
	node = 0 < this->streamingHeadDisplacement? VirtualNode_getNext(node): VirtualNode_getPrevious(node),  counter += loadProgressively? 1: 0){
		
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if(-1 == stageEntityDescription->ID) {
						
			VBVec3D position = {
					ITOFIX19_13(stageEntityDescription->positionedEntity->position.x),
					ITOFIX19_13(stageEntityDescription->positionedEntity->position.y),
					ITOFIX19_13(stageEntityDescription->positionedEntity->position.z)
			};

			// if entity in load range
			if(!loadOnlyInRangeEntities || Stage_inLoadRange(this, &position, 
					stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[0].textureDefinition->cols << 2, 
					stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[0].textureDefinition->rows << 2)){

				
				Entity entity = Stage_addEntity(this, stageEntityDescription->positionedEntity->entityDefinition, &position, stageEntityDescription->positionedEntity->extraInfo, false);
				stageEntityDescription->ID = Container_getId((Container)entity);

				if(!skippedEntity) {

					lastLoadedNode = node;
				}

				if(lastLoadedNode && loadProgressively) {

					break;
				}
			}
			else {

				skippedEntity = true;
			}
		}
		else if(!skippedEntity) {

			lastLoadedNode = node;
		}
	}
	
	if(lastLoadedNode) {

		if(0 < this->streamingHeadDisplacement) {

			this->streamingRightHead = lastLoadedNode;
		}
		else {
			
			this->streamingLeftHead = lastLoadedNode;
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
	VirtualNode node = 0 < this->streamingHeadDisplacement? VirtualList_begin(this->children): VirtualList_end(this->children);

//	int counter = 0;
	CACHE_ENABLE;
	// check which actors must be unloaded
	for(; node; node = 0 < this->streamingHeadDisplacement? VirtualNode_getNext(node): VirtualNode_getPrevious(node)){
//	for(; node && counter < this->streamingAmplitude; counter++){

		// get next entity
		Entity entity = (Entity)VirtualNode_getData(node);
		
		// if the entity isn't visible inside the view field, unload it
		if(!__VIRTUAL_CALL(int, Entity, isVisible, entity, __ARGUMENTS(__ENTITY_UNLOAD_PAD))){		

			s16 ID = Container_getId((Container)entity);
			
			int traverseNormally = ID < ((StageEntityDescription*)VirtualNode_getData(this->streamingLeftHead))->ID ||
				ID > ((StageEntityDescription*)VirtualNode_getData(this->streamingRightHead))->ID;

			VirtualNode auxNode = traverseNormally? VirtualList_begin(this->stageEntities): 0 < this->streamingHeadDisplacement? this->streamingRightHead: this->streamingLeftHead;

			for(; auxNode; auxNode = traverseNormally? VirtualNode_getNext(auxNode): 0 < this->streamingHeadDisplacement? VirtualNode_getPrevious(auxNode): VirtualNode_getNext(auxNode)){

				StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(auxNode);

				if(stageEntityDescription->ID == ID) {
					
					stageEntityDescription->ID = -1;
					break;
				}
			}
			
			ASSERT(auxNode, "Stage::unloadOutOfRangeEntities: entity definition not found");
			
			// register entity to remove
			VirtualList_pushBack(removedEntities, (const BYTE* const )entity);
			
			if(unloadProgressively) {
				
				break;
			}
		}
	}
	
	// now remove and delete entities
	for(node = VirtualList_begin(removedEntities); node; node = VirtualNode_getNext(node)){

		// get next entity
		Entity entity = (Entity)VirtualNode_getData(node);

		// don't need to remove manually from children list
		// since the entity will do it by itself on its 
		// destructor
		// destroy it
		__DELETE(entity);
	}

	CACHE_DISABLE;
	
	// repositione stream headers
	if(0 < VirtualList_getSize(removedEntities)){
		
		VirtualNode* modifierNode = 0 < this->streamingHeadDisplacement? &this->streamingLeftHead: &this->streamingRightHead;
		VirtualNode node = 0 < this->streamingHeadDisplacement? this->streamingRightHead: this->streamingLeftHead;
		VirtualNode (*nodeTraverseMethod)(VirtualNode) = 0 < this->streamingHeadDisplacement? &VirtualNode_getPrevious: &VirtualNode_getNext;
		
		ASSERT(node, "Stage::unloadOutOfRangeEntities: null node");
		
		int counter = 0;
		VirtualNode auxNode = node;
		for(; node && counter < this->streamingAmplitude; auxNode = node, node = nodeTraverseMethod(node), counter++){
				
			StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

			if(-1 == stageEntityDescription->ID){
				
				break;
			}
		}
	
		*modifierNode = auxNode;
	}
	
	// destroy the temporal list
	__DELETE(removedEntities);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute stage's logic
static void Stage_processRemovedEntities(Stage this){
	
	ASSERT(this, "Stage::processRemovedEntities: null this");

	VirtualNode node = VirtualList_begin(this->removedEntities);
	
	for(; node; node = VirtualNode_getNext(node)){
		
		// don't need to remove manually from children list
		// since the entity will do it by itself on its 
		// destructor
		// destroy it
		__DELETE(VirtualNode_getData(node));
	}
	
	VirtualList_clear(this->removedEntities);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute stage's logic
void Stage_update(Stage this){
	
	ASSERT(this, "Stage::update: null this");

	Stage_processRemovedEntities(this);
	
	Container_update((Container)this);
	
	if(this->ui) {

		Container_update((Container)this->ui);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stream entities according to screen's position
void Stage_stream(Stage this){

	ASSERT(this, "Stage::stream: null this");
	// if the screen is moving
	//if(_screenMovementState->x || _screenMovementState->y || _screenMovementState->z){
	static int load = __STREAM_CYCLE;
	if(!--load){

		// unload not visible objects
		Stage_unloadOutOfRangeEntities(this, false);
		
		load = __STREAM_CYCLE;
	}
	else if (__STREAM_LOAD_CYCLE_1 == load || __STREAM_LOAD_CYCLE_2 == load) {

		VBVec3D lastScreenDisplacement = Screen_getLastDisplacement(Screen_getInstance());
		
		this->streamingHeadDisplacement = 0 <= lastScreenDisplacement.x? 1: -1;

		// load visible objects	
		Stage_loadEntities(this, true, true);
	}	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stream entities according to screen's position
void Stage_streamAll(Stage this) {

	VBVec3D lastScreenDisplacement = Screen_getLastDisplacement(Screen_getInstance());
	this->streamingHeadDisplacement = 0 <= lastScreenDisplacement.x? 1: -1;

	Stage_unloadOutOfRangeEntities(this, false);
	Stage_loadEntities(this, true, false);
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