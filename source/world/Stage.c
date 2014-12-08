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
#define __ENTITY_LOAD_PAD 			15
#define __ENTITY_UNLOAD_PAD 		30

#define __STREAMING_AMPLITUDE	5
#define __STREAM_CYCLE	(__TARGET_FPS >> 2)
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
	u8 tested;
	
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
static StageEntityDescription* Stage_registerEntity(Stage this, PositionedEntity* positionedEntity);

// register the stage's definition entities in the streaming list
static void Stage_registerEntities(Stage this);

// delete removed entities
static void Stage_processRemovedEntities(Stage this);

// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, VirtualList sortedStageEntities, VirtualNode streamingHeads[], int streamingDisplacement, int loadOnlyInRangeEntities, int loadProgressively);

// unload non visible entities
static void Stage_unloadOutOfRangeEntities(Stage this, VirtualList sortedStageEntities, VirtualNode streamingHeads[], int streamingDisplacement, int unloadProgressively);

// preload textures
static void Stage_loadTextures(Stage this);

// load and retrieve a texture (for internal usage: use TextureManager_get)
Texture TextureManager_loadTexture(TextureManager this, TextureDefinition* textureDefinition, int isPreload);

// put down flag so entities are being tested in the next streaming cycle
static void Stage_prepareStageEntitiesForTesting(Stage this);

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
	
	this->ui = NULL;
	this->stageDefinition = NULL;
	
	this->flushCharGroups = true;
	
	this->streamingAmplitude = __STREAMING_AMPLITUDE;


	this->stageEntities = NULL;
	this->removedEntities = __NEW(VirtualList);

	int i = 0;
	for(; i < kLastAxis; i++){
		
		this->sortedStageEntities[i] = NULL;
		this->streamingDisplacements[i] = 1;
		
		int j = 0;
		for(; j < kLastHead; j++){
			
			this->streamingHeads[i][j] = NULL;
		}
	}
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
	
	int i = 0;
	for(; i < kLastAxis; i++){
		
		__DELETE(this->sortedStageEntities[i]);
		this->sortedStageEntities[i] = NULL;
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
inline static int Stage_inLoadRange(Stage this, VBVec3D* position3D, u8 width, u8 height){
	
	ASSERT(this, "Stage::inLoadRange: null this");

	Scale scale;

	scale.x = scale.y = FIX19_13TOFIX7_9(ITOFIX19_13(1) - 
		       FIX19_13_DIV(position3D->z , _optical->maximunViewDistance));

	return Optics_isVisible(*position3D,
			Optics_calculateRealSize(((u16)width) << 3, WRLD_BGMAP, scale.x),
			Optics_calculateRealSize(((u16)height) << 3, WRLD_BGMAP, scale.y),
			Optics_calculateParallax(position3D->x, position3D->z),
			__ENTITY_LOAD_PAD);
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
	int i = 0;
	for(; i < kLastAxis; i++){

		// load visible objects	
		Stage_loadEntities(this, this->sortedStageEntities[i], this->streamingHeads[i], this->streamingDisplacements[i],  true, false);
	}

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
			
			VBVec3D position = Screen_getPosition(Screen_getInstance());
		
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

	StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

	for(; node; node = VirtualNode_getNext(node)){

		if(stageEntityDescription->ID == ID) {
			
			stageEntityDescription->ID = -1;
			break;
		}
	}
	
	if(permanent) {

		ASSERT(entity, "Stage::removeEntity: null node");

		int i = 0;
		for(; i < kLastAxis; i++){
		
			VirtualNode nodeToRemove = VirtualList_find(this->sortedStageEntities[i], stageEntityDescription);

			if(this->streamingHeads[i][kStartHead] == nodeToRemove) {
				
				this->streamingHeads[i][kStartHead] = VirtualNode_getNext(this->streamingHeads[i][kStartHead]);
			}

			if(this->streamingHeads[i][kEndHead] == nodeToRemove) {
				
				this->streamingHeads[i][kEndHead] = VirtualNode_getPrevious(this->streamingHeads[i][kStartHead]);
			}

			VirtualList_removeElement(this->sortedStageEntities[i], VirtualNode_getData(node));
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
static StageEntityDescription* Stage_registerEntity(Stage this, PositionedEntity* positionedEntity) {
	
	ASSERT(this, "Stage::registerEntities: null this");

	StageEntityDescription* stageEntityDescription = __NEW_BASIC(StageEntityDescription);

	stageEntityDescription->ID = -1;
	stageEntityDescription->positionedEntity = positionedEntity;
	stageEntityDescription->tested = false;
	
	VirtualList_pushBack(this->stageEntities, stageEntityDescription);
	
	return stageEntityDescription;
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
	for(; i < kLastAxis; i++){
		
		if(this->sortedStageEntities[i]) {
			
			__DELETE(this->sortedStageEntities[i]);
			this->sortedStageEntities[i] = NULL;
		}
		
		this->sortedStageEntities[i] = __NEW(VirtualList);
	}

	for(i = 0; this->stageDefinition->entities[i].entityDefinition; i++){
		
		StageEntityDescription* stageEntityDescription = Stage_registerEntity(this, &this->stageDefinition->entities[i]);
		
		int j = 0;
		for(; j < kLastAxis; j++){

			VirtualNode node = VirtualList_begin(this->sortedStageEntities[j]);
			
			for(; node; node = VirtualNode_getNext(node)) {
				
				StageEntityDescription* auxStageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);
				
				ASSERT(auxStageEntityDescription, "Stage::registerEntities: null entity description");
				
				if(auxStageEntityDescription) {
					
					switch(j){
					
						case kXAxis:
							
							if(stageEntityDescription->positionedEntity->position.x > auxStageEntityDescription->positionedEntity->position.x) {
							
								continue;
							}
							break;
							
						case kYAxis:
							
							if(stageEntityDescription->positionedEntity->position.y > auxStageEntityDescription->positionedEntity->position.y) {
							
								continue;
							}
							break;
							
						case kZAxis:
							
							if(stageEntityDescription->positionedEntity->position.z > auxStageEntityDescription->positionedEntity->position.z) {
							
								continue;
							}
							break;
					}

					VirtualList_insertBefore(this->sortedStageEntities[j], node, stageEntityDescription);
					break;
				}
			}
				
			if(!node) {
				
				VirtualList_pushBack(this->sortedStageEntities[j], stageEntityDescription);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, VirtualList sortedStageEntities, VirtualNode streamingHeads[], int streamingDisplacement, int loadOnlyInRangeEntities, int loadProgressively){

	ASSERT(this, "Stage::loadEntities: null this");

	int counter = 0;
	VirtualNode lastLoadedNode = NULL;
	int skippedEntity = false;
	
	if(!streamingHeads[kStartHead]) {
		
		streamingHeads[kStartHead] = VirtualList_begin(sortedStageEntities); 
	}

	if(!streamingHeads[kEndHead]) {
		
		streamingHeads[kEndHead] = streamingHeads[kStartHead]; 
	}
	
	VirtualNode node = 0 < streamingDisplacement? streamingHeads[kEndHead]: streamingHeads[kStartHead];

	for(; (!loadProgressively || counter < this->streamingAmplitude) && node; 
	node = 0 < streamingDisplacement? VirtualNode_getNext(node): VirtualNode_getPrevious(node),  counter += loadProgressively? 1: 0){
		
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if(-1 == stageEntityDescription->ID && !stageEntityDescription->tested) {

			stageEntityDescription->tested = true;

			VBVec3D position3D = {
					ITOFIX19_13(stageEntityDescription->positionedEntity->position.x),
					ITOFIX19_13(stageEntityDescription->positionedEntity->position.y),
					ITOFIX19_13(stageEntityDescription->positionedEntity->position.z)
			};

			// if entity in load range
			if(!loadOnlyInRangeEntities || Stage_inLoadRange(this, &position3D, 
					stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[0].textureDefinition->cols, 
					stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[0].textureDefinition->rows)){

				Entity entity = Stage_addEntity(this, stageEntityDescription->positionedEntity->entityDefinition, &position3D, stageEntityDescription->positionedEntity->extraInfo, false);
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

		if(0 < streamingDisplacement) {

			streamingHeads[kEndHead] = lastLoadedNode;
		}
		else {
			
			streamingHeads[kStartHead] = lastLoadedNode;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// unload non visible entities
static void Stage_unloadOutOfRangeEntities(Stage this, VirtualList sortedStageEntities, VirtualNode streamingHeads[], int streamingDisplacement, int unloadProgressively){

	ASSERT(this, "Stage::unloadOutOfRangeEntities: null this");

	if(!this->children) {
		
		return;
	}
	
	// need a temporal list to remove and delete entities
	VirtualList removedEntities = __NEW(VirtualList);
	VirtualNode node = 0 < streamingDisplacement? VirtualList_begin(this->children): VirtualList_end(this->children);

//	int counter = 0;
	CACHE_ENABLE;
	// check which actors must be unloaded
	for(; node; node = 0 < streamingDisplacement? VirtualNode_getNext(node): VirtualNode_getPrevious(node)){
//	for(; node && counter < this->streamingAmplitude; counter++){

		// get next entity
		Entity entity = (Entity)VirtualNode_getData(node);
		
		// if the entity isn't visible inside the view field, unload it
		if(!__VIRTUAL_CALL(int, Entity, isVisible, entity, __ARGUMENTS(__ENTITY_UNLOAD_PAD))){		

			s16 ID = Container_getId((Container)entity);
			
			int traverseNormally = ID < ((StageEntityDescription*)VirtualNode_getData(streamingHeads[kStartHead]))->ID ||
				ID > ((StageEntityDescription*)VirtualNode_getData(streamingHeads[kEndHead]))->ID;

			VirtualNode auxNode = traverseNormally? VirtualList_begin(sortedStageEntities): 0 < streamingDisplacement? streamingHeads[kEndHead]: streamingHeads[kStartHead];

			for(; auxNode; auxNode = traverseNormally? VirtualNode_getNext(auxNode): 0 < streamingDisplacement? VirtualNode_getPrevious(auxNode): VirtualNode_getNext(auxNode)){

				StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(auxNode);

				if(stageEntityDescription->ID == ID) {
					
					stageEntityDescription->ID = -1;
					break;
				}
			}
			
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
		
		VirtualNode* modifierNode = 0 < streamingDisplacement? &streamingHeads[kStartHead]: &streamingHeads[kEndHead];
		VirtualNode node = 0 < streamingDisplacement? streamingHeads[kEndHead]: streamingHeads[kStartHead];
		VirtualNode (*nodeTraverseMethod)(VirtualNode) = 0 < streamingDisplacement? &VirtualNode_getPrevious: &VirtualNode_getNext;
		
		ASSERT(node, "Stage::unloadOutOfRangeEntities: null node");
		
		int counter = 0;
		VirtualNode auxNode = node;
		for(; node && counter < this->streamingAmplitude; auxNode = node, node = nodeTraverseMethod(node), counter++){
				
			StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

			if(-1 == stageEntityDescription->ID){
				
				break;
			}
		}
	
		*modifierNode = auxNode? auxNode: *modifierNode;
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

	static int load = __STREAM_CYCLE;
	if(!--load){

		int i = 0;
		for(; i < kLastAxis; i++){
		
			// unload not visible objects
			Stage_unloadOutOfRangeEntities(this, this->sortedStageEntities[i], this->streamingHeads[i], this->streamingDisplacements[i], false);
		}
		
		load = __STREAM_CYCLE;
	}
	else if (__STREAM_LOAD_CYCLE_1 == load || __STREAM_LOAD_CYCLE_2 == load) {

		VBVec3D lastScreenDisplacement = Screen_getLastDisplacement(Screen_getInstance());
		
		this->streamingDisplacements[kXAxis] = _screenMovementState->x? 0 <= lastScreenDisplacement.x? 1: -1: 0;
		this->streamingDisplacements[kYAxis] = _screenMovementState->x? 0 <= lastScreenDisplacement.y? 1: -1: 0;
		this->streamingDisplacements[kZAxis] = _screenMovementState->x? 0 <= lastScreenDisplacement.z? 1: -1: 0;

		int i = 0;
		
		for(; i < kLastAxis; i++){

			if(this->streamingDisplacements[i]) {
			
				// load visible objects	
				Stage_loadEntities(this, this->sortedStageEntities[i], this->streamingHeads[i], this->streamingDisplacements[i],  true, true);
			}
		}
	}
	else {
		
		Stage_prepareStageEntitiesForTesting(this);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// put down flag so entities are being tested in the next streaming cycle
static void Stage_prepareStageEntitiesForTesting(Stage this){
	
	ASSERT(this, "Stage::prepareStageEntitiesForTesting: null this");

	int i = 0;
	
	for(; i < kLastAxis; i++){

		VirtualNode node = this->streamingHeads[i][kStartHead];
	
		for(; node && (!this->streamingHeads[i][kEndHead] || node !=  VirtualNode_getNext(this->streamingHeads[i][kEndHead])); node = VirtualNode_getNext(node)){
			
			StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);
	
			stageEntityDescription->tested = false;
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// stream entities according to screen's position
void Stage_streamAll(Stage this) {

	ASSERT(this, "Stage::streamAll: null this");

	Stage_prepareStageEntitiesForTesting(this);

	VBVec3D lastScreenDisplacement = Screen_getLastDisplacement(Screen_getInstance());
	this->streamingDisplacements[kXAxis] = 0 <= lastScreenDisplacement.x? 1: -1;
	this->streamingDisplacements[kYAxis] = 0 <= lastScreenDisplacement.y? 1: -1;
	this->streamingDisplacements[kZAxis] = 0 <= lastScreenDisplacement.z? 1: -1;

	int i = 0;
	for(; i < kLastAxis; i++){
	
		// unload not visible objects
		Stage_unloadOutOfRangeEntities(this, this->sortedStageEntities[i], this->streamingHeads[i], this->streamingDisplacements[i], false);
	}

	for(i = 0; i < kLastAxis; i++){

		// load visible objects	
		Stage_loadEntities(this, this->sortedStageEntities[i], this->streamingHeads[i], this->streamingDisplacements[i],  true, false);
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve ui
UI Stage_getUI(Stage this){
	
	return this->ui;
}
