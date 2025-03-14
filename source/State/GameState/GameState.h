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

#include <ComponentManager.h>
#include <State.h>
#include <KeypadManager.h>
#include <Stage.h>
#include <UIContainer.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Actor;
class Clock;
class Telegram;
class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

enum GameState::stopClocks
{
	kGameStateAnimationsClock = 0,
	kGameStateLogicsClock,
	kGameStateMessagingClock,
	kGameStatePhysicsClock,

	kGameStateNoClock,
};

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
	/// @protectedsection

	/// A container for actors that componse the UI
	UIContainer uiContainer;

	/// A container for the game entites 
	Stage stage;

	/// Array of component managers
	ComponentManager componentManagers[kComponentTypes];
	
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

	/// Flags to enable or disable the behavior processing
	bool processBehaviors;

	/// Flags to enable or disable the mutator processing
	bool processMutators;

	/// Flags to enable or disable the physical simulations
	bool updatePhysics;

	/// Flags to enable or disable the collision detection and processing
	bool processCollisions;

	/// Each game state needs to keep track of the frame rate that it runs at
	/// in case it is paused and resumed
	uint8 framerate;

	/// If false, the game loop runs unlocked
	bool lockFrameRate;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Receive and process a Telegram.
	/// @param telegram: Received telegram to process
	/// @return True if the telegram was processed
	override bool handleMessage(Telegram telegram);

	/// Prepares the object to enter this state.
	/// @param owner: Object that is entering in this state
	override void start(void* owner);

	/// Updates the object in this state.
	/// @param owner: Object that is in this state
	override void update(void* owner);
	
	/// Prepares the object to exit this state.
	/// @param owner: Object that is exiting this state
	override void stop(void* owner);

	/// Prepares the object to become inactive in this state.
	/// @param owner: Object that is in this state
	override void pause(void* owner);

	/// Prepares the object to become active in this state.
	/// @param owner: Object that is in this state
	override void unpause(void* owner);
	
	/// Process a Telegram sent to an object that is in this state.
	/// @param owner: Object that is in this state
	/// @param telegram: Telegram to process
	override bool processMessage(void* owner, Telegram telegram);

	/// Configure the stage with the provided stage spec.
	/// @param stageSpec: Specification that determines how to configure the stage
	/// @param positionedActorsToIgnore: List of positioned actor structs to register for streaming
	void configureStage(StageSpec* stageSpec, VirtualList positionedActorsToIgnore);

	/// Retrieve the UI container.
	/// @return UI Container
	UIContainer getUIContainer();

	/// Force the purging of deleted components.
	void purgeComponentManagers();

	/// Start all the clocks.
	void startClocks();

	/// Pause all the clocks.
	void pauseClocks();

	/// Unpause all the clocks.
	void unpauseClocks();

	/// Stop all the clocks.
	void stopClocks();

	/// Start the clock used for logics.
	/// @param clockEnum: Enum that identifies the clock to start
	void startClock(uint32 clockEnum);

	/// Pause the clock used for logics.
	/// @param clockEnum: Enum that identifies the clock to pause
	void pauseClock(uint32 clockEnum);

	/// Unpause the clock used for logics.
	/// @param clockEnum: Enum that identifies the clock to unpause
	void unpauseClock(uint32 clockEnum);

	/// Stop the clock used for logics.
	/// @param clockEnum: Enum that identifies the clock to stop
	void stopClock(uint32 clockEnum);

	/// Update the UI's children' global transformations.
	void applyTransformationsUI();

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

	/// Force to completely stream in and out actors and to initialize all.
	void streamAll();

	/// Check if the framerate is locked or not
	/// @return True if the framerate is locked; false otherwise
	bool lockFrameRate();

	/// Print the clocks.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	void printClocks(int16 x, int16 y);

	/// Prepares the object to enter this state.
	/// @param owner: Object that is entering in this state
	virtual void enter(void* owner);

	/// Updates the object in this state.
	/// @param owner: Object that is in this state
	virtual void execute(void* owner);
	
	/// Prepares the object to exit this state.
	/// @param owner: Object that is exiting this state
	virtual void exit(void* owner);

	/// Prepares the object to become inactive in this state.
	/// @param owner: Object that is in this state
	virtual void suspend(void* owner);

	/// Prepares the object to become active in this state.
	/// @param owner: Object that is in this state
	virtual void resume(void* owner);

	/// Process the provided user input.
	/// @param userInput: Struct with the current user input information
	virtual void processUserInput(const UserInput*  userInput);
}

#endif
