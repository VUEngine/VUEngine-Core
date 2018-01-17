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
#include <Camera.h>
#include <HardwareManager.h>
#include <SpriteManager.h>
#include <BgmapTextureManager.h>
#include <CharSetManager.h>
#include <BgmapTexture.h>
#include <ParamTableManager.h>
#include <VIPManager.h>
#include <ParticleRemover.h>
#include <MessageDispatcher.h>
#include <debugConfig.h>
#ifdef __PROFILE_STREAMING
#include <TimerManager.h>
#endif


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __STREAMING_CYCLES		5

#define __MAXIMUM_PARALLAX		10
#define __LOAD_LOW_X_LIMIT		(-__MAXIMUM_PARALLAX - this->stageDefinition->streaming.loadPadding)
#define __LOAD_HIGHT_X_LIMIT	(__SCREEN_WIDTH + __MAXIMUM_PARALLAX + this->stageDefinition->streaming.loadPadding)
#define __LOAD_LOW_Y_LIMIT		(-this->stageDefinition->streaming.loadPadding)
#define __LOAD_HIGHT_Y_LIMIT	(__SCREEN_HEIGHT + this->stageDefinition->streaming.loadPadding)
#define __LOAD_LOW_Z_LIMIT		(-this->stageDefinition->streaming.loadPadding)
#define __LOAD_HIGHT_Z_LIMIT	(__SCREEN_DEPTH + this->stageDefinition->streaming.loadPadding)


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Stage
 * @extends Container
 * @ingroup stage
 */
__CLASS_DEFINITION(Stage, Container);
__CLASS_FRIEND_DEFINITION(Container);
__CLASS_FRIEND_DEFINITION(Entity);
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
	PixelRightBox pixelRightBox;
	s16 internalId;
	long distance;

} StageEntityDescription;

const Transformation neutralEnvironmentTransformation =
{
	// spatial local position
	{0, 0, 0},

	// spatial global position
	{0, 0, 0},

	// local rotation
	{0, 0, 0},

	// global rotation
	{0, 0, 0},

	// scale
	{__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9},

	// scale
	{__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9},

};


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
BgmapTexture BgmapTextureManager_loadTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition, int isPreload);

static void Stage_constructor(Stage this);
static void Stage_setupUI(Stage this);
static StageEntityDescription* Stage_registerEntity(Stage this, PositionedEntity* positionedEntity);
static void Stage_registerEntities(Stage this, VirtualList positionedEntitiesToIgnore);
static void Stage_setObjectSpritesContainers(Stage this);
static void Stage_preloadAssets(Stage this);
static void Stage_unloadChild(Stage this, Container child);
static void Stage_setFocusEntity(Stage this, Entity focusEntity);
static void Stage_loadInitialEntities(Stage this);
static bool Stage_unloadOutOfRangeEntities(Stage this, int defer);
static bool Stage_loadInRangeEntities(Stage this, int defer);
static bool Stage_processRemovedChildrenProgressively(Stage this);
static bool Stage_updateEntityFactory(Stage this);
static Entity Stage_doAddChildEntity(Stage this, const PositionedEntity* const positionedEntity, bool permanent __attribute__ ((unused)), s16 internalId);

#ifdef __PROFILE_STREAMING
extern s16 _renderingProcessTimeHelper;
void EntityFactory_showStatus(EntityFactory this __attribute__ ((unused)), int x, int y);
#endif

typedef bool (*StreamingPhase)(Stage, int);

static const StreamingPhase _streamingPhases[] =
{
	&Stage_unloadOutOfRangeEntities,
	&Stage_loadInRangeEntities
};

#ifdef __PROFILE_STREAMING
static u32 unloadOutOfRangeEntitiesHighestTime = 0;
static u32 loadInRangeEntitiesHighestTime = 0;
static u32 processRemovedEntitiesHighestTime = 0;
static u32 entityFactoryHighestTime = 0;
static u32 timeBeforeProcess = 0;
#endif

