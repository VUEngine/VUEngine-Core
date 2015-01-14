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

#define __STREAM_CYCLE	(__TARGET_FPS >> 2)
#define __STREAM_UNLOAD_CYCLE	(0)
#define __STREAM_LOAD_CYCLE_1	__STREAM_CYCLE / 3
#define __STREAM_LOAD_CYCLE_2	(__STREAM_CYCLE / 3) * 2


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Stage
__CLASS_DEFINITION(Stage);

typedef struct StageEntityDescription
{
	PositionedEntity* positionedEntity;
	s16 id;
	long distance;

} StageEntityDescription;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern const Optical* _optical;
void Container_processRemovedChildren(Container this);

static void Stage_constructor(Stage this);
static void Stage_setupUI(Stage this);
static StageEntityDescription* Stage_registerEntity(Stage this, PositionedEntity* positionedEntity);
static void Stage_registerEntities(Stage this);
static void Stage_loadEntities(Stage this, int loadOnlyInRangeEntities, int loadProgressively);
static void Stage_loadTextures(Stage this);
static void Stage_loadInRangeEntities(Stage this);
static void Stage_unloadOutOfRangeEntities(Stage this);
Texture TextureManager_loadTexture(TextureManager this, TextureDefinition* textureDefinition, int isPreload);


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
	__CONSTRUCT_BASE(Container, __ARGUMENTS(-1));

	this->stageEntities = NULL;
	this->stageEntitiesToTest = NULL;
	this->loadedStageEntities = NULL;
	this->removedEntities = NULL;//__NEW(VirtualList);

	this->ui = NULL;
	this->stageDefinition = NULL;

	this->flushCharGroups = true;
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

	if (this->stageEntitiesToTest)
	{
		__DELETE(this->stageEntitiesToTest);
		this->stageEntitiesToTest = NULL;
	}

	if (this->loadedStageEntities)
	{
		__DELETE(this->loadedStageEntities);
		this->loadedStageEntities = NULL;
	}
	
	// destroy the super object
	__DESTROY_BASE(Container);
}

// place holder for objects designed around OBJECTS in the VB hardware
void Stage_setupObjActor(Stage this, int *actor,int x,int y, int z)
{
	// TODO
	ASSERT(this, "Stage::setupObjActor: null this");
}

// determine if a point is visible
inline static int Stage_inLoadRange(Stage this, VBVec3D* position3D, u8 width, u8 height)
{
	ASSERT(this, "Stage::inLoadRange: null this");

	Scale scale;

	scale.x = scale.y = FIX19_13TOFIX7_9(ITOFIX19_13(1) -
		       FIX19_13_DIV(position3D->z , _optical->maximunViewDistance));

	return Optics_isVisible(*position3D,
			Optics_calculateRealSize(((u16)width), WRLD_BGMAP, scale.x),
			Optics_calculateRealSize(((u16)height), WRLD_BGMAP, scale.y),
			Optics_calculateParallax(position3D->x, position3D->z),
			__ENTITY_LOAD_PAD);
}

// load stage's entites
void Stage_load(Stage this, StageDefinition* stageDefinition, int loadOnlyInRangeEntities)
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

	// preload textures
	Stage_loadTextures(this);

	//load Stage's bgm
	//this->bgm = (u16 (*)[6])stageDefinition->bgm;

	// register all the entities in the stage's definition
	Stage_registerEntities(this);

	// load entities
	Stage_loadInRangeEntities(this);

	// retrieve focus entity for streaming
	this->focusEntity = (Entity)Screen_getFocusInGameEntity(Screen_getInstance());

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
		this->ui = (UI)((UI (*)(UIDefinition*, ...)) this->stageDefinition->uiDefinition.allocator)(0, &this->stageDefinition->uiDefinition);
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

			__VIRTUAL_CALL(void, Container, initialTransform, (Container)this->ui, __ARGUMENTS(&environmentTransform));
		}
	}
}

