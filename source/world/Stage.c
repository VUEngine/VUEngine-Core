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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Stage.h>
#include <Optics.h>
#include <SoundManager.h>
#include <Screen.h>
#include <HardwareManager.h>
#include <SpriteManager.h>
#include <Texture.h>
#include <ParamTableManager.h>

//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#undef __STREAM_CYCLE_DURATION
#define __STREAM_CYCLE_DURATION	(12)

#define __TOTAL_CYCLES	4
#define __STREAM_UNLOAD_CYCLE	(0)
#define __STREAM_SELECT_ENTITIES_IN_LOAD_RANGE	__STREAM_CYCLE_DURATION / __TOTAL_CYCLES
#define __STREAM_LOAD_CYCLE	(__STREAM_CYCLE_DURATION / __TOTAL_CYCLES) * 2
#define __STREAM_INITIALIZE_CYCLE	(__STREAM_CYCLE_DURATION / __TOTAL_CYCLES) * 3


#undef __ENTITY_LOAD_PAD 			
#undef __ENTITY_UNLOAD_PAD 		

#define __ENTITY_LOAD_PAD 			(64)
#define __ENTITY_UNLOAD_PAD 		(__ENTITY_LOAD_PAD + 32)

// since there are 32 layers, that's the theoretical limit of entities to display
#undef __STREAMING_AMPLITUDE
#define __STREAMING_AMPLITUDE		16

#define __MAXIMUM_PARALLAX		10
#define __LOAD_LOW_X_LIMIT		ITOFIX19_13(0 - __MAXIMUM_PARALLAX - __ENTITY_LOAD_PAD)
#define __LOAD_HIGHT_X_LIMIT	ITOFIX19_13(__SCREEN_WIDTH + __MAXIMUM_PARALLAX + __ENTITY_LOAD_PAD)
#define __LOAD_LOW_Y_LIMIT		ITOFIX19_13(- __ENTITY_LOAD_PAD)
#define __LOAD_HIGHT_Y_LIMIT	ITOFIX19_13(__SCREEN_HEIGHT + __ENTITY_LOAD_PAD)
//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Stage
__CLASS_DEFINITION(Stage, Container);

typedef struct StageEntityDescription
{
	PositionedEntity* positionedEntity;
	SmallRightcuboid smallRightcuboid;
	s16 id;
	long distance;

} StageEntityDescription;

typedef struct StageEntityToInitialize
{
	PositionedEntity* positionedEntity;
	Entity entity;

} StageEntityToInitialize;

//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenPosition;
extern const Optical* _optical;
void Container_processRemovedChildren(Container this);

static void Stage_constructor(Stage this);
static void Stage_setupUI(Stage this);
static StageEntityDescription* Stage_registerEntity(Stage this, PositionedEntity* positionedEntity);
static void Stage_registerEntities(Stage this);
static void Stage_selectEntitiesInLoadRange(Stage this);
static void Stage_setObjectSpritesContainers(Stage this);
static void Stage_loadTextures(Stage this);
static void Stage_loadInRangeEntities(Stage this);
static void Stage_unloadOutOfRangeEntities(Stage this);
BgmapTexture BgmapTextureManager_loadTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition, int isPreload);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Stage)
__CLASS_NEW_END(Stage);

