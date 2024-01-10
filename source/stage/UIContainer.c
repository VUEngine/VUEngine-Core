/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Camera.h>
#include <Printing.h>
#include <VirtualList.h>

#include "UIContainer.h"


//---------------------------------------------------------------------------------------------------------
//												CLASS'S DECLARATIONS
//---------------------------------------------------------------------------------------------------------

static Camera _camera = NULL;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void UIContainer::constructor(UIContainerSpec* uiContainerSpec)
{
	// construct base object
	Base::constructor(NULL);

	// add entities in the spec
	UIContainer::addEntities(this, uiContainerSpec->entities);

	_camera = Camera::getInstance();
}

// class's destructor
void UIContainer::destructor()
{
	// destroy base
	// must always be called at the end of the destructor
	Base::destructor();
}

// add entities
void UIContainer::addEntities(PositionedEntity* entities)
{
	ASSERT(entities, "UIContainer::addEntities: null entities");

	int32 i = 0;

	for(;NULL != entities && NULL != entities[i].entitySpec; i++)
	{
		UIContainer::addChildEntity(this, &entities[i]);
	}
}

// add entity to the stage
Entity UIContainer::addChildEntity(const PositionedEntity* const positionedEntity)
{
	if(NULL != positionedEntity)
	{
		Entity entity = Entity::loadEntity(positionedEntity, !isDeleted(this->children) ? VirtualList::getSize(this->children) : 0);
		ASSERT(entity, "UIContainer::doAddChildEntity: entity not loaded");

		if(!isDeleted(entity))
		{
			// create the entity and add it to the world
			UIContainer::addChild(this, Container::safeCast(entity));

			// apply transformations
			Transformation environmentTransform = UIContainer::getEnvironmentTransform(this);
			Entity::initialTransform(entity, &environmentTransform);
			Entity::ready(entity, true);
		}

		return entity;
	}

	return NULL;
}