bool reset = false;

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
	this->children = __NEW(VirtualList);

	this->stageEntities = NULL;
	this->loadedStageEntities = NULL;
	this->uiContainer = NULL;
	this->stageDefinition = NULL;
	this->focusEntity = NULL;
	this->streamingHeadNode = NULL;
	this->cameraPreviousDistance = 0;
	this->nextEntityId = 0;
	this->streamingPhase = 0;
	this->streamingCycleCounter = 0;
}

// class's destructor
void Stage_destructor(Stage this)
{
	ASSERT(this, "Stage::destructor: null this");

	__DELETE(this->particleRemover);
	this->particleRemover = NULL;

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

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// determine if a point is visible
static int Stage_isEntityInLoadRange(Stage this, ScreenPixelVector onScreenPosition, const PixelRightBox* pixelRightBox)
{
	ASSERT(this, "Stage::isEntityInLoadRange: null this");

	PixelVector position =
	{
		onScreenPosition.x,
		onScreenPosition.y,
		onScreenPosition.z,
		0
	};

	position.x -= __METERS_TO_PIXELS(_cameraPosition->x);
	position.y -= __METERS_TO_PIXELS(_cameraPosition->y);
	position.z -= __METERS_TO_PIXELS(_cameraPosition->z);

	// check x visibility
	if(position.x + pixelRightBox->x1 <  __LOAD_LOW_X_LIMIT || position.x + pixelRightBox->x0 >  __LOAD_HIGHT_X_LIMIT)
	{
		return false;
	}

	// check y visibility
	if(position.y + pixelRightBox->y1 <  __LOAD_LOW_Y_LIMIT || position.y + pixelRightBox->y0 >  __LOAD_HIGHT_Y_LIMIT)
	{
		return false;
	}

	// check z visibility
	if(position.z + pixelRightBox->z1 <  __LOAD_LOW_Z_LIMIT || position.z + pixelRightBox->z0 >  __LOAD_HIGHT_Z_LIMIT)
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
void Stage_load(Stage this, StageDefinition* stageDefinition, VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition)
{
	ASSERT(this, "Stage::load: null this");

	// set world's definition
	this->stageDefinition = stageDefinition;

	// set optical values
	Camera_setOptical(Camera_getInstance(), Optical_getFromPixelOptical(this->stageDefinition->rendering.pixelOptical));

	// stop all sounds
	SoundManager_stopAllSound(SoundManager_getInstance());

	// set world's limits
	Camera_setStageSize(Camera_getInstance(), Size_getFromPixelSize(stageDefinition->level.pixelSize));

	if(overrideCameraPosition)
	{
		Camera_setPosition(Camera_getInstance(), Vector3D_getFromPixelVector(stageDefinition->level.cameraInitialPosition));
	}

	// set palettes
	Stage_setupPalettes(this);

	// setup OBJs
	Stage_setObjectSpritesContainers(this);

	// setup SpriteManager's configuration
	SpriteManager_setCyclesToWaitForTextureWriting(SpriteManager_getInstance(), this->stageDefinition->rendering.cyclesToWaitForTextureWriting);
	SpriteManager_setTexturesMaximumRowsToWrite(SpriteManager_getInstance(), this->stageDefinition->rendering.texturesMaximumRowsToWrite);
	SpriteManager_setMaximumParamTableRowsToComputePerCall(SpriteManager_getInstance(), this->stageDefinition->rendering.maximumAffineRowsToComputePerCall);

	// preload textures
	Stage_preloadAssets(this);

	// setup ui
	Stage_setupUI(this);

	// register all the entities in the stage's definition
	Stage_registerEntities(this, positionedEntitiesToIgnore);

	// load entities
	Stage_loadInitialEntities(this);

	// retrieve focus entity for streaming
	Stage_setFocusEntity(this, Camera_getFocusEntity(Camera_getInstance()));

	// set physics
	PhysicalWorld_setFrictionCoefficient(Game_getPhysicalWorld(Game_getInstance()), stageDefinition->physics.frictionCoefficient);
	PhysicalWorld_setGravity(Game_getPhysicalWorld(Game_getInstance()), stageDefinition->physics.gravity);

	// load background music
	SoundManager_playBGM(SoundManager_getInstance(), (const u16 (*)[6])stageDefinition->assets.bgm);

	// setup colors and brightness
	VIPManager_setBackgroundColor(VIPManager_getInstance(), stageDefinition->rendering.colorConfig.backgroundColor);
	VIPManager_setupBrightnessRepeat(VIPManager_getInstance(), stageDefinition->rendering.colorConfig.brightnessRepeat);

	// set particle removal delay
	ParticleRemover_setRemovalDelayCycles(this->particleRemover, stageDefinition->streaming.particleRemovalDelayCycles);

	// apply transformations
	__VIRTUAL_CALL(Container, initialTransform, this, &neutralEnvironmentTransformation, true);

	if(this->uiContainer)
	{
		__VIRTUAL_CALL(Container, initialTransform, this->uiContainer, &neutralEnvironmentTransformation, true);
	}
}

void Stage_loadPostProcessingEffects(Stage this)
{
	ASSERT(this, "Stage::loadPostProcessingEffects: null this");

	if(this->stageDefinition->postProcessingEffects)
	{
		int i = 0;
		for(; this->stageDefinition->postProcessingEffects[i]; i++)
		{
			Game_pushFrontProcessingEffect(Game_getInstance(), this->stageDefinition->postProcessingEffects[i], NULL);
		}
	}
}

// retrieve size
Size Stage_getSize(Stage this)
{
	ASSERT(this, "Stage::getSize: null this");
	ASSERT(this->stageDefinition, "Stage::getSize: null stageDefinition");

	// set world's limits
	return Size_getFromPixelSize(this->stageDefinition->level.pixelSize);
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
			__VIRTUAL_CALL(Container, initialTransform, this->uiContainer, &neutralEnvironmentTransformation, true);
		}
	}
}