// class's constructor
static void Stage_constructor(Stage this)
{
	ASSERT(this, "Stage::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(-1);

	this->stageEntities = NULL;
	this->loadedStageEntities = NULL;
	this->removedEntities = NULL;
	this->entitiesToLoad = __NEW(VirtualList);
	this->entitiesToInitialize = __NEW(VirtualList);
	
	this->ui = NULL;
	this->stageDefinition = NULL;

	this->flushCharSets = true;
	this->focusEntity = NULL;
	
	this->nextEntityId = 0;
}

// class's destructor
void Stage_destructor(Stage this)
{
	ASSERT(this, "Stage::destructor: null this");

	if (this->ui)
	{
		__DELETE(this->ui);
		this->ui = NULL;
	}
	
	if (this->stageEntities)
	{
		VirtualNode node = VirtualList_begin(this->stageEntities);

		for (; node; node = VirtualNode_getNext(node))
		{
			__DELETE_BASIC(VirtualNode_getData(node));
		}
		
		__DELETE(this->stageEntities);

		this->stageEntities = NULL;
	}

	if (this->loadedStageEntities)
	{
		__DELETE(this->loadedStageEntities);
		this->loadedStageEntities = NULL;
	}
	
	if (this->entitiesToLoad)
	{
		__DELETE(this->entitiesToLoad);
		this->entitiesToLoad = NULL;
	}
	
	if (this->entitiesToInitialize)
	{
		VirtualNode node = VirtualList_begin(this->entitiesToInitialize);
		
		for(; node; node = VirtualNode_getNext(node))
		{
			StageEntityToInitialize* stageEntityToInitialize = (StageEntityToInitialize*)VirtualNode_getData(node);

			__DELETE(stageEntityToInitialize->entity);
			__DELETE_BASIC(stageEntityToInitialize);
		}
		
		__DELETE(this->entitiesToInitialize);
		this->entitiesToInitialize = NULL;
	}
	
	// destroy the super object
	__DESTROY_BASE;
}

// place holder for objects designed around OBJECTS in the VB hardware
void Stage_setupObjActor(Stage this, int *actor,int x,int y, int z)
{
	// TODO
	ASSERT(this, "Stage::setupObjActor: null this");
}

// determine if a point is visible
inline static int Stage_inLoadRange(Stage this, VBVec3D position3D, const SmallRightcuboid* smallRightcuboid)
{
	ASSERT(this, "Stage::inLoadRange: null this");

	VBVec2D position2D;

	//normalize position
	__OPTICS_NORMALIZE(position3D);

	//project the position to 2d space
	__OPTICS_PROJECT_TO_2D(position3D, position2D);

	// check x visibility
	if (position2D.x + ITOFIX19_13(smallRightcuboid->x1) < __LOAD_LOW_X_LIMIT || position2D.x - ITOFIX19_13(smallRightcuboid->x0) > __LOAD_HIGHT_X_LIMIT)
	{
		return false;
	}

	// check y visibility
	if (position2D.y + ITOFIX19_13(smallRightcuboid->y1) < __LOAD_LOW_Y_LIMIT || position2D.y - ITOFIX19_13(smallRightcuboid->y0) > __LOAD_HIGHT_Y_LIMIT)
	{
		return false;
	}

	return true;
}

static void Stage_setObjectSpritesContainers(Stage this)
{
	ASSERT(this, "Stage::setObjectSpritesContainers: null this");

	u8 i = 0;
	fix19_13 previousZ = this->stageDefinition->objectSpritesContainersZPosition[0];

	for(; i < __TOTAL_OBJECT_SEGMENTS; i++)
	{
		NM_ASSERT(this->stageDefinition->objectSpritesContainersZPosition[i] >= previousZ, "Stage::setObjectSpritesContainers: wrong z");
		ObjectSpriteContainerManager_setObjectSpriteContainerZPosition(ObjectSpriteContainerManager_getInstance(), i, this->stageDefinition->objectSpritesContainersZPosition[i]);
		previousZ = this->stageDefinition->objectSpritesContainersZPosition[i];
	}
}

// load stage's entites
void Stage_load(Stage this, StageDefinition* stageDefinition)
{
	ASSERT(this, "Stage::load: null this");

	// stop all sounds
	SoundManager_stopAllSound(SoundManager_getInstance());

	// set world's definition
	this->stageDefinition = stageDefinition;

	// set world's limits
	Screen_setStageSize(Screen_getInstance(), stageDefinition->size);

	// set screen's position
	Screen_setPosition(Screen_getInstance(), stageDefinition->screenPosition);

	// set OBJs' z position
	Stage_setObjectSpritesContainers(this);
	
	// preload textures
	Stage_loadTextures(this);

	//load Stage's bgm
	//this->bgm = (u16 (*)[6])stageDefinition->bgm;

	// register all the entities in the stage's definition
	Stage_registerEntities(this);

	// load entities
	Stage_loadInRangeEntities(this);

	// retrieve focus entity for streaming
	InGameEntity focusInGameEntity = Screen_getFocusInGameEntity(Screen_getInstance());
	this->focusEntity = focusInGameEntity? __UPCAST(Entity, focusInGameEntity): NULL;

	// setup ui
	Stage_setupUI(this);

	//load background music
	//SoundManager_loadBGM(SoundManager_getInstance(),(u16 (*)[6])this->bgm);
	SoundManager_loadBGM(SoundManager_getInstance(), (u16 (*)[6])stageDefinition->bgm);

	//setup the column table
	HardwareManager_setupColumnTable(HardwareManager_getInstance());
}

// retrieve size
Size Stage_getSize(Stage this)
{
	ASSERT(this, "Stage::getSize: null this");
	ASSERT(this->stageDefinition, "Stage::getSize: null stageDefinition");

	// set world's limits
	return this->stageDefinition->size;
}

// setup ui
static void Stage_setupUI(Stage this)
{
	ASSERT(this, "Stage::setupUI: null this");
	ASSERT(!this->ui, "Stage::setupUI: UI already exists");

	if (this->ui)
	{
		__DELETE(this->ui);
		this->ui = NULL;
	}

	if (this->stageDefinition->uiDefinition.allocator)
	{
		// call the appropiate allocator to support inheritance!
		this->ui = ((UI (*)(UIDefinition*, ...)) this->stageDefinition->uiDefinition.allocator)(&this->stageDefinition->uiDefinition);
		ASSERT(this->ui, "Stage::setupUI: null ui");

		// setup ui if allocated and constructed
		if (this->ui)
		{
			Transformation environmentTransform =
			{
					// local position
					{0, 0, 0},
					// global position
					{0, 0, 0},
					// scale
					{1, 1},
					// rotation
					{0, 0, 0}
			};

			__VIRTUAL_CALL(void, Container, initialTransform, this->ui, &environmentTransform);
		}
	}
}

// 
Entity Stage_addEntity(Stage this, EntityDefinition* entityDefinition, VBVec3D *position, void *extraInfo, bool permanent)
{
	ASSERT(this, "Stage::addEntity: null this");

	if (entityDefinition)
	{
		Entity entity = Entity_load(entityDefinition, this->nextEntityId++, extraInfo);

		if(entity)
		{
			// create the entity and add it to the world
			Container_addChild(__UPCAST(Container, this), __UPCAST(Container, entity));

			// initialize now
			__VIRTUAL_CALL(void, Entity, initialize, entity);

			Transformation environmentTransform = Container_getEnvironmentTransform(__UPCAST(Container, this));

			// apply transformations
			__VIRTUAL_CALL(void, Container, initialTransform, entity, &environmentTransform);
	
			if (permanent)
			{
				// TODO
			}
			
			return entity;
		}
	}

	return NULL;
}

// add entity to the stage
Entity Stage_addPositionedEntity(Stage this, PositionedEntity* positionedEntity, bool permanent)
{
	ASSERT(this, "Stage::addEntity: null this");

	if (positionedEntity)
	{
		Entity entity = Entity_loadFromDefinition(positionedEntity, this->nextEntityId++);

		if(entity)
		{
			// must initialize after adding the children
			__VIRTUAL_CALL(void, Entity, initialize, entity);

			// static to avoid call to memcpy
			static Transformation environmentTransform =
			{
					// local position
					{0, 0, 0},
					// global position
					{0, 0, 0},
					// scale
					{1, 1},
					// rotation
					{0, 0, 0}
			};
			
			// apply transformations
			__VIRTUAL_CALL(void, Container, initialTransform, entity, &environmentTransform);

			// create the entity and add it to the world
			Container_addChild(__UPCAST(Container, this), __UPCAST(Container, entity));
		}
/*
		if (permanent)
		{
			// TODO
		}
*/		
		return entity;
	}

	return NULL;
}

// add entity to the stage
void Stage_removeEntity(Stage this, Entity entity, bool permanent)
{
	ASSERT(this, "Stage::removeEntity: null this");
	ASSERT(entity, "Stage::removeEntity: null entity");

	if (!entity)
	{
		return;
	}

	// hide until effectively deleted
	Entity_hide(entity);

	Container_deleteMyself(__UPCAST(Container, entity));

	s16 id = Container_getId(__UPCAST(Container, entity));

	VirtualNode node = VirtualList_begin(this->stageEntities);

	for (; node; node = VirtualNode_getNext(node))
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if (stageEntityDescription->id == id)
		{
			stageEntityDescription->id = -1;
			break;
		}
	}

	if (permanent)
	{
		ASSERT(entity, "Stage::removeEntity: null node");

		VirtualList_removeElement(this->stageEntities, VirtualNode_getData(node));
		VirtualList_removeElement(this->loadedStageEntities, VirtualNode_getData(node));
		__DELETE_BASIC(VirtualNode_getData(node));
	}
}

