/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <Stage.h>
#include <Optics.h>
#include <PhysicalWorld.h>
#include <SoundManager.h>
#include <Screen.h>
#include <HardwareManager.h>
#include <SpriteManager.h>
#include <Texture.h>
#include <ParamTableManager.h>
#include <VPUManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------


#define __STREAMING_CYCLES		4

#define __MAXIMUM_PARALLAX		10
#define __LOAD_LOW_X_LIMIT		ITOFIX19_13(- __MAXIMUM_PARALLAX - this->stageDefinition->streaming.loadPadding)
#define __LOAD_HIGHT_X_LIMIT	ITOFIX19_13(__SCREEN_WIDTH + __MAXIMUM_PARALLAX + this->stageDefinition->streaming.loadPadding)
#define __LOAD_LOW_Y_LIMIT		ITOFIX19_13(- this->stageDefinition->streaming.loadPadding)
#define __LOAD_HIGHT_Y_LIMIT	ITOFIX19_13(__SCREEN_HEIGHT + this->stageDefinition->streaming.loadPadding)


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_FRIEND_DEFINITION(Container);

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
Shape SpatialObject_getShape(SpatialObject this);

static void Stage_constructor(Stage this);
static void Stage_setupUI(Stage this);
static StageEntityDescription* Stage_registerEntity(Stage this, PositionedEntity* positionedEntity);
static void Stage_registerEntities(Stage this, VirtualList entityNamesToIgnore);
static void Stage_selectEntitiesInLoadRange(Stage this);
static void Stage_setObjectSpritesContainers(Stage this);
static void Stage_preloadAssets(Stage this);
static void Stage_loadInRangeEntities(Stage this);
static void Stage_unloadOutOfRangeEntities(Stage this);
static void Stage_unloadChild(Stage this, Container child);
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
	__CONSTRUCT_BASE(-1, NULL);

	this->stageEntities = NULL;
	this->loadedStageEntities = NULL;
	this->removedEntities = NULL;
	this->entitiesToLoad = __NEW(VirtualList);
	this->entitiesToInitialize = __NEW(VirtualList);
	
	this->ui = NULL;
	this->stageDefinition = NULL;

	this->focusEntity = NULL;
	
	this->nextEntityId = 0;
}

// class's destructor
void Stage_destructor(Stage this)
{
	ASSERT(this, "Stage::destructor: null this");

	if(this->ui)
	{
		__DELETE(this->ui);
		this->ui = NULL;
	}
	
	if(this->stageEntities)
	{
		VirtualNode node = VirtualList_begin(this->stageEntities);

		for(; node; node = VirtualNode_getNext(node))
		{
			__DELETE_BASIC(VirtualNode_getData(node));
		}
		
		__DELETE(this->stageEntities);

		this->stageEntities = NULL;
	}

	if(this->loadedStageEntities)
	{
		__DELETE(this->loadedStageEntities);
		this->loadedStageEntities = NULL;
	}
	
	if(this->entitiesToLoad)
	{
		__DELETE(this->entitiesToLoad);
		this->entitiesToLoad = NULL;
	}
	
	if(this->entitiesToInitialize)
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
	if(position2D.x + ITOFIX19_13(smallRightcuboid->x1) < __LOAD_LOW_X_LIMIT || position2D.x - ITOFIX19_13(smallRightcuboid->x0) > __LOAD_HIGHT_X_LIMIT)
	{
		return false;
	}

	// check y visibility
	if(position2D.y + ITOFIX19_13(smallRightcuboid->y1) < __LOAD_LOW_Y_LIMIT || position2D.y - ITOFIX19_13(smallRightcuboid->y0) > __LOAD_HIGHT_Y_LIMIT)
	{
		return false;
	}

	return true;
}

static void Stage_setObjectSpritesContainers(Stage this)
{
	ASSERT(this, "Stage::setObjectSpritesContainers: null this");

	ObjectSpriteContainerManager_setupObjectSpriteContainers(ObjectSpriteContainerManager_getInstance(), this->stageDefinition->objectSpritesContainersSize, this->stageDefinition->objectSpritesContainersZPosition);
}

