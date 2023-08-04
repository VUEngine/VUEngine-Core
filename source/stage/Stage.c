/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <string.h>

#include <Stage.h>
#include <Optics.h>
#include <VUEngine.h>
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
#include <MessageDispatcher.h>
#include <Printing.h>
#include <debugConfig.h>
#ifdef __PROFILE_STREAMING
#include <TimerManager.h>
#endif


//---------------------------------------------------------------------------------------------------------
// 												MACROS
//---------------------------------------------------------------------------------------------------------

#define __STREAMING_CYCLES		5


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Container;
friend class Entity;
friend class VirtualNode;
friend class VirtualList;

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

#ifdef __PROFILE_STREAMING
extern int16 _renderingProcessTimeHelper;
#endif

typedef bool (*StreamingPhase)(void*, int32);

static const StreamingPhase _streamingPhases[] =
{
	&Stage::unloadOutOfRangeEntities,
	&Stage::loadInRangeEntities
};

#ifdef __PROFILE_STREAMING
static uint32 unloadOutOfRangeEntitiesHighestTime = 0;
static uint32 loadInRangeEntitiesHighestTime = 0;
static uint32 processRemovedEntitiesHighestTime = 0;
static uint32 entityFactoryHighestTime = 0;
static uint32 timeBeforeProcess = 0;
#endif

typedef struct EntityLoadingListener
{
	ListenerObject context;
	EventListener callback;
} EntityLoadingListener;


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Stage::constructor(StageSpec *stageSpec)
{
	// construct base object
	Base::constructor(NULL);

	this->entityFactory = new EntityFactory();
	this->children = new VirtualList();
	this->entityLoadingListeners = NULL;

	this->stageSpec = stageSpec;
	this->stageEntityDescriptions = NULL;
	this->uiContainer = NULL;
	this->focusEntity = NULL;
	this->streamingHeadNode = NULL;
	this->cameraPreviousDistance = 0;
	this->nextEntityId = 0;
	this->streamingPhase = 0;
	this->soundWrappers = NULL;
	this->streaming = this->stageSpec->streaming;
	this->streamingAmplitude = this->streaming.streamingAmplitude;
	this->forceNoPopIn = false;
	this->reverseStreaming = false;
}

// class's destructor
void Stage::destructor()
{
	Stage::setFocusEntity(this, NULL);

	if(!isDeleted(this->entityLoadingListeners))
	{
		VirtualList::deleteData(this->entityLoadingListeners);
		delete this->entityLoadingListeners;
		this->entityLoadingListeners = NULL;
	}

	if(!isDeleted(this->soundWrappers))
	{
		// Do not need to release sound wrappers here,
		// they are taken care by the SoundManager when
		// I called SoundManager::stopAllSounds
		for(VirtualNode node = this->soundWrappers->head; NULL != node; node = node->next)
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

			if(!isDeleted(soundWrapper))
			{
				SoundWrapper::removeEventListenerScopes(soundWrapper, ListenerObject::safeCast(this), kEventSoundReleased);
			}
		}

		delete this->soundWrappers;
		this->soundWrappers = NULL;
	}

	if(!isDeleted(this->entityFactory))
	{
		delete this->entityFactory;
		this->entityFactory = NULL;
	}

	if(!isDeleted(this->uiContainer))
	{
		delete this->uiContainer;
		this->uiContainer = NULL;
	}

	if(!isDeleted(this->stageEntityDescriptions))
	{
		VirtualList::deleteData(this->stageEntityDescriptions);
		delete this->stageEntityDescriptions;
		this->stageEntityDescriptions = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void Stage::fadeSounds(uint32 playbackType)
{
	if(!isDeleted(this->soundWrappers))
	{
		// Do not need to release sound wrappers here,
		// they are taken care by the SoundManager when
		// I called SoundManager::stopAllSounds
		for(VirtualNode node = this->soundWrappers->head; NULL != node; node = node->next)
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

			if(!isDeleted(soundWrapper))
			{
				SoundWrapper::removeEventListenerScopes(soundWrapper, ListenerObject::safeCast(this), kEventSoundReleased);
				SoundWrapper::play(soundWrapper, NULL, playbackType);
			}
		}
	}
}

void Stage::pauseSounds()
{
	if(!isDeleted(this->soundWrappers))
	{
		// Do not need to release sound wrappers here,
		// they are taken care by the SoundManager when
		// I called SoundManager::stopAllSounds
		for(VirtualNode node = this->soundWrappers->head; NULL != node; node = node->next)
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

			if(!isDeleted(soundWrapper))
			{
				SoundWrapper::removeEventListenerScopes(soundWrapper, ListenerObject::safeCast(this), kEventSoundReleased);
				SoundWrapper::pause(soundWrapper);
			}
		}
	}
}

void Stage::unpauseSounds()
{
	if(!isDeleted(this->soundWrappers))
	{
		// Do not need to release sound wrappers here,
		// they are taken care by the SoundManager when
		// I called SoundManager::stopAllSounds
		for(VirtualNode node = this->soundWrappers->head; NULL != node; node = node->next)
		{
			SoundWrapper soundWrapper = SoundWrapper::safeCast(node->data);

			if(!isDeleted(soundWrapper))
			{
				SoundWrapper::removeEventListenerScopes(soundWrapper, ListenerObject::safeCast(this), kEventSoundReleased);
				SoundWrapper::unpause(soundWrapper);
			}
		}
	}
}

