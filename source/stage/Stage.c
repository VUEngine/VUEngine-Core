/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

friend class Container;
friend class Entity;
friend class VirtualNode;
friend class VirtualList;

/**
 * Stage Entity Description
 *
 * @memberof Stage
 */
typedef struct StageEntityDescription
{
	PixelRightBox pixelRightBox;
	PositionedEntity* positionedEntity;
	u32 distance;
	s16 internalId;

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
BgmapTexture BgmapTextureManager::loadTexture(BgmapTextureManager this, BgmapTextureDefinition* bgmapTextureDefinition, int isPreload);

static void Stage::setupUI(Stage this);
static StageEntityDescription* Stage::registerEntity(Stage this, PositionedEntity* positionedEntity);
static void Stage::registerEntities(Stage this, VirtualList positionedEntitiesToIgnore);
static void Stage::setObjectSpritesContainers(Stage this);
static void Stage::preloadAssets(Stage this);
static void Stage::unloadChild(Stage this, Container child);
static void Stage::setFocusEntity(Stage this, Entity focusEntity);
static void Stage::loadInitialEntities(Stage this);
static bool Stage::unloadOutOfRangeEntities(Stage this, int defer);
static bool Stage::loadInRangeEntities(Stage this, int defer);
static bool Stage::purgeChildrenProgressively(Stage this);
static bool Stage::updateEntityFactory(Stage this);
static Entity Stage::doAddChildEntity(Stage this, const PositionedEntity* const positionedEntity, bool permanent __attribute__ ((unused)), s16 internalId, bool makeReady);
static void Stage::makeChildReady(Stage this, Entity entity);

#ifdef __PROFILE_STREAMING
extern s16 _renderingProcessTimeHelper;
void EntityFactory::showStatus(EntityFactory this __attribute__ ((unused)), int x, int y);
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


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Stage::constructor(Stage this, StageDefinition *stageDefinition)
{
	ASSERT(this, "Stage::constructor: null this");

	// construct base object
	Base::constructor(NULL);

	this->entityFactory = __NEW(EntityFactory);
	this->particleRemover = __NEW(ParticleRemover);
	this->children = __NEW(VirtualList);

	this->stageDefinition = stageDefinition;
	this->stageEntities = NULL;
	this->loadedStageEntities = NULL;
	this->uiContainer = NULL;
	this->focusEntity = NULL;
	this->streamingHeadNode = NULL;
	this->cameraPreviousDistance = 0;
	this->nextEntityId = 0;
	this->streamingPhase = 0;
	this->streamingCycleCounter = 0;
}

// class's destructor
void Stage::destructor(Stage this)
{
	ASSERT(this, "Stage::destructor: null this");

	Stage::setFocusEntity(this, NULL);

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
	Base::destructor();
}

// determine if a point is visible
static int Stage::isEntityInLoadRange(Stage this, ScreenPixelVector onScreenPosition, const PixelRightBox* pixelRightBox)
{
	ASSERT(this, "Stage::isEntityInLoadRange: null this");

	Vector3D spatialPosition = Vector3D::getFromScreenPixelVector(onScreenPosition);

	PixelVector position = Vector3D::projectToPixelVector(Vector3D::getRelativeToCamera(spatialPosition), 0);

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

static void Stage::setObjectSpritesContainers(Stage this)
{
	ASSERT(this, "Stage::setObjectSpritesContainers: null this");

	SpriteManager::setupObjectSpriteContainers(SpriteManager::getInstance(), this->stageDefinition->rendering.objectSpritesContainersSize, this->stageDefinition->rendering.objectSpritesContainersZPosition);
}

void Stage::setupPalettes(Stage this)
{
	ASSERT(this, "Stage::setupPalettes: null this");

	VIPManager::setupPalettes(VIPManager::getInstance(), &this->stageDefinition->rendering.paletteConfig);
}

// load stage's entites
void Stage::load(Stage this, VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition)
{
	ASSERT(this, "Stage::load: null this");

	// set optical values
	Camera::setOptical(Camera::getInstance(), Optical::getFromPixelOptical(this->stageDefinition->rendering.pixelOptical));

	// stop all sounds
	SoundManager::stopAllSound(SoundManager::getInstance());

	Camera::setCameraFrustum(Camera::getInstance(), this->stageDefinition->level.cameraFrustum);
	Camera::setStageSize(Camera::getInstance(), Size::getFromPixelSize(this->stageDefinition->level.pixelSize));

	if(overrideCameraPosition)
	{
		Camera::reset(Camera::getInstance());
		Camera::setPosition(Camera::getInstance(), Vector3D::getFromPixelVector(this->stageDefinition->level.cameraInitialPosition));
	}

	// set palettes
	Stage::setupPalettes(this);

	// setup OBJs
	Stage::setObjectSpritesContainers(this);

	// setup SpriteManager's configuration
	SpriteManager::setCyclesToWaitForTextureWriting(SpriteManager::getInstance(), this->stageDefinition->rendering.cyclesToWaitForTextureWriting);
	SpriteManager::setTexturesMaximumRowsToWrite(SpriteManager::getInstance(), this->stageDefinition->rendering.texturesMaximumRowsToWrite);
	SpriteManager::setMaximumParamTableRowsToComputePerCall(SpriteManager::getInstance(), this->stageDefinition->rendering.maximumAffineRowsToComputePerCall);

	// preload textures
	Stage::preloadAssets(this);

	// setup ui
	Stage::setupUI(this);

	// register all the entities in the stage's definition
	Stage::registerEntities(this, positionedEntitiesToIgnore);

	// load entities
	Stage::loadInitialEntities(this);

	// retrieve focus entity for streaming
	Stage::setFocusEntity(this, Camera::getFocusEntity(Camera::getInstance()));

	// set physics
	PhysicalWorld::setFrictionCoefficient(Game::getPhysicalWorld(Game::getInstance()), this->stageDefinition->physics.frictionCoefficient);
	PhysicalWorld::setGravity(Game::getPhysicalWorld(Game::getInstance()), this->stageDefinition->physics.gravity);

	// load background music
	SoundManager::playBGM(SoundManager::getInstance(), (const u16 (*)[6])this->stageDefinition->assets.bgm);

	// setup colors and brightness
	VIPManager::setBackgroundColor(VIPManager::getInstance(), this->stageDefinition->rendering.colorConfig.backgroundColor);
	VIPManager::setupBrightnessRepeat(VIPManager::getInstance(), this->stageDefinition->rendering.colorConfig.brightnessRepeat);

	// set particle removal delay
	ParticleRemover::setRemovalDelayCycles(this->particleRemover, this->stageDefinition->streaming.particleRemovalDelayCycles);

	// apply transformations
	 Container::initialTransform(this, &neutralEnvironmentTransformation, true);

	if(this->uiContainer)
	{
		 Container::initialTransform(this->uiContainer, &neutralEnvironmentTransformation, true);
	}
}

void Stage::loadPostProcessingEffects(Stage this)
{
	ASSERT(this, "Stage::loadPostProcessingEffects: null this");

	if(this->stageDefinition->postProcessingEffects)
	{
		int i = 0;
		for(; this->stageDefinition->postProcessingEffects[i]; i++)
		{
			Game::pushFrontProcessingEffect(Game::getInstance(), this->stageDefinition->postProcessingEffects[i], NULL);
		}
	}
}

// retrieve size
Size Stage::getSize(Stage this)
{
	ASSERT(this, "Stage::getSize: null this");
	ASSERT(this->stageDefinition, "Stage::getSize: null stageDefinition");

	// set world's limits
	return Size::getFromPixelSize(this->stageDefinition->level.pixelSize);
}

CameraFrustum Stage::getCameraFrustum(Stage this)
{
	ASSERT(this, "Stage::getCameraFrustum: null this");
	ASSERT(this->stageDefinition, "Stage::getCameraFrustum: null stageDefinition");

	// set world's limits
	return this->stageDefinition->level.cameraFrustum;
}
// setup ui
static void Stage::setupUI(Stage this)
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
			 Container::initialTransform(this->uiContainer, &neutralEnvironmentTransformation, true);
		}
	}
}