void Stage_setupPalettes(Stage this)
{
	ASSERT(this, "Stage::setupPalettes: null this");

	VPUManager_setupPalettes(VPUManager_getInstance(), &this->stageDefinition->paletteConfig);
}


// load stage's entites
void Stage_load(Stage this, StageDefinition* stageDefinition, VirtualList entityNamesToIgnore, bool overrideScreenPosition)
{
	ASSERT(this, "Stage::load: null this");

	// set world's definition
	this->stageDefinition = stageDefinition;

	// set optical values
	Screen_setOptical(Screen_getInstance(), this->stageDefinition->optical);

	// stop all sounds
	SoundManager_stopAllSound(SoundManager_getInstance());

	// set world's limits
	Screen_setStageSize(Screen_getInstance(), stageDefinition->size);

	if(overrideScreenPosition)
	{
		Screen_setPosition(Screen_getInstance(), stageDefinition->screenPosition);
	}

	// set palettes
	Stage_setupPalettes(this);
	
	// setup OBJs
	Stage_setObjectSpritesContainers(this);

	// preload textures
	Stage_preloadAssets(this);

	// register all the entities in the stage's definition
	Stage_registerEntities(this, entityNamesToIgnore);

	// load entities
	Stage_loadInRangeEntities(this);

	// retrieve focus entity for streaming
	InGameEntity focusInGameEntity = Screen_getFocusInGameEntity(Screen_getInstance());
	this->focusEntity = focusInGameEntity ? __SAFE_CAST(Entity, focusInGameEntity) : NULL;

	// setup ui
	Stage_setupUI(this);
	
	// set physics
	PhysicalWorld_setFriction(PhysicalWorld_getInstance(), stageDefinition->friction);
	PhysicalWorld_setGravity(PhysicalWorld_getInstance(), stageDefinition->gravity);

	// load background music
	SoundManager_playBGM(SoundManager_getInstance(), (const u16 (*)[6])stageDefinition->bgm);

	// setup the column table
	HardwareManager_setupColumnTable(HardwareManager_getInstance());
	
	// apply transformations
	Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
	__VIRTUAL_CALL(void, Container, initialTransform, this, &environmentTransform);

	if(this->ui)
	{
		__VIRTUAL_CALL(void, Container, initialTransform, this->ui, &environmentTransform);
	}
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

	if(this->ui)
	{
		__DELETE(this->ui);
		this->ui = NULL;
	}

	if(this->stageDefinition->uiDefinition.allocator)
	{
		// call the appropiate allocator to support inheritance!
		this->ui = ((UI (*)(UIDefinition*, ...)) this->stageDefinition->uiDefinition.allocator)(&this->stageDefinition->uiDefinition);
		ASSERT(this->ui, "Stage::setupUI: null ui");

		// setup ui if allocated and constructed
		if(this->ui)
		{
			// apply transformations
			Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
			__VIRTUAL_CALL(void, Container, initialTransform, this->ui, &environmentTransform);
		}
	}
}

// 
Entity Stage_addEntity(Stage this, const EntityDefinition* const entityDefinition, const char* const name, const VBVec3D* const position, void* const extraInfo, bool permanent)
{
	ASSERT(this, "Stage::addEntity: null this");

	if(entityDefinition)
	{
		Entity entity = Entity_load(entityDefinition, this->nextEntityId++, name, extraInfo);

		if(entity)
		{
			// set spatial position
			__VIRTUAL_CALL(void, Entity, setLocalPosition, entity, position);

			// initialize now
			__VIRTUAL_CALL(void, Entity, initialize, entity);

			// create the entity and add it to the world
			Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, entity));

			// apply transformations
			Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
			__VIRTUAL_CALL(void, Container, initialTransform, entity, &environmentTransform);
/*
			if(permanent)
			{
				// TODO
			}
*/
			__VIRTUAL_CALL(void, Entity, ready, entity);

			return entity;
		}
	}

	return NULL;
}