// determine if a point is visible
int32 Stage::isEntityInLoadRange(ScreenPixelVector onScreenPosition, const PixelRightBox* pixelRightBox, const PixelVector* cameraPosition, bool forceNoPopIn)
{
	onScreenPosition.x -= cameraPosition->x;
	onScreenPosition.y -= cameraPosition->y;
	onScreenPosition.z -= cameraPosition->z;

	Vector3D position3D = Vector3D::rotate(Vector3D::getFromScreenPixelVector(onScreenPosition), *_cameraInvertedRotation);
	PixelVector position2D = PixelVector::getFromVector3D(position3D, 0);

	if(forceNoPopIn)
	{
		// check x visibility
		if(position2D.x + pixelRightBox->x1 < 0 || position2D.x + pixelRightBox->x0 > __SCREEN_WIDTH)
		{
			return true;
		}

		// check y visibility
		if(position2D.y + pixelRightBox->y1 < 0 || position2D.y + pixelRightBox->y0 > __SCREEN_HEIGHT)
		{
			return true;
		}

		// check z visibility
		if(position2D.z + pixelRightBox->z1 < 0 || position2D.z + pixelRightBox->z0 > __SCREEN_DEPTH)
		{
			return true;
		}

		return false;
	}
	else
	{
		if(NULL == pixelRightBox)
		{
			int32 pad = this->streaming.loadPadding + __ABS(position2D.z);

			// check x visibility
			if(position2D.x < _cameraFrustum->x0 - pad || position2D.x > _cameraFrustum->x1 + pad)
			{
				return false;
			}

			// check y visibility
			if(position2D.y < _cameraFrustum->y0 - pad || position2D.y > _cameraFrustum->y1 + pad)
			{
				return false;
			}

			// check z visibility
			if(position2D.z < _cameraFrustum->z0 - this->streaming.loadPadding || position2D.z > _cameraFrustum->z1 + this->streaming.loadPadding)
			{
				return false;
			}
		}
		else
		{
			// check x visibility
			if(position2D.x + pixelRightBox->x1 < _cameraFrustum->x0 || position2D.x + pixelRightBox->x0 > _cameraFrustum->x1)
			{
				return false;
			}

			// check y visibility
			if(position2D.y + pixelRightBox->y1 < _cameraFrustum->y0 || position2D.y + pixelRightBox->y0 > _cameraFrustum->y1)
			{
				return false;
			}

			// check z visibility
			if(position2D.z + pixelRightBox->z1 < _cameraFrustum->z0 || position2D.z + pixelRightBox->z0 > _cameraFrustum->z1)
			{
				return false;
			}
		}
	}

	return true;
}

void Stage::setObjectSpritesContainers()
{
	SpriteManager::setupObjectSpriteContainers(SpriteManager::getInstance(), this->stageSpec->rendering.objectSpritesContainersSize, this->stageSpec->rendering.objectSpritesContainersZPosition);
}

void Stage::setupPalettes()
{
	VIPManager::setupPalettes(VIPManager::getInstance(), &this->stageSpec->rendering.paletteConfig);
}

// load stage's entites
void Stage::load(VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition)
{
	// Setup timer
	Stage::setupTimer(this);

	// load background music
	Stage::setupSounds(this);

	Entity::setVisibilityPadding(this->streaming.loadPadding + this->streaming.unloadPadding);

	if(overrideCameraPosition)
	{
		Camera::reset(Camera::getInstance());
		Camera::setStageSize(Camera::getInstance(), Size::getFromPixelSize(this->stageSpec->level.pixelSize));
		Camera::setPosition(Camera::getInstance(), Vector3D::getFromPixelVector(this->stageSpec->level.cameraInitialPosition), true);
	}

	// set optical values
	Camera::setup(Camera::getInstance(), this->stageSpec->rendering.pixelOptical, this->stageSpec->level.cameraFrustum);

	Stage::prepareGraphics(this);

	// setup ui
	Stage::setupUI(this);

	// register all the entities in the stage's spec
	Stage::registerEntities(this, positionedEntitiesToIgnore);

	// load entities
	Stage::loadInitialEntities(this);

	// retrieve focus entity for streaming
	Stage::setFocusEntity(this, Camera::getFocusEntity(Camera::getInstance()));

	// set physics
	PhysicalWorld::setFrictionCoefficient(VUEngine::getPhysicalWorld(_vuEngine), this->stageSpec->physics.frictionCoefficient);
	PhysicalWorld::setGravity(VUEngine::getPhysicalWorld(_vuEngine), this->stageSpec->physics.gravity);

	// setup colors and brightness
	VIPManager::setBackgroundColor(VIPManager::getInstance(), this->stageSpec->rendering.colorConfig.backgroundColor);
	VIPManager::setupBrightness(VIPManager::getInstance(), &this->stageSpec->rendering.colorConfig.brightness);
	VIPManager::setupBrightnessRepeat(VIPManager::getInstance(), this->stageSpec->rendering.colorConfig.brightnessRepeat);

	// apply transformations
	Stage::initialTransform(this, &neutralEnvironmentTransformation, true);

	if(!isDeleted(this->uiContainer))
	{
		UIContainer::initialTransform(this->uiContainer, &neutralEnvironmentTransformation, true);
	}
}

void Stage::loadPostProcessingEffects()
{
	if(this->stageSpec->postProcessingEffects)
	{
		int32 i = 0;
		for(; this->stageSpec->postProcessingEffects[i]; i++)
		{
			VUEngine::pushFrontPostProcessingEffect(_vuEngine, this->stageSpec->postProcessingEffects[i], NULL);
		}
	}
}

// retrieve size
Size Stage::getSize()
{
	ASSERT(this->stageSpec, "Stage::getSize: null stageSpec");

	// set world's limits
	return Size::getFromPixelSize(this->stageSpec->level.pixelSize);
}

PixelSize Stage::getPixelSize()
{
	ASSERT(this->stageSpec, "Stage::getPixelSize: null stageSpec");

	// set world's limits
	return this->stageSpec->level.pixelSize;
}

CameraFrustum Stage::getCameraFrustum()
{
	ASSERT(this->stageSpec, "Stage::getCameraFrustum: null stageSpec");

	// set world's limits
	return this->stageSpec->level.cameraFrustum;
}

