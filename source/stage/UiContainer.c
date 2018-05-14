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

#define UiContainer_ATTRIBUTES								    		    		        			\
        Container_ATTRIBUTES								    		    			        	    \

/**
 * @class	UiContainer
 * @extends Container
 * @ingroup stage
 */
__CLASS_DEFINITION(UiContainer, Container);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(UiContainer, UiContainerDefinition* uiContainerDefinition)
__CLASS_NEW_END(UiContainer, uiContainerDefinition);

// class's constructor
void UiContainer::constructor(UiContainer this, UiContainerDefinition* uiContainerDefinition)
{
	ASSERT(this, "UiContainer::constructor: null this");

	// construct base object
	Base::constructor(this, NULL);

	// add entities in the definition
	 UiContainer::addEntities(this, uiContainerDefinition->entities);
}

// class's destructor
void UiContainer::destructor(UiContainer this)
{
	ASSERT(this, "UiContainer::destructor: null this");

	// destroy base
	// must always be called at the end of the destructor
	Base::destructor();
}

// add entities
void UiContainer::addEntities(UiContainer this, PositionedEntity* entities)
{
	ASSERT(this, "UiContainer::addEntities: null this");
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
			Container::addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, entity));

			// apply transformations
			Transformation environmentTransform = Container::getEnvironmentTransform(__SAFE_CAST(Container, this));
			 Container::initialTransform(entity, &environmentTransform, true);

			 Entity::ready(entity, true);
		}
	}
}

// transformation
void UiContainer::transform(UiContainer this, const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	ASSERT(this, "UiContainer::transform: null this");

	Camera camera = Camera::getInstance();
	ASSERT(camera, "UiContainer::transform: null camera");

	Camera::prepareForUITransform(camera);

	Base::transform(this, environmentTransform, invalidateTransformationFlag);

	Camera::doneUITransform(camera);
}

// transformation
void UiContainer::initialTransform(UiContainer this, const Transformation* environmentTransform, u32 recursive)
{
	ASSERT(this, "UiContainer::initialTransform: null this");

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