// add entity to the stage
Entity Stage_addChildEntity(Stage this, const PositionedEntity* const positionedEntity, bool permanent)
{
	ASSERT(this, "Stage::addEntity: null this");

	return Stage_doAddChildEntity(this, positionedEntity, permanent, this->nextEntityId++);
}

// add entity to the stage
static Entity Stage_doAddChildEntity(Stage this, const PositionedEntity* const positionedEntity, bool permanent __attribute__ ((unused)), s16 internalId)
{
	ASSERT(this, "Stage::addEntity: null this");

	if(positionedEntity)
	{
		Entity entity = Entity_loadEntity(positionedEntity, internalId);

		if(entity)
		{
			// must add graphics
			__VIRTUAL_CALL(Container, setupGraphics, entity);
			__VIRTUAL_CALL(Entity, initialize, entity, true);

			// create the entity and add it to the world
			Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, entity));

			// apply transformations
			__VIRTUAL_CALL(Container, initialTransform, entity, &neutralEnvironmentTransformation, true);

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

	__CALL_BASE_METHOD(Container, removeChild, this, child);

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

	child->deleteMe = true;
	Container_removeChild(__SAFE_CAST(Container, this), child);
	__VIRTUAL_CALL(Container, releaseGraphics, child);
	Object_fireEvent(__SAFE_CAST(Object, child), kStageChildStreamedOut);
	Object_removeAllEventListeners(__SAFE_CAST(Object, child), kStageChildStreamedOut);
	MessageDispatcher_discardAllDelayedMessagesFromSender(MessageDispatcher_getInstance(), __SAFE_CAST(Object, child));
	MessageDispatcher_discardAllDelayedMessagesForReceiver(MessageDispatcher_getInstance(), __SAFE_CAST(Object, child));

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
	if(this->stageDefinition->assets.charSetDefinitions)
	{
		int i = 0;

		for(; this->stageDefinition->assets.charSetDefinitions[i]; i++)
		{
			if(__ANIMATED_SINGLE != this->stageDefinition->assets.charSetDefinitions[i]->allocationType &&
				__ANIMATED_SINGLE_OPTIMIZED != this->stageDefinition->assets.charSetDefinitions[i]->allocationType)
			{
				CharSetManager_getCharSet(CharSetManager_getInstance(), this->stageDefinition->assets.charSetDefinitions[i]);
			}
			else
			{
				ASSERT(this, "Stage::preloadAssets: preloading an animated single char set");
			}
		}
	}

	// textures
	if(this->stageDefinition->assets.textureDefinitions)
	{
		VirtualList recyclableTextures = __NEW(VirtualList);
		int i = 0;

		for(; this->stageDefinition->assets.textureDefinitions[i]; i++)
		{
			if(__ANIMATED_SINGLE != this->stageDefinition->assets.textureDefinitions[i]->charSetDefinition->allocationType &&
				__ANIMATED_SINGLE_OPTIMIZED != this->stageDefinition->assets.textureDefinitions[i]->charSetDefinition->allocationType)
			{
				BgmapTexture bgmapTexture = BgmapTextureManager_getTexture(BgmapTextureManager_getInstance(), this->stageDefinition->assets.textureDefinitions[i]);

				if(bgmapTexture)
				{
					__VIRTUAL_CALL(Texture, write, bgmapTexture);

					if(this->stageDefinition->assets.textureDefinitions[i]->recyclable)
					{
						VirtualList_pushBack(recyclableTextures, bgmapTexture);
					}
				}
			}
			else
			{
				ASSERT(this, "Stage::preloadAssets: loading an Object texture");
			}
		}

		VirtualNode node = VirtualList_begin(recyclableTextures);

		for(;node; node = node->next)
		{
			BgmapTextureManager_releaseTexture(BgmapTextureManager_getInstance(), __SAFE_CAST(BgmapTexture, node->data));
		}

		__DELETE(recyclableTextures);
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

	PixelVector environmentPosition = {0, 0, 0, 0};
	stageEntityDescription->pixelRightBox = Entity_getTotalSizeFromDefinition(stageEntityDescription->positionedEntity, &environmentPosition);

	int x = stageEntityDescription->positionedEntity->onScreenPosition.x;
	int y = stageEntityDescription->positionedEntity->onScreenPosition.y;
	int z = stageEntityDescription->positionedEntity->onScreenPosition.z;

	stageEntityDescription->distance = (x * x + y * y + z * z);

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
			if(stageEntityDescription->positionedEntity->loadRegardlessOfPosition || Stage_isEntityInLoadRange(this, stageEntityDescription->positionedEntity->onScreenPosition, &stageEntityDescription->pixelRightBox))
			{
				stageEntityDescription->internalId = this->nextEntityId++;
				Entity entity = Stage_doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId);
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
static bool Stage_unloadOutOfRangeEntities(Stage this, int defer)
{
	ASSERT(this, "Stage::unloadOutOfRangeEntities: null this");

	if(!this->children)
	{
		return false;
	}

#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager_getMillisecondsElapsed(TimerManager_getInstance());
#endif

	// need a temporal list to remove and delete entities
	VirtualNode node = this->children->head;

	// check which actors must be unloaded
	for(; node; node = node->next)
	{
		// get next entity
		Entity entity = __SAFE_CAST(Entity, node->data);

		// if the entity isn't visible inside the view field, unload it
		if(entity->parent == __SAFE_CAST(Container, this) && !__VIRTUAL_CALL(Entity, isVisible, entity, (this->stageDefinition->streaming.loadPadding + this->stageDefinition->streaming.unloadPadding + __MAXIMUM_PARALLAX), true))
		{
			s16 internalId = Entity_getInternalId(entity);

			VirtualNode auxNode = this->loadedStageEntities->head;
			StageEntityDescription* stageEntityDescription = NULL;

			for(; auxNode; auxNode = auxNode->next)
			{
				stageEntityDescription = (StageEntityDescription*)auxNode->data;

				if(stageEntityDescription->internalId == internalId)
				{
//					stageEntityDescription->internalId = -1;

					break;
				}
			}

			bool unloaded = false;

			if(stageEntityDescription)
			{
				if(!stageEntityDescription->positionedEntity->loadRegardlessOfPosition)
				{
					// unload it
					Stage_unloadChild(this, __SAFE_CAST(Container, entity));
					VirtualList_removeElement(this->loadedStageEntities, stageEntityDescription);

					unloaded = true;
				}
			}
			else
			{
				// unload it
				Stage_unloadChild(this, __SAFE_CAST(Container, entity));

				unloaded = true;
			}


			if(unloaded && defer)
			{
#ifdef __PROFILE_STREAMING
				u32 processTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(TimerManager_getInstance()) - timeBeforeProcess;
				unloadOutOfRangeEntitiesHighestTime = processTime > unloadOutOfRangeEntitiesHighestTime ? processTime : unloadOutOfRangeEntitiesHighestTime;
#endif
				return true;
			}
		}
	}

#ifdef __PROFILE_STREAMING
		u32 processTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(TimerManager_getInstance()) - timeBeforeProcess;
		unloadOutOfRangeEntitiesHighestTime = processTime > unloadOutOfRangeEntitiesHighestTime ? processTime : unloadOutOfRangeEntitiesHighestTime;
#endif

	return true;
}

static bool Stage_loadInRangeEntities(Stage this, int defer __attribute__ ((unused)))
{
	ASSERT(this, "Stage::selectEntitiesInLoadRange: null this");

#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager_getMillisecondsElapsed(TimerManager_getInstance());
#endif

	bool loadedEntities = false;
	int xCameraPosition = __METERS_TO_PIXELS(_cameraPosition->x) + (__HALF_SCREEN_WIDTH);
	int yCameraPosition = __METERS_TO_PIXELS(_cameraPosition->y) + (__HALF_SCREEN_HEIGHT);
	int zCameraPosition = __METERS_TO_PIXELS(_cameraPosition->z) + (__HALF_SCREEN_DEPTH);

	long cameraDistance = ((long)xCameraPosition * (long)xCameraPosition +
							(long)yCameraPosition * (long)yCameraPosition +
							(long)zCameraPosition * (long)zCameraPosition);

	static int advancing __INITIALIZED_DATA_SECTION_ATTRIBUTE = true;
	u16 amplitude = this->stageDefinition->streaming.streamingAmplitude;

	if(this->cameraPreviousDistance != cameraDistance)
	{
		advancing = this->cameraPreviousDistance < cameraDistance;
	}

	if(!defer)
	{
		this->streamingHeadNode = this->stageEntities->head;
		advancing = true;
		amplitude = 10000;
	}

	VirtualNode node = this->streamingHeadNode ? this->streamingHeadNode : advancing? this->stageEntities->head : this->stageEntities->tail;

	int counter = 0;

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
					if(cameraDistance < stageEntityDescription->distance)
					{
						this->streamingHeadNode = node;
					}
				}

				// if entity in load range
				if(Stage_isEntityInLoadRange(this, stageEntityDescription->positionedEntity->onScreenPosition, &stageEntityDescription->pixelRightBox))
				{
					loadedEntities = true;

					stageEntityDescription->internalId = this->nextEntityId++;
					VirtualList_pushBack(this->loadedStageEntities, stageEntityDescription);

					if(defer)
					{
						EntityFactory_spawnEntity(this->entityFactory, stageEntityDescription->positionedEntity, __SAFE_CAST(Container, this), NULL, stageEntityDescription->internalId);
					}
					else
					{
						Stage_doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId);
					}
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
					if(cameraDistance > stageEntityDescription->distance)
					{
						this->streamingHeadNode = node;
					}
				}

				// if entity in load range
				if(Stage_isEntityInLoadRange(this, stageEntityDescription->positionedEntity->onScreenPosition, &stageEntityDescription->pixelRightBox))
				{
					loadedEntities = true;

					stageEntityDescription->internalId = this->nextEntityId++;
					VirtualList_pushBack(this->loadedStageEntities, stageEntityDescription);

					if(defer)
					{
						EntityFactory_spawnEntity(this->entityFactory, stageEntityDescription->positionedEntity, __SAFE_CAST(Container, this), NULL, stageEntityDescription->internalId);
					}
					else
					{
						Stage_doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId);
					}
				}
			}
		}
	}

	this->cameraPreviousDistance = cameraDistance;