// add entity to the stage
Entity Stage::addChildEntity(Stage this, const PositionedEntity* const positionedEntity, bool permanent)
{
	ASSERT(this, "Stage::addEntity: null this");

	return Stage::doAddChildEntity(this, positionedEntity, permanent, this->nextEntityId++, true);
}

// add entity to the stage
static Entity Stage::doAddChildEntity(Stage this, const PositionedEntity* const positionedEntity, bool permanent __attribute__ ((unused)), s16 internalId, bool makeReady)
{
	ASSERT(this, "Stage::doAddChildEntity: null this");

	if(positionedEntity)
	{
		Entity entity = Entity::loadEntity(positionedEntity, internalId);
		ASSERT(entity, "Stage::doAddChildEntity: entity not loaded");

		if(entity)
		{
			// must add graphics
			 Container::setupGraphics(entity);
			 Entity::initialize(entity, true);

			// create the entity and add it to the world
			Container::addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, entity));

			// apply transformations
			 Container::initialTransform(entity, &neutralEnvironmentTransformation, true);

			if(makeReady)
			{
				Stage::makeChildReady(this, entity);
			}

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

// initialize child
static void Stage::makeChildReady(Stage this, Entity entity)
{
	ASSERT(this, "Stage::setChildReady: null this");
	ASSERT(entity, "Stage::setChildReady: null entity");
	ASSERT(entity->parent == __SAFE_CAST(Container, this), "Stage::setChildReady: I'm not its parent");

	if(entity->parent == __SAFE_CAST(Container, this))
	{
		 Entity::ready(entity, true);
	}
}

bool Stage::registerEntityId(Stage this, s16 internalId, EntityDefinition* entityDefinition)
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

void Stage::spawnEntity(Stage this, PositionedEntity* positionedEntity, Container requester, EventListener callback)
{
	ASSERT(this, "Stage::spawnEntity: null this");

	EntityFactory::spawnEntity(this->entityFactory, positionedEntity, requester, callback, this->nextEntityId++);
}

// remove entity from the stage
void Stage::removeChild(Stage this, Container child, bool deleteChild)
{
	ASSERT(this, "Stage::removeEntity: null this");
	ASSERT(child, "Stage::removeEntity: null child");

	if(!child)
	{
		return;
	}

	Base::removeChild(this, child, deleteChild);

	s16 internalId = Entity::getInternalId(__SAFE_CAST(Entity, child));

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

		VirtualList::removeElement(this->stageEntities, node->data);
		VirtualList::removeElement(this->loadedStageEntities, node->data);
		__DELETE_BASIC(node->data);
	}
}