// preload textures
static void Stage_loadTextures(Stage this)
{
	ASSERT(this, "Stage::loadTextures: null this");

	int i = 0;

	for (; this->stageDefinition->textures[i]; i++)
	{
		if(__ANIMATED_SHARED != this->stageDefinition->textures[i]->charSetDefinition.allocationType)
		{
			BgmapTextureManager_loadTexture(BgmapTextureManager_getInstance(), this->stageDefinition->textures[i], this->flushCharSets);
		}
		else
		{
			ASSERT(this, "Stage::loadTextures: loading an Object texture");
		}
	}
	
	if(0 < i)
	{
		BgmapTextureManager_calculateAvailableBgmapSegments(BgmapTextureManager_getInstance());
		ParamTableManager_reset(ParamTableManager_getInstance());
	}
	else 
	{
		BgmapTextureManager_resetAvailableBgmapSegments(BgmapTextureManager_getInstance());
	}
}

// register an entity in the streaming list
static StageEntityDescription* Stage_registerEntity(Stage this, PositionedEntity* positionedEntity)
{
	ASSERT(this, "Stage::registerEntities: null this");

	StageEntityDescription* stageEntityDescription = __NEW_BASIC(StageEntityDescription);

	stageEntityDescription->id = -1;
	stageEntityDescription->positionedEntity = positionedEntity;

	VBVec3D environmentPosition3D = {0, 0, 0};
	stageEntityDescription->smallRightcuboid = Entity_getTotalSizeFromDefinition(stageEntityDescription->positionedEntity, &environmentPosition3D);

	return stageEntityDescription;
}