PixelOptical Stage::getPixelOptical()
{
	ASSERT(this->stageSpec, "Stage::getPixelOptical: null stageSpec");

	// set world's limits
	return this->stageSpec->rendering.pixelOptical;
}

// setup ui
void Stage::setupUI()
{
	ASSERT(!this->uiContainer, "Stage::setupUI: UI already exists");

	if(!isDeleted(this->uiContainer))
	{
		delete this->uiContainer;
		this->uiContainer = NULL;
	}

	if(this->stageSpec->entities.uiContainerSpec.allocator)
	{
		// call the appropriate allocator to support inheritance
		this->uiContainer = ((UIContainer (*)(UIContainerSpec*)) this->stageSpec->entities.uiContainerSpec.allocator)(&this->stageSpec->entities.uiContainerSpec);
		ASSERT(this->uiContainer, "Stage::setupUI: null ui");

		// setup ui if allocated and constructed
		if(!isDeleted(this->uiContainer))
		{
			// apply transformations
			UIContainer::initialTransform(this->uiContainer, &neutralEnvironmentTransformation, true);
		}
	}
}

void Stage::onEntityLoaded(ListenerObject eventFirer)
{
	Entity entity = Entity::safeCast(eventFirer);

	if(!isDeleted(entity) && !isDeleted(this->entityLoadingListeners))
	{
		Entity::removeEventListeners(entity, NULL, kEventEntityLoaded);
		Stage::alertOfLoadedEntity(this, entity);
	}
}

void Stage::alertOfLoadedEntity(Entity entity)
{
	if(isDeleted(entity) || isDeleted(this->entityLoadingListeners))
	{
		return;
	}

	for(VirtualNode node = this->entityLoadingListeners->head; NULL != node; node = node->next)
	{
		EntityLoadingListener* entityLoadingListener = (EntityLoadingListener*)node->data;

		if(!isDeleted(entityLoadingListener->context))
		{
			Entity::addEventListener(entity, entityLoadingListener->context, entityLoadingListener->callback, kEventEntityLoaded);
		}
	}

	Entity::fireEvent(entity, kEventEntityLoaded);
	NM_ASSERT(!isDeleted(entity), "Stage::alertOfLoadedEntity: deleted entity during kEventEntityLoaded");
	Entity::removeEventListeners(entity, NULL, kEventEntityLoaded);
}

// add entity to the stage
Entity Stage::addChildEntity(const PositionedEntity* const positionedEntity, bool permanent)
{
	return Stage::doAddChildEntity(this, positionedEntity, permanent, this->nextEntityId++);
}

Entity Stage::addChildEntityWithId(const PositionedEntity* const positionedEntity, bool permanent, int16 internalId)
{
	return Stage::doAddChildEntity(this, positionedEntity, permanent, internalId);
}

// add entity to the stage
Entity Stage::doAddChildEntity(const PositionedEntity* const positionedEntity, bool permanent __attribute__ ((unused)), int16 internalId)
{
	if(positionedEntity)
	{
		Entity entity = Entity::loadEntity(positionedEntity, internalId);
		ASSERT(entity, "Stage::doAddChildEntity: entity not loaded");

		if(!isDeleted(entity))
		{
			// create the entity and add it to the world
			Stage::addChild(this, Container::safeCast(entity));

			// apply transformations
			Entity::initialTransform(entity, &neutralEnvironmentTransformation, true);

			entity->dontStreamOut = entity->dontStreamOut || permanent;
			
			if(entity->parent == Container::safeCast(this))
			{
				Entity::ready(entity, true);
			}

			Stage::alertOfLoadedEntity(this, entity);
		}

		return entity;
	}

	return NULL;
}

void Stage::addEntityLoadingListener(ListenerObject context, EventListener callback)
{
	if(isDeleted(context) || NULL == callback)
	{
		return;
	}

	if(isDeleted(this->entityLoadingListeners))
	{
		this->entityLoadingListeners = new VirtualList();
	}

	EntityLoadingListener* entityLoadingListener = new EntityLoadingListener;
	entityLoadingListener->context = context;
	entityLoadingListener->callback = callback;

	VirtualList::pushBack(this->entityLoadingListeners, entityLoadingListener);
}

bool Stage::registerEntityId(int16 internalId, EntitySpec* entitySpec)
{
	VirtualNode node = this->stageEntityDescriptions->head;

	for(; NULL != node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(entitySpec == stageEntityDescription->positionedEntity->entitySpec)
		{
			stageEntityDescription->internalId = internalId;
			return true;
		}
	}

	return false;
}

void Stage::spawnEntity(PositionedEntity* positionedEntity, Container requester, EventListener callback)
{
	if(NULL == requester && !isDeleted(this->entityLoadingListeners))
	{
		requester = Container::safeCast(this);
		callback = (EventListener)Stage::onEntityLoaded;
	}

	EntityFactory::spawnEntity(this->entityFactory, positionedEntity, requester, callback, this->nextEntityId++);
}

// remove entity from the stage
void Stage::removeChild(Container child, bool deleteChild)
{
	NM_ASSERT(!isDeleted(child), "Stage::removeEntity: null child");

	if(isDeleted(child))
	{
		return;
	}

	Base::removeChild(this, child, deleteChild);

	int16 internalId = Entity::getInternalId(child);

	VirtualNode node = this->stageEntityDescriptions->head;

	for(; NULL != node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(stageEntityDescription->internalId == internalId)
		{
			stageEntityDescription->internalId = -1;
			break;
		}
	}

	if(NULL != node)
	{
		if(this->streamingHeadNode == node)
		{
			this->streamingHeadNode = this->reverseStreaming ? this->streamingHeadNode->next : this->streamingHeadNode->previous;
		}

		delete node->data;
		VirtualList::removeNode(this->stageEntityDescriptions, node);
	}
}

