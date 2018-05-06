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

static void UiContainer_constructor(UiContainer this, UiContainerDefinition* uiContainerDefinition);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(UiContainer, UiContainerDefinition* uiContainerDefinition)
__CLASS_NEW_END(UiContainer, uiContainerDefinition);

// class's constructor
static void UiContainer_constructor(UiContainer this, UiContainerDefinition* uiContainerDefinition)
{
	ASSERT(this, "UiContainer::constructor: null this");

	// construct base object
	Base_constructor(this, NULL);

	// add entities in the definition
	 UiContainer_addEntities(this, uiContainerDefinition->entities);
}

// class's destructor
void UiContainer_destructor(UiContainer this)
{
	ASSERT(this, "UiContainer::destructor: null this");

	// destroy base
	// must always be called at the end of the destructor
	Base_destructor();
}

// add entities
void UiContainer_addEntities(UiContainer this, PositionedEntity* entities)
{
	ASSERT(this, "UiContainer::addEntities: null this");
	ASSERT(entities, "UiContainer::addEntities: null entities");

	static int internalId = 0;
	int i = 0;

	for(;entities && entities[i].entityDefinition; i++)
	{
		Entity entity = Entity_loadEntity(&entities[i], internalId++);

		if(entity)
		{
			// setup graphics
			 Container_setupGraphics(entity);

			// create the entity and add it to the world
			Container_addChild(__SAFE_CAST(Container, this), __SAFE_CAST(Container, entity));

			// apply transformations
			Transformation environmentTransform = Container_getEnvironmentTransform(__SAFE_CAST(Container, this));
			 Container_initialTransform(entity, &environmentTransform, true);

			 Entity_ready(entity, true);
		}
	}
}

// transformation
void UiContainer_transform(UiContainer this, const Transformation* environmentTransform, u8 invalidateTransformationFlag)
{
	ASSERT(this, "UiContainer::transform: null this");

	Camera camera = Camera_getInstance();
	ASSERT(camera, "UiContainer::transform: null camera");

	Camera_prepareForUITransform(camera);

	Base_transform(this, environmentTransform, invalidateTransformationFlag);

	Camera_doneUITransform(camera);
}

// transformation
void UiContainer_initialTransform(UiContainer this, Transformation* environmentTransform, u32 recursive)
{
	ASSERT(this, "UiContainer::initialTransform: null this");

	Camera camera = Camera_getInstance();
	ASSERT(camera, "UiContainer::initialTransform: null camera");

	Vector3D originalCameraPosition  =
	{
		0, 0, 0
	};

	if(camera)
	{
		// must hack the camera position for my children's sprites
		// being properly rendered
		originalCameraPosition = Camera_getPosition(camera);

		Vector3D tempCameraPosition =
		{
			0, 0, 0
		};

		Camera_setPosition(camera, tempCameraPosition);
	}

	Base_initialTransform(this, environmentTransform, recursive);

	 Container_synchronizeGraphics(this);

	if(camera)
	{
		// recover camera
		Camera_setPosition(camera, originalCameraPosition);
	}
}
