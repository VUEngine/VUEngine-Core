/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ACTOR_H_
#define ACTOR_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <AnimatedEntity.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

class State;
class StateMachine;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

/// An Actor Spec
/// @memberof Actor
typedef struct ActorSpec
{
	/// AnimatedEntity spec
	AnimatedEntitySpec animatedEntitySpec;

	/// Axises around which to rotate the entity when syncronizing with body
	uint16 axisForSynchronizationWithBody;

} ActorSpec;

/// An Actor spec that is stored in ROM
/// @memberof Actor
typedef const ActorSpec ActorROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class Actor
///
/// Inherits from AnimatedEntity
///
/// Implements an animated entity that can have complex behavior and physical simulations.
class Actor : AnimatedEntity
{
	/// @protectedsection

	/// State machine to handle complex logic
	StateMachine stateMachine;

	/// @publicsection

	/// Class' constructor
	/// @param actorSpec: Specification that determines how to configure the actor
	/// @param internalId: ID to keep track internally of the new instance
	/// @param name: Name to assign to the new instance
	void constructor(const ActorSpec* actorSpec, int16 internalId, const char* const name);

	/// Process a Telegram.
	/// @param telegram: Telegram to process
	/// @return True if the Telegram was processed
	override bool handleMessage(Telegram telegram);

	/// Set the direction towards which the object must move.
	/// @param direction: Pointer to a direction vector
	override void setDirection(const Vector3D* direction);

	/// Set the local position.
	/// @param position: New local position
	override void setLocalPosition(const Vector3D* position);

	/// Update the local transformation in function of the provided environment transform.
	/// @param environmentTransform: New reference environment for the local transformation
	override void changeEnvironment(Transformation* environmentTransform);

	/// Update this instance's logic.
	override void update();

	/// Create the state machine and inintialize it with the provided state.
	/// @param state: State that the state machine must enter
	void createStateMachine(State state);
}

#endif
