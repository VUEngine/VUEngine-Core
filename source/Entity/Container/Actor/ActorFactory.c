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
	EventListener callback;
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
	&ActorFactory::instantiateEntities,
	&ActorFactory::transformEntities,
	&ActorFactory::addChildEntities
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

	this->entitiesToInstantiate = new VirtualList();
	this->entitiesToTransform = new VirtualList();
	this->entitiesToAddAsChildren = new VirtualList();
	this->spawnedEntities = new VirtualList();

	this->instantiationPhase = 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ActorFactory::destructor()
{
	VirtualNode node = this->entitiesToInstantiate->head;

	for(; NULL != node; node = node->next)
	{
		PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)node->data;

		if(!isDeleted(positionedActorDescription->actor))
		{
			Actor::deleteMyself(positionedActorDescription->actor);
		}

		delete positionedActorDescription;
	}

	delete this->entitiesToInstantiate;
	this->entitiesToInstantiate = NULL;

	node = this->entitiesToTransform->head;

	for(; NULL != node; node = node->next)
	{
		PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)node->data;

		if(!isDeleted(positionedActorDescription->actor))
		{
			Actor::deleteMyself(positionedActorDescription->actor);
		}

		delete positionedActorDescription;
	}

	delete this->entitiesToTransform;
	this->entitiesToTransform = NULL;

	node = this->entitiesToAddAsChildren->head;

	for(; NULL != node; node = node->next)
	{
		PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)node->data;

		if(!isDeleted(positionedActorDescription->actor))
		{
			Actor::deleteMyself(positionedActorDescription->actor);
		}

		delete positionedActorDescription;
	}

	delete this->entitiesToAddAsChildren;
	this->entitiesToAddAsChildren = NULL;

	node = this->spawnedEntities->head;

	for(; NULL != node; node = node->next)
	{
		PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)node->data;

		delete positionedActorDescription;
	}

	delete this->spawnedEntities;
	this->spawnedEntities = NULL;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ActorFactory::spawnActor(const PositionedActor* positionedActor, Container parent, EventListener callback, int16 internalId)
{
	if(NULL == positionedActor || NULL == parent)
	{
		return;
	}

	PositionedActorDescription* positionedActorDescription = new PositionedActorDescription;

	positionedActorDescription->positionedActor = positionedActor;
	positionedActorDescription->parent = parent;
	positionedActorDescription->actor = NULL;
	positionedActorDescription->callback = callback;
	positionedActorDescription->internalId = internalId;
	positionedActorDescription->transformed = false;
	positionedActorDescription->graphicsSynchronized = false;
	positionedActorDescription->componentsCreated = false;
	positionedActorDescription->componentIndex = 0;

	VirtualList::pushBack(this->entitiesToInstantiate, positionedActorDescription);
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

	this->instantiationPhase += __ENTITY_PENDING_PROCESSING != result ? 1 : 0;

	return __LIST_EMPTY != result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ActorFactory::hasEntitiesPending()
{
	return NULL != this->entitiesToInstantiate->head ||
			NULL != this->entitiesToTransform->head ||
			NULL != this->entitiesToAddAsChildren->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
#ifdef __PROFILE_STREAMING
void ActorFactory::print(int32 x, int32 y)
{	int32 xDisplacement = 18;

	Printing::text(Printing::getInstance(), "Factory's status", x, y++, NULL);
	Printing::text(Printing::getInstance(), "", x, y++, NULL);

	Printing::text(Printing::getInstance(), "Phase: ", x, y, NULL);
	Printing::int32(Printing::getInstance(), this->instantiationPhase, x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "Entities pending...", x, y++, NULL);

	Printing::text(Printing::getInstance(), "1 Instantiation:			", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->entitiesToInstantiate), x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "2 Transformation:			", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->entitiesToTransform), x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "3 Make ready:			", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->entitiesToAddAsChildren), x + xDisplacement, y++, NULL);

	Printing::text(Printing::getInstance(), "4 Call listeners:			", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->spawnedEntities), x + xDisplacement, y++, NULL);
}
#endif
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 ActorFactory::instantiateEntities()
{
	ASSERT(this, "ActorFactory::spawnEntities: null spawnEntities");

	if(NULL == this->entitiesToInstantiate->head)
	{
		return __LIST_EMPTY;
	}

	PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)this->entitiesToInstantiate->head->data;

	if(!isDeleted(positionedActorDescription->parent))
	{
		if(!isDeleted(positionedActorDescription->actor))
		{
			ActorFactory actorFactory = Actor::getActorFactory(positionedActorDescription->actor);

			if(NULL == actorFactory || __LIST_EMPTY == ActorFactory::instantiateEntities(actorFactory))
			{
				VirtualList::pushBack(this->entitiesToTransform, positionedActorDescription);
				VirtualList::removeData(this->entitiesToInstantiate, positionedActorDescription);

				return __ENTITY_PROCESSED;
			}

			return __ENTITY_PENDING_PROCESSING;
		}
		else
		{
			NM_ASSERT(!isDeleted(positionedActorDescription), "ActorFactory::spawnEntities: deleted positionedActorDescription");
			NM_ASSERT(NULL != positionedActorDescription->positionedActor, "ActorFactory::spawnEntities: null positionedActor");
			NM_ASSERT(NULL != positionedActorDescription->positionedActor->actorSpec, "ActorFactory::spawnEntities: null spec");
			NM_ASSERT
			(
				NULL != positionedActorDescription->positionedActor->actorSpec->allocator, 
				"ActorFactory::spawnEntities: no allocator defined"
			);

			positionedActorDescription->actor = 
				Actor::createActorDeferred(positionedActorDescription->positionedActor, positionedActorDescription->internalId);
			NM_ASSERT(positionedActorDescription->actor, "ActorFactory::spawnEntities: actor not loaded");

			if(!isDeleted(positionedActorDescription->actor) && NULL != positionedActorDescription->callback)
			{
				Actor::addEventListener
				(
					positionedActorDescription->actor, ListenerObject::safeCast(positionedActorDescription->parent), 
					positionedActorDescription->callback, kEventActorLoaded
				);
			}
		}
	}
	else
	{
		VirtualList::removeData(this->entitiesToInstantiate, positionedActorDescription);
		delete positionedActorDescription;
	}

	return __ENTITY_PROCESSED;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 ActorFactory::transformEntities()
{
	if(NULL == this->entitiesToTransform->head)
	{
		return __LIST_EMPTY;
	}

	PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)this->entitiesToTransform->head->data;
	ASSERT(positionedActorDescription->actor, "ActorFactory::transformEntities: null actor");
	ASSERT(positionedActorDescription->parent, "ActorFactory::transformEntities: null parent");

	if(!isDeleted(positionedActorDescription->parent))
	{
		if(!positionedActorDescription->transformed)
		{
			const Transformation* environmentTransform = Actor::getTransformation(positionedActorDescription->parent);
			Actor::invalidateTransformation(positionedActorDescription->actor);
			Actor::transform(positionedActorDescription->actor, environmentTransform, false);

			positionedActorDescription->transformed = true;

			return __ENTITY_PENDING_PROCESSING;
		}

		if(!positionedActorDescription->componentsCreated)
		{
			ActorSpec* actorSpec = Actor::getSpec(positionedActorDescription->actor);
			
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
					return __ENTITY_PENDING_PROCESSING;
				}
			}

			positionedActorDescription->componentsCreated = true;
			positionedActorDescription->componentIndex = 0;
		}

		ActorFactory actorFactory = Actor::getActorFactory(positionedActorDescription->actor);

		if(NULL == actorFactory || __LIST_EMPTY == ActorFactory::transformEntities(actorFactory))
		{
			VirtualList::pushBack(this->entitiesToAddAsChildren, positionedActorDescription);
			VirtualList::removeData(this->entitiesToTransform, positionedActorDescription);

			return __ENTITY_PROCESSED;
		}

		return __ENTITY_PENDING_PROCESSING;
	}
	else
	{
		VirtualList::removeData(this->entitiesToTransform, positionedActorDescription);

		if(!isDeleted(positionedActorDescription->actor))
		{
			Actor::deleteMyself(positionedActorDescription->actor);
		}

		delete positionedActorDescription;
	}

	return __ENTITY_PROCESSED;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 ActorFactory::addChildEntities()
{
	if(NULL == this->entitiesToAddAsChildren->head)
	{
		return __LIST_EMPTY;
	}

	PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)this->entitiesToAddAsChildren->head->data;

	if(!isDeleted(positionedActorDescription->parent))
	{
		if(!positionedActorDescription->graphicsSynchronized)
		{
			Actor::calculateSize(positionedActorDescription->actor);
			Actor::invalidateTransformation(positionedActorDescription->actor);
			positionedActorDescription->graphicsSynchronized = true;

			return __ENTITY_PENDING_PROCESSING;
		}

		ActorFactory actorFactory = Actor::getActorFactory(positionedActorDescription->actor);

		if(NULL == actorFactory || __LIST_EMPTY == ActorFactory::addChildEntities(actorFactory))
		{
			NM_ASSERT(!isDeleted(positionedActorDescription->parent), "ActorFactory::addChildEntities: deleted parent");

			// Must add the child to its parent before making it ready
			Container::addChild(positionedActorDescription->parent, Container::safeCast(positionedActorDescription->actor));

			VirtualList::pushBack(this->spawnedEntities, positionedActorDescription);
			VirtualList::removeData(this->entitiesToAddAsChildren, positionedActorDescription);

			return __ENTITY_PROCESSED;
		}

		return __ENTITY_PENDING_PROCESSING;
	}
	else
	{
		VirtualList::removeData(this->entitiesToAddAsChildren, positionedActorDescription);

		// don't need to delete the created actor since the parent takes care of that at this point

		delete positionedActorDescription;
	}

	return __ENTITY_PROCESSED;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 ActorFactory::cleanUp()
{
	if(NULL == this->spawnedEntities->head)
	{
		return __LIST_EMPTY;
	}

	PositionedActorDescription* positionedActorDescription = (PositionedActorDescription*)this->spawnedEntities->head->data;

	if(!isDeleted(positionedActorDescription->parent) && !isDeleted(positionedActorDescription->actor))
	{
		if(NULL != positionedActorDescription->callback)
		{
			Actor::fireEvent(positionedActorDescription->actor, kEventActorLoaded);
			NM_ASSERT(!isDeleted(positionedActorDescription->actor), "ActorFactory::cleanUp: deleted actor during kEventActorLoaded");
			Actor::removeEventListeners(positionedActorDescription->actor, NULL, kEventActorLoaded);
		}

		VirtualList::removeData(this->spawnedEntities, positionedActorDescription);
		delete positionedActorDescription;

		return __ENTITY_PROCESSED;
	}
	else
	{
		VirtualList::removeData(this->spawnedEntities, positionedActorDescription);
		delete positionedActorDescription;
	}

	return __ENTITY_PROCESSED;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