// register the stage's definition entities in the streaming list
static void Stage_registerEntities(Stage this)
{
	ASSERT(this, "Stage::registerEntities: null this");

	if (this->stageEntities)
	{
		__DELETE(this->stageEntities);
	}

	this->stageEntities = __NEW(VirtualList);

	if (this->loadedStageEntities)
	{
		__DELETE(this->loadedStageEntities);
	}

	this->loadedStageEntities = __NEW(VirtualList);

	// register entities ordering them according to their distances to the origin
	// givin increasing weight (more distance) to the objects according to their
	// position in the stage's definition
	int weightIncrement = Math_squareRoot(__SCREEN_WIDTH * __SCREEN_WIDTH + __SCREEN_HEIGHT * __SCREEN_HEIGHT);
	int i = 0;
	for (;this->stageDefinition->entities[i].entityDefinition; i++)
	{
		StageEntityDescription* stageEntityDescription = Stage_registerEntity(this, &this->stageDefinition->entities[i]);

		u8 width = 0;
		u8 height = 0;

		int j = 0;
		for (; stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[j]->allocator; j++)
		{
			const SpriteDefinition* spriteDefinition = stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[j];

			if (spriteDefinition)
			{
				if (spriteDefinition->textureDefinition)
				{
					if (width < spriteDefinition->textureDefinition->cols << 3)
					{
						width = spriteDefinition->textureDefinition->cols << 3;
					}

					if (height < spriteDefinition->textureDefinition->rows << 3)
					{
						height = spriteDefinition->textureDefinition->rows << 3;
					}
				}
			}
		}

		stageEntityDescription->distance = (FIX19_13TOI(stageEntityDescription->positionedEntity->position.x) - (width >> 1)) * (FIX19_13TOI(stageEntityDescription->positionedEntity->position.x) - (width >> 1)) +
		(FIX19_13TOI(stageEntityDescription->positionedEntity->position.y) - (height >> 1)) * (FIX19_13TOI(stageEntityDescription->positionedEntity->position.y) - (height >> 1)) +
		FIX19_13TOI(stageEntityDescription->positionedEntity->position.z) * FIX19_13TOI(stageEntityDescription->positionedEntity->position.z);

		VirtualNode auxNode = VirtualList_begin(this->stageEntities);

		for (; auxNode; auxNode = VirtualNode_getNext(auxNode))
		{
			StageEntityDescription* auxStageEntityDescription = (StageEntityDescription*)VirtualNode_getData(auxNode);

			if (stageEntityDescription->distance + weightIncrement * i > auxStageEntityDescription->distance)
			{
				continue;
			}

			VirtualList_insertBefore(this->stageEntities, auxNode, stageEntityDescription);
			break;
		}

		if (!auxNode)
		{
			VirtualList_pushBack(this->stageEntities, stageEntityDescription);
		}
	}
}