// unload entity from the stage
static void Stage::unloadChild(Stage this, Container child)
{
	ASSERT(this, "Stage::unloadChild: null this");
	ASSERT(child, "Stage::unloadChild: null child");

	if(!child)
	{
		return;
	}

	Base::removeChild(__SAFE_CAST(Container, this), child, true);
	Object::fireEvent(__SAFE_CAST(Object, child), kStageChildStreamedOut);
	Object::removeAllEventListeners(__SAFE_CAST(Object, child), kStageChildStreamedOut);
	MessageDispatcher::discardAllDelayedMessagesFromSender(MessageDispatcher::getInstance(), __SAFE_CAST(Object, child));
	MessageDispatcher::discardAllDelayedMessagesForReceiver(MessageDispatcher::getInstance(), __SAFE_CAST(Object, child));

	s16 internalId = Entity::getInternalId(__SAFE_CAST(Entity, child));

	VirtualNode node = this->stageEntities->head;

	for(; node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(stageEntityDescription->internalId == internalId)
		{
			stageEntityDescription->internalId = -1;

			// remove from list of entities that are to be loaded by the streaming,
			// if the entity is not to be respawned
			if(! Entity::respawn(child))
			{
				VirtualList::removeElement(this->stageEntities, node->data);
			}

			break;
		}
	}
}

