/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ENTITY_FACTORY_H_
#define ENTITY_FACTORY_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>
#include <Entity.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __ENTITY_PENDING_PROCESSING		0x00
#define __LIST_EMPTY					0x01
#define __ENTITY_PROCESSED				0x02

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class EntityFactory
///
/// Inherits from Object
///
/// Implements a factory that creates entities over time.
class EntityFactory : Object
{
	/// @protectedsection

	/// List of entities pending instantiation
	VirtualList entitiesToInstantiate;

	/// List of entities pending transformation
	VirtualList entitiesToTransform;

	/// List of entities pending being added to their parent
	VirtualList entitiesToAddAsChildren;

	/// List of entities that have been completely instantianted and configured
	VirtualList spawnedEntities;

	/// Index of the current phase to process for the instantiation and configuration
	/// of entities
	int32 instantiationPhase;

	/// @publicsection
	
	/// Class' constructor
	void constructor();

	/// Create a new entity instance and configure it with the provided arguments.
	/// @param positionedEntity: Struct that defines which entity spec to use to configure the new entity
	/// and the spatial information about where and how to positione it
	/// @param parent: The parent of the new entity instance
	/// @param callback: Callback to inform the parent when the new entity is ready
	/// @param internalId: ID to keep track internally of the new instance
	void spawnEntity(const PositionedEntity* positionedEntity, Container parent, EventListener callback, int16 internalId);

	/// Create the next queued entity.
	/// @return False if there are no entities pending instantiation; true otherwise
	bool createNextEntity();

	/// Check if there are entities pending instantiation.
	/// @return True if there are entities pending instantiation; false otherwise
	bool hasEntitiesPending();

	/// Print the factory's state.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);

	/// @privatesection

	/// These are not meant to be called externally. They are declared here
	/// because of the preprocessor's limitations for forward declarations
	/// in source files. Don't call these.
	uint32 instantiateEntities();
	uint32 transformEntities();
	uint32 addChildEntities();
}

#endif
