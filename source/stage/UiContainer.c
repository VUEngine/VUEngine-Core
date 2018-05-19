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
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <UiContainer.h>
#include <Optics.h>
#include <Game.h>
#include <Camera.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	UiContainer
 * @extends Container
 * @ingroup stage
 */


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void UiContainer::constructor(UiContainerDefinition* uiContainerDefinition)
{
	// construct base object
	Base::constructor(NULL);

	// add entities in the definition
	 UiContainer::addEntities(this, uiContainerDefinition->entities);
}

// class's destructor
void UiContainer::destructor()
{
	// destroy base
	// must always be called at the end of the destructor
	Base::destructor();
}

// add entities
void UiContainer::addEntities(PositionedEntity* entities)
{
	ASSERT(entities, "UiContainer::addEntities: null entities");

	static int internalId = 0;
	int i = 0;

	for(;entities && entities[i].entityDefinition; i++)
	{
		Entity entity = Entity::loadEntity(&entities[i], internalId++);

		if(entity)
		{
			// setup graphics
			 Container::setupGraphics(entity);

			// create the entity and add it to the world
			Container::addChild(this, Container::safeCast(entity));

			// apply transformations
			Transformation environmentTransform = Container::getEnvironmentTransform(this);
			 Container::initialTransform(entity, &environmentTransform, true);

			 Entity::ready(entity, true);
		}
	}
}

// transformation
void UiContainer::transform(const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	Camera camera = Camera::getInstance();
	ASSERT(camera, "UiContainer::transform: null camera");

	Camera::prepareForUITransform(camera);

	Base::transform(this, environmentTransform, invalidateTransformationFlag);

	Camera::doneUITransform(camera);
}

// transformation
void UiContainer::initialTransform(const Transformation* environmentTransform, u32 recursive)
{
	Camera camera = Camera::getInstance();
	ASSERT(camera, "UiContainer::initialTransform: null camera");

	Vector3D originalCameraPosition  =
	{
		0, 0, 0
	};

	if(camera)
	{
		// must hack the camera position for my children's sprites
		// being properly rendered
		originalCameraPosition = Camera::getPosition(camera);

		Vector3D tempCameraPosition =
		{
			0, 0, 0
		};

		Camera::setPosition(camera, tempCameraPosition);
	}

	Base::initialTransform(this, environmentTransform, recursive);

	 Container::synchronizeGraphics(this);

	if(camera)
	{
		// recover camera
		Camera::setPosition(camera, originalCameraPosition);
	}
}
