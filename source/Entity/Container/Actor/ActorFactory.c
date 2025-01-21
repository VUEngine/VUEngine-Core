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

#include <ColliderManager.h>
#include <DebugConfig.h>
#include <Actor.h>
#include <Printing.h>
#include <SpriteManager.h>
#include <VirtualList.h>
#include <VUEngine.h>
#include <WireframeManager.h>

#include "ActorFactory.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

typedef uint32 (*InstantiationPhase)(void*);

/// @memberof ActorFactory
typedef struct PositionedActorDescription
{
	const PositionedActor* positionedActor;
	Container parent;
	Actor actor;
	int16 internalId;
	uint8 componentIndex;
	bool componentsCreated;
	bool wireframesCreated;
	bool collidersCreated;
	bool behaviorsCreated;
	bool transformed;
	bool graphicsSynchronized;

} PositionedActorDescription;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static const InstantiationPhase _instantiationPhases[] =
{
	&ActorFactory::instantiateActors,
	&ActorFactory::transformActors,
	&ActorFactory::addChildActors
};

static int32 _instantiationPhasesCount = sizeof(_instantiationPhases) / sizeof(InstantiationPhase);

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ActorFactory::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->actorsToInstantiate = new VirtualList();
	this->actorsToTransform = new VirtualList();
	this->actorsToAddAsChildren = new VirtualList();
	this->spawnedActors = new VirtualList();

	this->instantiationPhase = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ActorFactory::destructor()
{
	VirtualNode node = this->actorsToInstantiate->head;

	for(; NULL != node; node = node->next)
	{
		PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)node->data;

		if(!isDeleted(positionedActorDescription->actor))
		{
			Actor::deleteMyself(positionedActorDescription->actor);
		}

		delete positionedActorDescription;
	}

	delete this->actorsToInstantiate;
	this->actorsToInstantiate = NULL;

	node = this->actorsToTransform->head;

	for(; NULL != node; node = node->next)
	{
		PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)node->data;

		if(!isDeleted(positionedActorDescription->actor))
		{
			Actor::deleteMyself(positionedActorDescription->actor);
		}

		delete positionedActorDescription;
	}

	delete this->actorsToTransform;
	this->actorsToTransform = NULL;

	node = this->actorsToAddAsChildren->head;

	for(; NULL != node; node = node->next)
	{
		PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)node->data;

		if(!isDeleted(positionedActorDescription->actor))
		{
			Actor::deleteMyself(positionedActorDescription->actor);
		}

		delete positionedActorDescription;
	}

	delete this->actorsToAddAsChildren;
	this->actorsToAddAsChildren = NULL;

	node = this->spawnedActors->head;

	for(; NULL != node; node = node->next)
	{
		PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)node->data;

		delete positionedActorDescription;
	}

	delete this->spawnedActors;
	this->spawnedActors = NULL;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ActorFactory::spawnActor(const PositionedActor* positionedActor, Container parent, int16 internalId)
{
	if(NULL == positionedActor || NULL == parent)
	{
		return;
	}

	PositionedActorDescription* positionedActorDescription = new PositionedActorDescription;

	positionedActorDescription->positionedActor = positionedActor;
	positionedActorDescription->parent = parent;
	positionedActorDescription->actor = NULL;
	positionedActorDescription->internalId = internalId;
	positionedActorDescription->transformed = false;
	positionedActorDescription->graphicsSynchronized = false;
	positionedActorDescription->componentsCreated = false;
	positionedActorDescription->componentIndex = 0;

	VirtualList::pushBack(this->actorsToInstantiate, positionedActorDescription);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ActorFactory::createNextActor()
{
	ActorFactory::cleanUp(this);

	if(this->instantiationPhase >= _instantiationPhasesCount)
	{
		this->instantiationPhase = 0;
	}

	uint32 result = _instantiationPhases[this->instantiationPhase](this);

	int32 counter = _instantiationPhasesCount;

	while(__LIST_EMPTY == result)
	{
		if(!--counter)
		{
			return false;
		}

		this->instantiationPhase++;

		if(this->instantiationPhase >= _instantiationPhasesCount)
		{
			this->instantiationPhase = 0;
		}

		result = _instantiationPhases[this->instantiationPhase](this);
	}

	this->instantiationPhase += __ACTOR_PENDING_PROCESSING != result ? 1 : 0;

	return __LIST_EMPTY != result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ActorFactory::hasActorsPending()
{
	return NULL != this->actorsToInstantiate->head ||
			NULL != this->actorsToTransform->head ||
			NULL != this->actorsToAddAsChildren->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
#ifdef __PROFILE_STREAMING
void ActorFactory::print(int32 x, int32 y)
{	int32 xDisplacement = 18;

	Printing::text("Factory's status", x, y++, NULL);
	Printing::text("", x, y++, NULL);

	Printing::text("Phase: ", x, y, NULL);
	Printing::int32(this->instantiationPhase, x + xDisplacement, y++, NULL);

	Printing::text("Actors pending...", x, y++, NULL);

	Printing::text("1 Instantiation:			", x, y, NULL);
	Printing::int32(VirtualList::getCount(this->actorsToInstantiate), x + xDisplacement, y++, NULL);

	Printing::text("2 Transformation:			", x, y, NULL);
	Printing::int32(VirtualList::getCount(this->actorsToTransform), x + xDisplacement, y++, NULL);

	Printing::text("3 Make ready:			", x, y, NULL);
	Printing::int32(VirtualList::getCount(this->actorsToAddAsChildren), x + xDisplacement, y++, NULL);

	Printing::text("4 Call listeners:			", x, y, NULL);
	Printing::int32(VirtualList::getCount(this->spawnedActors), x + xDisplacement, y++, NULL);
}
#endif
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 ActorFactory::instantiateActors()
{
	ASSERT(this, "ActorFactory::spawnActors: null spawnActors");

	if(NULL == this->actorsToInstantiate->head)
	{
		return __LIST_EMPTY;
	}

	PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)this->actorsToInstantiate->head->data;

	if(!isDeleted(positionedActorDescription->parent))
	{
		if(!isDeleted(positionedActorDescription->actor))
		{
			ActorFactory actorFactory = Actor::getActorFactory(positionedActorDescription->actor);

			if(NULL == actorFactory || __LIST_EMPTY == ActorFactory::instantiateActors(actorFactory))
			{
				VirtualList::pushBack(this->actorsToTransform, positionedActorDescription);
				VirtualList::removeData(this->actorsToInstantiate, positionedActorDescription);

				return __ACTOR_PROCESSED;
			}

			return __ACTOR_PENDING_PROCESSING;
		}
		else
		{
			NM_ASSERT(!isDeleted(positionedActorDescription), "ActorFactory::spawnActors: deleted positionedActorDescription");
			NM_ASSERT(NULL != positionedActorDescription->positionedActor, "ActorFactory::spawnActors: null positionedActor");
			NM_ASSERT(NULL != positionedActorDescription->positionedActor->actorSpec, "ActorFactory::spawnActors: null spec");
			NM_ASSERT
			(
				NULL != positionedActorDescription->positionedActor->actorSpec->allocator, 
				"ActorFactory::spawnActors: no allocator defined"
			);

			positionedActorDescription->actor = 
				Actor::createActorDeferred(positionedActorDescription->positionedActor, positionedActorDescription->internalId);
			NM_ASSERT(positionedActorDescription->actor, "ActorFactory::spawnActors: actor not loaded");

			if(!isDeleted(positionedActorDescription->actor))
			{
				Actor::addEventListener
				(
					positionedActorDescription->actor, ListenerObject::safeCast(positionedActorDescription->parent), kEventActorLoaded
				);
			}
		}
	}
	else
	{
		VirtualList::removeData(this->actorsToInstantiate, positionedActorDescription);
		delete positionedActorDescription;
	}

	return __ACTOR_PROCESSED;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 ActorFactory::transformActors()
{
	if(NULL == this->actorsToTransform->head)
	{
		return __LIST_EMPTY;
	}

	PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)this->actorsToTransform->head->data;
	ASSERT(positionedActorDescription->actor, "ActorFactory::transformActors: null actor");
	ASSERT(positionedActorDescription->parent, "ActorFactory::transformActors: null parent");

	if(!isDeleted(positionedActorDescription->parent))
	{
		if(!positionedActorDescription->transformed)
		{
			const Transformation* environmentTransform = Actor::getTransformation(positionedActorDescription->parent);
			Actor::invalidateTransformation(positionedActorDescription->actor);
			Actor::transform(positionedActorDescription->actor, environmentTransform, false);

			positionedActorDescription->transformed = true;

			return __ACTOR_PENDING_PROCESSING;
		}

		if(!positionedActorDescription->componentsCreated)
		{
			const ActorSpec* actorSpec = Actor::getSpec(positionedActorDescription->actor);
			
			if
			(
				NULL != actorSpec && NULL != actorSpec->componentSpecs && 
				NULL != actorSpec->componentSpecs[positionedActorDescription->componentIndex]
			)
			{
				bool createdComponent = 
					NULL != Actor::addComponent
					(
						positionedActorDescription->actor, 
						(ComponentSpec*)actorSpec->componentSpecs[positionedActorDescription->componentIndex]
					);
					
				positionedActorDescription->componentIndex++;

				if(createdComponent)
				{
					return __ACTOR_PENDING_PROCESSING;
				}
			}

			positionedActorDescription->componentsCreated = true;
			positionedActorDescription->componentIndex = 0;
		}

		ActorFactory actorFactory = Actor::getActorFactory(positionedActorDescription->actor);

		if(NULL == actorFactory || __LIST_EMPTY == ActorFactory::transformActors(actorFactory))
		{
			VirtualList::pushBack(this->actorsToAddAsChildren, positionedActorDescription);
			VirtualList::removeData(this->actorsToTransform, positionedActorDescription);

			return __ACTOR_PROCESSED;
		}

		return __ACTOR_PENDING_PROCESSING;
	}
	else
	{
		VirtualList::removeData(this->actorsToTransform, positionedActorDescription);

		if(!isDeleted(positionedActorDescription->actor))
		{
			Actor::deleteMyself(positionedActorDescription->actor);
		}

		delete positionedActorDescription;
	}

	return __ACTOR_PROCESSED;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 ActorFactory::addChildActors()
{
	if(NULL == this->actorsToAddAsChildren->head)
	{
		return __LIST_EMPTY;
	}

	PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)this->actorsToAddAsChildren->head->data;

	if(!isDeleted(positionedActorDescription->parent))
	{
		if(!positionedActorDescription->graphicsSynchronized)
		{
			Actor::calculateSize(positionedActorDescription->actor);
			Actor::invalidateTransformation(positionedActorDescription->actor);
			positionedActorDescription->graphicsSynchronized = true;

			return __ACTOR_PENDING_PROCESSING;
		}

		ActorFactory actorFactory = Actor::getActorFactory(positionedActorDescription->actor);

		if(NULL == actorFactory || __LIST_EMPTY == ActorFactory::addChildActors(actorFactory))
		{
			NM_ASSERT(!isDeleted(positionedActorDescription->parent), "ActorFactory::addChildActors: deleted parent");

			// Must add the child to its parent before making it ready
			Container::addChild(positionedActorDescription->parent, Container::safeCast(positionedActorDescription->actor));

			VirtualList::pushBack(this->spawnedActors, positionedActorDescription);
			VirtualList::removeData(this->actorsToAddAsChildren, positionedActorDescription);

			return __ACTOR_PROCESSED;
		}

		return __ACTOR_PENDING_PROCESSING;
	}
	else
	{
		VirtualList::removeData(this->actorsToAddAsChildren, positionedActorDescription);

		// Don't need to delete the created actor since the parent takes care of that at this point

		delete positionedActorDescription;
	}

	return __ACTOR_PROCESSED;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 ActorFactory::cleanUp()
{
	if(NULL == this->spawnedActors->head)
	{
		return __LIST_EMPTY;
	}

	PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)this->spawnedActors->head->data;

	if(!isDeleted(positionedActorDescription->parent) && !isDeleted(positionedActorDescription->actor))
	{
		Actor::fireEvent(positionedActorDescription->actor, kEventActorLoaded);
		NM_ASSERT(!isDeleted(positionedActorDescription->actor), "ActorFactory::cleanUp: deleted actor during kEventActorLoaded");
		Actor::removeEventListeners(positionedActorDescription->actor, kEventActorLoaded);

		VirtualList::removeData(this->spawnedActors, positionedActorDescription);
		delete positionedActorDescription;

		return __ACTOR_PROCESSED;
	}
	else
	{
		VirtualList::removeData(this->spawnedActors, positionedActorDescription);
		delete positionedActorDescription;
	}

	return __ACTOR_PROCESSED;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
