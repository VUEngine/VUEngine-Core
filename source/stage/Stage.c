/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <Stage.h>
#include <Optics.h>
#include <Game.h>
#include <EntityFactory.h>
#include <PhysicalWorld.h>
#include <TimerManager.h>
#include <SoundManager.h>
#include <Screen.h>
#include <HardwareManager.h>
#include <SpriteManager.h>
#include <BgmapTextureManager.h>
#include <CharSetManager.h>
#include <Texture.h>
#include <ParamTableManager.h>
#include <VIPManager.h>
#include <RecyclableBgmapTextureManager.h>
#include <ParticleRemover.h>
#include <debugConfig.h>
#ifdef __PROFILE_STREAMING
#include <TimerManager.h>
#include <Printing.h>
#endif


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __STREAMING_CYCLES		5

#define __MAXIMUM_PARALLAX		10
#define __LOAD_LOW_X_LIMIT		(- __MAXIMUM_PARALLAX - this->stageDefinition->streaming.loadPadding)
#define __LOAD_HIGHT_X_LIMIT	(__SCREEN_WIDTH + __MAXIMUM_PARALLAX + this->stageDefinition->streaming.loadPadding)
#define __LOAD_LOW_Y_LIMIT		(- this->stageDefinition->streaming.loadPadding)
#define __LOAD_HIGHT_Y_LIMIT	(__SCREEN_HEIGHT + this->stageDefinition->streaming.loadPadding)
#define __LOAD_LOW_Z_LIMIT		(- this->stageDefinition->streaming.loadPadding)
#define __LOAD_HIGHT_Z_LIMIT	(__SCREEN_DEPTH + this->stageDefinition->streaming.loadPadding)


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Stage
 * @extends Container
 */
__CLASS_DEFINITION(Stage, Container);
__CLASS_FRIEND_DEFINITION(Container);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);

/**
 * Stage Entity Description
 *
 * @memberof Stage
 */
typedef struct StageEntityDescription
{
	PositionedEntity* positionedEntity;
	SmallRightCuboid smallRightCuboid;
	s16 internalId;
	long distance;

} StageEntityDescription;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenPosition;
extern const Optical* _optical;
Shape SpatialObject_getShape(SpatialObject this);
BgmapTexture BgmapTextureManager_loadTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition, int isPreload);

static void Stage_constructor(Stage this);
static void Stage_setupUI(Stage this);
static StageEntityDescription* Stage_registerEntity(Stage this, PositionedEntity* positionedEntity);
static void Stage_registerEntities(Stage this, VirtualList positionedEntitiesToIgnore);
static void Stage_setObjectSpritesContainers(Stage this);
static void Stage_preloadAssets(Stage this);
static void Stage_unloadChild(Stage this, Container child);
static void Stage_setFocusEntity(Stage this, InGameEntity focusInGameEntity);
static void Stage_loadInitialEntities(Stage this);
static void Stage_unloadOutOfRangeEntities(Stage this, int defer);
static void Stage_loadInRangeEntities(Stage this, int defer);

#ifdef __PROFILE_STREAMING
void EntityFactory_showStatus(EntityFactory this __attribute__ ((unused)), int x, int y);
#endif

typedef void (*StreamingPhase)(Stage, int);

static const StreamingPhase _streamingPhases[] =
{
	&Stage_unloadOutOfRangeEntities,
	&Stage_loadInRangeEntities,
};

#ifdef __PROFILE_STREAMING
static u32 unloadOutOfRangeEntitiesHighestTime = 0;
static u32 loadInRangeEntitiesHighestTime = 0;
static u32 entityFactoryHighestTime = 0;
static u32 timeBeforeProcess = 0;
#endif


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
	__CONSTRUCT_BASE(Container, NULL);

	this->entityFactory = __NEW(EntityFactory);
	this->particleRemover = __NEW(ParticleRemover);

	this->stageEntities = NULL;
	this->loadedStageEntities = NULL;
	this->uiContainer = NULL;
	this->stageDefinition = NULL;
	this->focusInGameEntity = NULL;
	this->streamingHeadNode = NULL;
	this->previousFocusEntityDistance = 0;
	this->nextEntityId = 0;
	this->streamingPhase = 0;
	this->streamingCycleCounter = 0;
	this->hasRemovedChildren = false;
}

