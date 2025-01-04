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
#include <Actor.h>
#include <ActorFactory.h>
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
friend class Actor;
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

typedef struct ActorLoadingListener
{
	ListenerObject scope;
	EventListener callback;
} ActorLoadingListener;

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
static uint32 actorFactoryHighestTime = 0;
static uint32 timeBeforeProcess = 0;
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static uint32 Stage::computeDistanceToOrigin(StageActorDescription* stageActorDescription)
{
	
	int32 x = 
		stageActorDescription->positionedActor->onScreenPosition.x - 
		__METERS_TO_PIXELS(stageActorDescription->rightBox.x1 - stageActorDescription->rightBox.x0) / 2;
	
	int32 y = 
		stageActorDescription->positionedActor->onScreenPosition.y - 
		__METERS_TO_PIXELS(stageActorDescription->rightBox.y1 - stageActorDescription->rightBox.y0) / 2;
	
	int32 z = 
		stageActorDescription->positionedActor->onScreenPosition.z - 
		__METERS_TO_PIXELS(stageActorDescription->rightBox.z1 - stageActorDescription->rightBox.z0) / 2;

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

	this->actorFactory = new ActorFactory();
	this->children = new VirtualList();
	this->actorLoadingListeners = NULL;

	this->stageSpec = stageSpec;
	this->stageActorDescriptions = NULL;
	this->focusActor = NULL;
	this->streamingHeadNode = NULL;
	this->nextActorId = 0;
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
	
	Stage::setFocusActor(this, NULL);

	if(!isDeleted(this->actorLoadingListeners))
	{
		VirtualList::deleteData(this->actorLoadingListeners);
		delete this->actorLoadingListeners;
		this->actorLoadingListeners = NULL;
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

	if(!isDeleted(this->actorFactory))
	{
		delete this->actorFactory;
		this->actorFactory = NULL;
	}

	if(!isDeleted(this->stageActorDescriptions))
	{
		VirtualList::deleteData(this->stageActorDescriptions);
		delete this->stageActorDescriptions;
		this->stageActorDescriptions = NULL;
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
	// the stage actor registries
	while(ActorFactory::createNextActor(this->actorFactory));

	Base::suspend(this);

	// relinquish camera focus priority
	if(!isDeleted(this->focusActor))
	{
		if(this->focusActor == Camera::getFocusActor(Camera::getInstance()))
		{
			// relinquish focus actor
			Camera::setFocusActor(Camera::getInstance(), NULL);
		}
	}
	else
	{
		Stage::setFocusActor(this, Camera::getFocusActor(Camera::getInstance()));
	}

	delete this->actorFactory;
	this->actorFactory = NULL;
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

	if(!isDeleted(this->focusActor))
	{
		// recover focus actor
		Camera::setFocusActor(Camera::getInstance(), Actor::safeCast(this->focusActor));
	}

	Base::resume(this);

	// apply transformations
	Stage::transform(this, &_neutralEnvironmentTransformation, __INVALIDATE_TRANSFORMATION);

	// setup colors and brightness
	VIPManager::setBackgroundColor(VIPManager::getInstance(), this->stageSpec->rendering.colorConfig.backgroundColor);
	// TODO: properly handle brightness and brightness repeat on resume

	this->actorFactory = new ActorFactory();

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
	if(!isDeleted(this->stageActorDescriptions))
	{
		return;
	}

	this->stageActorDescriptions = new VirtualList();

	// register entities ordering them according to their distances to the origin
	int32 i = 0;

	for(;this->stageSpec->entities.children[i].actorSpec; i++)
	{
		if(positionedEntitiesToIgnore)
		{
			VirtualNode node = positionedEntitiesToIgnore->head;

			for(; NULL != node; node = node->next)
			{
				if(&this->stageSpec->entities.children[i] == (PositionedActor*)node->data)
				{
					break;
				}
			}

			if(node)
			{
				continue;
			}
		}

		StageActorDescription* stageActorDescription = Stage::registerActor(this, &this->stageSpec->entities.children[i]);

		Vector3D stageActorPosition = (Vector3D)
		{
			(__PIXELS_TO_METERS(stageActorDescription->positionedActor->onScreenPosition.x) - 
			(stageActorDescription->rightBox.x1 - stageActorDescription->rightBox.x0) / 2),
			(__PIXELS_TO_METERS(stageActorDescription->positionedActor->onScreenPosition.y) - 
			(stageActorDescription->rightBox.y1 - stageActorDescription->rightBox.y0) / 2),
			(__PIXELS_TO_METERS(stageActorDescription->positionedActor->onScreenPosition.z) - 
			(stageActorDescription->rightBox.z1 - stageActorDescription->rightBox.z0) / 2)
		};

		VirtualNode closestEnitryDescriptionNode = NULL;
		VirtualNode auxNode = this->stageActorDescriptions->head;
		StageActorDescription* auxStageActorDescription = (StageActorDescription*)auxNode->data;

		fixed_ext_t closestDistance = 0;

		for(; auxNode; auxNode = auxNode->next)
		{
			auxStageActorDescription = (StageActorDescription*)auxNode->data;

			Vector3D auxStageActorPosition = (Vector3D)
			{
				(
					__PIXELS_TO_METERS(auxStageActorDescription->positionedActor->onScreenPosition.x) - 
					(auxStageActorDescription->rightBox.x1 - auxStageActorDescription->rightBox.x0) / 2
				),
				(
					__PIXELS_TO_METERS(auxStageActorDescription->positionedActor->onScreenPosition.y) - 
					(auxStageActorDescription->rightBox.y1 - auxStageActorDescription->rightBox.y0) / 2
				),
				(
					__PIXELS_TO_METERS(auxStageActorDescription->positionedActor->onScreenPosition.z) - 
					(auxStageActorDescription->rightBox.z1 - auxStageActorDescription->rightBox.z0) / 2
				)
			};

			fixed_ext_t squaredDistance = Vector3D::squareLength(Vector3D::get(stageActorPosition, auxStageActorPosition));

			if(NULL == closestEnitryDescriptionNode || closestDistance > squaredDistance)
			{
				closestEnitryDescriptionNode = auxNode;
				closestDistance = squaredDistance;
			}
		}

		if(NULL == auxNode)
		{
			VirtualList::pushBack(this->stageActorDescriptions, stageActorDescription);
		}
		else
		{
			uint32 stageActorDistanceToOrigin = Stage::computeDistanceToOrigin(stageActorDescription);
			uint32 auxStageActorDistanceToOrigin = Stage::computeDistanceToOrigin(auxStageActorDescription);

			if(stageActorDistanceToOrigin > auxStageActorDistanceToOrigin)
			{
				VirtualList::insertAfter(this->stageActorDescriptions, auxNode, stageActorDescription);
			}
			else
			{
				VirtualList::insertBefore(this->stageActorDescriptions, auxNode, stageActorDescription);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualList Stage::getStageActorDescriptions()
{
	return this->stageActorDescriptions;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::addActorLoadingListener(ListenerObject scope, EventListener callback)
{
	if(isDeleted(scope) || NULL == callback)
	{
		return;
	}

	if(isDeleted(this->actorLoadingListeners))
	{
		this->actorLoadingListeners = new VirtualList();
	}

	ActorLoadingListener* actorLoadingListener = new ActorLoadingListener;
	actorLoadingListener->scope = scope;
	actorLoadingListener->callback = callback;

	VirtualList::pushBack(this->actorLoadingListeners, actorLoadingListener);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Actor Stage::spawnChildActor(const PositionedActor* const positionedActor, bool permanent)
{
	return Stage::doAddChildActor(this, positionedActor, permanent, this->nextActorId++);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::destroyChildActor(Actor child)
{
	NM_ASSERT(!isDeleted(child), "Stage::removeActor: null child");

	if(isDeleted(child))
	{
		return;
	}

	int16 internalId = Actor::getInternalId(child);

	Stage::removeChild(this, Container::safeCast(child), true);

	VirtualNode node = this->stageActorDescriptions->head;

	for(; NULL != node; node = node->next)
	{
		StageActorDescription* stageActorDescription = (StageActorDescription*)node->data;

		if(stageActorDescription->internalId == internalId)
		{
			stageActorDescription->internalId = -1;
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
		VirtualList::removeNode(this->stageActorDescriptions, node);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::streamAll()
{
	Stage::transform(this, &_neutralEnvironmentTransformation, __INVALIDATE_TRANSFORMATION);

	this->streamingHeadNode = NULL;

	// Make sure that the actor factory doesn't have any pending operations
	while(ActorFactory::createNextActor(this->actorFactory));

	do
	{
		this->streamingHeadNode = NULL;

		Stage::purgeChildren(this);

		VUEngine::prepareGraphics(VUEngine::getInstance());

	}while(Stage::unloadOutOfRangeEntities(this, false));

	this->streamingHeadNode = NULL;
	this->streamingAmplitude = (uint16)-1;

	while(Stage::loadInRangeEntities(this, false));

	while(ActorFactory::createNextActor(this->actorFactory))
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
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->stageActorDescriptions), x + xDisplacement, y++, NULL);
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
	Printing::int32(Printing::getInstance(), actorFactoryHighestTime, x + xDisplacement, y++, NULL);

	unloadOutOfRangeEntitiesHighestTime = 0;
	loadInRangeEntitiesHighestTime = 0;
	processRemovedEntitiesHighestTime = 0;
	actorFactoryHighestTime = 0;
#endif
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::stream()
{
#ifdef __SHOW_STREAMING_PROFILING
	if(!VUEngine::isInToolState(_vuEngine))
	{
		ActorFactory::print(this->actorFactory, 25, 3);
	}
#endif

	if(NULL == this->stageActorDescriptions->head)
	{
		return false;
	}

	if(Stage::updateActorFactory(this))
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

	// retrieve focus actor for streaming
	Stage::setFocusActor(this, Camera::getFocusActor(Camera::getInstance()));

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
		// get next actor
		Actor actor = Actor::safeCast(node->data);

		if(actor->dontStreamOut)
		{
			continue;
		}

		// if the actor isn't visible inside the view field, unload it
		if(!actor->deleteMe && actor->parent == Container::safeCast(this))
		{
			if(Actor::isInCameraRange(actor, this->stageSpec->streaming.loadPadding + this->stageSpec->streaming.unloadPadding, true))
			{
				continue;
			}

			VirtualNode auxNode = this->stageActorDescriptions->head;
			StageActorDescription* stageActorDescription = NULL;

			for(; auxNode; auxNode = auxNode->next)
			{
				StageActorDescription* auxStageActorDescription = (StageActorDescription*)auxNode->data;

				if(auxStageActorDescription->internalId == actor->internalId)
				{
					stageActorDescription = auxStageActorDescription;
					break;
				}
			}

			if(NULL != stageActorDescription)
			{
				if(stageActorDescription->positionedActor->loadRegardlessOfPosition)
				{
					continue;
				}

				stageActorDescription->internalId = -1;
			}

			// unload it
			Stage::destroyChildActor(this, actor);

			// remove from list of entities that are to be loaded by the streaming,
			// if the actor is not to be alwaysStreamIned
			if(!Actor::alwaysStreamIn(actor))
			{
				VirtualList::removeNode(this->stageActorDescriptions, auxNode);
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
			this->streamingHeadNode = this->stageActorDescriptions->tail;
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
				this->streamingHeadNode = this->stageActorDescriptions->tail;

				if(negativeStreamingAmplitude)
				{
					break;
				}
			}

			StageActorDescription* stageActorDescription = (StageActorDescription*)this->streamingHeadNode->data;

			if(0 > stageActorDescription->internalId)
			{
				// if actor in load range
				if
				(
					Stage::isActorInLoadRange
					(
						this, stageActorDescription->positionedActor->onScreenPosition, stageActorDescription->validRightBox ? 
						&stageActorDescription->rightBox : NULL
					)
				)
				{
					loadedEntities = true;

					stageActorDescription->internalId = this->nextActorId++;

					if(defer)
					{
						ActorFactory::spawnActor
						(
							this->actorFactory, stageActorDescription->positionedActor, Container::safeCast(this), 
							!isDeleted(this->actorLoadingListeners) ? 
								(EventListener)Stage::onActorLoaded 
								: 
								NULL, stageActorDescription->internalId
						);
					}
					else
					{
						Stage::doAddChildActor
						(
							this, stageActorDescription->positionedActor, false, stageActorDescription->internalId
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
			this->streamingHeadNode = this->stageActorDescriptions->head;
		}

		bool negativeStreamingAmplitude = 0 > ((int16)this->streamingAmplitude);

		for(uint16 counter = 0; counter < this->streamingAmplitude; this->streamingHeadNode = this->streamingHeadNode->next, counter++)
		{
			if(NULL == this->streamingHeadNode)
			{
				this->streamingHeadNode = this->stageActorDescriptions->head;

				if(negativeStreamingAmplitude)
				{
					break;
				}
			}

			StageActorDescription* stageActorDescription = (StageActorDescription*)this->streamingHeadNode->data;

			if(0 > stageActorDescription->internalId)
			{
				// if actor in load range
				if
				(
					Stage::isActorInLoadRange
					(
						this, stageActorDescription->positionedActor->onScreenPosition, 
						stageActorDescription->validRightBox ? 
							&stageActorDescription->rightBox : NULL
					)
				)
				{
					loadedEntities = true;

					stageActorDescription->internalId = this->nextActorId++;

					if(defer)
					{
						ActorFactory::spawnActor
						(
							this->actorFactory, stageActorDescription->positionedActor, Container::safeCast(this), 
							!isDeleted(this->actorLoadingListeners) ? 
								(EventListener)Stage::onActorLoaded : NULL, 
							stageActorDescription->internalId
						);
					}
					else
					{
						Stage::doAddChildActor
						(
							this, stageActorDescription->positionedActor, false, stageActorDescription->internalId
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
	VirtualNode node = this->stageActorDescriptions->head;

	for(; NULL != node; node = node->next)
	{
		StageActorDescription* stageActorDescription = (StageActorDescription*)node->data;

		if(-1 == stageActorDescription->internalId)
		{
			// if actor in load range
			if
			(
				stageActorDescription->positionedActor->loadRegardlessOfPosition 
				|| 
				Stage::isActorInLoadRange
				(
					this, stageActorDescription->positionedActor->onScreenPosition, &stageActorDescription->rightBox
				)
			)
			{
				stageActorDescription->internalId = this->nextActorId++;
				Actor actor = 
					Stage::doAddChildActor(this, stageActorDescription->positionedActor, false, stageActorDescription->internalId);
				ASSERT(actor, "Stage::loadInitialEntities: actor not loaded");

				if(!isDeleted(actor))
				{
					if(!stageActorDescription->positionedActor->loadRegardlessOfPosition)
					{
						this->streamingHeadNode = node;
					}

					stageActorDescription->internalId = Actor::getInternalId(actor);
				}
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Actor Stage::doAddChildActor(const PositionedActor* const positionedActor, bool permanent __attribute__ ((unused)), int16 internalId)
{
	if(NULL != positionedActor)
	{
		Actor actor = Actor::createActor(positionedActor, internalId);
		ASSERT(actor, "Stage::doAddChildActor: actor not loaded");

		if(!isDeleted(actor))
		{
			// create the actor and add it to the world
			Stage::addChild(this, Container::safeCast(actor));

			actor->dontStreamOut = actor->dontStreamOut || permanent;
			
			Stage::alertOfLoadedActor(this, actor);
		}

		return actor;
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::onActorLoaded(ListenerObject eventFirer)
{
	Actor actor = Actor::safeCast(eventFirer);

	if(!isDeleted(actor) && !isDeleted(this->actorLoadingListeners))
	{
		Stage::alertOfLoadedActor(this, actor);
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::alertOfLoadedActor(Actor actor)
{
	if(isDeleted(actor) || isDeleted(this->actorLoadingListeners))
	{
		return;
	}

	for(VirtualNode node = this->actorLoadingListeners->head; NULL != node; node = node->next)
	{
		ActorLoadingListener* actorLoadingListener = (ActorLoadingListener*)node->data;

		if(!isDeleted(actorLoadingListener->scope))
		{
			Actor::addEventListener(actor, actorLoadingListener->scope, actorLoadingListener->callback, kEventActorLoaded);
		}
	}

	Actor::fireEvent(actor, kEventActorLoaded);
	NM_ASSERT(!isDeleted(actor), "Stage::alertOfLoadedActor: deleted actor during kEventActorLoaded");
	Actor::removeEventListeners(actor, NULL, kEventActorLoaded);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 Stage::isActorInLoadRange(ScreenPixelVector onScreenPosition, const RightBox* rightBox)
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

		return Actor::isInsideFrustrum(Vector3D::getFromScreenPixelVector(onScreenPosition), helperRightBox);
	}
	else
	{
		return Actor::isInsideFrustrum(Vector3D::getFromScreenPixelVector(onScreenPosition), *rightBox);
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::updateActorFactory()
{
#ifdef __PROFILE_STREAMING
	_renderingProcessTimeHelper = 0;
	timeBeforeProcess = TimerManager::getElapsedMilliseconds(TimerManager::getInstance());
#endif

	bool preparingEntities = ActorFactory::createNextActor(this->actorFactory);

#ifdef __PROFILE_STREAMING
	uint32 processTime = 
		-_renderingProcessTimeHelper + TimerManager::getElapsedMilliseconds(TimerManager::getInstance()) - timeBeforeProcess;
	actorFactoryHighestTime = processTime > actorFactoryHighestTime ? processTime : actorFactoryHighestTime;
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

StageActorDescription* Stage::registerActor(PositionedActor* positionedActor)
{
	ASSERT(positionedActor, "Stage::registerActor: null positionedActor");

	StageActorDescription* stageActorDescription = new StageActorDescription;

	stageActorDescription->extraInfo = NULL;
	stageActorDescription->internalId = -1;
	stageActorDescription->positionedActor = positionedActor;

	Vector3D environmentPosition = Vector3D::zero();
	stageActorDescription->rightBox = Actor::getRightBoxFromSpec(stageActorDescription->positionedActor, &environmentPosition);

	stageActorDescription->validRightBox = 
		(0 != stageActorDescription->rightBox.x1 - stageActorDescription->rightBox.x0) 
		|| 
		(0 != stageActorDescription->rightBox.y1 - stageActorDescription->rightBox.y0) 
		|| 
		(0 != stageActorDescription->rightBox.z1 - stageActorDescription->rightBox.z0);

	fixed_t padding = __PIXELS_TO_METERS(this->stageSpec->streaming.loadPadding);
	
	// Bake the padding in the bounding box to save on performance
	stageActorDescription->rightBox.x0 -= padding;
	stageActorDescription->rightBox.x1 += padding;
	stageActorDescription->rightBox.y0 -= padding;
	stageActorDescription->rightBox.y1 += padding;
	stageActorDescription->rightBox.z0 -= padding;
	stageActorDescription->rightBox.z1 += padding;

	return stageActorDescription;
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

Actor Stage::findChildByInternalId(int16 internalId)
{
	VirtualNode node = this->children->head;

	for(; NULL != node; node = node->next)
	{
		if(Actor::getInternalId(Actor::safeCast(node->data)) == internalId)
		{
			return Actor::safeCast(node->data);
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::setFocusActor(Actor focusActor)
{
	if(!isDeleted(this->focusActor))
	{
		Actor::removeEventListener
		(
			this->focusActor, ListenerObject::safeCast(this), (EventListener)Stage::onFocusActorDeleted, kEventContainerDeleted
		);
	}

	this->focusActor = focusActor;

	if(!isDeleted(this->focusActor))
	{
		Actor::addEventListener
		(
			this->focusActor, ListenerObject::safeCast(this), (EventListener)Stage::onFocusActorDeleted, kEventContainerDeleted
		);

		Vector3D focusActorPosition = *Container::getPosition(this->focusActor);
		focusActorPosition.x = __METERS_TO_PIXELS(focusActorPosition.x);
		focusActorPosition.y = __METERS_TO_PIXELS(focusActorPosition.y);
		focusActorPosition.z = __METERS_TO_PIXELS(focusActorPosition.z);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::onFocusActorDeleted(ListenerObject eventFirer __attribute__ ((unused)))
{
	if(!isDeleted(this->focusActor) && ListenerObject::safeCast(this->focusActor) == eventFirer)
	{
		if(this->focusActor == Camera::getFocusActor(Camera::getInstance()))
		{
			Camera::setFocusActor(Camera::getInstance(), NULL);
		}
	}

	this->focusActor = NULL;

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