// unload entity from the stage
void Stage::removeChildEntity(Entity child)
{
	ASSERT(child, "Stage::removeChildEntity: null child");

	if(!child)
	{
		return;
	}

	Base::removeChild(this, Container::safeCast(child), true);

	int16 internalId = Entity::getInternalId(child);

	VirtualNode node = this->stageEntityDescriptions->head;

	for(; NULL != node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(stageEntityDescription->internalId == internalId)
		{
			stageEntityDescription->internalId = -1;

			// remove from list of entities that are to be loaded by the streaming,
			// if the entity is not to be respawned
			if(!Entity::respawn(child))
			{
				delete node->data;
				VirtualList::removeNode(this->stageEntityDescriptions, node);
			}

			break;
		}
	}
}

// preload fonts, charsets and textures
void Stage::preloadAssets()
{
	// fonts
	Printing::loadFonts(Printing::getInstance(), this->stageSpec->assets.fontSpecs);

	// charsets
	if(this->stageSpec->assets.charSetSpecs)
	{
		int32 i = 0;

		for(; this->stageSpec->assets.charSetSpecs[i]; i++)
		{
			if(__ANIMATED_SINGLE != this->stageSpec->assets.charSetSpecs[i]->allocationType &&
				__ANIMATED_SINGLE_OPTIMIZED != this->stageSpec->assets.charSetSpecs[i]->allocationType)
			{
				CharSetManager::getCharSet(CharSetManager::getInstance(), this->stageSpec->assets.charSetSpecs[i]);
			}
			else
			{
				ASSERT(this, "Stage::preloadAssets: preloading an animated single char set");
			}
		}
	}

	// textures
	if(this->stageSpec->assets.textureSpecs)
	{
		VirtualList recyclableTextures = new VirtualList();
		int32 i = 0;

		for(; this->stageSpec->assets.textureSpecs[i]; i++)
		{
			if(NULL == this->stageSpec->assets.textureSpecs[i]->charSetSpec)
			{
				continue;
			}
			
			if(__ANIMATED_SINGLE != this->stageSpec->assets.textureSpecs[i]->charSetSpec->allocationType &&
				__ANIMATED_SINGLE_OPTIMIZED != this->stageSpec->assets.textureSpecs[i]->charSetSpec->allocationType)
			{
				BgmapTexture bgmapTexture = BgmapTextureManager::getTexture(BgmapTextureManager::getInstance(), this->stageSpec->assets.textureSpecs[i], 0, false, __WORLD_1x1);

				NM_ASSERT(!isDeleted(bgmapTexture), "Stage::preloadAssets: failed to load bgmapTexture");

				if(!isDeleted(bgmapTexture))
				{
					if(this->stageSpec->assets.textureSpecs[i]->recyclable)
					{
						VirtualList::pushBack(recyclableTextures, bgmapTexture);
					}
					else
					{
						Texture::write(bgmapTexture, -1);
						Texture::releaseCharSet(bgmapTexture);
					}
				}
			}
			else
			{
				ASSERT(this, "Stage::preloadAssets: loading an ListenerObject texture");
			}
		}

		for(VirtualNode node = VirtualList::begin(recyclableTextures); NULL != node; node = node->next)
		{
			BgmapTextureManager::releaseTexture(BgmapTextureManager::getInstance(), BgmapTexture::safeCast(node->data));
		}

		delete recyclableTextures;
	}

	ParamTableManager::calculateParamTableBase(ParamTableManager::getInstance(), this->stageSpec->rendering.paramTableSegments);
}

// register an entity in the streaming list
StageEntityDescription* Stage::registerEntity(PositionedEntity* positionedEntity)
{
	ASSERT(positionedEntity, "Stage::registerEntities: null positionedEntity");

	StageEntityDescription* stageEntityDescription = new StageEntityDescription;

	stageEntityDescription->extraInfo = NULL;
	stageEntityDescription->internalId = -1;
	stageEntityDescription->positionedEntity = positionedEntity;

	PixelVector environmentPosition = {0, 0, 0, 0};
	stageEntityDescription->pixelRightBox = Entity::getTotalSizeFromSpec(stageEntityDescription->positionedEntity, &environmentPosition);

	stageEntityDescription->validRightBox = (0 != stageEntityDescription->pixelRightBox.x1 - stageEntityDescription->pixelRightBox.x0) || (0 != stageEntityDescription->pixelRightBox.y1 - stageEntityDescription->pixelRightBox.y0) || (0 != stageEntityDescription->pixelRightBox.z1 - stageEntityDescription->pixelRightBox.z0);

	int32 pad = this->streaming.loadPadding + __ABS(stageEntityDescription->positionedEntity->onScreenPosition.z);

	stageEntityDescription->pixelRightBox.x0 -= pad;
	stageEntityDescription->pixelRightBox.x1 += pad;
	stageEntityDescription->pixelRightBox.y0 -= pad;
	stageEntityDescription->pixelRightBox.y1 += pad;
	stageEntityDescription->pixelRightBox.z0 -= this->streaming.loadPadding;
	stageEntityDescription->pixelRightBox.z1 += this->streaming.loadPadding;

	return stageEntityDescription;
}

static uint32 Stage::computeDistanceToOrigin(StageEntityDescription* stageEntityDescription)
{
	int32 x = stageEntityDescription->positionedEntity->onScreenPosition.x - (stageEntityDescription->pixelRightBox.x1 - stageEntityDescription->pixelRightBox.x0) / 2;
	int32 y = stageEntityDescription->positionedEntity->onScreenPosition.y - (stageEntityDescription->pixelRightBox.y1 - stageEntityDescription->pixelRightBox.y0) / 2;
	int32 z = stageEntityDescription->positionedEntity->onScreenPosition.z - (stageEntityDescription->pixelRightBox.z1 - stageEntityDescription->pixelRightBox.z0) / 2;

	return x * x + y * y + z * z;
} 