// class's destructor
void Stage_destructor(Stage this)
{
	ASSERT(this, "Stage::destructor: null this");

	__DELETE(this->entityFactory);

	if(this->uiContainer)
	{
		__DELETE(this->uiContainer);
		this->uiContainer = NULL;
	}

	if(this->stageEntities)
	{
		VirtualNode node = this->stageEntities->head;

		for(; node; node = node->next)
		{
			__DELETE_BASIC(node->data);
		}

		__DELETE(this->stageEntities);

		this->stageEntities = NULL;
	}

	if(this->loadedStageEntities)
	{
		__DELETE(this->loadedStageEntities);
		this->loadedStageEntities = NULL;
	}

	__DELETE(this->particleRemover);
	this->particleRemover = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// determine if a point is visible
inline static int Stage_isEntityInLoadRange(Stage this, VBVec3D position3D, const SmallRightCuboid* smallRightCuboid)
{
	ASSERT(this, "Stage::isEntityInLoadRange: null this");

	VBVec2D position2D;

	__OPTICS_NORMALIZE(position3D);

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, position2D);
	position2D.z = position3D.z;
	position2D.parallax = Optics_calculateParallax(position3D.x, position3D.z);

	// check x visibility
	if(FIX19_13TOI(position2D.x) + smallRightCuboid->x1 + position2D.parallax <  __LOAD_LOW_X_LIMIT || FIX19_13TOI(position2D.x) + smallRightCuboid->x0 - position2D.parallax >  __LOAD_HIGHT_X_LIMIT)
	{
		return false;
	}

	// check y visibility
	if(FIX19_13TOI(position2D.y) + smallRightCuboid->y1 <  __LOAD_LOW_Y_LIMIT || FIX19_13TOI(position2D.y) + smallRightCuboid->y0 >  __LOAD_HIGHT_Y_LIMIT)
	{
		return false;
	}

	// check z visibility
	if(FIX19_13TOI(position2D.z) + smallRightCuboid->z1 <  __LOAD_LOW_Z_LIMIT || FIX19_13TOI(position2D.z) + smallRightCuboid->z0 >  __LOAD_HIGHT_Z_LIMIT)
	{
		return false;
	}

	return true;
}

static void Stage_setObjectSpritesContainers(Stage this)
{
	ASSERT(this, "Stage::setObjectSpritesContainers: null this");

	ObjectSpriteContainerManager_setupObjectSpriteContainers(ObjectSpriteContainerManager_getInstance(), this->stageDefinition->rendering.objectSpritesContainersSize, this->stageDefinition->rendering.objectSpritesContainersZPosition);
}

void Stage_setupPalettes(Stage this)
{
	ASSERT(this, "Stage::setupPalettes: null this");

	VIPManager_setupPalettes(VIPManager_getInstance(), &this->stageDefinition->rendering.paletteConfig);
}


// load stage's entites
void Stage_load(Stage this, StageDefinition* stageDefinition, VirtualList positionedEntitiesToIgnore, bool overrideScreenPosition)
{
	ASSERT(this, "Stage::load: null this");

	// set world's definition
	this->stageDefinition = stageDefinition;

	// set optical values
	Screen_setOptical(Screen_getInstance(), this->stageDefinition->rendering.optical);

	// stop all sounds
	SoundManager_stopAllSound(SoundManager_getInstance());

	// set world's limits
	Screen_setStageSize(Screen_getInstance(), stageDefinition->level.size);

	if(overrideScreenPosition)
	{
		Screen_setPosition(Screen_getInstance(), stageDefinition->level.screenInitialPosition);
	}

	// set palettes
	Stage_setupPalettes(this);

	// setup OBJs
	Stage_setObjectSpritesContainers(this);

	// setup SpriteManager's configuration
	SpriteManager_setCyclesToWaitForTextureWriting(SpriteManager_getInstance(), this->stageDefinition->rendering.cyclesToWaitForTextureWriting);
	SpriteManager_setTexturesMaximumRowsToWrite(SpriteManager_getInstance(), this->stageDefinition->rendering.texturesMaximumRowsToWrite);
	SpriteManager_setMaximumAffineRowsToComputePerCall(SpriteManager_getInstance(), this->stageDefinition->rendering.maximumAffineRowsToComputePerCall);

	// preload textures
	Stage_preloadAssets(this);

	// register all the entities in the stage's definition
	Stage_registerEntities(this, positionedEntitiesToIgnore);

	// load entities
	Stage_loadInitialEntities(this);

	// retrieve focus entity for streaming
	Stage_setFocusEntity(this, Screen_getFocusInGameEntity(Screen_getInstance()));

	// setup ui
	Stage_setupUI(this);

	// set physics
	PhysicalWorld_setFriction(Game_getPhysicalWorld(Game_getInstance()), stageDefinition->physics.friction);
	PhysicalWorld_setGravity(Game_getPhysicalWorld(Game_getInstance()), stageDefinition->physics.gravity);

	// load background music
	SoundManager_playBGM(SoundManager_getInstance(), (const u16 (*)[6])stageDefinition->assets.bgm);

	// setup colors and brightness
	VIPManager_setBackgroundColor(VIPManager_getInstance(), stageDefinition->rendering.colorConfig.backgroundColor);
	VIPManager_setupBrightnessRepeat(VIPManager_getInstance(), stageDefinition->rendering.colorConfig.brightnessRepeat);

	// set particle removal delay
	ParticleRemover_setRemovalDelayCycles(this->particleRemover, stageDefinition->streaming.particleRemovalDelayCycles);

	// apply transformations
	Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
	__VIRTUAL_CALL(Container, initialTransform, this, &environmentTransform, true);

	if(this->uiContainer)
	{
		__VIRTUAL_CALL(Container, initialTransform, this->uiContainer, &environmentTransform, true);
	}
}