#ifdef __PROFILE_STREAMING
	u32 processTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(TimerManager_getInstance()) - timeBeforeProcess;
	loadInRangeEntitiesHighestTime = processTime > loadInRangeEntitiesHighestTime ? processTime : loadInRangeEntitiesHighestTime;
#endif

	return loadedEntities;
}

// process removed children
static bool Stage_processRemovedChildrenProgressively(Stage this)
{
	ASSERT(this, "Stage::processRemovedChildrenProgressively: null this");

	if(!this->removedChildren || !this->removedChildren->head)
	{
		return false;
	}

#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager_getMillisecondsElapsed(TimerManager_getInstance());
#endif

	Container child = __SAFE_CAST(Container, VirtualList_front(this->removedChildren));

	VirtualList_popFront(this->removedChildren);
	VirtualList_removeElement(this->children, child);

	if(__IS_OBJECT_ALIVE(child))
	{
		if(child->deleteMe)
		{
			__DELETE(child);

#ifdef __PROFILE_STREAMING
			u32 processTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(TimerManager_getInstance()) - timeBeforeProcess;
			processRemovedEntitiesHighestTime = processTime > processRemovedEntitiesHighestTime ? processTime : processRemovedEntitiesHighestTime;
#endif
			return true;
		}
	}

#ifdef __PROFILE_STREAMING
	u32 processTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(TimerManager_getInstance()) - timeBeforeProcess;
	processRemovedEntitiesHighestTime = processTime > processRemovedEntitiesHighestTime ? processTime : processRemovedEntitiesHighestTime;
#endif

	return false;
}

