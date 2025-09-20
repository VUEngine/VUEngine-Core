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

#include <ActorFactory.h>
#include <Actor.h>
#include <BgmapTexture.h>
#include <BgmapTextureManager.h>
#include <Camera.h>
#include <CharSetManager.h>
#include <DebugConfig.h>
#include <HardwareManager.h>
#include <Printer.h>
#include <SoundManager.h>
#include <TimerManager.h>
#include <UIContainer.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <VIPManager.h>

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
	ListenerObject listener;

} ActorLoadingListener;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static const StreamingPhase _streamingPhases[] =
{
	&Stage::unloadOutOfRangeActors,
	&Stage::loadInRangeActors
};

#define __DEBUGGING_STREAMING
#ifdef __DEBUGGING_STREAMING
#ifndef __SHIPPING
static uint32 unloadOutOfRangeActorsHighestTime = 0;
static uint32 loadInRangeActorsHighestTime = 0;
static uint32 processRemovedActorsHighestTime = 0;
static uint32 actorFactoryHighestTime = 0;
#endif
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
	this->cameraTransformation.position = Vector3D::getFromPixelVector(this->stageSpec->level.cameraInitialPosition);
	this->cameraTransformation.rotation = Rotation::zero();
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
		// They are taken care by the SoundManager when
		// I called SoundManager::stopAllSounds
		for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
		{
			Sound sound = Sound::safeCast(node->data);

			if(!isDeleted(sound))
			{
				Sound::removeEventListener(sound, ListenerObject::safeCast(this), kEventSoundReleased);
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
	Base::suspend(this);

	// Save the camera position for resume reconfiguration
	this->cameraTransformation = Camera::getTransformation(Camera::getInstance());

	// Relinquish camera focus priority
	if(!isDeleted(this->focusActor))
	{
		if(this->focusActor == Camera::getFocusActor(Camera::getInstance()))
		{
			// Relinquish focus actor
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
	Stage::configureCamera(this, true);
	Stage::configureGraphics(this);
	Stage::configureSounds(this);

	if(!isDeleted(this->focusActor))
	{
		// Recover focus actor
		Camera::setFocusActor(Camera::getInstance(), Actor::safeCast(this->focusActor));
	}

	Base::resume(this);

	Stage::transform(this, NULL, __INVALIDATE_TRANSFORMATION);

	this->actorFactory = new ActorFactory();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

const StageSpec* Stage::getSpec()
{
	return this->stageSpec;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::configurePalettes()
{
	VIPManager::configurePalettes(this->stageSpec->rendering.paletteConfig);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

PaletteConfig Stage::getPaletteConfig()
{
	return this->stageSpec->rendering.paletteConfig;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::registerActors(VirtualList positionedActorsToIgnore)
{
	if(!isDeleted(this->stageActorDescriptions))
	{
		return;
	}

	this->stageActorDescriptions = new VirtualList();

	// Register actors ordering them according to their distances to the origin
	int32 i = 0;

	for(;this->stageSpec->actors.children[i].actorSpec; i++)
	{
		if(positionedActorsToIgnore)
		{
			VirtualNode node = positionedActorsToIgnore->head;

			for(; NULL != node; node = node->next)
			{
				if(&this->stageSpec->actors.children[i] == (PositionedActor*)node->data)
				{
					break;
				}
			}

			if(node)
			{
				continue;
			}
		}

		StageActorDescription* stageActorDescription = Stage::registerActor(this, &this->stageSpec->actors.children[i]);

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

void Stage::addActorLoadingListener(ListenerObject listener)
{
	if(isDeleted(listener))
	{
		return;
	}

	if(isDeleted(this->actorLoadingListeners))
	{
		this->actorLoadingListeners = new VirtualList();
	}

	for(VirtualNode node = this->actorLoadingListeners->head; NULL != node; node = node->next)
	{
		ActorLoadingListener* actorLoadingListener = (ActorLoadingListener*)node->data;

		if(listener == actorLoadingListener->listener)
		{
			return;
		}
	}

	ActorLoadingListener* actorLoadingListener = new ActorLoadingListener;
	actorLoadingListener->listener = listener;

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
	Stage::transform(this, NULL, __INVALIDATE_TRANSFORMATION);

	this->streamingHeadNode = NULL;

	// Make sure that the actor factory doesn't have any pending operations
	while(ActorFactory::createNextActor(this->actorFactory));

	do
	{
		this->streamingHeadNode = NULL;

		Stage::purgeChildren(this);

	} while(Stage::unloadOutOfRangeActors(this, false));

	this->streamingHeadNode = NULL;
	this->streamingAmplitude = (uint16)-1;

	while(Stage::loadInRangeActors(this, false));

	while(ActorFactory::createNextActor(this->actorFactory))
	{
		Stage::transform(this, NULL, __INVALIDATE_TRANSFORMATION);
	}

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
		// They are taken care by the SoundManager when
		// I called SoundManager::stopAllSounds
		for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
		{
			Sound sound = Sound::safeCast(node->data);

			if(!isDeleted(sound))
			{
				Sound::removeEventListener(sound, ListenerObject::safeCast(this), kEventSoundReleased);
				Sound::play(sound, NULL, playbackType);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void Stage::print(int32 x, int32 y)
{
	Printer::text("STREAMING STATUS", x, y++, NULL);

	Printer::text("Stage's status", x, ++y, NULL);

	int32 originalY __attribute__ ((unused)) = y;
	int32 xDisplacement = 21;
	y++;

	Printer::text("Registered actors:            ", x, ++y, NULL);
	Printer::int32(VirtualList::getCount(this->stageActorDescriptions), x + xDisplacement, y++, NULL);

	if(NULL != this->children)
	{
		Printer::text("Child actors:                 ", x, y, NULL);
		Printer::int32(VirtualList::getCount(this->children), x + xDisplacement, y++, NULL);
	}

	xDisplacement = 10;

	Printer::text("Process duration (ms)", x, ++y, NULL);
	y++;

	Printer::text("Unload:           ", x, ++y, NULL);
	Printer::int32(unloadOutOfRangeActorsHighestTime, x + xDisplacement, y, NULL);

	Printer::text("Load:             ", x, ++y, NULL);
	Printer::int32(loadInRangeActorsHighestTime, x + xDisplacement, y, NULL);

	Printer::text("Removing:         ", x, ++y, NULL);
	Printer::int32(processRemovedActorsHighestTime, x + xDisplacement, y, NULL);

	Printer::text("Factory:          ", x, ++y, NULL);
	Printer::int32(actorFactoryHighestTime, x + xDisplacement, y++, NULL);

	unloadOutOfRangeActorsHighestTime = 0;
	loadInRangeActorsHighestTime = 0;
	processRemovedActorsHighestTime = 0;
	actorFactoryHighestTime = 0;

	if(NULL != this->actorFactory)
	{
		ActorFactory::print(this->actorFactory, x, y);
	}
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::stream()
{
	if(NULL == this->stageActorDescriptions->head)
	{
		return;
	}

	if(!Stage::updateActorFactory(this))
	{
		do
		{
			if(++this->streamingPhase >= sizeof(_streamingPhases) / sizeof(StreamingPhase))
			{
				this->streamingPhase = 0;
			}
		}
		while(_streamingPhases[this->streamingPhase](this, this->stageSpec->streaming.deferred) && this->stageSpec->streaming.deferred);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::configure(VirtualList positionedActorsToIgnore)
{
	Stage::configureCamera(this, true);
	Stage::configureGraphics(this);
	Stage::configureSounds(this);

	// Register all the actors in the stage's spec
	Stage::registerActors(this, positionedActorsToIgnore);

	Stage::loadInitialActors(this);

	// Retrieve focus actor for streaming
	Stage::setFocusActor(this, Camera::getFocusActor(Camera::getInstance()));

	// Apply transformations
	Stage::transform(this, NULL, __INVALIDATE_TRANSFORMATION);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::unloadOutOfRangeActors(int32 defer __attribute__((unused)))
{
	if(isDeleted(this->children))
	{
		return false;
	}

	bool unloadedActors = false;

	VirtualNode node = this->children->head;

	CACHE_RESET;

	// Check which entites must be unloaded
	for(; NULL != node; node = node->next)
	{
		// Get next actor
		Actor actor = Actor::safeCast(node->data);

		if(actor->dontStreamOut)
		{
			continue;
		}

		// If the actor isn't visible inside the view field, unload it
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

			// Unload it
			Stage::destroyChildActor(this, actor);

			// Remove from list of actors that are to be loaded by the streaming,
			// If the actor is not to be alwaysStreamIned
			if(!Actor::alwaysStreamIn(actor))
			{
				VirtualList::removeNode(this->stageActorDescriptions, auxNode);
			}

			unloadedActors = true;
		}
	}

	return unloadedActors;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::loadInRangeActors(int32 defer)
{
	bool loadedActors = false;

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
				// If actor in load range
				if
				(
					Stage::isActorInLoadRange
					(
						this, stageActorDescription->positionedActor->onScreenPosition, stageActorDescription->validRightBox ? 
						&stageActorDescription->rightBox : NULL
					)
				)
				{
					loadedActors = true;

					stageActorDescription->internalId = this->nextActorId++;

					if(defer)
					{
						ActorFactory::spawnActor
						(
							this->actorFactory, stageActorDescription->positionedActor, Container::safeCast(this), 
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
				// If actor in load range
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
					loadedActors = true;

					stageActorDescription->internalId = this->nextActorId++;

					if(defer)
					{
						ActorFactory::spawnActor
						(
							this->actorFactory, stageActorDescription->positionedActor, Container::safeCast(this), 
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

	return loadedActors;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::loadInitialActors()
{
	// Need a temporary list to remove and delete actors
	VirtualNode node = this->stageActorDescriptions->head;

	for(; NULL != node; node = node->next)
	{
		StageActorDescription* stageActorDescription = (StageActorDescription*)node->data;

		if(-1 == stageActorDescription->internalId)
		{
			// If actor in load range
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
				ASSERT(actor, "Stage::loadInitialActors: actor not loaded");

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
			// Create the actor and add it to the world
			Stage::addChild(this, Container::safeCast(actor));

			actor->dontStreamOut = actor->dontStreamOut || permanent;
			
			Stage::alertOfLoadedActor(this, actor);
		}

		return actor;
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Stage::onEvent(ListenerObject eventFirer, uint16 eventCode)
{
	switch(eventCode)
	{
		case kEventActorCreated:
		{
			Actor actor = Actor::safeCast(eventFirer);

			if(!isDeleted(actor) && !isDeleted(this->actorLoadingListeners))
			{
				Stage::alertOfLoadedActor(this, actor);
			}

			return false;
		}

		case kEventSoundReleased:
		{
			VirtualList::removeData(this->sounds, eventFirer);

			Stage::fireEvent(this, kEventSoundReleased);

			return false;
		}

		case kEventActorDeleted:
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
	}

	return Base::onEvent(this, eventFirer, eventCode);
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

		if(!isDeleted(actorLoadingListener->listener))
		{
			Actor::addEventListener(actor, actorLoadingListener->listener, kEventActorCreated);
		}
	}

	Actor::fireEvent(actor, kEventActorCreated);
	NM_ASSERT(!isDeleted(actor), "Stage::alertOfLoadedActor: deleted actor during kEventActorCreated");
	Actor::removeEventListeners(actor, kEventActorCreated);
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
	bool preparingActors = ActorFactory::createNextActor(this->actorFactory);

	return preparingActors;
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

void Stage::configureCamera(bool reset)
{
	if(reset)
	{
		Camera::reset(Camera::getInstance());
	}

	Camera::setStageSize(Camera::getInstance(), Size::getFromPixelSize(this->stageSpec->level.pixelSize));
	Camera::setTransformation(Camera::getInstance(), this->cameraTransformation, false);
	Camera::setup(Camera::getInstance(), this->stageSpec->rendering.pixelOptical, this->stageSpec->level.cameraFrustum);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::configureGraphics()
{
	BgmapTextureManager::reset(BgmapTextureManager::getInstance());

	Printer::loadFonts(this->stageSpec->assets.fontSpecs);
	CharSetManager::loadCharSets(CharSetManager::getInstance(), (const CharSetSpec**)this->stageSpec->assets.charSetSpecs);

	BgmapTextureManager::loadTextures
	(
		BgmapTextureManager::getInstance(), (const TextureSpec**)this->stageSpec->assets.textureSpecs
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Stage::configureSounds()
{
	SoundManager::unlock(SoundManager::getInstance());

	int32 i = 0;

	// Stop all sounds
	SoundManager::stopAllSounds(SoundManager::getInstance(), true, this->stageSpec->assets.sounds);

	for(; NULL != this->stageSpec->assets.sounds[i]; i++)
	{
		Sound sound = SoundManager::findSound(this->stageSpec->assets.sounds[i], ListenerObject::safeCast(this));

		if(isDeleted(sound))
		{
			sound = SoundManager::getSound(this->stageSpec->assets.sounds[i], ListenerObject::safeCast(this));
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
		// They are taken care by the SoundManager when
		// I called SoundManager::stopAllSounds
		for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
		{
			Sound sound = Sound::safeCast(node->data);

			if(!isDeleted(sound))
			{
				Sound::removeEventListener(sound, ListenerObject::safeCast(this), kEventSoundReleased);
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
		// They are taken care by the SoundManager when
		// I called SoundManager::stopAllSounds
		for(VirtualNode node = this->sounds->head; NULL != node; node = node->next)
		{
			Sound sound = Sound::safeCast(node->data);

			if(!isDeleted(sound))
			{
				Sound::removeEventListener(sound, ListenerObject::safeCast(this), kEventSoundReleased);
				Sound::unpause(sound);
			}
		}
	}
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
		Actor::removeEventListener(this->focusActor, ListenerObject::safeCast(this), kEventActorDeleted);
	}

	this->focusActor = focusActor;

	if(!isDeleted(this->focusActor))
	{
		Actor::addEventListener(this->focusActor, ListenerObject::safeCast(this), kEventActorDeleted);

		Vector3D focusActorPosition = *Container::getPosition(this->focusActor);
		focusActorPosition.x = __METERS_TO_PIXELS(focusActorPosition.x);
		focusActorPosition.y = __METERS_TO_PIXELS(focusActorPosition.y);
		focusActorPosition.z = __METERS_TO_PIXELS(focusActorPosition.z);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
