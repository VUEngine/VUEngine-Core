/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <string.h>

#include <BgmapTexture.h>
#include <BgmapTextureManager.h>
#include <Camera.h>
#include <CharSetManager.h>
#include <DebugConfig.h>
#include <Entity.h>
#include <EntityFactory.h>
#include <HardwareManager.h>
#include <ParamTableManager.h>
#include <BodyManager.h>
#include <Printing.h>
#include <SpriteManager.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <UIContainer.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VIPManager.h>
#include <VUEngine.h>

#include "Stage.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class Container;
friend class Entity;
friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __STREAMING_CYCLES		5

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

typedef bool (*StreamingPhase)(void*, int32);

typedef struct EntityLoadingListener
{
	ListenerObject scope;
	EventListener callback;
} EntityLoadingListener;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const Transformation _neutralEnvironmentTransformation =
{
	// spatial position
	{0, 0, 0},
 
	// spatial rotation
	{0, 0, 0},
 
	// spatial scale
	{__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9},

	// invalidty flag
	__VALID_TRANSFORMATION
};

static const StreamingPhase _streamingPhases[] =
{
	&Stage::unloadOutOfRangeEntities,
	&Stage::loadInRangeEntities
};

#ifdef __PROFILE_STREAMING
extern int16 _renderingProcessTimeHelper;
static uint32 unloadOutOfRangeEntitiesHighestTime = 0;
static uint32 loadInRangeEntitiesHighestTime = 0;
static uint32 processRemovedEntitiesHighestTime = 0;
static uint32 entityFactoryHighestTime = 0;
static uint32 timeBeforeProcess = 0;
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 Stage::computeDistanceToOrigin(StageEntityDescription* stageEntityDescription)
{
	
	int32 x = 
		stageEntityDescription->positionedEntity->onScreenPosition.x - 
		__METERS_TO_PIXELS(stageEntityDescription->rightBox.x1 - stageEntityDescription->rightBox.x0) / 2;
	
	int32 y = 
		stageEntityDescription->positionedEntity->onScreenPosition.y - 
		__METERS_TO_PIXELS(stageEntityDescription->rightBox.y1 - stageEntityDescription->rightBox.y0) / 2;
	
	int32 z = 
		stageEntityDescription->positionedEntity->onScreenPosition.z - 
		__METERS_TO_PIXELS(stageEntityDescription->rightBox.z1 - stageEntityDescription->rightBox.z0) / 2;

	return x * x + y * y + z * z;
} 

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::constructor(StageSpec *stageSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(0, NULL);

	this->entityFactory = new EntityFactory();
	this->children = new VirtualList();
	this->entityLoadingListeners = NULL;

	this->stageSpec = stageSpec;
	this->stageEntityDescriptions = NULL;
	this->focusEntity = NULL;
	this->streamingHeadNode = NULL;
	this->nextEntityId = 0;
	this->streamingPhase = 0;
	this->sounds = NULL;
	this->streamingAmplitude = this->stageSpec->streaming.streamingAmplitude;
	this->reverseStreaming = false;
	this->cameraPosition = Vector3D::getFromPixelVector(this->stageSpec->level.cameraInitialPosition);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::destructor()
{
	this->deleteMe = true;
	
	Stage::setFocusEntity(this, NULL);

	if(!isDeleted(this->entityLoadingListeners))
	{
		VirtualList::deleteData(this->entityLoadingListeners);
		delete this->entityLoadingListeners;
		this->entityLoadingListeners = NULL;
	}

	if(!isDeleted(this->sounds))
	{
		// Do not need to release sound wrappers here,
		// they are taken care by the SoundManager when
		// I called SoundManager::stopAllSounds
		for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
		{
			Sound sound = Sound::safeCast(node->data);

			if(!isDeleted(sound))
			{
				Sound::removeEventListenerScopes(sound, ListenerObject::safeCast(this), kEventSoundReleased);
			}
		}

		delete this->sounds;
		this->sounds = NULL;
	}

	if(!isDeleted(this->entityFactory))
	{
		delete this->entityFactory;
		this->entityFactory = NULL;
	}

	if(!isDeleted(this->stageEntityDescriptions))
	{
		VirtualList::deleteData(this->stageEntityDescriptions);
		delete this->stageEntityDescriptions;
		this->stageEntityDescriptions = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::suspend()
{
	// Save the camera position for resume reconfiguration
	this->cameraPosition = Camera::getPosition(Camera::getInstance());

	// stream all pending entities to avoid having manually recover
	// the stage entity registries
	while(EntityFactory::createNextEntity(this->entityFactory));

	Base::suspend(this);

	// relinquish camera focus priority
	if(!isDeleted(this->focusEntity))
	{
		if(this->focusEntity == Camera::getFocusEntity(Camera::getInstance()))
		{
			// relinquish focus entity
			Camera::setFocusEntity(Camera::getInstance(), NULL);
		}
	}
	else
	{
		Stage::setFocusEntity(this, Camera::getFocusEntity(Camera::getInstance()));
	}

	delete this->entityFactory;
	this->entityFactory = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::resume()
{
	// Set camera to its previous position
	Camera::setStageSize(Camera::getInstance(), Size::getFromPixelSize(this->stageSpec->level.pixelSize));
	Camera::setPosition(Camera::getInstance(), this->cameraPosition, true);
	Camera::setup(Camera::getInstance(), this->stageSpec->rendering.pixelOptical, this->stageSpec->level.cameraFrustum);

	// Setup timer
	Stage::configureTimer(this);

	// load background sounds
	Stage::setupSounds(this);

	// set physics
	BodyManager::setFrictionCoefficient(VUEngine::getBodyManager(_vuEngine), this->stageSpec->physics.frictionCoefficient);
	BodyManager::setGravity(VUEngine::getBodyManager(_vuEngine), this->stageSpec->physics.gravity);

	Stage::prepareGraphics(this);

	if(!isDeleted(this->focusEntity))
	{
		// recover focus entity
		Camera::setFocusEntity(Camera::getInstance(), Entity::safeCast(this->focusEntity));
	}

	Base::resume(this);

	// apply transformations
	Stage::transform(this, &_neutralEnvironmentTransformation, __INVALIDATE_TRANSFORMATION);

	// setup colors and brightness
	VIPManager::setBackgroundColor(VIPManager::getInstance(), this->stageSpec->rendering.colorConfig.backgroundColor);
	// TODO: properly handle brightness and brightness repeat on resume

	this->entityFactory = new EntityFactory();

	Stage::loadPostProcessingEffects(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

StageSpec* Stage::getSpec()
{
	return this->stageSpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::configureTimer()
{
	TimerManager::configure
	(
		TimerManager::getInstance(), this->stageSpec->timer.resolution, this->stageSpec->timer.targetTimePerInterrupt, 
		this->stageSpec->timer.targetTimePerInterrupttUnits
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::configurePalettes()
{
	VIPManager::configurePalettes(VIPManager::getInstance(), &this->stageSpec->rendering.paletteConfig);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

PaletteConfig Stage::getPaletteConfig()
{
	return this->stageSpec->rendering.paletteConfig;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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
			(__PIXELS_TO_METERS(stageEntityDescription->positionedEntity->onScreenPosition.x) - 
			(stageEntityDescription->rightBox.x1 - stageEntityDescription->rightBox.x0) / 2),
			(__PIXELS_TO_METERS(stageEntityDescription->positionedEntity->onScreenPosition.y) - 
			(stageEntityDescription->rightBox.y1 - stageEntityDescription->rightBox.y0) / 2),
			(__PIXELS_TO_METERS(stageEntityDescription->positionedEntity->onScreenPosition.z) - 
			(stageEntityDescription->rightBox.z1 - stageEntityDescription->rightBox.z0) / 2)
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
				(
					__PIXELS_TO_METERS(auxStageEntityDescription->positionedEntity->onScreenPosition.x) - 
					(auxStageEntityDescription->rightBox.x1 - auxStageEntityDescription->rightBox.x0) / 2
				),
				(
					__PIXELS_TO_METERS(auxStageEntityDescription->positionedEntity->onScreenPosition.y) - 
					(auxStageEntityDescription->rightBox.y1 - auxStageEntityDescription->rightBox.y0) / 2
				),
				(
					__PIXELS_TO_METERS(auxStageEntityDescription->positionedEntity->onScreenPosition.z) - 
					(auxStageEntityDescription->rightBox.z1 - auxStageEntityDescription->rightBox.z0) / 2
				)
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualList Stage::getStageEntityDescriptions()
{
	return this->stageEntityDescriptions;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::addEntityLoadingListener(ListenerObject scope, EventListener callback)
{
	if(isDeleted(scope) || NULL == callback)
	{
		return;
	}

	if(isDeleted(this->entityLoadingListeners))
	{
		this->entityLoadingListeners = new VirtualList();
	}

	EntityLoadingListener* entityLoadingListener = new EntityLoadingListener;
	entityLoadingListener->scope = scope;
	entityLoadingListener->callback = callback;

	VirtualList::pushBack(this->entityLoadingListeners, entityLoadingListener);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Entity Stage::spawnChildEntity(const PositionedEntity* const positionedEntity, bool permanent)
{
	return Stage::doAddChildEntity(this, positionedEntity, permanent, this->nextEntityId++);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::destroyChildEntity(Entity child)
{
	NM_ASSERT(!isDeleted(child), "Stage::removeEntity: null child");

	if(isDeleted(child))
	{
		return;
	}

	int16 internalId = Entity::getInternalId(child);

	Stage::removeChild(this, Container::safeCast(child), true);

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::streamAll()
{
	Stage::transform(this, &_neutralEnvironmentTransformation, __INVALIDATE_TRANSFORMATION);

	this->streamingHeadNode = NULL;

	// Make sure that the entity factory doesn't have any pending operations
	while(EntityFactory::createNextEntity(this->entityFactory));

	do
	{
		this->streamingHeadNode = NULL;

		Stage::purgeChildren(this);

		VUEngine::prepareGraphics(VUEngine::getInstance());

	}while(Stage::unloadOutOfRangeEntities(this, false));

	this->streamingHeadNode = NULL;
	this->streamingAmplitude = (uint16)-1;

	while(Stage::loadInRangeEntities(this, false));

	while(EntityFactory::createNextEntity(this->entityFactory))
	{
		Stage::transform(this, &_neutralEnvironmentTransformation, __INVALIDATE_TRANSFORMATION);

		VUEngine::prepareGraphics(VUEngine::getInstance());
	}

	VUEngine::prepareGraphics(VUEngine::getInstance());

	this->streamingAmplitude = this->stageSpec->streaming.streamingAmplitude;
	this->streamingHeadNode = NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualList Stage::getSounds()
{
	return this->sounds;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::fadeSounds(uint32 playbackType)
{
	if(!isDeleted(this->sounds))
	{
		// Do not need to release sound wrappers here,
		// they are taken care by the SoundManager when
		// I called SoundManager::stopAllSounds
		for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
		{
			Sound sound = Sound::safeCast(node->data);

			if(!isDeleted(sound))
			{
				Sound::removeEventListenerScopes(sound, ListenerObject::safeCast(this), kEventSoundReleased);
				Sound::play(sound, NULL, playbackType);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void Stage::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "STREAMING STATUS", x, y++, NULL);

	Printing::text(Printing::getInstance(), "Stage's status", x, ++y, NULL);

	int32 originalY __attribute__ ((unused)) = y;
	int32 xDisplacement = 21;
	y++;

	Printing::text(Printing::getInstance(), "Registered entities:            ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->stageEntityDescriptions), x + xDisplacement, y++, NULL);
	Printing::text(Printing::getInstance(), "Child entities:                 ", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->children), x + xDisplacement, y++, NULL);

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
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::stream()
{
#ifdef __SHOW_STREAMING_PROFILING
	if(!VUEngine::isInToolState(_vuEngine))
	{
		EntityFactory::print(this->entityFactory, 25, 3);
	}
#endif

	if(NULL == this->stageEntityDescriptions->head)
	{
		return false;
	}

	if(Stage::updateEntityFactory(this))
	{
		return true;
	}

	int32 streamingPhases = sizeof(_streamingPhases) / sizeof(StreamingPhase);

	if(++this->streamingPhase >= streamingPhases)
	{
		this->streamingPhase = 0;
	}

	return _streamingPhases[this->streamingPhase](this, this->stageSpec->streaming.deferred);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::configure(VirtualList positionedEntitiesToIgnore)
{
	// Setup timer
	Stage::configureTimer(this);

	// load background music
	Stage::setupSounds(this);

	Camera::reset(Camera::getInstance());
	Camera::setStageSize(Camera::getInstance(), Size::getFromPixelSize(this->stageSpec->level.pixelSize));
	Camera::setPosition(Camera::getInstance(), this->cameraPosition, true);

	// set optical values
	Camera::setup(Camera::getInstance(), this->stageSpec->rendering.pixelOptical, this->stageSpec->level.cameraFrustum);

	// set physics
	BodyManager::setFrictionCoefficient(VUEngine::getBodyManager(_vuEngine), this->stageSpec->physics.frictionCoefficient);
	BodyManager::setGravity(VUEngine::getBodyManager(_vuEngine), this->stageSpec->physics.gravity);

	// preload graphics
	Stage::prepareGraphics(this);

	// register all the entities in the stage's spec
	Stage::registerEntities(this, positionedEntitiesToIgnore);

	// load entities
	Stage::loadInitialEntities(this);

	// retrieve focus entity for streaming
	Stage::setFocusEntity(this, Camera::getFocusEntity(Camera::getInstance()));

	// setup colors and brightness
	VIPManager::setBackgroundColor(VIPManager::getInstance(), this->stageSpec->rendering.colorConfig.backgroundColor);
	VIPManager::setupBrightness(VIPManager::getInstance(), &this->stageSpec->rendering.colorConfig.brightness);
	VIPManager::setupBrightnessRepeat(VIPManager::getInstance(), this->stageSpec->rendering.colorConfig.brightnessRepeat);

	// apply transformations
	Stage::transform(this, &_neutralEnvironmentTransformation, __INVALIDATE_TRANSFORMATION);

	Stage::loadPostProcessingEffects(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::unloadOutOfRangeEntities(int32 defer __attribute__((unused)))
{
	if(isDeleted(this->children))
	{
		return false;
	}

#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getElapsedMilliseconds(TimerManager::getInstance());
#endif

	bool unloadedEntities = false;

	VirtualNode node = this->children->head;

	CACHE_RESET;

	// check which entites must be unloaded
	for(; NULL != node; node = node->next)
	{
		// get next entity
		Entity entity = Entity::safeCast(node->data);

		if(entity->dontStreamOut)
		{
			continue;
		}

		// if the entity isn't visible inside the view field, unload it
		if(!entity->deleteMe && entity->parent == Container::safeCast(this))
		{
			if(Entity::isInCameraRange(entity, this->stageSpec->streaming.loadPadding + this->stageSpec->streaming.unloadPadding, true))
			{
				continue;
			}

			VirtualNode auxNode = this->stageEntityDescriptions->head;
			StageEntityDescription* stageEntityDescription = NULL;

			for(; auxNode; auxNode = auxNode->next)
			{
				StageEntityDescription* auxStageEntityDescription = (StageEntityDescription*)auxNode->data;

				if(auxStageEntityDescription->internalId == entity->internalId)
				{
					stageEntityDescription = auxStageEntityDescription;
					break;
				}
			}

			if(NULL != stageEntityDescription)
			{
				if(stageEntityDescription->positionedEntity->loadRegardlessOfPosition)
				{
					continue;
				}

				stageEntityDescription->internalId = -1;
			}

			// unload it
			Stage::destroyChildEntity(this, entity);

			// remove from list of entities that are to be loaded by the streaming,
			// if the entity is not to be alwaysStreamIned
			if(!Entity::alwaysStreamIn(entity))
			{
				VirtualList::removeNode(this->stageEntityDescriptions, auxNode);
			}

			unloadedEntities = true;
		}
	}

#ifdef __PROFILE_STREAMING
		uint32 processTime = 
			-_renderingProcessTimeHelper + TimerManager::getElapsedMilliseconds(TimerManager::getInstance()) - timeBeforeProcess;
		
		unloadOutOfRangeEntitiesHighestTime = 
			processTime > unloadOutOfRangeEntitiesHighestTime ? processTime : unloadOutOfRangeEntitiesHighestTime;
#endif

	return unloadedEntities;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::loadInRangeEntities(int32 defer)
{
#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getElapsedMilliseconds(TimerManager::getInstance());
#endif

	bool loadedEntities = false;

	CACHE_RESET;

	if(this->reverseStreaming)
	{
		if(NULL == this->streamingHeadNode)
		{
			this->streamingHeadNode = this->stageEntityDescriptions->tail;
		}

		bool negativeStreamingAmplitude = 0 > ((int16)this->streamingAmplitude);

		for
		(
			uint16 counter = 0; counter < this->streamingAmplitude; 
			this->streamingHeadNode = this->streamingHeadNode->previous, 
			counter++
		)
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
				if
				(
					Stage::isEntityInLoadRange
					(
						this, stageEntityDescription->positionedEntity->onScreenPosition, stageEntityDescription->validRightBox ? 
						&stageEntityDescription->rightBox : NULL
					)
				)
				{
					loadedEntities = true;

					stageEntityDescription->internalId = this->nextEntityId++;

					if(defer)
					{
						EntityFactory::spawnEntity
						(
							this->entityFactory, stageEntityDescription->positionedEntity, Container::safeCast(this), 
							!isDeleted(this->entityLoadingListeners) ? 
								(EventListener)Stage::onEntityLoaded 
								: 
								NULL, stageEntityDescription->internalId
						);
					}
					else
					{
						Stage::doAddChildEntity
						(
							this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId
						);
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
				if
				(
					Stage::isEntityInLoadRange
					(
						this, stageEntityDescription->positionedEntity->onScreenPosition, 
						stageEntityDescription->validRightBox ? 
							&stageEntityDescription->rightBox : NULL
					)
				)
				{
					loadedEntities = true;

					stageEntityDescription->internalId = this->nextEntityId++;

					if(defer)
					{
						EntityFactory::spawnEntity
						(
							this->entityFactory, stageEntityDescription->positionedEntity, Container::safeCast(this), 
							!isDeleted(this->entityLoadingListeners) ? 
								(EventListener)Stage::onEntityLoaded : NULL, 
							stageEntityDescription->internalId
						);
					}
					else
					{
						Stage::doAddChildEntity
						(
							this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId
						);
						break;
					}
				}
			}
		}
	}

#ifdef __PROFILE_STREAMING
	uint32 processTime = 
		-_renderingProcessTimeHelper + TimerManager::getElapsedMilliseconds(TimerManager::getInstance()) - timeBeforeProcess;
	loadInRangeEntitiesHighestTime = processTime > loadInRangeEntitiesHighestTime ? processTime : loadInRangeEntitiesHighestTime;
#endif

	return loadedEntities;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::loadInitialEntities()
{
	// need a temporary list to remove and delete entities
	VirtualNode node = this->stageEntityDescriptions->head;

	for(; NULL != node; node = node->next)
	{
		StageEntityDescription* stageEntityDescription = (StageEntityDescription*)node->data;

		if(-1 == stageEntityDescription->internalId)
		{
			// if entity in load range
			if
			(
				stageEntityDescription->positionedEntity->loadRegardlessOfPosition 
				|| 
				Stage::isEntityInLoadRange
				(
					this, stageEntityDescription->positionedEntity->onScreenPosition, &stageEntityDescription->rightBox
				)
			)
			{
				stageEntityDescription->internalId = this->nextEntityId++;
				Entity entity = 
					Stage::doAddChildEntity(this, stageEntityDescription->positionedEntity, false, stageEntityDescription->internalId);
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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Entity Stage::doAddChildEntity(const PositionedEntity* const positionedEntity, bool permanent __attribute__ ((unused)), int16 internalId)
{
	if(NULL != positionedEntity)
	{
		Entity entity = Entity::createEntity(positionedEntity, internalId);
		ASSERT(entity, "Stage::doAddChildEntity: entity not loaded");

		if(!isDeleted(entity))
		{
			// create the entity and add it to the world
			Stage::addChild(this, Container::safeCast(entity));

			entity->dontStreamOut = entity->dontStreamOut || permanent;
			
			Stage::alertOfLoadedEntity(this, entity);
		}

		return entity;
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::onEntityLoaded(ListenerObject eventFirer)
{
	Entity entity = Entity::safeCast(eventFirer);

	if(!isDeleted(entity) && !isDeleted(this->entityLoadingListeners))
	{
		Stage::alertOfLoadedEntity(this, entity);
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::alertOfLoadedEntity(Entity entity)
{
	if(isDeleted(entity) || isDeleted(this->entityLoadingListeners))
	{
		return;
	}

	for(VirtualNode node = this->entityLoadingListeners->head; NULL != node; node = node->next)
	{
		EntityLoadingListener* entityLoadingListener = (EntityLoadingListener*)node->data;

		if(!isDeleted(entityLoadingListener->scope))
		{
			Entity::addEventListener(entity, entityLoadingListener->scope, entityLoadingListener->callback, kEventEntityLoaded);
		}
	}

	Entity::fireEvent(entity, kEventEntityLoaded);
	NM_ASSERT(!isDeleted(entity), "Stage::alertOfLoadedEntity: deleted entity during kEventEntityLoaded");
	Entity::removeEventListeners(entity, NULL, kEventEntityLoaded);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 Stage::isEntityInLoadRange(ScreenPixelVector onScreenPosition, const RightBox* rightBox)
{
	if(NULL == rightBox)
	{
		fixed_t padding = __PIXELS_TO_METERS(this->stageSpec->streaming.loadPadding) >> 1;
		
		RightBox helperRightBox =
		{
			-padding, padding,
			-padding, padding,
			-padding, padding
		};

		return Entity::isInsideFrustrum(Vector3D::getFromScreenPixelVector(onScreenPosition), helperRightBox);
	}
	else
	{
		return Entity::isInsideFrustrum(Vector3D::getFromScreenPixelVector(onScreenPosition), *rightBox);
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::updateEntityFactory()
{
#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getElapsedMilliseconds(TimerManager::getInstance());
#endif

	bool preparingEntities = EntityFactory::createNextEntity(this->entityFactory);

#ifdef __PROFILE_STREAMING
	uint32 processTime = 
		-_renderingProcessTimeHelper + TimerManager::getElapsedMilliseconds(TimerManager::getInstance()) - timeBeforeProcess;
	entityFactoryHighestTime = processTime > entityFactoryHighestTime ? processTime : entityFactoryHighestTime;
#endif

	return preparingEntities;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

StageEntityDescription* Stage::registerEntity(PositionedEntity* positionedEntity)
{
	ASSERT(positionedEntity, "Stage::registerEntity: null positionedEntity");

	StageEntityDescription* stageEntityDescription = new StageEntityDescription;

	stageEntityDescription->extraInfo = NULL;
	stageEntityDescription->internalId = -1;
	stageEntityDescription->positionedEntity = positionedEntity;

	Vector3D environmentPosition = Vector3D::zero();
	stageEntityDescription->rightBox = Entity::getRightBoxFromSpec(stageEntityDescription->positionedEntity, &environmentPosition);

	stageEntityDescription->validRightBox = 
		(0 != stageEntityDescription->rightBox.x1 - stageEntityDescription->rightBox.x0) 
		|| 
		(0 != stageEntityDescription->rightBox.y1 - stageEntityDescription->rightBox.y0) 
		|| 
		(0 != stageEntityDescription->rightBox.z1 - stageEntityDescription->rightBox.z0);

	fixed_t padding = __PIXELS_TO_METERS(this->stageSpec->streaming.loadPadding);
	
	// Bake the padding in the bounding box to save on performance
	stageEntityDescription->rightBox.x0 -= padding;
	stageEntityDescription->rightBox.x1 += padding;
	stageEntityDescription->rightBox.y0 -= padding;
	stageEntityDescription->rightBox.y1 += padding;
	stageEntityDescription->rightBox.z0 -= padding;
	stageEntityDescription->rightBox.z1 += padding;

	return stageEntityDescription;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::prepareGraphics()
{
	// Must clean DRAM
	SpriteManager::reset(SpriteManager::getInstance());

	// set palettes
	Stage::configurePalettes(this);

	// setup OBJs
	SpriteManager::setupObjectSpriteContainers
	(
		SpriteManager::getInstance(), this->stageSpec->rendering.objectSpritesContainersSize,
		this->stageSpec->rendering.objectSpritesContainersZPosition
	);

	// preload textures
	Stage::preloadAssets(this);

	// setup SpriteManager's configuration
	SpriteManager::setTexturesMaximumRowsToWrite(SpriteManager::getInstance(), this->stageSpec->rendering.texturesMaximumRowsToWrite);
	SpriteManager::setMaximumParamTableRowsToComputePerCall
	(
		SpriteManager::getInstance(), this->stageSpec->rendering.maximumAffineRowsToComputePerCall
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::preloadAssets()
{
	Printing::loadFonts(Printing::getInstance(), this->stageSpec->assets.fontSpecs);
	CharSetManager::loadCharSets(CharSetManager::getInstance(), (const CharSetSpec**)this->stageSpec->assets.charSetSpecs);
	BgmapTextureManager::loadTextures(BgmapTextureManager::getInstance(), (const TextureSpec**)this->stageSpec->assets.textureSpecs);
	ParamTableManager::setup(ParamTableManager::getInstance(), this->stageSpec->rendering.paramTableSegments);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::setupSounds()
{
	SoundManager::unlock(SoundManager::getInstance());
	SoundManager::setPCMTargetPlaybackRefreshRate(SoundManager::getInstance(), this->stageSpec->sound.pcmTargetPlaybackRefreshRate);

	int32 i = 0;

	// stop all sounds
	SoundManager::stopAllSounds(SoundManager::getInstance(), true, this->stageSpec->assets.sounds);

	for(; NULL != this->stageSpec->assets.sounds[i]; i++)
	{
		Sound sound = 
			SoundManager::findSound
			(
				SoundManager::getInstance(), this->stageSpec->assets.sounds[i], 
				(EventListener)Stage::onSoundReleased, ListenerObject::safeCast(this)
			);

		if(isDeleted(sound))
		{
			sound = 
				SoundManager::getSound
				(
					SoundManager::getInstance(), this->stageSpec->assets.sounds[i], 
					(EventListener)Stage::onSoundReleased, ListenerObject::safeCast(this)
				);
		}

		if(!isDeleted(sound))
		{
			if(isDeleted(this->sounds))
			{
				this->sounds = new VirtualList();
			}

			VirtualList::pushBack(this->sounds, sound);

			if(!Sound::isPlaying(sound))
			{
				Sound::play(sound, NULL, kSoundPlaybackFadeIn);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::pauseSounds()
{
	if(!isDeleted(this->sounds))
	{
		// Do not need to release sound wrappers here,
		// they are taken care by the SoundManager when
		// I called SoundManager::stopAllSounds
		for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
		{
			Sound sound = Sound::safeCast(node->data);

			if(!isDeleted(sound))
			{
				Sound::removeEventListenerScopes(sound, ListenerObject::safeCast(this), kEventSoundReleased);
				Sound::pause(sound);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::unpauseSounds()
{
	if(!isDeleted(this->sounds))
	{
		// Do not need to release sound wrappers here,
		// they are taken care by the SoundManager when
		// I called SoundManager::stopAllSounds
		for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
		{
			Sound sound = Sound::safeCast(node->data);

			if(!isDeleted(sound))
			{
				Sound::removeEventListenerScopes(sound, ListenerObject::safeCast(this), kEventSoundReleased);
				Sound::unpause(sound);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::onSoundReleased(ListenerObject eventFirer __attribute__((unused)))
{
	VirtualList::removeData(this->sounds, eventFirer);

	Stage::fireEvent(this, kEventSoundReleased);

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::setFocusEntity(Entity focusEntity)
{
	if(!isDeleted(this->focusEntity))
	{
		Entity::removeEventListener
		(
			this->focusEntity, ListenerObject::safeCast(this), (EventListener)Stage::onFocusEntityDeleted, kEventContainerDeleted
		);
	}

	this->focusEntity = focusEntity;

	if(!isDeleted(this->focusEntity))
	{
		Entity::addEventListener
		(
			this->focusEntity, ListenerObject::safeCast(this), (EventListener)Stage::onFocusEntityDeleted, kEventContainerDeleted
		);

		Vector3D focusEntityPosition = *Container::getPosition(this->focusEntity);
		focusEntityPosition.x = __METERS_TO_PIXELS(focusEntityPosition.x);
		focusEntityPosition.y = __METERS_TO_PIXELS(focusEntityPosition.y);
		focusEntityPosition.z = __METERS_TO_PIXELS(focusEntityPosition.z);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::onFocusEntityDeleted(ListenerObject eventFirer __attribute__ ((unused)))
{
	if(!isDeleted(this->focusEntity) && ListenerObject::safeCast(this->focusEntity) == eventFirer)
	{
		if(this->focusEntity == Camera::getFocusEntity(Camera::getInstance()))
		{
			Camera::setFocusEntity(Camera::getInstance(), NULL);
		}
	}

	this->focusEntity = NULL;

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