// register the stage's spec entities in the streaming list
void Stage::registerEntities(VirtualList positionedEntitiesToIgnore)
{
	if(!isDeleted(this->stageEntityDescriptions))
	{
		return;
	}

	this->stageEntityDescriptions = new VirtualList();

	// register entities ordering them according to their distances to the origin
	int32 i = 0;

	for(;this->stageSpec->entities.children[i].entitySpec; i++)
	{
		if(positionedEntitiesToIgnore)
		{
			VirtualNode node = positionedEntitiesToIgnore->head;

			for(; NULL != node; node = node->next)
			{
				if(&this->stageSpec->entities.children[i] == (PositionedEntity*)node->data)
				{
					break;
				}
			}

			if(node)
			{
				continue;
			}
		}

		StageEntityDescription* stageEntityDescription = Stage::registerEntity(this, &this->stageSpec->entities.children[i]);

		Vector3D stageEntityPosition = (Vector3D)
		{
			__PIXELS_TO_METERS(stageEntityDescription->positionedEntity->onScreenPosition.x - (stageEntityDescription->pixelRightBox.x1 - stageEntityDescription->pixelRightBox.x0) / 2),
			__PIXELS_TO_METERS(stageEntityDescription->positionedEntity->onScreenPosition.y - (stageEntityDescription->pixelRightBox.y1 - stageEntityDescription->pixelRightBox.y0) / 2),
			__PIXELS_TO_METERS(stageEntityDescription->positionedEntity->onScreenPosition.z - (stageEntityDescription->pixelRightBox.z1 - stageEntityDescription->pixelRightBox.z0) / 2)
		};

		VirtualNode closestEnitryDescriptionNode = NULL;
		VirtualNode auxNode = this->stageEntityDescriptions->head;
		StageEntityDescription* auxStageEntityDescription = (StageEntityDescription*)auxNode->data;

		fixed_ext_t closestDistance = 0;

		for(; auxNode; auxNode = auxNode->next)
		{
			auxStageEntityDescription = (StageEntityDescription*)auxNode->data;

			Vector3D auxStageEntityPosition = (Vector3D)
			{
				__PIXELS_TO_METERS(auxStageEntityDescription->positionedEntity->onScreenPosition.x - (auxStageEntityDescription->pixelRightBox.x1 - auxStageEntityDescription->pixelRightBox.x0) / 2),
				__PIXELS_TO_METERS(auxStageEntityDescription->positionedEntity->onScreenPosition.y - (auxStageEntityDescription->pixelRightBox.y1 - auxStageEntityDescription->pixelRightBox.y0) / 2),
				__PIXELS_TO_METERS(auxStageEntityDescription->positionedEntity->onScreenPosition.z - (auxStageEntityDescription->pixelRightBox.z1 - auxStageEntityDescription->pixelRightBox.z0) / 2)
			};

			fixed_ext_t squaredDistance = Vector3D::squareLength(Vector3D::get(stageEntityPosition, auxStageEntityPosition));

			if(NULL == closestEnitryDescriptionNode || closestDistance > squaredDistance)
			{
				closestEnitryDescriptionNode = auxNode;
				closestDistance = squaredDistance;
			}
		}

		if(NULL == auxNode)
		{
			VirtualList::pushBack(this->stageEntityDescriptions, stageEntityDescription);
		}
		else
		{
			uint32 stageEntityDistanceToOrigin = Stage::computeDistanceToOrigin(stageEntityDescription);
			uint32 auxStageEntityDistanceToOrigin = Stage::computeDistanceToOrigin(auxStageEntityDescription);

			if(stageEntityDistanceToOrigin > auxStageEntityDistanceToOrigin)
			{
				VirtualList::insertAfter(this->stageEntityDescriptions, auxNode, stageEntityDescription);
			}
			else
			{
				VirtualList::insertBefore(this->stageEntityDescriptions, auxNode, stageEntityDescription);
			}
		}
	}
}

// load all visible entities
void Stage::loadInitialEntities()
{
	extern const Vector3D* _cameraPosition;
	PixelVector cameraPosition = PixelVector::getFromVector3D(*_cameraPosition, 0);

	// need a temporary list to remove and delete entities
	VirtualNode node = this->stageEntityDescriptions->head;

	for(; NULL != node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(-1 == stageEntityDescription->internalId)
		{
			// if entity in load range
			if(stageEntityDescription->positionedEntity->loadRegardlessOfPosition || Stage::isEntityInLoadRange(this, stageEntityDescription->positionedEntity->onScreenPosition, &stageEntityDescription->pixelRightBox, &cameraPosition, false))
			{
				stageEntityDescription->internalId = this->nextEntityId++;
				Entity entity = Stage::doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId);
				ASSERT(entity, "Stage::loadInitialEntities: entity not loaded");

				if(!isDeleted(entity))
				{
					if(!stageEntityDescription->positionedEntity->loadRegardlessOfPosition)
					{
						this->streamingHeadNode = node;
					}

					stageEntityDescription->internalId = Entity::getInternalId(entity);
				}
			}
		}
	}
}