// preload fonts, charsets and textures
static void Stage::preloadAssets(Stage this)
{
	ASSERT(this, "Stage::preloadAssets: null this");

	// fonts
	Printing::loadFonts(Printing::getInstance(), this->stageDefinition->assets.fontDefinitions);

	// charsets
	if(this->stageDefinition->assets.charSetDefinitions)
	{
		int i = 0;

		for(; this->stageDefinition->assets.charSetDefinitions[i]; i++)
		{
			if(__ANIMATED_SINGLE != this->stageDefinition->assets.charSetDefinitions[i]->allocationType &&
				__ANIMATED_SINGLE_OPTIMIZED != this->stageDefinition->assets.charSetDefinitions[i]->allocationType)
			{
				CharSetManager::getCharSet(CharSetManager::getInstance(), this->stageDefinition->assets.charSetDefinitions[i]);
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
				BgmapTexture bgmapTexture = BgmapTextureManager::getTexture(BgmapTextureManager::getInstance(), this->stageDefinition->assets.textureDefinitions[i]);

				if(bgmapTexture)
				{
					 Texture::write(bgmapTexture);

					if(this->stageDefinition->assets.textureDefinitions[i]->recyclable)
					{
						VirtualList::pushBack(recyclableTextures, bgmapTexture);
					}
				}
			}
			else
			{
				ASSERT(this, "Stage::preloadAssets: loading an Object texture");
			}
		}

		VirtualNode node = VirtualList::begin(recyclableTextures);

		for(;node; node = node->next)
		{
			BgmapTextureManager::releaseTexture(BgmapTextureManager::getInstance(), __SAFE_CAST(BgmapTexture, node->data));
		}

		__DELETE(recyclableTextures);
	}

	ParamTableManager::calculateParamTableBase(ParamTableManager::getInstance(), this->stageDefinition->rendering.paramTableSegments);
}

// register an entity in the streaming list
static StageEntityDescription* Stage::registerEntity(Stage this __attribute__ ((unused)), PositionedEntity* positionedEntity)
{
	ASSERT(this, "Stage::registerEntities: null this");
	ASSERT(positionedEntity, "Stage::registerEntities: null positionedEntity");

	StageEntityDescription* stageEntityDescription = __NEW_BASIC(StageEntityDescription);

	stageEntityDescription->internalId = -1;
	stageEntityDescription->positionedEntity = positionedEntity;

	PixelVector environmentPosition = {0, 0, 0, 0};
	stageEntityDescription->pixelRightBox = Entity::getTotalSizeFromDefinition(stageEntityDescription->positionedEntity, &environmentPosition);

	int x = stageEntityDescription->positionedEntity->onScreenPosition.x - (stageEntityDescription->pixelRightBox.x1 - stageEntityDescription->pixelRightBox.x0) / 2;
	int y = stageEntityDescription->positionedEntity->onScreenPosition.y - (stageEntityDescription->pixelRightBox.y1 - stageEntityDescription->pixelRightBox.y0) / 2;
	int z = stageEntityDescription->positionedEntity->onScreenPosition.z - (stageEntityDescription->pixelRightBox.z1 - stageEntityDescription->pixelRightBox.z0) / 2;

	stageEntityDescription->distance = x * x + y * y + z * z;

	return stageEntityDescription;
}

// register the stage's definition entities in the streaming list
static void Stage::registerEntities(Stage this, VirtualList positionedEntitiesToIgnore)
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

		StageEntityDescription* stageEntityDescription = Stage::registerEntity(this, &this->stageDefinition->entities.children[i]);

		VirtualNode auxNode = this->stageEntities->head;

		for(; auxNode; auxNode = auxNode->next)
		{
			StageEntityDescription* auxStageEntityDescription = (StageEntityDescription*)auxNode->data;

			if(stageEntityDescription->distance > auxStageEntityDescription->distance)
			{
				continue;
			}

			VirtualList::insertBefore(this->stageEntities, auxNode, stageEntityDescription);
			break;
		}

		if(!auxNode)
		{
			VirtualList::pushBack(this->stageEntities, stageEntityDescription);
		}
	}
}

