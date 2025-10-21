/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ACTOR_FACTORY_H_
#define ACTOR_FACTORY_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>
#include <Actor.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __ACTOR_PENDING_PROCESSING		0x00
#define __LIST_EMPTY					0x01
#define __ACTOR_PROCESSED				0x02

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class ActorFactory
///
/// Inherits from Object
///
/// Implements a factory that creates actors over time.
class ActorFactory : Object
{
	/// @protectedsection

	/// List of actors pending instantiation
	VirtualList actorsToInstantiate;

	/// List of actors pending transformation
	VirtualList actorsToTransform;

	/// List of actors pending being added to their parent
	VirtualList actorsToAddAsChildren;

	/// List of actors that have been completely instantianted and configured
	VirtualList spawnedActors;

	/// Index of the current phase to process for the instantiation and configuration
	/// of actors
	int32 instantiationPhase;

	/// @publicsection
	
	/// Class' constructor
	void constructor();

	/// Create a new actor instance and configure it with the provided arguments.
	/// @param positionedActor: Struct that defines which actor spec to use to configure the new actor
	/// and the spatial information about where and how to positione it
	/// @param parent: The parent of the new actor instance
	/// @param internalId: ID to keep track internally of the new instance
	/// @param highPriority: If true, the requested actor will be put at the top of the queue.
	void spawnActor(const PositionedActor* positionedActor, Container parent, int16 internalId, bool highPriority);

	/// Create the next queued actor.
	/// @return False if there are no actors pending instantiation; true otherwise
	bool createNextActor();

	/// Check if there are actors pending instantiation.
	/// @return True if there are actors pending instantiation; false otherwise
	bool hasActorsPending();

	/// Print the factory's state.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void print(int32 x, int32 y);

	/// @privatesection

	/// These are not meant to be called externally. They are declared here
	/// because of the preprocessor's limitations for forward declarations
	/// in source files. Don't call these.
	uint32 instantiateActors();
	uint32 transformActors();
	uint32 addChildActors();
	uint32 cleanUp();
}

#endif