// unload non visible entities
bool Stage::unloadOutOfRangeEntities(int32 defer __attribute__((unused)))
{
	if(isDeleted(this->children))
	{
		return false;
	}

#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getMillisecondsElapsed(TimerManager::getInstance());
#endif

	bool unloadedEntities = false;

	// need a temporary list to remove and delete entities
	VirtualNode node = this->children->head;

	// check which actors must be unloaded
	for(; NULL != node; node = node->next)
	{
		// get next entity
		Entity entity = Entity::safeCast(node->data);

		if(entity->dontStreamOut)
		{
			continue;
		}

		// if the entity isn't visible inside the view field, unload it
		if(!entity->deleteMe && entity->parent == Container::safeCast(this) && !entity->inCameraRange)
		{
			int16 internalId = Entity::getInternalId(entity);

			VirtualNode auxNode = this->stageEntityDescriptions->head;
			StageEntityDescription* stageEntityDescription = NULL;

			for(; auxNode; auxNode = auxNode->next)
			{
				stageEntityDescription = (StageEntityDescription*)auxNode->data;

				if(stageEntityDescription->internalId == internalId)
				{
					break;
				}
			}

			if(NULL != stageEntityDescription)
			{
				if(stageEntityDescription->positionedEntity->loadRegardlessOfPosition)
				{
					continue;
				}
			}

			// unload it
			Stage::removeChildEntity(this, entity);

			unloadedEntities = true;
		}
	}

#ifdef __PROFILE_STREAMING
		uint32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
		unloadOutOfRangeEntitiesHighestTime = processTime > unloadOutOfRangeEntitiesHighestTime ? processTime : unloadOutOfRangeEntitiesHighestTime;
#endif

	return unloadedEntities;
}

bool Stage::loadInRangeEntities(int32 defer)
{
#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getMillisecondsElapsed(TimerManager::getInstance());
#endif

	bool loadedEntities = false;

	PixelVector cameraPosition = PixelVector::getFromVector3D(*_cameraPosition, 0);

	if(this->reverseStreaming)
	{
		if(NULL == this->streamingHeadNode)
		{
			this->streamingHeadNode = this->stageEntityDescriptions->tail;
		}

		bool negativeStreamingAmplitude = 0 > ((int16)this->streamingAmplitude);

		for(uint16 counter = 0; counter < this->streamingAmplitude; this->streamingHeadNode = this->streamingHeadNode->previous, counter++)
		{
			if(NULL == this->streamingHeadNode)
			{
				this->streamingHeadNode = this->stageEntityDescriptions->tail;

				if(negativeStreamingAmplitude)
				{
					break;
				}
			}

			StageEntityDescription* stageEntityDescription = (StageEntityDescription*)this->streamingHeadNode->data;

			if(0 > stageEntityDescription->internalId)
			{
				// if entity in load range
				if(Stage::isEntityInLoadRange(this, stageEntityDescription->positionedEntity->onScreenPosition, stageEntityDescription->validRightBox ? &stageEntityDescription->pixelRightBox : NULL, &cameraPosition, this->forceNoPopIn))
				{
					loadedEntities = true;

					stageEntityDescription->internalId = this->nextEntityId++;

					if(defer)
					{
						EntityFactory::spawnEntity(this->entityFactory, stageEntityDescription->positionedEntity, Container::safeCast(this), !isDeleted(this->entityLoadingListeners) ? (EventListener)Stage::onEntityLoaded : NULL, stageEntityDescription->internalId);
					}
					else
					{
						Stage::doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId);
						break;
					}
				}
			}
		}
	}
	else
	{
		if(NULL == this->streamingHeadNode)
		{
			this->streamingHeadNode = this->stageEntityDescriptions->head;
		}

		bool negativeStreamingAmplitude = 0 > ((int16)this->streamingAmplitude);

		for(uint16 counter = 0; counter < this->streamingAmplitude; this->streamingHeadNode = this->streamingHeadNode->next, counter++)
		{
			if(NULL == this->streamingHeadNode)
			{
				this->streamingHeadNode = this->stageEntityDescriptions->head;

				if(negativeStreamingAmplitude)
				{
					break;
				}
			}

			StageEntityDescription* stageEntityDescription = (StageEntityDescription*)this->streamingHeadNode->data;

			if(0 > stageEntityDescription->internalId)
			{
				// if entity in load range
				if(Stage::isEntityInLoadRange(this, stageEntityDescription->positionedEntity->onScreenPosition, stageEntityDescription->validRightBox ? &stageEntityDescription->pixelRightBox : NULL, &cameraPosition, this->forceNoPopIn))
				{
					loadedEntities = true;

					stageEntityDescription->internalId = this->nextEntityId++;

					if(defer)
					{
						EntityFactory::spawnEntity(this->entityFactory, stageEntityDescription->positionedEntity, Container::safeCast(this), !isDeleted(this->entityLoadingListeners) ? (EventListener)Stage::onEntityLoaded : NULL, stageEntityDescription->internalId);
					}
					else
					{
						Stage::doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId);
						break;
					}
				}
			}
		}
	}

#ifdef __PROFILE_STREAMING
	uint32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
	loadInRangeEntitiesHighestTime = processTime > loadInRangeEntitiesHighestTime ? processTime : loadInRangeEntitiesHighestTime;
#endif

	return loadedEntities;
}

Entity Stage::findChildByInternalId(int16 internalId)
{
	VirtualNode node = this->children->head;

	for(; NULL != node; node = node->next)
	{
		if(Entity::getInternalId(Entity::safeCast(node->data)) == internalId)
		{
			return Entity::safeCast(node->data);
		}
	}

	return NULL;
}

//
bool Stage::updateEntityFactory()
{
#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getMillisecondsElapsed(TimerManager::getInstance());
#endif

	bool preparingEntities = EntityFactory::prepareEntities(this->entityFactory);

#ifdef __PROFILE_STREAMING
	uint32 processTime = -_renderingProcessTimeHelper + TimerManager::getMillisecondsElapsed(TimerManager::getInstance()) - timeBeforeProcess;
	entityFactoryHighestTime = processTime > entityFactoryHighestTime ? processTime : entityFactoryHighestTime;
#endif

	return preparingEntities;
}

EntityFactory Stage::getEntityFactory()
{
	return this->entityFactory;
}