//
static bool Stage_updateEntityFactory(Stage this)
{
	ASSERT(this, "Stage::updateEntityFactory: null this");

#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager_getMillisecondsElapsed(TimerManager_getInstance());
#endif

	bool preparingEntities = EntityFactory_prepareEntities(this->entityFactory);

#ifdef __PROFILE_STREAMING
	u32 processTime = -_renderingProcessTimeHelper + TimerManager_getMillisecondsElapsed(TimerManager_getInstance()) - timeBeforeProcess;
	entityFactoryHighestTime = processTime > entityFactoryHighestTime ? processTime : entityFactoryHighestTime;
#endif

	return preparingEntities;
}

bool Stage_stream(Stage this)
{
	ASSERT(this, "Stage::stream: null this");

#ifdef __SHOW_STREAMING_PROFILING
	if(!Game_isInSpecialMode(Game_getInstance()))
	{
		EntityFactory_showStatus(this->entityFactory, 25, 3);
	}
#endif

	if(Stage_processRemovedChildrenProgressively(this))
	{
		return true;
	}

	if(Stage_updateEntityFactory(this))
	{
		return false;
	}

	int streamingPhases = sizeof(_streamingPhases) / sizeof(StreamingPhase);

	if(++this->streamingPhase >= streamingPhases)
	{
		this->streamingPhase = 0;
	}

	return _streamingPhases[this->streamingPhase](this, true);
}