// 
Entity Stage_addEntity(Stage this, EntityDefinition* entityDefinition, VBVec3D *position, void *extraInfo, int permanent)
{
	ASSERT(this, "Stage::addEntity: null this");

	if (entityDefinition)
	{
		Entity entity = Entity_load(entityDefinition, this->nextEntityId++, extraInfo);

		if(entity)
		{
			// create the entity and add it to the world
			Container_addChild((Container)this, (Container)entity);
	
			Transformation environmentTransform = Container_getEnvironmentTransform((Container)this);
	
			// set spatial position
			__VIRTUAL_CALL(void, Entity, setLocalPosition, entity, __ARGUMENTS(position));
	
			// apply transformations
			__VIRTUAL_CALL(void, Container, initialTransform, (Container)entity, __ARGUMENTS(&environmentTransform));
	
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
Entity Stage_addPositionedEntity(Stage this, PositionedEntity* positionedEntity, int permanent)
{
	ASSERT(this, "Stage::addEntity: null this");

	if (positionedEntity)
	{
		Transformation environmentTransform = Container_getEnvironmentTransform((Container)this);

		Entity entity = Entity_loadFromDefinition(positionedEntity, &environmentTransform, this->nextEntityId++);

		if(entity)
		{
			// create the entity and add it to the world
			Container_addChild((Container)this, (Container)entity);
		}

		if (permanent)
		{
			// TODO
		}
		
		return entity;
	}

	return NULL;
}

// add entity to the stage
void Stage_removeEntity(Stage this, Entity entity, int permanent)
{
	ASSERT(this, "Stage::removeEntity: null this");
	ASSERT(entity, "Stage::removeEntity: null entity");

	if (!entity)
	{
		return;
	}

	// hide until effectively deleted
	Entity_hide(entity);

	Container_deleteMyself((Container)entity);

	VirtualNode node = VirtualList_begin(this->stageEntities);

	s16 id = Container_getId((Container)entity);

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
	}
}

// preload textures
static void Stage_loadTextures(Stage this)
{
	ASSERT(this, "Entity::loadTextures: null this");

	int i = 0;

	for (; this->stageDefinition->textures[i]; i++)
	{
		TextureManager_loadTexture(TextureManager_getInstance(), this->stageDefinition->textures[i], this->flushCharGroups);
	}
	
	if(0 < i)
	{
		TextureManager_calculateAvailableBgmapSegments(TextureManager_getInstance());
		ParamTableManager_reset(ParamTableManager_getInstance());
	}
	else 
	{
		TextureManager_resetAvailableBgmapSegments(TextureManager_getInstance());
	}
}

// register an entity in the streaming list
static StageEntityDescription* Stage_registerEntity(Stage this, PositionedEntity* positionedEntity)
{
	ASSERT(this, "Stage::registerEntities: null this");

	StageEntityDescription* stageEntityDescription = __NEW_BASIC(StageEntityDescription);

	stageEntityDescription->id = -1;
	stageEntityDescription->positionedEntity = positionedEntity;

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

	if (this->stageEntitiesToTest)
	{
		__DELETE(this->stageEntitiesToTest);
	}

	this->stageEntitiesToTest = __NEW(VirtualList);

	if (this->loadedStageEntities)
	{
		__DELETE(this->loadedStageEntities);
	}

	this->loadedStageEntities = __NEW(VirtualList);

	// register entities ordering them according to their distances to the origin
	// givin increasing weight (more distance) to the objects according to their
	// position in the stage's definition
	int weightIncrement = Math_squareRoot(2* __SCREEN_WIDTH * __SCREEN_WIDTH + __SCREEN_HEIGHT * __SCREEN_HEIGHT);
	int i = 0;
	for (;this->stageDefinition->entities[i].entityDefinition; i++)
	{
		StageEntityDescription* stageEntityDescription = Stage_registerEntity(this, &this->stageDefinition->entities[i]);

		u8 width = 0;
		u8 height = 0;

		/*
		int i = 0;
		for (; stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[i].allocator; i++)
		{
			const SpriteDefinition* spriteDefinition = &stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[i];

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
		*/

		stageEntityDescription->distance = (stageEntityDescription->positionedEntity->position.x - (width >> 1)) * (stageEntityDescription->positionedEntity->position.x - (width >> 1)) +
		(stageEntityDescription->positionedEntity->position.y - (height >> 1)) * (stageEntityDescription->positionedEntity->position.y - (height >> 1)) +
		stageEntityDescription->positionedEntity->position.z * stageEntityDescription->positionedEntity->position.z;

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

// load entities on demand (if they aren't loaded and are visible)
static void Stage_loadEntities(Stage this, int loadOnlyInRangeEntities, int loadProgressively)
{
	ASSERT(this, "Stage::loadEntities: null this");

	VBVec3D focusEntityPosition = Container_getGlobalPosition((Container)this->focusEntity);
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

	node = node ? node: direction ? VirtualList_begin(this->stageEntities) : VirtualList_end(this->stageEntities);
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

		if (loadProgressively && entityLoaded)
		{
			if (savedNode)
			{
				break;
			}

			continue;
		}

		if (0 > stageEntityDescription->id)
		{
			VBVec3D position3D =
			{
					FTOFIX19_13(stageEntityDescription->positionedEntity->position.x),
					FTOFIX19_13(stageEntityDescription->positionedEntity->position.y),
					FTOFIX19_13(stageEntityDescription->positionedEntity->position.z)
			};
			
			u8 hasSprites = stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions? true: false;
			u8 width = hasSprites? stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[0].textureDefinition->cols << 3: __SCREEN_WIDTH;
			u8 height = hasSprites? stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[0].textureDefinition->rows << 3: __SCREEN_HEIGHT;

			// if entity in load range
			if (!loadOnlyInRangeEntities || Stage_inLoadRange(this, &position3D, width, height))
			{
				Entity entity = Stage_addPositionedEntity(this, stageEntityDescription->positionedEntity, false);

				ASSERT(entity, "Stage::loadInRangeEntities: entity not loaded");

				if(entity)
				{
					stageEntityDescription->id = Container_getId((Container)entity);
	
					VirtualList_pushBack(this->loadedStageEntities, stageEntityDescription);
	
					entityLoaded = true;
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

	previousFocusEntityDistance = focusEntityDistance;
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
			VBVec3D position3D =
			{
					FTOFIX19_13(stageEntityDescription->positionedEntity->position.x),
					FTOFIX19_13(stageEntityDescription->positionedEntity->position.y),
					FTOFIX19_13(stageEntityDescription->positionedEntity->position.z)
			};

			u8 hasSprites = stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions? true: false;
			u8 width = hasSprites? stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[0].textureDefinition->cols << 3: __SCREEN_WIDTH;
			u8 height = hasSprites? stageEntityDescription->positionedEntity->entityDefinition->spritesDefinitions[0].textureDefinition->rows << 3: __SCREEN_HEIGHT;

			// if entity in load range
			if (Stage_inLoadRange(this, &position3D, width, height))
			{
				Entity entity = Stage_addPositionedEntity(this, stageEntityDescription->positionedEntity, false);

				ASSERT(entity, "Stage::loadInRangeEntities: entity not loaded");

				if(entity) 
				{
					stageEntityDescription->id = Container_getId((Container)entity);
	
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
		Entity entity = (Entity)VirtualNode_getData(node);

		// if the entity isn't visible inside the view field, unload it
		if (!__VIRTUAL_CALL(bool, Entity, isVisible, entity, __ARGUMENTS(__ENTITY_UNLOAD_PAD)))
		{
			s16 id = Container_getId((Container)entity);

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
			Container_deleteMyself((Container)entity);
		}
	}

	CACHE_DISABLE;
}

// execute stage's logic
void Stage_update(Stage this)
{
	ASSERT(this, "Stage::update: null this");

	Container_update((Container)this);

	if (this->ui)
	{
		Container_update((Container)this->ui);
	}
}

// stream entities according to screen's position
void Stage_stream(Stage this)
{
	ASSERT(this, "Stage::stream: null this");

	// if the screen is moving
	static int load = __STREAM_CYCLE;
	if (!--load)
	{
		// unload not visible objects
		Stage_unloadOutOfRangeEntities(this);

		load = __STREAM_CYCLE;
	}
	else if (__STREAM_LOAD_CYCLE_1 == load || __STREAM_LOAD_CYCLE_2 == load)
	{
		if (this->removedChildren && VirtualList_getSize(this->removedChildren))
		{
			Container_processRemovedChildren((Container)this);
		}
		else if (this->focusEntity)
		{
			// load visible objects
			Stage_loadEntities(this, true, true);
		}
		else
		{
			this->focusEntity = (Entity)Screen_getFocusInGameEntity(Screen_getInstance());
		}
	}
}

// stream entities according to screen's position
void Stage_streamAll(Stage this)
{
	ASSERT(this, "Stage::streamAll: null this");

	// must make sure there are not pending entities for removal
	Container_processRemovedChildren((Container)this);
	Stage_unloadOutOfRangeEntities(this);
	Stage_loadInRangeEntities(this);
}

// if set to false, the char set memory is flushed when  a char defintion is no longer used
// only useful to false when preloading textures
// otherwise it doesn't have any effect add flushing is the default behvior
void Stage_setFlushCharGroups(Stage this, int flushCharGroups)
{
	ASSERT(this, "Stage::setFlushCharGroups: null this");

	this->flushCharGroups = flushCharGroups;

	/*
	if (!flushCharGroups)
	{
		if (this->stageDefinition->textures[0])
		{
			this->flushCharGroups = flushCharGroups;
		}
	}
	else
	{
		this->flushCharGroups = flushCharGroups;
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

	Container_suspend((Container)this);
	
	if(this->ui)
	{
		__VIRTUAL_CALL(void, Container, suspend, (Container)this->ui);
	}
}

// resume after pause
void Stage_resume(Stage this)
{
	ASSERT(this, "Stage::resume: null this");

	// reload textures
	Stage_loadTextures(this);
	
	Container_resume((Container)this);

	if(this->ui)
	{
		__VIRTUAL_CALL(void, Container, resume, (Container)this->ui);
		
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

		__VIRTUAL_CALL(void, Container, transform, (Container)this->ui, __ARGUMENTS(&environmentTransform));
	}
}