// retrieve size
Size Stage_getSize(Stage this)
{
	ASSERT(this, "Stage::getSize: null this");
	ASSERT(this->stageDefinition, "Stage::getSize: null stageDefinition");

	// set world's limits
	return this->stageDefinition->level.size;
}

// setup ui
static void Stage_setupUI(Stage this)
{
	ASSERT(this, "Stage::setupUI: null this");
	ASSERT(!this->uiContainer, "Stage::setupUI: UI already exists");

	if(this->uiContainer)
	{
		__DELETE(this->uiContainer);
		this->uiContainer = NULL;
	}

	if(this->stageDefinition->entities.uiContainerDefinition.allocator)
	{
		// call the appropriate allocator to support inheritance
		this->uiContainer = ((UiContainer (*)(UiContainerDefinition*)) this->stageDefinition->entities.uiContainerDefinition.allocator)(&this->stageDefinition->entities.uiContainerDefinition);
		ASSERT(this->uiContainer, "Stage::setupUI: null ui");

		// setup ui if allocated and constructed
		if(this->uiContainer)
		{
			// apply transformations
			Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
			__VIRTUAL_CALL(Container, initialTransform, this->uiContainer, &environmentTransform, true);
		}
	}
}

// add entity to the stage
Entity Stage_addChildEntity(Stage this, const PositionedEntity* const positionedEntity, bool permanent __attribute__ ((unused)))
{
	ASSERT(this, "Stage::addEntity: null this");

	if(positionedEntity)
	{
		Entity entity = Entity_loadEntity(positionedEntity, this->nextEntityId++);

		if(entity)
		{
			// must initialize after adding the children
			__VIRTUAL_CALL(Entity, initialize, entity, true);

			// create the entity and add it to the world
			Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, entity));

			// apply transformations
			Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
			__VIRTUAL_CALL(Container, initialTransform, entity, &environmentTransform, true);

			__VIRTUAL_CALL(Entity, ready, entity, true);
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

bool Stage_registerEntityId(Stage this, s16 internalId, EntityDefinition* entityDefinition)
{
	ASSERT(this, "Stage::registerEntityId: null this");

	VirtualNode node = this->stageEntities->head;

	for(; node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(entityDefinition == stageEntityDescription->positionedEntity->entityDefinition)
		{
			stageEntityDescription->internalId = internalId;
			return true;
		}
	}

	return false;
}

void Stage_spawnEntity(Stage this, PositionedEntity* positionedEntity, Container requester, EventListener callback)
{
	ASSERT(this, "Stage::spawnEntity: null this");

	EntityFactory_spawnEntity(this->entityFactory, positionedEntity, requester, callback, this->nextEntityId++);
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
	__VIRTUAL_CALL(Container, hide, child);

	Container_removeChild(__SAFE_CAST(Container, this), child);

	s16 internalId = Entity_getInternalId(__SAFE_CAST(Entity, child));

	VirtualNode node = this->stageEntities->head;

	for(; node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(stageEntityDescription->internalId == internalId)
		{
			stageEntityDescription->internalId = -1;
			break;
		}
	}

	if(node)
	{
		if(this->streamingHeadNode == node)
		{
			this->streamingHeadNode = this->streamingHeadNode->previous;
		}

		VirtualList_removeElement(this->stageEntities, node->data);
		VirtualList_removeElement(this->loadedStageEntities, node->data);
		__DELETE_BASIC(node->data);
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

	__VIRTUAL_CALL(Container, hide, child);

	child->deleteMe = true;
	Container_removeChild(__SAFE_CAST(Container, this), child);

	s16 internalId = Entity_getInternalId(__SAFE_CAST(Entity, child));

	VirtualNode node = this->stageEntities->head;

	for(; node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(stageEntityDescription->internalId == internalId)
		{
			stageEntityDescription->internalId = -1;
			break;
		}
	}
}

// preload fonts, charsets and textures
static void Stage_preloadAssets(Stage this)
{
	ASSERT(this, "Stage::preloadAssets: null this");

	// fonts
	Printing_loadFonts(Printing_getInstance(), this->stageDefinition->assets.fontDefinitions);

	// charsets
	if(this->stageDefinition->assets.charSets)
	{
		int i = 0;

		for(; this->stageDefinition->assets.charSets[i]; i++)
		{
			if(__ANIMATED_SINGLE != this->stageDefinition->assets.charSets[i]->allocationType)
			{
				CharSetManager_getCharSet(CharSetManager_getInstance(), this->stageDefinition->assets.charSets[i]);
			}
			else
			{
				ASSERT(this, "Stage::preloadAssets: preloading an animated single char set");
			}
		}
	}

	// textures
	if(this->stageDefinition->assets.stageTextureEntryDefinitions)
	{
		VirtualList managedTextures = __NEW(VirtualList);

		int i = 0;

		for(; this->stageDefinition->assets.stageTextureEntryDefinitions[i].textureDefinition; i++)
		{
			if(__ANIMATED_SINGLE != this->stageDefinition->assets.stageTextureEntryDefinitions[i].textureDefinition->charSetDefinition->allocationType)
			{
				Texture texture = NULL;

				if(this->stageDefinition->assets.stageTextureEntryDefinitions[i].isManaged)
				{
					texture = RecyclableBgmapTextureManager_registerTexture(RecyclableBgmapTextureManager_getInstance(), this->stageDefinition->assets.stageTextureEntryDefinitions[i].textureDefinition);
					VirtualList_pushBack(managedTextures, texture );
				}
				else
				{
					texture = __SAFE_CAST(Texture, BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), this->stageDefinition->assets.stageTextureEntryDefinitions[i].textureDefinition));
				}

				if(texture)
				{
					__VIRTUAL_CALL(Texture, write, texture);
				}
			}
			else
			{
				ASSERT(this, "Stage::preloadAssets: loading an Object texture");
			}
		}

		VirtualNode node = managedTextures->head;

		for(; node; node = node->next)
		{
			RecyclableBgmapTextureManager_removeTexture(RecyclableBgmapTextureManager_getInstance(), __SAFE_CAST(Texture, node->data));
		}

		__DELETE(managedTextures);
	}

	ParamTableManager_calculateParamTableBase(ParamTableManager_getInstance(), this->stageDefinition->rendering.paramTableSegments);
}

