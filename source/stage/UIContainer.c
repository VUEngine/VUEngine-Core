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

#include <UIContainer.h>
#include <Optics.h>
#include <Game.h>
#include <Camera.h>


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

	for(;entities && entities[i].entitySpec; i++)
	{
		UIContainer::addChildEntity(this, &entities[i]);
	}
}

// add entity to the stage
Entity UIContainer::addChildEntity(const PositionedEntity* const positionedEntity)
{
	if(positionedEntity)
	{
		Entity entity = Entity::loadEntity(positionedEntity, !isDeleted(this->children) ? VirtualList::getSize(this->children) : 0);
		ASSERT(entity, "UIContainer::doAddChildEntity: entity not loaded");

		if(entity)
		{
			// create the entity and add it to the world
			UIContainer::addChild(this, Container::safeCast(entity));

			// apply transformations
			Transformation environmentTransform = Container::getEnvironmentTransform(this);
			Entity::initialTransform(entity, &environmentTransform, true);

			Entity::synchronizeGraphics(entity);
			Entity::ready(entity, true);
		}

		return entity;
	}

	return NULL;
}

void UIContainer::synchronizeGraphics()
{
	Camera camera = Camera::getInstance();
	ASSERT(camera, "UIContainer::transform: null camera");

	Camera::prepareForUI(camera);

	Base::synchronizeGraphics(this);

	Camera::doneUITransform(camera);
}

// transformation
void UIContainer::initialTransform(const Transformation* environmentTransform, uint32 recursive)
{
	Camera camera = Camera::getInstance();
	ASSERT(camera, "UIContainer::initialTransform: null camera");

	Vector3D originalCameraPosition  = Vector3D::zero();
	Rotation originalCameraRotation  = Rotation::zero();

	if(camera)
	{
		// must hack the camera position for my children's sprites
		// being properly rendered
		originalCameraPosition = Camera::getPosition(camera);
		originalCameraRotation = Camera::getRotation(camera);

		Camera::setPosition(camera, Vector3D::zero(), true);
		Camera::setRotation(camera, Rotation::zero());
	}

	Base::initialTransform(this, environmentTransform, recursive);

	Container::synchronizeGraphics(this);

	if(camera)
	{
		// recover camera
		Camera::setPosition(camera, originalCameraPosition, true);
		Camera::setRotation(camera, originalCameraRotation);
	}
}