bool Stage_registerEntityId(Stage this, s16 id, EntityDefinition* entityDefinition)
{
	ASSERT(this, "Stage::registerEntityId: null this");

	VirtualNode node = VirtualList_begin(this->stageEntities);

	for(; node; node = VirtualNode_getNext(node))
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if(entityDefinition == stageEntityDescription->positionedEntity->entityDefinition)
		{
			stageEntityDescription->id = id;
			return true;
		}
	}
	
	return false;
}


// add entity to the stage
Entity Stage_addPositionedEntity(Stage this, const PositionedEntity* const positionedEntity, bool permanent)
{
	ASSERT(this, "Stage::addEntity: null this");

	if(positionedEntity)
	{
		Entity entity = Entity_loadFromDefinition(positionedEntity, this->nextEntityId++);

		if(entity)
		{
			// must initialize after adding the children
			__VIRTUAL_CALL(void, Entity, initialize, entity);

			// create the entity and add it to the world
			Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, entity));

			// apply transformations
			Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
			__VIRTUAL_CALL(void, Container, initialTransform, entity, &environmentTransform);
			
			__VIRTUAL_CALL(void, Entity, ready, entity);
		}
/*
		if(permanent)
		{
			// TODO
		}
*/
		return entity;
	}

	return NULL;
}

// add entity to the stage
void Stage_removeChild(Stage this, Container child)
{
	ASSERT(this, "Stage::removeEntity: null this");
	ASSERT(child, "Stage::removeEntity: null child");

	if(!child)
	{
		return;
	}

	// hide until effectively deleted
	if(__GET_CAST(Entity, child))
	{
		Entity_hide(__SAFE_CAST(Entity, child));
	}

	Container_removeChild(__SAFE_CAST(Container, this), child);

	s16 id = Container_getId(__SAFE_CAST(Container, child));

	VirtualNode node = VirtualList_begin(this->stageEntities);

	for(; node; node = VirtualNode_getNext(node))
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if(stageEntityDescription->id == id)
		{
			stageEntityDescription->id = -1;
			break;
		}
	}

	if(node)
	{
		VirtualList_removeElement(this->stageEntities, VirtualNode_getData(node));
		VirtualList_removeElement(this->loadedStageEntities, VirtualNode_getData(node));
		__DELETE_BASIC(VirtualNode_getData(node));
	}
}

// unload entity from the stage
static void Stage_unloadChild(Stage this, Container child)
{
	ASSERT(this, "Stage::unloadChild: null this");
	ASSERT(child, "Stage::unloadChild: null child");

	if(!child)
	{
		return;
	}

	// hide until effectively deleted
	if(__GET_CAST(Entity, child))
	{
		Entity_hide(__SAFE_CAST(Entity, child));
	}

	child->deleteMe = true;
	Container_removeChild(__SAFE_CAST(Container, this), child);

	s16 id = Container_getId(__SAFE_CAST(Container, child));

	VirtualNode node = VirtualList_begin(this->stageEntities);

	for(; node; node = VirtualNode_getNext(node))
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if(stageEntityDescription->id == id)
		{
			stageEntityDescription->id = -1;
			break;
		}
	}
}