// register an entity in the streaming list
static StageEntityDescription* Stage_registerEntity(Stage this __attribute__ ((unused)), PositionedEntity* positionedEntity)
{
	ASSERT(this, "Stage::registerEntities: null this");
	ASSERT(positionedEntity, "Stage::registerEntities: null positionedEntity");

	StageEntityDescription* stageEntityDescription = __NEW_BASIC(StageEntityDescription);

	stageEntityDescription->internalId = -1;
	stageEntityDescription->positionedEntity = positionedEntity;

	VBVec3D environmentPosition3D = {0, 0, 0};
	stageEntityDescription->smallRightCuboid = Entity_getTotalSizeFromDefinition(stageEntityDescription->positionedEntity, &environmentPosition3D);

	return stageEntityDescription;
}

// register the stage's definition entities in the streaming list
static void Stage_registerEntities(Stage this, VirtualList positionedEntitiesToIgnore)
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
	int i = 0;

	for(;this->stageDefinition->entities.children[i].entityDefinition; i++)
	{
		if(positionedEntitiesToIgnore)
		{
			VirtualNode node = positionedEntitiesToIgnore->head;

			for(; node; node = node->next)
			{
				if(&this->stageDefinition->entities.children[i] == (PositionedEntity*)node->data)
				{
					break;
				}
			}

			if(node)
			{
				continue;
			}
		}

		StageEntityDescription* stageEntityDescription = Stage_registerEntity(this, &this->stageDefinition->entities.children[i]);

//		VBVec3D environmentPosition3D = {0, 0, 0};
//		SmallRightCuboid smallRightCuboid = Entity_getTotalSizeFromDefinition(stageEntityDescription->positionedEntity, &environmentPosition3D);

		int x = FIX19_13TOI(stageEntityDescription->positionedEntity->position.x);
		int y = FIX19_13TOI(stageEntityDescription->positionedEntity->position.y);
		int z = FIX19_13TOI(stageEntityDescription->positionedEntity->position.z);

		stageEntityDescription->distance = (x * x + y * y + z * z);

		VirtualNode auxNode = this->stageEntities->head;

		for(; auxNode; auxNode = auxNode->next)
		{
			StageEntityDescription* auxStageEntityDescription = (StageEntityDescription*)auxNode->data;

			if(stageEntityDescription->distance > auxStageEntityDescription->distance)
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

// load all visible entities
static void Stage_loadInitialEntities(Stage this)
{
	ASSERT(this, "Stage::loadInRangeEntities: null this");

	// need a temporal list to remove and delete entities
	VirtualNode node = this->stageEntities->head;

	for(; node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(-1 == stageEntityDescription->internalId)
		{
			// if entity in load range
			if(stageEntityDescription->positionedEntity->loadRegardlessOfPosition || Stage_isEntityInLoadRange(this, stageEntityDescription->positionedEntity->position, &stageEntityDescription->smallRightCuboid))
			{
				Entity entity = Stage_addChildEntity(this, stageEntityDescription->positionedEntity, false);
				ASSERT(entity, "Stage::loadInRangeEntities: entity not loaded");

				if(!stageEntityDescription->positionedEntity->loadRegardlessOfPosition)
				{
					this->streamingHeadNode = node;
				}

				stageEntityDescription->internalId = Entity_getInternalId(entity);

				VirtualList_pushBack(this->loadedStageEntities, stageEntityDescription);
			}
		}
	}
}

// unload non visible entities
static void Stage_unloadOutOfRangeEntities(Stage this, int defer)
{
	ASSERT(this, "Stage::unloadOutOfRangeEntities: null this");

	if(!this->children)
	{
		return;
	}

	// need a temporal list to remove and delete entities
	VirtualNode node = this->children->head;

	// check which actors must be unloaded
	for(; node; node = node->next)
	{
		// get next entity
		Entity entity = __SAFE_CAST(Entity, node->data);

		// if the entity isn't visible inside the view field, unload it
		if(!__VIRTUAL_CALL(Entity, isVisible, entity, (this->stageDefinition->streaming.loadPadding + this->stageDefinition->streaming.unloadPadding + __MAXIMUM_PARALLAX), true))
		{
			s16 internalId = Entity_getInternalId(entity);

			VirtualNode auxNode = this->loadedStageEntities->head;

			for(; auxNode; auxNode = auxNode->next)
			{
				StageEntityDescription* stageEntityDescription = (StageEntityDescription*)auxNode->data;

				if(stageEntityDescription->internalId == internalId)
				{
//					stageEntityDescription->internalId = -1;

					VirtualList_removeElement(this->loadedStageEntities, stageEntityDescription);
					break;
				}
			}

			// unload it
			Stage_unloadChild(this, __SAFE_CAST(Container, entity));

			if(defer)
			{
				break;
			}
		}
	}

#ifdef __PROFILE_STREAMING
		u32 processTime = TimerManager_getMillisecondsElapsed(TimerManager_getInstance()) - timeBeforeProcess;
		unloadOutOfRangeEntitiesHighestTime = processTime > unloadOutOfRangeEntitiesHighestTime ? processTime : unloadOutOfRangeEntitiesHighestTime;
#endif
}

static void Stage_loadInRangeEntities(Stage this, int defer __attribute__ ((unused)))
{
	ASSERT(this, "Stage::selectEntitiesInLoadRange: null this");

	VBVec3D focusInGameEntityPosition = Screen_getPosition(Screen_getInstance());

	if(!this->focusInGameEntity)
	{
		Stage_setFocusEntity(this, Screen_getFocusInGameEntity(Screen_getInstance()));
	}
	else
	{
		focusInGameEntityPosition = *Container_getGlobalPosition(__SAFE_CAST(Container, this->focusInGameEntity));
	}

	focusInGameEntityPosition.x = FIX19_13TOI(focusInGameEntityPosition.x);
	focusInGameEntityPosition.y = FIX19_13TOI(focusInGameEntityPosition.y);
	focusInGameEntityPosition.z = FIX19_13TOI(focusInGameEntityPosition.z);

	long focusInGameEntityDistance = ((long)focusInGameEntityPosition.x * (long)focusInGameEntityPosition.x +
									(long)focusInGameEntityPosition.y * (long)focusInGameEntityPosition.y +
									(long)focusInGameEntityPosition.z * (long)focusInGameEntityPosition.z);

	static int advancing __INITIALIZED_DATA_SECTION_ATTRIBUTE = true;

	if(this->previousFocusEntityDistance != focusInGameEntityDistance)
	{
		advancing = this->previousFocusEntityDistance < focusInGameEntityDistance;
	}

	VirtualNode node = this->streamingHeadNode ? this->streamingHeadNode : advancing? this->stageEntities->head : this->stageEntities->tail;

	int counter = 0;
	int amplitude = this->stageDefinition->streaming.streamingAmplitude;

	this->streamingHeadNode = NULL;

	if(advancing)
	{
		for(; node && counter < amplitude >> 1; node = node->previous, counter++);

		node = node ? node : this->stageEntities->head;

		for(counter = 0; node && (!this->streamingHeadNode || counter < amplitude); node = node->next)
		{
			StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

			if(0 > stageEntityDescription->internalId)
			{
				counter++;

				if(!this->streamingHeadNode)
				{
					if(focusInGameEntityDistance < stageEntityDescription->distance)
					{
						this->streamingHeadNode = node;
					}
				}

				// if entity in load range
				if(Stage_isEntityInLoadRange(this, stageEntityDescription->positionedEntity->position, &stageEntityDescription->smallRightCuboid))
				{
					stageEntityDescription->internalId = this->nextEntityId++;
					VirtualList_pushBack(this->loadedStageEntities, stageEntityDescription);
					EntityFactory_spawnEntity(this->entityFactory, stageEntityDescription->positionedEntity, __SAFE_CAST(Container, this), NULL, stageEntityDescription->internalId);
				}
			}
		}
	}
	else
	{
		for(; node && counter < amplitude >> 1; node = node->next, counter++);

		node = node ? node : this->stageEntities->tail;

		for(counter = 0; node && (!this->streamingHeadNode || counter < amplitude); node = node->previous)
		{
			StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

			if(0 > stageEntityDescription->internalId)
			{
				counter++;

				if(!this->streamingHeadNode)
				{
					if(focusInGameEntityDistance > stageEntityDescription->distance)
					{
						this->streamingHeadNode = node;
					}
				}

				// if entity in load range
				if(Stage_isEntityInLoadRange(this, stageEntityDescription->positionedEntity->position, &stageEntityDescription->smallRightCuboid))
				{
					stageEntityDescription->internalId = this->nextEntityId++;
					VirtualList_pushBack(this->loadedStageEntities, stageEntityDescription);
					EntityFactory_spawnEntity(this->entityFactory, stageEntityDescription->positionedEntity, __SAFE_CAST(Container, this), NULL, stageEntityDescription->internalId);
				}
			}
		}
	}

	this->previousFocusEntityDistance = focusInGameEntityDistance;

#ifdef __PROFILE_STREAMING
		u32 processTime = TimerManager_getMillisecondsElapsed(TimerManager_getInstance()) - timeBeforeProcess;
		loadInRangeEntitiesHighestTime = processTime > loadInRangeEntitiesHighestTime ? processTime : loadInRangeEntitiesHighestTime;
#endif
}

void Stage_stream(Stage this)
{
	ASSERT(this, "Stage::stream: null this");

	// TODO: fix me
	// this check makes that big minimumSpareMilliSecondsToAllowStreaming values
	// inhibit the streaming to kick in
	/*
	if(this->stageDefinition->streaming.minimumSpareMilliSecondsToAllowStreaming + TimerManager_getMillisecondsElapsed(TimerManager_getInstance()) > __GAME_FRAME_DURATION)
	{
		return;
	}
	*/

	u32 alreadyStreamingIn = false;

	if(!this->hasRemovedChildren)
	{
#ifdef __PROFILE_STREAMING
		timeBeforeProcess = TimerManager_getMillisecondsElapsed(TimerManager_getInstance());
#endif
		alreadyStreamingIn = EntityFactory_prepareEntities(this->entityFactory);

#ifdef __PROFILE_STREAMING
		u32 processTime = TimerManager_getMillisecondsElapsed(TimerManager_getInstance()) - timeBeforeProcess;
		entityFactoryHighestTime = processTime > entityFactoryHighestTime ? processTime : entityFactoryHighestTime;
#endif
	}

	if(alreadyStreamingIn)
	{
		return;
	}

	int streamingPhases = sizeof(_streamingPhases) / sizeof(StreamingPhase);

	if(++this->streamingPhase >= streamingPhases)
	{
		this->streamingPhase = 0;
	}

#ifdef __PROFILE_STREAMING
	timeBeforeProcess = TimerManager_getMillisecondsElapsed(TimerManager_getInstance());
#endif

	_streamingPhases[this->streamingPhase](this, true);
}

void Stage_streamAll(Stage this)
{
	ASSERT(this, "Stagge::streamAll: null this");

	// must make sure there are not pending entities for removal
	Container_processRemovedChildren(__SAFE_CAST(Container, this));

	Stage_unloadOutOfRangeEntities(this, false);
	Stage_loadInRangeEntities(this, false);
	EntityFactory_prepareAllEntities(this->entityFactory);
	SpriteManager_sortLayers(SpriteManager_getInstance());
}


// execute stage's logic
void Stage_update(Stage this, u32 elapsedTime)
{
	ASSERT(this, "Stage::update: null this");

	// set now to control the streaming
	this->hasRemovedChildren = this->removedChildren && VirtualList_getSize(this->removedChildren);

	Container_update(__SAFE_CAST(Container, this), elapsedTime);

	if(this->uiContainer)
	{
		Container_update(__SAFE_CAST(Container, this->uiContainer), elapsedTime);
	}

	ParticleRemover_update(this->particleRemover);

#ifdef __SHOW_STREAMING_PROFILING
	Stage_showStreamingProfiling(this, 1, 1);
#endif

}

// transform state
void Stage_transform(Stage this, const Transformation* environmentTransform)
{
	ASSERT(this, "Stage::transform: null this");

	Container_transform(__SAFE_CAST(Container, this), environmentTransform);

	if(this->uiContainer)
	{
		// static to avoid call to _memcpy
		static Transformation uiEnvironmentTransform __INITIALIZED_DATA_SECTION_ATTRIBUTE =
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
				{__1I_FIX7_9, __1I_FIX7_9},
				// global scale
				{__1I_FIX7_9, __1I_FIX7_9}
		};

		uiEnvironmentTransform.globalPosition = (VBVec3D){_screenPosition->x, _screenPosition->y, _screenPosition->z};


		__VIRTUAL_CALL(Container, transform, this->uiContainer, &uiEnvironmentTransform);
	}
}

// retrieve ui
UiContainer Stage_getUiContainer(Stage this)
{
	ASSERT(this, "Stage::getUiContainer: null this");

	return this->uiContainer;
}

// suspend for pause
void Stage_suspend(Stage this)
{
	ASSERT(this, "Stage::suspend: null this");

	// stream all pending entities to avoid having manually recover
	// the stage entity registries
	EntityFactory_prepareAllEntities(this->entityFactory);

	Container_suspend(__SAFE_CAST(Container, this));

	if(this->uiContainer)
	{
		__VIRTUAL_CALL(Container, suspend, __SAFE_CAST(Container, this->uiContainer));
	}

	// relinquish screen focus priority
	if(this->focusInGameEntity && Screen_getFocusInGameEntity(Screen_getInstance()))
	{
		if(this->focusInGameEntity == Screen_getFocusInGameEntity(Screen_getInstance()))
		{
			// relinquish focus entity
			Screen_setFocusInGameEntity(Screen_getInstance(), NULL);
		}
	}
	else
	{
		Stage_setFocusEntity(this, Screen_getFocusInGameEntity(Screen_getInstance()));
	}

	__DELETE(this->entityFactory);
	ParticleRemover_reset(this->particleRemover);
}

// resume after pause
void Stage_resume(Stage this)
{
	ASSERT(this, "Stage::resume: null this");

	// set back optical values
	Screen_setOptical(Screen_getInstance(), this->stageDefinition->rendering.optical);

	// set physics
	PhysicalWorld_setFriction(Game_getPhysicalWorld(Game_getInstance()), this->stageDefinition->physics.friction);
	PhysicalWorld_setGravity(Game_getPhysicalWorld(Game_getInstance()), this->stageDefinition->physics.gravity);

	// set palettes
	Stage_setupPalettes(this);

	// set OBJs' z position
	Stage_setObjectSpritesContainers(this);

	// setup SpriteManager's configuration
	SpriteManager_setCyclesToWaitForTextureWriting(SpriteManager_getInstance(), this->stageDefinition->rendering.cyclesToWaitForTextureWriting);
	SpriteManager_setTexturesMaximumRowsToWrite(SpriteManager_getInstance(), this->stageDefinition->rendering.texturesMaximumRowsToWrite);
	SpriteManager_setMaximumAffineRowsToComputePerCall(SpriteManager_getInstance(), this->stageDefinition->rendering.maximumAffineRowsToComputePerCall);

	// reload textures
	Stage_preloadAssets(this);

	if(this->focusInGameEntity)
	{
		// recover focus entity
		Screen_setFocusInGameEntity(Screen_getInstance(), __SAFE_CAST(InGameEntity, this->focusInGameEntity));
	}

	// load background music
	SoundManager_playBGM(SoundManager_getInstance(), (const u16 (*)[6])this->stageDefinition->assets.bgm);

	Container_resume(__SAFE_CAST(Container, this));

	// apply transformations
	Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
	__VIRTUAL_CALL(Container, initialTransform, this, &environmentTransform, true);

	if(this->uiContainer)
	{
		__VIRTUAL_CALL(Container, resume, __SAFE_CAST(Container, this->uiContainer));

		__VIRTUAL_CALL(Container, initialTransform, this->uiContainer, &environmentTransform, true);
	}

	this->entityFactory = __NEW(EntityFactory);
}

bool Stage_handlePropagatedMessage(Stage this, int message)
{
	ASSERT(this, "Stage::handlePropagatedMessage: null this");

	if(this->uiContainer)
	{
		// propagate message to ui
		return Container_propagateMessage(__SAFE_CAST(Container, this->uiContainer), Container_onPropagatedMessage, message);
	}

	return false;
}

void Stage_onFocusEntityDeleted(Stage this, Object eventFirer __attribute__ ((unused)))
{
	ASSERT(this, "Stage::onFocusEntityDeleted: null this");

	this->focusInGameEntity = NULL;

	if(this->focusInGameEntity && Screen_getFocusInGameEntity(Screen_getInstance()))
	{
		if(this->focusInGameEntity == Screen_getFocusInGameEntity(Screen_getInstance()))
		{
			Screen_setFocusInGameEntity(Screen_getInstance(), NULL);
		}
	}
}

static void Stage_setFocusEntity(Stage this, InGameEntity focusInGameEntity)
{
	ASSERT(this, "Stage::setFocusEntity: null this");

	if(this->focusInGameEntity)
	{
		Object_removeEventListener(__SAFE_CAST(Object, this->focusInGameEntity), __SAFE_CAST(Object, this), (EventListener)Stage_onFocusEntityDeleted, kEventContainerDeleted);
	}

	this->focusInGameEntity = focusInGameEntity;

	if(this->focusInGameEntity)
	{
		Object_addEventListener(__SAFE_CAST(Object, this->focusInGameEntity), __SAFE_CAST(Object, this), (EventListener)Stage_onFocusEntityDeleted, kEventContainerDeleted);

		VBVec3D focusInGameEntityPosition = *Container_getGlobalPosition(__SAFE_CAST(Container, this->focusInGameEntity));
		focusInGameEntityPosition.x = FIX19_13TOI(focusInGameEntityPosition.x);
		focusInGameEntityPosition.y = FIX19_13TOI(focusInGameEntityPosition.y);
		focusInGameEntityPosition.z = FIX19_13TOI(focusInGameEntityPosition.z);

		this->previousFocusEntityDistance = (long)focusInGameEntityPosition.x * (long)focusInGameEntityPosition.x +
											(long)focusInGameEntityPosition.y * (long)focusInGameEntityPosition.y +
											(long)focusInGameEntityPosition.z * (long)focusInGameEntityPosition.z;
	}
}

// get stage definition
StageDefinition* Stage_getStageDefinition(Stage this)
{
	ASSERT(this, "Stage::getStageDefinition: null this");

	return this->stageDefinition;
}

ParticleRemover Stage_getParticleRemover(Stage this)
{
	ASSERT(this, "Stage::getParticleRemover: null this");

	return this->particleRemover;
}

void Stage_showStreamingProfiling(Stage this __attribute__ ((unused)), int x, int y)
{
	ASSERT(this, "Stage::showStreamingProfiling: null this");

	Printing_text(Printing_getInstance(), "STREAMING'S STATUS", x, y++, NULL);

	Printing_text(Printing_getInstance(), "Stage's status", x, ++y, NULL);

	int originalY __attribute__ ((unused)) = y;
	int xDisplacement = 18;
	y++;

	Printing_text(Printing_getInstance(), "Regist. entities: ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->stageEntities), x + xDisplacement, y++, NULL);
	Printing_text(Printing_getInstance(), "Loaded entities: ", x, y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->loadedStageEntities), x + xDisplacement, y++, NULL);

#ifdef __PROFILE_STREAMING
	Printing_text(Printing_getInstance(), "Process duration (ms):", x, ++y, NULL);

	Printing_text(Printing_getInstance(), "Unload:			   ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), unloadOutOfRangeEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing_text(Printing_getInstance(), "Load:				 ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), loadInRangeEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing_text(Printing_getInstance(), "Factory:			  ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), entityFactoryHighestTime, x + xDisplacement, y++, NULL);

	unloadOutOfRangeEntitiesHighestTime = 0;
	loadInRangeEntitiesHighestTime = 0;
	entityFactoryHighestTime = 0;

	++y;
	EntityFactory_showStatus(this->entityFactory, x + 24, originalY);

#endif
}