// load all visible entities
static void Stage::loadInitialEntities(Stage this)
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
			if(stageEntityDescription->positionedEntity->loadRegardlessOfPosition || Stage::isEntityInLoadRange(this, stageEntityDescription->positionedEntity->onScreenPosition, &stageEntityDescription->pixelRightBox))
			{
				stageEntityDescription->internalId = this->nextEntityId++;
				Entity entity = Stage::doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId, false);
				ASSERT(entity, "Stage::loadInRangeEntities: entity not loaded");

				if(!stageEntityDescription->positionedEntity->loadRegardlessOfPosition)
				{
					this->streamingHeadNode = node;
				}

				stageEntityDescription->internalId = Entity::getInternalId(entity);

				VirtualList::pushBack(this->loadedStageEntities, stageEntityDescription);
			}
		}
	}

	node = this->children->head;

	for(; node; node = node->next)
	{
		Stage::makeChildReady(this, __SAFE_CAST(Entity, node->data));
	}
}

// unload non visible entities
static bool Stage::unloadOutOfRangeEntities(Stage this, int defer)
{
	ASSERT(this, "Stage::unloadOutOfRangeEntities: null this");

	if(!this->children)
	{
		return false;
	}

#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getMillisecondsElapsed(TimerManager::getInstance());
#endif

	// need a temporal list to remove and delete entities
	VirtualNode node = this->children->head;

	// check which actors must be unloaded
	for(; node; node = node->next)
	{
		// get next entity
		Entity entity = __SAFE_CAST(Entity, node->data);

		// if the entity isn't visible inside the view field, unload it
		if(!entity->deleteMe && entity->parent == __SAFE_CAST(Container, this) && !Entity::isVisible(entity, (this->stageDefinition->streaming.loadPadding + this->stageDefinition->streaming.unloadPadding + __MAXIMUM_PARALLAX), true))
		{
			s16 internalId = Entity::getInternalId(entity);

			VirtualNode auxNode = this->loadedStageEntities->head;
			StageEntityDescription* stageEntityDescription = NULL;

			for(; auxNode; auxNode = auxNode->next)
			{
				stageEntityDescription = (StageEntityDescription*)auxNode->data;

				if(stageEntityDescription->internalId == internalId)
				{
					break;
				}
			}

			bool unloaded = false;

			if(stageEntityDescription)
			{
				if(!stageEntityDescription->positionedEntity->loadRegardlessOfPosition)
				{
					// unload it
					Stage::unloadChild(this, __SAFE_CAST(Container, entity));
					VirtualList::removeElement(this->loadedStageEntities, stageEntityDescription);

					unloaded = true;
				}
			}
			else
			{
				// unload it
				Stage::unloadChild(this, __SAFE_CAST(Container, entity));

				unloaded = true;
			}


			if(unloaded && defer)
			{
#ifdef __PROFILE_STREAMING
				u32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
				unloadOutOfRangeEntitiesHighestTime = processTime > unloadOutOfRangeEntitiesHighestTime ? processTime : unloadOutOfRangeEntitiesHighestTime;
#endif
				return true;
			}
		}
	}

#ifdef __PROFILE_STREAMING
		u32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
		unloadOutOfRangeEntitiesHighestTime = processTime > unloadOutOfRangeEntitiesHighestTime ? processTime : unloadOutOfRangeEntitiesHighestTime;
#endif

	return true;
}

static bool Stage::loadInRangeEntities(Stage this, int defer __attribute__ ((unused)))
{
	ASSERT(this, "Stage::selectEntitiesInLoadRange: null this");

#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getMillisecondsElapsed(TimerManager::getInstance());
#endif

	bool loadedEntities = false;

	PixelVector cameraPosition = PixelVector::getFromVector3D(*_cameraPosition, 0);

	u32 cameraDistance = (cameraPosition.x * cameraPosition.x +
							cameraPosition.y * cameraPosition.y +
							cameraPosition.z * cameraPosition.z);

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
				if(Stage::isEntityInLoadRange(this, stageEntityDescription->positionedEntity->onScreenPosition, &stageEntityDescription->pixelRightBox))
				{
					loadedEntities = true;

					stageEntityDescription->internalId = this->nextEntityId++;
					VirtualList::pushBack(this->loadedStageEntities, stageEntityDescription);

					if(defer)
					{
						EntityFactory::spawnEntity(this->entityFactory, stageEntityDescription->positionedEntity, __SAFE_CAST(Container, this), NULL, stageEntityDescription->internalId);
					}
					else
					{
						Stage::doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId, true);
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
				if(Stage::isEntityInLoadRange(this, stageEntityDescription->positionedEntity->onScreenPosition, &stageEntityDescription->pixelRightBox))
				{
					loadedEntities = true;

					stageEntityDescription->internalId = this->nextEntityId++;
					VirtualList::pushBack(this->loadedStageEntities, stageEntityDescription);

					if(defer)
					{
						EntityFactory::spawnEntity(this->entityFactory, stageEntityDescription->positionedEntity, __SAFE_CAST(Container, this), NULL, stageEntityDescription->internalId);
					}
					else
					{
						Stage::doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId, true);
					}
				}
			}
		}
	}

	this->cameraPreviousDistance = cameraDistance;