void Stage_streamAll(Stage this)
{
	ASSERT(this, "Stage::streamAll: null this");

	// must make sure there are not pending entities for removal
	Container_processRemovedChildren(__SAFE_CAST(Container, this));

	Stage_unloadOutOfRangeEntities(this, false);
	SpriteManager_disposeSprites(SpriteManager_getInstance());
	Container_processRemovedChildren(__SAFE_CAST(Container, this));
	Stage_loadInRangeEntities(this, false);
	EntityFactory_prepareAllEntities(this->entityFactory);
	SpriteManager_writeTextures(SpriteManager_getInstance());
	SpriteManager_sortLayers(SpriteManager_getInstance());
}

// execute stage's logic
void Stage_update(Stage this, u32 elapsedTime)
{
	ASSERT(this, "Stage::update: null this");

	VirtualNode node = this->children->head;

	for(; node ; node = node->next)
	{
		Container child = __SAFE_CAST(Container, node->data);

		if(child->parent != __SAFE_CAST(Container, this))
		{
			continue;
		}

		__VIRTUAL_CALL(Container, update, child, elapsedTime);
	}

	if(this->uiContainer)
	{
		Container_update(__SAFE_CAST(Container, this->uiContainer), elapsedTime);
	}

	ParticleRemover_update(this->particleRemover);
}