// select visible entities to load
static void Stage_selectEntitiesInLoadRange(Stage this)
{
	ASSERT(this, "Stage::loadEntities: null this");

	VBVec3D focusEntityPosition = Container_getGlobalPosition(__UPCAST(Container, this->focusEntity));
	focusEntityPosition.x = FIX19_13TOI(focusEntityPosition.x);
	focusEntityPosition.y = FIX19_13TOI(focusEntityPosition.y);
	focusEntityPosition.z = FIX19_13TOI(focusEntityPosition.z);

	long focusEntityDistance = focusEntityPosition.x * focusEntityPosition.x +
	focusEntityPosition.y * focusEntityPosition.y +
	focusEntityPosition.z * focusEntityPosition.z;

	static long previousFocusEntityDistance = 0;

	u8 direction = previousFocusEntityDistance <= focusEntityDistance;

	static VirtualNode savedNode = NULL;

	VirtualNode node = savedNode ? savedNode : VirtualList_begin(this->stageEntities);
	int counter = 0;

	for (; node && counter < __STREAMING_AMPLITUDE / 4; node = direction ? VirtualNode_getPrevious(node) : VirtualNode_getNext(node), counter++);

	node = node ? node : direction ? VirtualList_begin(this->stageEntities) : VirtualList_end(this->stageEntities);
	savedNode = NULL;

	int entityLoaded = false;

	for (counter = 0; node && (!savedNode || counter < __STREAMING_AMPLITUDE); node = direction ? VirtualNode_getNext(node) : VirtualNode_getPrevious(node), counter++)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if (!savedNode)
		{
			if (direction)
			{
				if (focusEntityDistance < stageEntityDescription->distance)
				{
					savedNode = node;
				}
			}
			else
			{
				if (focusEntityDistance > stageEntityDescription->distance)
				{
					savedNode = node;
				}
			}
		}
/*
		if (entityLoaded)
		{
			if (savedNode)
			{
				break;
			}

			continue;
		}
*/
		if (0 > stageEntityDescription->id)
		{
			// if entity in load range
			if (Stage_inLoadRange(this, stageEntityDescription->positionedEntity->position, &stageEntityDescription->smallRightcuboid))
			{
				stageEntityDescription->id = 0x7FFF;
				VirtualList_pushBack(this->entitiesToLoad, stageEntityDescription);
				entityLoaded = true;
			}
		}
	}

	previousFocusEntityDistance = focusEntityDistance;
}

// load selected entities
static void Stage_loadEntities(Stage this)
{
	ASSERT(this, "Stage::loadEntities: entity not loaded");

	VirtualNode node = VirtualList_begin(this->entitiesToLoad);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		Entity entity = Entity_loadFromDefinitionWithoutInitilization(stageEntityDescription->positionedEntity, this->nextEntityId++);

		if(entity)
		{
			StageEntityToInitialize* stageEntityToInitialize = __NEW_BASIC(StageEntityToInitialize);
			stageEntityToInitialize->positionedEntity = stageEntityDescription->positionedEntity;
			stageEntityToInitialize->entity = entity;
			VirtualList_pushBack(this->entitiesToInitialize, stageEntityToInitialize);
			stageEntityDescription->id = Container_getId(__UPCAST(Container, entity));
			VirtualList_pushBack(this->loadedStageEntities, stageEntityDescription);
			
			VirtualList_removeElement(this->entitiesToLoad, stageEntityDescription);
			break;
		}
		else 
		{
			// can't load this entity, so remove its definition
			VirtualList_removeElement(this->stageEntities, stageEntityDescription);
			
			__DELETE_BASIC(stageEntityDescription);
		}
	}
}