#ifdef __PROFILE_STREAMING
	u32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
	loadInRangeEntitiesHighestTime = processTime > loadInRangeEntitiesHighestTime ? processTime : loadInRangeEntitiesHighestTime;
#endif

	return loadedEntities;
}

// process removed children
static bool Stage::purgeChildrenProgressively(Stage this)
{
	ASSERT(this, "Stage::processRemovedChildrenProgressively: null this");

	if(!this->removedChildren || !this->removedChildren->head)
	{
		return false;
	}

#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getMillisecondsElapsed(TimerManager::getInstance());
#endif

	Container child = __SAFE_CAST(Container, VirtualList::front(this->removedChildren));

	VirtualList::popFront(this->removedChildren);
	VirtualList::removeElement(this->children, child);

	if(__IS_OBJECT_ALIVE(child))
	{
		if(child->deleteMe)
		{
			__DELETE(child);

#ifdef __PROFILE_STREAMING
			u32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
			processRemovedEntitiesHighestTime = processTime > processRemovedEntitiesHighestTime ? processTime : processRemovedEntitiesHighestTime;
#endif
			return true;
		}
	}

#ifdef __PROFILE_STREAMING
	u32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
	processRemovedEntitiesHighestTime = processTime > processRemovedEntitiesHighestTime ? processTime : processRemovedEntitiesHighestTime;
#endif

	return false;
}

//
static bool Stage::updateEntityFactory(Stage this)
{
	ASSERT(this, "Stage::updateEntityFactory: null this");

#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getMillisecondsElapsed(TimerManager::getInstance());
#endif

	bool preparingEntities = EntityFactory::prepareEntities(this->entityFactory);

#ifdef __PROFILE_STREAMING
	u32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
	entityFactoryHighestTime = processTime > entityFactoryHighestTime ? processTime : entityFactoryHighestTime;
#endif

	return preparingEntities;
}

bool Stage::stream(Stage this)
{
	ASSERT(this, "Stage::stream: null this");

#ifdef __SHOW_STREAMING_PROFILING
	if(!Game::isInSpecialMode(Game::getInstance()))
	{
		EntityFactory::showStatus(this->entityFactory, 25, 3);
	}
#endif

	if(Stage::purgeChildrenProgressively(this) && this->stageDefinition->streaming.deferred)
	{
		return true;
	}

	if(Stage::updateEntityFactory(this) && this->stageDefinition->streaming.deferred)
	{
		return false;
	}

	int streamingPhases = sizeof(_streamingPhases) / sizeof(StreamingPhase);

	if(++this->streamingPhase >= streamingPhases)
	{
		this->streamingPhase = 0;
	}

	return _streamingPhases[this->streamingPhase](this, this->stageDefinition->streaming.deferred);
}

void Stage::streamAll(Stage this)
{
	ASSERT(this, "Stage::streamAll: null this");

	// must make sure there are not pending entities for removal
	Container::purgeChildren(__SAFE_CAST(Container, this));

	Stage::unloadOutOfRangeEntities(this, false);
	SpriteManager::disposeSprites(SpriteManager::getInstance());
	Container::purgeChildren(__SAFE_CAST(Container, this));
	Stage::loadInRangeEntities(this, false);
	EntityFactory::prepareAllEntities(this->entityFactory);
	SpriteManager::writeTextures(SpriteManager::getInstance());
	SpriteManager::sortLayers(SpriteManager::getInstance());
}

