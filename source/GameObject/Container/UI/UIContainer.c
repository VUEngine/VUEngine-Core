/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Camera.h>
#include <Printing.h>
#include <VirtualList.h>

#include "UIContainer.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

static Camera _camera = NULL;


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void UIContainer::constructor(PositionedEntity* childrenPositionedEntities)
{
	_camera = Camera::getInstance();

	// Always explicitly call the base's constructor 
	Base::constructor(0, NULL);

	for(int16 i = 0; NULL != childrenPositionedEntities && NULL != childrenPositionedEntities[i].entitySpec; i++)
	{
		UIContainer::spawnChildEntity(this, &childrenPositionedEntities[i]);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void UIContainer::destructor()
{
	this->deleteMe = true;

	// destroy base

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

void UIContainer::transform(const Transformation* environmentTransform __attribute__((unused)), uint8 invalidateTransformationFlag __attribute__((unused)))
{
	extern Transformation _neutralEnvironmentTransformation;

	this->localTransformation.position = *_cameraPosition;
	this->localTransformation.rotation = *_cameraInvertedRotation;
	this->transformation.invalid = __INVALIDATE_POSITION | __INVALIDATE_ROTATION;

	Base::transform(this, &_neutralEnvironmentTransformation, __INVALIDATE_POSITION | __INVALIDATE_ROTATION);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

Entity UIContainer::spawnChildEntity(const PositionedEntity* const positionedEntity)
{
	if(NULL != positionedEntity)
	{
		Entity entity = Entity::createEntity(positionedEntity, !isDeleted(this->children) ? VirtualList::getCount(this->children) : 0);
		ASSERT(entity, "UIContainer::doAddChildEntity: entity not loaded");

		if(!isDeleted(entity))
		{
			// create the entity and add it to the world
			UIContainer::addChild(this, Container::safeCast(entity));
		}

		return entity;
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————