// intialize loaded entities
static void Stage_initializeEntities(Stage this)
{
	ASSERT(this, "Stage::initializeEntities: null this");

	static Transformation environmentTransform =
	{
			// local position
			{0, 0, 0},
			// global position
			{0, 0, 0},
			// scale
			{1, 1},
			// rotation
			{0, 0, 0}
	};

	VirtualNode node = VirtualList_begin(this->entitiesToInitialize);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		StageEntityToInitialize* stageEntityToInitialize = (StageEntityToInitialize*)VirtualNode_getData(node);

		__VIRTUAL_CALL(void, Entity, initialize, stageEntityToInitialize->entity);
		
		// create the entity and add it to the world
		Container_addChild(__UPCAST(Container, this), __UPCAST(Container, stageEntityToInitialize->entity));
		
		// apply transformations
		__VIRTUAL_CALL(void, Container, initialTransform, stageEntityToInitialize->entity, &environmentTransform);

		VirtualList_removeElement(this->entitiesToInitialize, stageEntityToInitialize);
		__DELETE_BASIC(stageEntityToInitialize);
		break;
	}
}

// load all visible entities
static void Stage_loadInRangeEntities(Stage this)
{
	ASSERT(this, "Stage::loadInRangeEntities: null this");

	// need a temporal list to remove and delete entities
	VirtualNode node = VirtualList_begin(this->stageEntities);

	for (; node; node = VirtualNode_getNext(node))
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if (-1 == stageEntityDescription->id)
		{
			// if entity in load range
			if (Stage_inLoadRange(this, stageEntityDescription->positionedEntity->position, &stageEntityDescription->smallRightcuboid))
			{
				Entity entity = Stage_addPositionedEntity(this, stageEntityDescription->positionedEntity, false);

				ASSERT(entity, "Stage::loadInRangeEntities: entity not loaded");

				if(entity) 
				{
					stageEntityDescription->id = Container_getId(__UPCAST(Container, entity));
	
					VirtualList_pushBack(this->loadedStageEntities, stageEntityDescription);
				}
				else 
				{
					// can't load this entity, so remove its definition
					VirtualList_removeElement(this->stageEntities, stageEntityDescription);
					
					__DELETE_BASIC(stageEntityDescription);
					break;
				}
			}
		}
	}
}

// unload non visible entities
static void Stage_unloadOutOfRangeEntities(Stage this)
{
	ASSERT(this, "Stage::unloadOutOfRangeEntities: null this");

	if (!this->children)
	{
		return;
	}

	CACHE_ENABLE;

	// need a temporal list to remove and delete entities
	VirtualNode node = VirtualList_begin(this->children);

	// check which actors must be unloaded
	for (; node; node = VirtualNode_getNext(node))
	{
		// get next entity
		Entity entity = __UPCAST(Entity, VirtualNode_getData(node));

		// if the entity isn't visible inside the view field, unload it
		if (!__VIRTUAL_CALL(bool, Entity, isVisible, entity, __ENTITY_UNLOAD_PAD))
		{
			s16 id = Container_getId(__UPCAST(Container, entity));

			VirtualNode auxNode = VirtualList_begin(this->loadedStageEntities);

			for (; auxNode; auxNode = VirtualNode_getNext(auxNode))
			{
				StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(auxNode);

				if (stageEntityDescription->id == id)
				{
					stageEntityDescription->id = -1;

					VirtualList_removeElement(this->loadedStageEntities, stageEntityDescription);
					break;
				}
			}

			// delete it
			Container_deleteMyself(__UPCAST(Container, entity));
		}
	}

	CACHE_DISABLE;
}

// execute stage's logic
void Stage_update(Stage this)
{
	ASSERT(this, "Stage::update: null this");

	Container_update(__UPCAST(Container, this));

	if (this->ui)
	{
		Container_update(__UPCAST(Container, this->ui));
	}
}

