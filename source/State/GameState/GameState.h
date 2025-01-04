/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef GAME_STATE_H_
#define GAME_STATE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <State.h>
#include <KeypadManager.h>
#include <Stage.h>
#include <UIContainer.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Clock;
class ColliderManager;
class Actor;
class BodyManager;
class Telegram;
class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class GameState
///
/// Inherits from State
///
/// Implements a state that the VUEngine's state machine can transition into.
class GameState : State
{
	/// A container for entities that componse the UI
	UIContainer uiContainer;

	/// A container for the game entites 
	Stage stage;

	/// A world where physical bodies exist
	BodyManager bodyManager;

	/// A manager for collisions and colliders
	ColliderManager colliderManager;
	
	/// A clock for logics
	Clock logicsClock;

	/// A clock for messaging
	Clock messagingClock;

	/// A clock for animations
	Clock animationsClock;

	/// A clock for physics
	Clock physicsClock;
	
	/// Flags to enable or disable the streaming
	bool stream;

	/// Flags to enable or disable the transformations
	bool transform;
	
	/// Flags to enable or disable the physical simulations
	bool updatePhysics;

	/// Flags to enable or disable the collision detection and processing
	bool processCollisions;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Receive and process a Telegram.
	/// @param telegram: Received telegram to process
	/// @return True if the telegram was processed
	override bool handleMessage(Telegram telegram);

	/// Prepares the object to enter this state.
	/// @param owner: Object that is entering in this state
	override void enter(void* owner);

	/// Updates the object in this state.
	/// @param owner: Object that is in this state
	override void execute(void* owner);
	
	/// Prepares the object to exit this state.
	/// @param owner: Object that is exiting this state
	override void exit(void* owner);

	/// Prepares the object to become inactive in this state.
	/// @param owner: Object that is in this state
	override void suspend(void* owner);

	/// Prepares the object to become active in this state.
	/// @param owner: Object that is in this state
	override void resume(void* owner);

	/// Process a Telegram sent to an object that is in this state.
	/// @param owner: Object that is in this state
	/// @param telegram: Telegram to process
	override bool processMessage(void* owner, Telegram telegram);

	/// Configure the stage with the provided stage spec.
	/// @param stageSpec: Specification that determines how to configure the stage
	/// @param positionedEntitiesToIgnore: List of positioned actor structs to register for streaming
	void configureStage(StageSpec* stageSpec, VirtualList positionedEntitiesToIgnore);

	/// Retrieve the UI container.
	/// @return UI Container
	UIContainer getUIContainer();

	/// Retrieve the stage instance.
	/// @return Game state's stage
	Stage getStage();
	
	/// Retrieve the physical world.
	/// @return Game state's physical world
	BodyManager getBodyManager();

	/// Retrieve the collision manager.
	/// @return Game state's collision manager
	ColliderManager getColliderManager();

	/// Retrieve the clock that serves to control the game's logic.
	/// @return Game state's logics clock
	Clock getLogicsClock();

	/// Retrieve the clock that is used for the timing of messaging.
	/// @return Game state's messaging clock
	Clock getMessagingClock();

	/// Retrieve the clock that serves to control the animations.
	/// @return Game state's animations clocks
	Clock getAnimationsClock();

	/// Retrieve the clock that serves to control the game's physics.
	/// @return Game state's physics clock
	Clock getPhysicsClock();

	/// Start all the clocks.
	void startClocks();

	/// Pause all the clocks.
	void pauseClocks();

	/// Unpause all the clocks.
	void unpauseClocks();

	/// Stop all the clocks.
	void stopClocks();

	/// Start the clock used for logics.
	void startLogics();

	/// Pause the clock used for logics.
	void pauseLogics();

	/// Unpause the clock used for logics.
	void unpauseLogics();

	/// Start the clock used for delayed messages.
	void startMessaging();

	/// Pause the clock used for delayed messages.
	void pauseMessaging();

	/// Unpause the clock used for delayed messages.
	void unpauseMessaging();

	/// Start the clock used for animations.
	void startAnimations();

	/// Pause the clock used for animations.
	void pauseAnimations();

	/// Unpause the clock used for animations.
	void unpauseAnimations();

	/// Start the clock used for physics simulations.
	void startPhysics();

	/// Pause the clock used for physics simulations.
	void pausePhysics();

	/// Unpause the clock used for physics simulations.
	void unpausePhysics();

	/// Update the stage's children' global transformations.
	void transform();

	/// Update the UI's children' global transformations.
	void transformUI();

	/// Continue physics simulations.
	void simulatePhysics();
	
	/// Test and process collisions./
	void processCollisions();

	/// Propagate an integer message through the whole parenting hierarchy of the stage (children, grand children, etc.).
	/// @param message: The message to propagate
	/// @return True if some actor processed the message
	bool propagateMessage(int32 message);

	/// Propagate a string through the whole parenting hierarchy of the stage (children, grand children, etc.).
	/// @param string: The string to propagate
	/// @return True if some actor processed the string
	bool propagateString(const char* string);

	/// Find a stage's child (grand child, etc.) by its name.
	/// @param actorName: Name to look for
	Actor getActorByName(const char* actorName);

	/// Show a stage's child (grand child, etc.) with the provided name.
	/// @param actorName: Name to look for
	void showActorWithName(const char* actorName);

	/// Hide a stage's child (grand child, etc.) with the provided name.
	/// @param actorName: Name to look for
	void hideActorWithName(const char* actorName);

	/// Change the target frame rate.
	/// @param targetFPS: New target frame rate
	/// @param duration: Amount of time to keep the change on the frame rate before
	/// setting back the default target (0 or negative to make it permanent as long 
	/// as the state is active)
	void changeFramerate(int16 targetFPS, int32 duration);

	/// Force to completely stream in and out entities and to initialize all.
	void streamAll();

	/// Stream in or out the stage entities within or outside the camera's range.
	/// @return True if at least some actor was streamed in or out
	virtual bool stream();

	/// Process the provided user input.
	/// @param userInput: Struct with the current user input information
	virtual void processUserInput(const UserInput*  userInput);

	/// Check if the game state is in versus mode.
	/// @return True if the state is in versus mode; false otherwise
	virtual bool isVersusMode();
}

#endif
