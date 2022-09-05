/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef UI_CONTAINER_H_
#define UI_CONTAINER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Container.h>
#include <Entity.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

// defines a UI for ROM memory
typedef struct UIContainerSpec
{
	// ui's entities
	PositionedEntity* entities;

	// class allocator
	AllocatorPointer allocator;

} UIContainerSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage
class UIContainer : Container
{
	/// @publicsection
	void constructor(UIContainerSpec* uiContainerSpec);
	Entity addChildEntity(const PositionedEntity* const positionedEntity);
	void addEntities(PositionedEntity* entities);
	override void synchronizeGraphics();
}


#endif