// execute stage's logic
void Stage::update(Stage this, u32 elapsedTime)
{
	ASSERT(this, "Stage::update: null this");

	Base::update(this, elapsedTime);

	if(this->uiContainer)
	{
		Container::update(__SAFE_CAST(Container, this->uiContainer), elapsedTime);
	}

	ParticleRemover::update(this->particleRemover);
}

// transformation state
void Stage::transform(Stage this, const Transformation* environmentTransform __attribute__ ((unused)), u8 invalidateTransformationFlag)
{
	ASSERT(this, "Stage::transform: null this");

	Base::transform(this, environmentTransform, invalidateTransformationFlag);

	if(this->uiContainer)
	{
		 Container::transform(this->uiContainer, environmentTransform, invalidateTransformationFlag);
	}
}

void Stage::synchronizeGraphics(Stage this)
{
	ASSERT(this, "Stage::synchronizeGraphics: null this");

	Base::synchronizeGraphics(this);
}

// retrieve ui
UiContainer Stage::getUiContainer(Stage this)
{
	ASSERT(this, "Stage::getUiContainer: null this");

	return this->uiContainer;
}

// suspend for pause
void Stage::suspend(Stage this)
{
	ASSERT(this, "Stage::suspend: null this");

	// stream all pending entities to avoid having manually recover
	// the stage entity registries
	EntityFactory::prepareAllEntities(this->entityFactory);

	Base::suspend(this);

	if(this->uiContainer)
	{
		 Container::suspend(__SAFE_CAST(Container, this->uiContainer));
	}

	// relinquish camera focus priority
	if(this->focusEntity && Camera::getFocusEntity(Camera::getInstance()))
	{
		if(this->focusEntity == Camera::getFocusEntity(Camera::getInstance()))
		{
			// relinquish focus entity
			Camera::setFocusGameEntity(Camera::getInstance(), NULL);
		}
	}
	else
	{
		Stage::setFocusEntity(this, Camera::getFocusEntity(Camera::getInstance()));
	}

	__DELETE(this->entityFactory);
	ParticleRemover::reset(this->particleRemover);
}

// resume after pause
void Stage::resume(Stage this)
{
	ASSERT(this, "Stage::resume: null this");

	// set back optical values
	Camera::setOptical(Camera::getInstance(), Optical::getFromPixelOptical(this->stageDefinition->rendering.pixelOptical));

	// set physics
	PhysicalWorld::setFrictionCoefficient(Game::getPhysicalWorld(Game::getInstance()), this->stageDefinition->physics.frictionCoefficient);
	PhysicalWorld::setGravity(Game::getPhysicalWorld(Game::getInstance()), this->stageDefinition->physics.gravity);

	// set palettes
	Stage::setupPalettes(this);

	// set OBJs' z position
	Stage::setObjectSpritesContainers(this);

	// setup SpriteManager's configuration
	SpriteManager::setCyclesToWaitForTextureWriting(SpriteManager::getInstance(), this->stageDefinition->rendering.cyclesToWaitForTextureWriting);
	SpriteManager::setTexturesMaximumRowsToWrite(SpriteManager::getInstance(), this->stageDefinition->rendering.texturesMaximumRowsToWrite);
	SpriteManager::setMaximumParamTableRowsToComputePerCall(SpriteManager::getInstance(), this->stageDefinition->rendering.maximumAffineRowsToComputePerCall);

	// reload textures
	Stage::preloadAssets(this);

	if(this->focusEntity)
	{
		// recover focus entity
		Camera::setFocusGameEntity(Camera::getInstance(), __SAFE_CAST(Entity, this->focusEntity));
	}

	// load background music
	SoundManager::playBGM(SoundManager::getInstance(), (const u16 (*)[6])this->stageDefinition->assets.bgm);

	Base::resume(this);

	// apply transformations
	 Container::initialTransform(this, &neutralEnvironmentTransformation, true);

	if(this->uiContainer)
	{
		 Container::resume(__SAFE_CAST(Container, this->uiContainer));

		 Container::initialTransform(this->uiContainer, &neutralEnvironmentTransformation, true);
	}

	this->entityFactory = __NEW(EntityFactory);
}

