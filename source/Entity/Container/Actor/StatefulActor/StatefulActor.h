/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef STATEFUL_ACTOR_H_
#define STATEFUL_ACTOR_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Actor.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class State;
class StateMachine;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// An StatefulActor Spec
/// @memberof StatefulActor
typedef struct StatefulActorSpec
{
	/// Actor spec
	ActorSpec actorSpec;

} StatefulActorSpec;

/// An StatefulActor spec that is stored in ROM
/// @memberof StatefulActor
typedef const StatefulActorSpec StatefulActorROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class StatefulActor
///
/// Inherits from Actor
///
/// Implements an animated actor that can have complex behavior and physical simulations.
class StatefulActor : Actor
{
	/// @protectedsection

	/// State machine to handle complex logic
	StateMachine stateMachine;

	/// @publicsection

	/// Class' constructor
	/// @param statefulActorSpec: Specification that determines how to configure the statefulActor
	/// @param internalId: ID to keep track internally of the new instance
	/// @param name: Name to assign to the new instance
	void constructor(const StatefulActorSpec* statefulActorSpec, int16 internalId, const char* const name);

	/// Process a Telegram.
	/// @param telegram: Telegram to process
	/// @return True if the Telegram was processed
	override bool handleMessage(Telegram telegram);

	/// Update this instance's logic.
	override void update();

	/// Create the state machine and inintialize it with the provided state.
	/// @param state: State that the state machine must enter
	void createStateMachine(State state);
}

#endif