VirtualList Stage::getSoundWrappers()
{
	return this->soundWrappers;
}

bool Stage::stream()
{
#ifdef __SHOW_STREAMING_PROFILING
	if(!VUEngine::isInSpecialMode(_vuEngine))
	{
		EntityFactory::showStatus(this->entityFactory, 25, 3);
	}
#endif

	if(Stage::updateEntityFactory(this))
	{
		return true;
	}

	int32 streamingPhases = sizeof(_streamingPhases) / sizeof(StreamingPhase);

	if(++this->streamingPhase >= streamingPhases)
	{
		this->streamingPhase = 0;
	}

	return _streamingPhases[this->streamingPhase](this, this->streaming.deferred);
}

bool Stage::streamAll()
{
	this->streamingPhase = 0;
	this->streamingHeadNode = NULL;
	this->streamingAmplitude = (uint16)-1;

	bool result = false;

	do
	{
		// Force deletion
		Stage::purgeChildren(this);

		result = Stage::stream(this);

		// Force deletion
		Stage::purgeChildren(this);
	}
	while(result);

	this->streamingAmplitude = this->streaming.streamingAmplitude;

	return EntityFactory::hasEntitiesPending(this->entityFactory);
}

bool Stage::streamInAll()
{
	this->streamingPhase = 0;
	this->streamingHeadNode = NULL;
	this->streamingAmplitude = (uint16)-1;

	// make sure that the entity factory doesn't have any pending operations
	EntityFactory::prepareAllEntities(this->entityFactory);

	// Force deletion
	Stage::purgeChildren(this);

	bool result = Stage::loadInRangeEntities(this, false);

	this->streamingAmplitude = this->streaming.streamingAmplitude;

	return result || EntityFactory::hasEntitiesPending(this->entityFactory);
}

bool Stage::streamOutAll()
{
	this->streamingPhase = 0;
	this->streamingHeadNode = NULL;

	// make sure that the entity factory doesn't have any pending operations
	EntityFactory::prepareAllEntities(this->entityFactory);

	// Force deletion
	Stage::purgeChildren(this);

	bool result = Stage::unloadOutOfRangeEntities(this, false);

	// Force deletion
	Stage::purgeChildren(this);

	return result;
}

// execute stage's logic
void Stage::update()
{
	Base::update(this);

	if(!isDeleted(this->uiContainer))
	{
		Container::update(this->uiContainer);
	}
}

// transformation state
void Stage::transform(const Transformation* environmentTransform __attribute__ ((unused)), uint8 invalidateTransformationFlag)
{
	Base::transform(this, environmentTransform, invalidateTransformationFlag);

	if(!isDeleted(this->uiContainer))
	{
		Container::transform(this->uiContainer, environmentTransform, invalidateTransformationFlag);
	}
}

void Stage::synchronizeGraphics()
{
	Base::synchronizeGraphics(this);

	if(!isDeleted(this->uiContainer))
	{
		UIContainer::synchronizeGraphics(this->uiContainer);
	}
}

// retrieve ui
UIContainer Stage::getUIContainer()
{
	return this->uiContainer;
}