// preload textures
static void Stage_preloadAssets(Stage this)
{
	ASSERT(this, "Stage::preloadAssets: null this");

	int i = 0;

	if(this->stageDefinition->charSets)
	{
		for (; this->stageDefinition->charSets[i]; i++)
		{
			if(__ANIMATED_SINGLE != this->stageDefinition->charSets[i]->allocationType)
			{
				CharSetManager_getCharSet(CharSetManager_getInstance(), this->stageDefinition->charSets[i]);
			}
			else
			{
				ASSERT(this, "Stage::preloadAssets: preloading an animated single char set");
			}
		}
	}

	if(this->stageDefinition->textures)
	{
		for (i = 0; this->stageDefinition->textures[i]; i++)
		{
			if(__ANIMATED_SINGLE != this->stageDefinition->textures[i]->charSetDefinition->allocationType)
			{
				BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), this->stageDefinition->textures[i]);
			}
			else
			{
				ASSERT(this, "Stage::preloadAssets: loading an Object texture");
			}
		}
	}

	BgmapTextureManager_setSpareBgmapSegments(BgmapTextureManager_getInstance(), this->stageDefinition->spareBgmapSegments);

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
static void Stage_registerEntities(Stage this, VirtualList entityNamesToIgnore)
{
	ASSERT(this, "Stage::registerEntities: null this");

	if(this->stageEntities)
	{
		__DELETE(this->stageEntities);
	}

	this->stageEntities = __NEW(VirtualList);

	if(this->loadedStageEntities)
	{
		__DELETE(this->loadedStageEntities);
	}

	this->loadedStageEntities = __NEW(VirtualList);

	// register entities ordering them according to their distances to the origin
	// givin increasing weight (more distance) to the objects according to their
	// position in the stage's definition
	int weightIncrement = Math_squareRoot(__SCREEN_WIDTH * __SCREEN_WIDTH + __SCREEN_HEIGHT * __SCREEN_HEIGHT);
	int i = 0;
	for(;this->stageDefinition->entities[i].entityDefinition; i++)
	{
		if(this->stageDefinition->entities[i].name && entityNamesToIgnore)
		{
			VirtualNode node = VirtualList_begin(entityNamesToIgnore);
			
			for(; node; node = VirtualNode_getNext(node))
			{
				const char* name = (char*)VirtualNode_getData(node);
				if(!strncmp(name, this->stageDefinition->entities[i].name, __MAX_CONTAINER_NAME_LENGTH))
				{
					break;
				}
			}
			
			if(node)
			{
				continue;
			}
		}
		
		StageEntityDescription* stageEntityDescription = Stage_registerEntity(this, &this->stageDefinition->entities[i]);

		u8 width = 0;
		u8 height = 0;

		int j = 0;
		for(; stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[j]->allocator; j++)
		{
			const SpriteDefinition* spriteDefinition = stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[j];

			if(spriteDefinition)
			{
				if(spriteDefinition->textureDefinition)
				{
					if(width < spriteDefinition->textureDefinition->cols << 3)
					{
						width = spriteDefinition->textureDefinition->cols << 3;
					}

					if(height < spriteDefinition->textureDefinition->rows << 3)
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

		for(; auxNode; auxNode = VirtualNode_getNext(auxNode))
		{
			StageEntityDescription* auxStageEntityDescription = (StageEntityDescription*)VirtualNode_getData(auxNode);

			if(stageEntityDescription->distance + weightIncrement * i > auxStageEntityDescription->distance)
			{
				continue;
			}

			VirtualList_insertBefore(this->stageEntities, auxNode, stageEntityDescription);
			break;
		}

		if(!auxNode)
		{
			VirtualList_pushBack(this->stageEntities, stageEntityDescription);
		}
	}
}

// select visible entities to load
static void Stage_selectEntitiesInLoadRange(Stage this)
{
	ASSERT(this, "Stage::loadEntities: null this");

	VBVec3D focusEntityPosition = *Container_getGlobalPosition(__SAFE_CAST(Container, this->focusEntity));
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

	for(; node && counter < this->stageDefinition->streaming.streamingAmplitude / 4; node = direction ? VirtualNode_getPrevious(node) : VirtualNode_getNext(node), counter++);

	node = node ? node : direction ? VirtualList_begin(this->stageEntities) : VirtualList_end(this->stageEntities);
	savedNode = NULL;

	int entityLoaded = false;

	for(counter = 0; node && (!savedNode || counter < this->stageDefinition->streaming.streamingAmplitude); node = direction ? VirtualNode_getNext(node) : VirtualNode_getPrevious(node), counter++)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if(!savedNode)
		{
			if(direction)
			{
				if(focusEntityDistance < stageEntityDescription->distance)
				{
					savedNode = node;
				}
			}
			else
			{
				if(focusEntityDistance > stageEntityDescription->distance)
				{
					savedNode = node;
				}
			}
		}
/*
		if(entityLoaded)
		{
			if(savedNode)
			{
				break;
			}

			continue;
		}
*/
		if(0 > stageEntityDescription->id)
		{
			// if entity in load range
			if(Stage_inLoadRange(this, stageEntityDescription->positionedEntity->position, &stageEntityDescription->smallRightcuboid))
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
			stageEntityDescription->id = Container_getId(__SAFE_CAST(Container, entity));
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

	Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));

	VirtualNode node = VirtualList_begin(this->entitiesToInitialize);
	
	for(; node; node = VirtualNode_getNext(node))
	{
		StageEntityToInitialize* stageEntityToInitialize = (StageEntityToInitialize*)VirtualNode_getData(node);

		__VIRTUAL_CALL(void, Entity, initialize, stageEntityToInitialize->entity);
		
		// create the entity and add it to the world
		Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, stageEntityToInitialize->entity));
		
		// apply transformations
		__VIRTUAL_CALL(void, Container, initialTransform, stageEntityToInitialize->entity, &environmentTransform);

		__VIRTUAL_CALL(void, Entity, ready, stageEntityToInitialize->entity);

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

	for(; node; node = VirtualNode_getNext(node))
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(node);

		if(-1 == stageEntityDescription->id)
		{
			// if entity in load range
			if(stageEntityDescription->positionedEntity->loadRegardlessOfPosition || Stage_inLoadRange(this, stageEntityDescription->positionedEntity->position, &stageEntityDescription->smallRightcuboid))
			{
				Entity entity = Stage_addPositionedEntity(this, stageEntityDescription->positionedEntity, false);

				ASSERT(entity, "Stage::loadInRangeEntities: entity not loaded");

				if(entity) 
				{
					stageEntityDescription->id = Container_getId(__SAFE_CAST(Container, entity));
	
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

	if(!this->children)
	{
		return;
	}

	CACHE_ENABLE;

	// need a temporal list to remove and delete entities
	VirtualNode node = VirtualList_begin(this->children);

	// check which actors must be unloaded
	for(; node; node = VirtualNode_getNext(node))
	{
		// get next entity
		Entity entity = __SAFE_CAST(Entity, VirtualNode_getData(node));

		// if the entity isn't visible inside the view field, unload it
		if(!__VIRTUAL_CALL(bool, Entity, isVisible, entity, (this->stageDefinition->streaming.loadPadding + this->stageDefinition->streaming.unloadPadding)))
		{
			s16 id = Container_getId(__SAFE_CAST(Container, entity));

			VirtualNode auxNode = VirtualList_begin(this->loadedStageEntities);

			for(; auxNode; auxNode = VirtualNode_getNext(auxNode))
			{
				StageEntityDescription* stageEntityDescription = (StageEntityDescription*)VirtualNode_getData(auxNode);

				if(stageEntityDescription->id == id)
				{
					stageEntityDescription->id = -1;

					VirtualList_removeElement(this->loadedStageEntities, stageEntityDescription);
					break;
				}
			}

			// unload it
			Stage_unloadChild(this, __SAFE_CAST(Container, entity));
		}
	}

	CACHE_DISABLE;
}

// execute stage's logic
void Stage_update(Stage this)
{
	ASSERT(this, "Stage::update: null this");

	Container_update(__SAFE_CAST(Container, this));

	if(this->ui)
	{
		Container_update(__SAFE_CAST(Container, this->ui));
	}
}

// transform state
void Stage_transform(Stage this, const Transformation* environmentTransform)
{
	ASSERT(this, "Stage::transform: null this");

	Container_transform(__SAFE_CAST(Container, this), environmentTransform);
	
	if(this->ui)
	{
		// static to avoid call to _memcpy
		static Transformation uiEnvironmentTransform =
		{
				// local position
				{0, 0, 0},
				// global position
				{0, 0, 0},
				// local rotation
				{0, 0, 0},
				// global rotation
				{0, 0, 0},
				// local scale
				{ITOFIX7_9(1), ITOFIX7_9(1)},
				// global scale
				{ITOFIX7_9(1), ITOFIX7_9(1)}
		};
		
		uiEnvironmentTransform.globalPosition = (VBVec3D){_screenPosition->x, _screenPosition->y, _screenPosition->z};


		__VIRTUAL_CALL(void, Container, transform, this->ui, &uiEnvironmentTransform);
	}
}

// stream entities according to screen's position
void Stage_stream(Stage this)
{
	ASSERT(this, "Stage::stream: null this");

	// if the screen is moving
	static int streamingCycleCounter = 0;
	int streamingCycleBase = this->stageDefinition->streaming.delayPerCycle / __STREAMING_CYCLES;
	int streamingDelayPerCycle = this->stageDefinition->streaming.delayPerCycle;
	
	if(!streamingCycleCounter)
	{
		// unload not visible objects
		Stage_unloadOutOfRangeEntities(this);
	}
	else if(streamingCycleCounter == streamingCycleBase)
	{			
		if(this->focusEntity)
		{
			// load visible objects
			Stage_selectEntitiesInLoadRange(this);
		}
		else
		{
			InGameEntity focusInGameEntity = Screen_getFocusInGameEntity(Screen_getInstance());
			this->focusEntity = focusInGameEntity ? __SAFE_CAST(Entity, focusInGameEntity) : NULL;
		}
	}			
	else if(streamingCycleCounter == streamingCycleBase * 2)
	{
		if(VirtualList_begin(this->entitiesToLoad))
		{
			Stage_loadEntities(this);
		}
		/*else
		{
			streamingCycleCounter = streamingCycleBase * 3;
		}*/
	}
	else if(streamingCycleCounter == streamingCycleBase * 3)
	{		
		if(VirtualList_begin(this->entitiesToInitialize))
		{
			Stage_initializeEntities(this);
		}

		//streamingCycleCounter = streamingDelayPerCycle;
	}
	
	if(++streamingCycleCounter >= streamingDelayPerCycle)
	{
		streamingCycleCounter  = 0;
	}
}

// stream entities according to screen's position
void Stage_streamAll(Stage this)
{
	ASSERT(this, "Stage::streamAll: null this");

	// must make sure there are not pending entities for removal
	Container_processRemovedChildren(__SAFE_CAST(Container, this));
	Stage_unloadOutOfRangeEntities(this);
	Stage_loadInRangeEntities(this);
	SpriteManager_processFreedLayers(SpriteManager_getInstance());
	SpriteManager_sortLayers(SpriteManager_getInstance(), false);
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

	Container_suspend(__SAFE_CAST(Container, this));
	
	if(this->ui)
	{
		__VIRTUAL_CALL(void, Container, suspend, __SAFE_CAST(Container, this->ui));
	}
	
	// relinquish screen focus priority
	if(this->focusEntity && Screen_getFocusInGameEntity(Screen_getInstance()))
	{
		if(this->focusEntity == __SAFE_CAST(Entity, Screen_getFocusInGameEntity(Screen_getInstance())))
		{
			// relinquish focus entity
		    Screen_setFocusInGameEntity(Screen_getInstance(), NULL);
		}
	}
}

// resume after pause
void Stage_resume(Stage this)
{
	ASSERT(this, "Stage::resume: null this");

	// set back optical values
	Screen_setOptical(Screen_getInstance(), this->stageDefinition->optical);
	
	// set physics
	PhysicalWorld_setFriction(PhysicalWorld_getInstance(), this->stageDefinition->friction);
	PhysicalWorld_setGravity(PhysicalWorld_getInstance(), this->stageDefinition->gravity);

	// set palettes
	Stage_setupPalettes(this);

	// set OBJs' z position
	Stage_setObjectSpritesContainers(this);

	// reload textures
	Stage_preloadAssets(this);

	if(this->focusEntity)
	{
		// recover focus entity
	    Screen_setFocusInGameEntity(Screen_getInstance(), __SAFE_CAST(InGameEntity, this->focusEntity));
	}

	// load background music
	SoundManager_playBGM(SoundManager_getInstance(), (const u16 (*)[6])this->stageDefinition->bgm);

	Container_resume(__SAFE_CAST(Container, this));

	// apply transformations
	Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
	__VIRTUAL_CALL(void, Container, initialTransform, this, &environmentTransform);

	if(this->ui)
	{
		__VIRTUAL_CALL(void, Container, resume, __SAFE_CAST(Container, this->ui));
		
		__VIRTUAL_CALL(void, Container, initialTransform, this->ui, &environmentTransform);
	}
}