// stream entities according to screen's position
void Stage_stream(Stage this)
{
	ASSERT(this, "Stage::stream: null this");

	// if the screen is moving
	static int streamCycle = 0;

	switch(streamCycle)
	{
		case __STREAM_UNLOAD_CYCLE:
			
			// unload not visible objects
			Stage_unloadOutOfRangeEntities(this);
			break;
			
		case __STREAM_SELECT_ENTITIES_IN_LOAD_RANGE:
			
			if (this->focusEntity)
			{
				// load visible objects
				Stage_selectEntitiesInLoadRange(this);
			}
			else
			{
				InGameEntity focusInGameEntity = Screen_getFocusInGameEntity(Screen_getInstance());
				this->focusEntity = focusInGameEntity? __UPCAST(Entity, focusInGameEntity): NULL;
			}

			break;
			
		case __STREAM_LOAD_CYCLE:

			if(VirtualList_begin(this->entitiesToLoad))
			{
				Stage_loadEntities(this);
				break;
			}
			
			streamCycle = __STREAM_INITIALIZE_CYCLE;

		case __STREAM_INITIALIZE_CYCLE:
			
			if(VirtualList_begin(this->entitiesToInitialize))
			{
				Stage_initializeEntities(this);
				break;
			}

			streamCycle = __STREAM_CYCLE_DURATION;
	}
	
	if(++streamCycle >= __STREAM_CYCLE_DURATION)
	{
		streamCycle  = __STREAM_UNLOAD_CYCLE;
	}
}

// stream entities according to screen's position
void Stage_streamAll(Stage this)
{
	ASSERT(this, "Stage::streamAll: null this");

	// must make sure there are not pending entities for removal
	Container_processRemovedChildren(__UPCAST(Container, this));
	Stage_unloadOutOfRangeEntities(this);
	Stage_loadInRangeEntities(this);
}

// if set to false, the char set memory is flushed when  a char defintion is no longer used
// only useful to false when preloading textures
// otherwise it doesn't have any effect add flushing is the default behvior
void Stage_setFlushCharSets(Stage this, bool flushCharSets)
{
	ASSERT(this, "Stage::setFlushCharSets: null this");

	this->flushCharSets = flushCharSets;

	/*
	if (!flushCharSets)
	{
		if (this->stageDefinition->textures[0])
		{
			this->flushCharSets = flushCharSets;
		}
	}
	else
	{
		this->flushCharSets = flushCharSets;
	}
	*/
}

// retrieve ui
UI Stage_getUI(Stage this)
{
	ASSERT(this, "Stage::getUI: null this");

	return this->ui;
}

// suspend for pause
void Stage_suspend(Stage this)
{
	ASSERT(this, "Stage::suspend: null this");

	Container_suspend(__UPCAST(Container, this));
	
	if(this->ui)
	{
		__VIRTUAL_CALL(void, Container, suspend, __UPCAST(Container, this->ui));
	}
}

// resume after pause
void Stage_resume(Stage this)
{
	ASSERT(this, "Stage::resume: null this");

	// clean up streaming lists
	VirtualNode node = VirtualList_begin(this->entitiesToLoad);
	for(; node; node = VirtualNode_getNext(node))
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		stageEntityDescription->id = -1;
	}
	
	VirtualList_clear(this->entitiesToLoad);
	
	node = VirtualList_begin(this->entitiesToInitialize);
	for(; node; node = VirtualNode_getNext(node))
	{
		StageEntityToInitialize* stageEntityToInitialize = (StageEntityToInitialize*)VirtualNode_getData(node);

		__DELETE(stageEntityToInitialize->entity);
		__DELETE_BASIC(stageEntityToInitialize);
	}
	
	VirtualList_clear(this->entitiesToInitialize);
	
	// set OBJs' z position
	Stage_setObjectSpritesContainers(this);

	// reload textures
	Stage_loadTextures(this);
	
	Container_resume(__UPCAST(Container, this));

	Transformation environmentTransform =
	{
			// local position
			{0, 0, 0},
			// global position
			{0, 0, 0},
			// scale
			{1, 1},
			// rotation
			{0, 0, 0}
	};
	
	__VIRTUAL_CALL(void, Container, initialTransform, this, &environmentTransform);

	if(this->ui)
	{
		__VIRTUAL_CALL(void, Container, resume, __UPCAST(Container, this->ui));
		
		__VIRTUAL_CALL(void, Container, transform, this->ui, &environmentTransform);
	}
}