// suspend for pause
void Stage::suspend()
{
	// stream all pending entities to avoid having manually recover
	// the stage entity registries
	while(EntityFactory::prepareEntities(this->entityFactory));
//	EntityFactory::prepareAllEntities(this->entityFactory); // It seems buggy

	Base::suspend(this);

	if(!isDeleted(this->uiContainer))
	{
		Container::suspend(this->uiContainer);
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

	delete this->entityFactory;
	this->entityFactory = NULL;
}

// resume after pause
void Stage::resume()
{
	// Setup timer
	Stage::setupTimer(this);

	// load background sounds
	Stage::setupSounds(this);

	// set back optical values
	Camera::setup(Camera::getInstance(), this->stageSpec->rendering.pixelOptical, this->stageSpec->level.cameraFrustum);

	// set physics
	PhysicalWorld::setFrictionCoefficient(VUEngine::getPhysicalWorld(_vuEngine), this->stageSpec->physics.frictionCoefficient);
	PhysicalWorld::setGravity(VUEngine::getPhysicalWorld(_vuEngine), this->stageSpec->physics.gravity);

	Stage::prepareGraphics(this);

	if(this->focusEntity)
	{
		// recover focus entity
		Camera::setFocusGameEntity(Camera::getInstance(), Entity::safeCast(this->focusEntity));
	}

	Base::resume(this);

	// apply transformations
	Stage::initialTransform(this, &neutralEnvironmentTransformation, true);

	// setup colors and brightness
	VIPManager::setBackgroundColor(VIPManager::getInstance(), this->stageSpec->rendering.colorConfig.backgroundColor);
	// TODO: properly handle brightness and brightness repeat on resume

	if(!isDeleted(this->uiContainer))
	{
		UIContainer::resume(this->uiContainer);
		UIContainer::initialTransform(this->uiContainer, &neutralEnvironmentTransformation, true);
	}

	this->entityFactory = new EntityFactory();
}

void Stage::prepareGraphics()
{
	// Must clean DRAM
	SpriteManager::reset(SpriteManager::getInstance());

	// set palettes
	Stage::setupPalettes(this);

	// setup OBJs
	Stage::setObjectSpritesContainers(this);

	// preload textures
	Stage::preloadAssets(this);

	// setup SpriteManager's configuration
	SpriteManager::setTexturesMaximumRowsToWrite(SpriteManager::getInstance(), this->stageSpec->rendering.texturesMaximumRowsToWrite);
	SpriteManager::setMaximumParamTableRowsToComputePerCall(SpriteManager::getInstance(), this->stageSpec->rendering.maximumAffineRowsToComputePerCall);
}

void Stage::setupSounds()
{
	SoundManager::unlock(SoundManager::getInstance());
	SoundManager::deferMIDIPlayback(SoundManager::getInstance(), this->stageSpec->sound.MIDIPlaybackCounterPerInterrupt);
	SoundManager::setTargetPlaybackFrameRate(SoundManager::getInstance(), this->stageSpec->sound.pcmTargetPlaybackFrameRate);

	int32 i = 0;

	// stop all sounds
	SoundManager::stopAllSounds(SoundManager::getInstance(), true, this->stageSpec->assets.sounds);

	for(; NULL != this->stageSpec->assets.sounds[i]; i++)
	{
		SoundWrapper soundWrapper = SoundManager::findSound(SoundManager::getInstance(), this->stageSpec->assets.sounds[i], (EventListener)Stage::onSoundWrapperReleased, ListenerObject::safeCast(this));

		if(isDeleted(soundWrapper))
		{
			soundWrapper = SoundManager::getSound(SoundManager::getInstance(), this->stageSpec->assets.sounds[i], kPlayAll, (EventListener)Stage::onSoundWrapperReleased, ListenerObject::safeCast(this));
		}

		if(!isDeleted(soundWrapper))
		{
			if(isDeleted(this->soundWrappers))
			{
				this->soundWrappers = new VirtualList();
			}

			VirtualList::pushBack(this->soundWrappers, soundWrapper);

			if(!SoundWrapper::isTurnedOn(soundWrapper))
			{
				SoundWrapper::play(soundWrapper, NULL, kSoundWrapperPlaybackFadeIn);
			}
		}
	}
}

void Stage::onSoundWrapperReleased(ListenerObject eventFirer __attribute__((unused)))
{
	VirtualList::removeElement(this->soundWrappers, eventFirer);

	Stage::fireEvent(this, kEventSoundReleased);
}

void Stage::setupTimer()
{
	HardwareManager::setupTimer(HardwareManager::getInstance(), this->stageSpec->timer.resolution, this->stageSpec->timer.timePerInterrupt, this->stageSpec->timer.timePerInterruptUnits);
}

bool Stage::handlePropagatedMessage(int32 message)
{
	if(!isDeleted(this->uiContainer))
	{
		// propagate message to ui
		return Container::propagateMessage(this->uiContainer, Container::onPropagatedMessage, message);
	}

	return false;
}

bool Stage::handlePropagatedString(const char* string)
{
	if(!isDeleted(this->uiContainer))
	{
		// propagate message to ui
		return Container::propagateMessage(this->uiContainer, Container::onPropagatedString, string);
	}

	return false;
}

void Stage::onFocusEntityDeleted(ListenerObject eventFirer __attribute__ ((unused)))
{
	this->focusEntity = NULL;

	if(this->focusEntity && Camera::getFocusEntity(Camera::getInstance()))
	{
		if(this->focusEntity == Camera::getFocusEntity(Camera::getInstance()))
		{
			Camera::setFocusGameEntity(Camera::getInstance(), NULL);
		}
	}
}

void Stage::setFocusEntity(Entity focusEntity)
{
	if(this->focusEntity)
	{
		Entity::removeEventListener(this->focusEntity, ListenerObject::safeCast(this), (EventListener)Stage_onFocusEntityDeleted, kEventContainerDeleted);
	}

	this->focusEntity = focusEntity;

	if(this->focusEntity)
	{
		Entity::addEventListener(this->focusEntity, ListenerObject::safeCast(this), (EventListener)Stage_onFocusEntityDeleted, kEventContainerDeleted);

		Vector3D focusEntityPosition = *Container::getGlobalPosition(this->focusEntity);
		focusEntityPosition.x = __METERS_TO_PIXELS(focusEntityPosition.x);
		focusEntityPosition.y = __METERS_TO_PIXELS(focusEntityPosition.y);
		focusEntityPosition.z = __METERS_TO_PIXELS(focusEntityPosition.z);

		this->cameraPreviousDistance = (long)focusEntityPosition.x * (long)focusEntityPosition.x +
											(long)focusEntityPosition.y * (long)focusEntityPosition.y +
											(long)focusEntityPosition.z * (long)focusEntityPosition.z;
	}
}

// get stage spec
StageSpec* Stage::getStageSpec()
{
	return this->stageSpec;
}

void Stage::forceNoPopIn(bool forceNoPopIn)
{
	this->forceNoPopIn = forceNoPopIn;
}

VirtualList Stage::getStageEntityDescriptions()
{
	return this->stageEntityDescriptions;
}

void Stage::showStreamingProfiling(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "STREAMING STATUS", x, y++, NULL);

	Printing::text(Printing::getInstance(), "Stage's status", x, ++y, NULL);

	int32 originalY __attribute__ ((unused)) = y;
	int32 xDisplacement = 21;
	y++;

	Printing::text(Printing::getInstance(), "Registered entities:            ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->stageEntityDescriptions), x + xDisplacement, y++, NULL);
	Printing::text(Printing::getInstance(), "Child entities:                 ", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->children), x + xDisplacement, y++, NULL);

#ifdef __PROFILE_STREAMING

	xDisplacement = 10;

	Printing::text(Printing::getInstance(), "Process duration (ms)", x, ++y, NULL);
	y++;

	Printing::text(Printing::getInstance(), "Unload:           ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), unloadOutOfRangeEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "Load:             ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), loadInRangeEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "Removing:         ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), processRemovedEntitiesHighestTime, x + xDisplacement, y, NULL);

	Printing::text(Printing::getInstance(), "Factory:          ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), entityFactoryHighestTime, x + xDisplacement, y++, NULL);

	unloadOutOfRangeEntitiesHighestTime = 0;
	loadInRangeEntitiesHighestTime = 0;
	processRemovedEntitiesHighestTime = 0;
	entityFactoryHighestTime = 0;
#endif
}