bool Stage::handlePropagatedMessage(Stage this, int message)
{
	ASSERT(this, "Stage::handlePropagatedMessage: null this");

	if(this->uiContainer)
	{
		// propagate message to ui
		return Container::propagateMessage(__SAFE_CAST(Container, this->uiContainer), Container_onPropagatedMessage, message);
	}

	return false;
}

void Stage::onFocusEntityDeleted(Stage this, Object eventFirer __attribute__ ((unused)))
{
	ASSERT(this, "Stage::onFocusEntityDeleted: null this");

	this->focusEntity = NULL;

	if(this->focusEntity && Camera::getFocusEntity(Camera::getInstance()))
	{
		if(this->focusEntity == Camera::getFocusEntity(Camera::getInstance()))
		{
			Camera::setFocusGameEntity(Camera::getInstance(), NULL);
		}
	}
}

static void Stage::setFocusEntity(Stage this, Entity focusEntity)
{
	ASSERT(this, "Stage::setFocusEntity: null this");

	if(this->focusEntity)
	{
		Object::removeEventListener(__SAFE_CAST(Object, this->focusEntity), __SAFE_CAST(Object, this), (EventListener)Stage_onFocusEntityDeleted, kEventContainerDeleted);
	}

	this->focusEntity = focusEntity;

	if(this->focusEntity)
	{
		Object::addEventListener(__SAFE_CAST(Object, this->focusEntity), __SAFE_CAST(Object, this), (EventListener)Stage_onFocusEntityDeleted, kEventContainerDeleted);

		Vector3D focusEntityPosition = *Container::getGlobalPosition(__SAFE_CAST(Container, this->focusEntity));
		focusEntityPosition.x = __METERS_TO_PIXELS(focusEntityPosition.x);
		focusEntityPosition.y = __METERS_TO_PIXELS(focusEntityPosition.y);
		focusEntityPosition.z = __METERS_TO_PIXELS(focusEntityPosition.z);

		this->cameraPreviousDistance = (long)focusEntityPosition.x * (long)focusEntityPosition.x +
											(long)focusEntityPosition.y * (long)focusEntityPosition.y +
											(long)focusEntityPosition.z * (long)focusEntityPosition.z;
	}
}

// get stage definition
StageDefinition* Stage::getStageDefinition(Stage this)
{
	ASSERT(this, "Stage::getStageDefinition: null this");

	return this->stageDefinition;
}

ParticleRemover Stage::getParticleRemover(Stage this)
{
	ASSERT(this, "Stage::getParticleRemover: null this");

	return this->particleRemover;
}

void Stage::showStreamingProfiling(Stage this __attribute__ ((unused)), int x, int y)
{
	ASSERT(this, "Stage::showStreamingProfiling: null this");

	Printing::text(Printing::getInstance(), "STREAMING'S STATUS", x, y++, NULL);

	Printing::text(Printing::getInstance(), "Stage's status", x, ++y, NULL);

	int originalY __attribute__ ((unused)) = y;
	int xDisplacement = 18;
	y++;

	Printing::text(Printing::getInstance(), "Regist. entities:      ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->stageEntities), x + xDisplacement, y++, NULL);
	Printing::text(Printing::getInstance(), "Loaded entities:       ", x, y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->loadedStageEntities), x + xDisplacement, y++, NULL);
	Printing::text(Printing::getInstance(), "Children entities:       ", x, y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->children), x + xDisplacement, y++, NULL);

#ifdef __PROFILE_STREAMING

	xDisplacement = 12;

	Printing::text(Printing::getInstance(), "Process duration (ms):", x, ++y, NULL);

	Printing::text(Printing::getInstance(), "Unload:           ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), unloadOutOfRangeEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "Load:             ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), loadInRangeEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "Removing:         ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), processRemovedEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "Factory:          ", x, ++y, NULL);
	Printing::int(Printing::getInstance(), entityFactoryHighestTime, x + xDisplacement, y++, NULL);

	unloadOutOfRangeEntitiesHighestTime = 0;
	loadInRangeEntitiesHighestTime = 0;
	processRemovedEntitiesHighestTime = 0;
	entityFactoryHighestTime = 0;
#endif
}
