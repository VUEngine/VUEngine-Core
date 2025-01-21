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

#include <Camera.h>
#include <Printing.h>
#include <VirtualList.h>

#include "UIContainer.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void UIContainer::constructor(PositionedActor* childrenPositionedActors)
{
	// Always explicitly call the base's constructor 
	Base::constructor(0, NULL);

	for(int16 i = 0; NULL != childrenPositionedActors && NULL != childrenPositionedActors[i].actorSpec; i++)
	{
		UIContainer::spawnChildActor(this, &childrenPositionedActors[i]);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void UIContainer::destructor()
{
	this->deleteMe = true;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void UIContainer::transform
(
	const Transformation* environmentTransform __attribute__((unused)), uint8 invalidateTransformationFlag __attribute__((unused))
)
{
	extern Transformation _neutralEnvironmentTransformation;

	this->localTransformation.position = *_cameraPosition;
	this->localTransformation.rotation = *_cameraInvertedRotation;
	this->transformation.invalid = __INVALIDATE_POSITION | __INVALIDATE_ROTATION;

	Base::transform(this, &_neutralEnvironmentTransformation, __INVALIDATE_POSITION | __INVALIDATE_ROTATION);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Actor UIContainer::spawnChildActor(const PositionedActor* const positionedActor)
{
	if(NULL != positionedActor)
	{
		Actor actor = Actor::createActor(positionedActor, !isDeleted(this->children) ? VirtualList::getCount(this->children) : 0);
		ASSERT(actor, "UIContainer::doAddChildActor: actor not loaded");

		if(!isDeleted(actor))
		{
			// Create the actor and add it to the world
			UIContainer::addChild(this, Container::safeCast(actor));
		}

		return actor;
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