// transformation state
void Stage_transform(Stage this, const Transformation* environmentTransform __attribute__ ((unused)), u8 invalidateTransformationFlag)
{
	ASSERT(this, "Stage::transform: null this");

	VirtualNode node = this->children->head;

	for(; node; node = node->next)
	{
		Container child = __SAFE_CAST(Container, node->data);

		if(child->parent != __SAFE_CAST(Container, this))
		{
			continue;
		}

		__VIRTUAL_CALL(Container, transform, child, environmentTransform, invalidateTransformationFlag);
	}

	if(this->uiContainer)
	{
		__VIRTUAL_CALL(Container, transform, this->uiContainer, environmentTransform, invalidateTransformationFlag);
	}
}

void Stage_synchronizeGraphics(Stage this)
{
	ASSERT(this, "Stage::synchronizeGraphics: null this");

	VirtualNode node = this->children->head;

	for(; node; node = node->next)
	{
		Container child = __SAFE_CAST(Container, node->data);

		if(child->parent != __SAFE_CAST(Container, this))
		{
			continue;
		}

		__VIRTUAL_CALL(Container, synchronizeGraphics, child);
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

	__CALL_BASE_METHOD(Container, suspend, this);

	if(this->uiContainer)
	{
		__VIRTUAL_CALL(Container, suspend, __SAFE_CAST(Container, this->uiContainer));
	}

	// relinquish camera focus priority
	if(this->focusEntity && Camera_getFocusEntity(Camera_getInstance()))
	{
		if(this->focusEntity == Camera_getFocusEntity(Camera_getInstance()))
		{
			// relinquish focus entity
			Camera_setFocusGameEntity(Camera_getInstance(), NULL);
		}
	}
	else
	{
		Stage_setFocusEntity(this, Camera_getFocusEntity(Camera_getInstance()));
	}

	__DELETE(this->entityFactory);
	ParticleRemover_reset(this->particleRemover);
}

// resume after pause
void Stage_resume(Stage this)
{
	ASSERT(this, "Stage::resume: null this");

	// set back optical values
	Camera_setOptical(Camera_getInstance(), Optical_getFromPixelOptical(this->stageDefinition->rendering.pixelOptical));

	// set physics
	PhysicalWorld_setFrictionCoefficient(Game_getPhysicalWorld(Game_getInstance()), this->stageDefinition->physics.frictionCoefficient);
	PhysicalWorld_setGravity(Game_getPhysicalWorld(Game_getInstance()), this->stageDefinition->physics.gravity);

	// set palettes
	Stage_setupPalettes(this);

	// set OBJs' z position
	Stage_setObjectSpritesContainers(this);

	// setup SpriteManager's configuration
	SpriteManager_setCyclesToWaitForTextureWriting(SpriteManager_getInstance(), this->stageDefinition->rendering.cyclesToWaitForTextureWriting);
	SpriteManager_setTexturesMaximumRowsToWrite(SpriteManager_getInstance(), this->stageDefinition->rendering.texturesMaximumRowsToWrite);
	SpriteManager_setMaximumParamTableRowsToComputePerCall(SpriteManager_getInstance(), this->stageDefinition->rendering.maximumAffineRowsToComputePerCall);

	// reload textures
	Stage_preloadAssets(this);

	if(this->focusEntity)
	{
		// recover focus entity
		Camera_setFocusGameEntity(Camera_getInstance(), __SAFE_CAST(Entity, this->focusEntity));
	}

	// load background music
	SoundManager_playBGM(SoundManager_getInstance(), (const u16 (*)[6])this->stageDefinition->assets.bgm);

	__CALL_BASE_METHOD(Container, resume, this);

	// apply transformations
	__VIRTUAL_CALL(Container, initialTransform, this, &neutralEnvironmentTransformation, true);

	if(this->uiContainer)
	{
		__VIRTUAL_CALL(Container, resume, __SAFE_CAST(Container, this->uiContainer));

		__VIRTUAL_CALL(Container, initialTransform, this->uiContainer, &neutralEnvironmentTransformation, true);
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

	this->focusEntity = NULL;

	if(this->focusEntity && Camera_getFocusEntity(Camera_getInstance()))
	{
		if(this->focusEntity == Camera_getFocusEntity(Camera_getInstance()))
		{
			Camera_setFocusGameEntity(Camera_getInstance(), NULL);
		}
	}
}

static void Stage_setFocusEntity(Stage this, Entity focusEntity)
{
	ASSERT(this, "Stage::setFocusEntity: null this");

	if(this->focusEntity)
	{
		Object_removeEventListener(__SAFE_CAST(Object, this->focusEntity), __SAFE_CAST(Object, this), (EventListener)Stage_onFocusEntityDeleted, kEventContainerDeleted);
	}

	this->focusEntity = focusEntity;

	if(this->focusEntity)
	{
		Object_addEventListener(__SAFE_CAST(Object, this->focusEntity), __SAFE_CAST(Object, this), (EventListener)Stage_onFocusEntityDeleted, kEventContainerDeleted);

		Vector3D focusEntityPosition = *Container_getGlobalPosition(__SAFE_CAST(Container, this->focusEntity));
		focusEntityPosition.x = __METERS_TO_PIXELS(focusEntityPosition.x);
		focusEntityPosition.y = __METERS_TO_PIXELS(focusEntityPosition.y);
		focusEntityPosition.z = __METERS_TO_PIXELS(focusEntityPosition.z);

		this->cameraPreviousDistance = (long)focusEntityPosition.x * (long)focusEntityPosition.x +
											(long)focusEntityPosition.y * (long)focusEntityPosition.y +
											(long)focusEntityPosition.z * (long)focusEntityPosition.z;
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

	Printing_text(Printing_getInstance(), "Regist. entities:      ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->stageEntities), x + xDisplacement, y++, NULL);
	Printing_text(Printing_getInstance(), "Loaded entities:       ", x, y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->loadedStageEntities), x + xDisplacement, y++, NULL);
	Printing_text(Printing_getInstance(), "Children entities:       ", x, y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->children), x + xDisplacement, y++, NULL);

#ifdef __PROFILE_STREAMING

	xDisplacement = 12;

	Printing_text(Printing_getInstance(), "Process duration (ms):", x, ++y, NULL);

	Printing_text(Printing_getInstance(), "Unload:           ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), unloadOutOfRangeEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing_text(Printing_getInstance(), "Load:             ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), loadInRangeEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing_text(Printing_getInstance(), "Removing:         ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), processRemovedEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing_text(Printing_getInstance(), "Factory:          ", x, ++y, NULL);
	Printing_int(Printing_getInstance(), entityFactoryHighestTime, x + xDisplacement, y++, NULL);

	unloadOutOfRangeEntitiesHighestTime = 0;
	loadInRangeEntitiesHighestTime = 0;
	processRemovedEntitiesHighestTime = 0;
	entityFactoryHighestTime = 0;
#endif
}
