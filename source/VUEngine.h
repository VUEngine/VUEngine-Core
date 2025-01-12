/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VUENGINE_H_
#define VUENGINE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <ListenerObject.h>
#include <VIPManager.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Clock;
class ColliderManager;
class GameState;
class BodyManager;
class Entity;
class StateMachine;
class Stage;
class ToolState;
class UIContainer;
class VUEngine;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define PROCESS_NAME_CAMERA					"CAMERA"
#define PROCESS_NAME_COLLISIONS				"COLLISIONS"
#define PROCESS_NAME_END_FRAME				"END FRAME"
#define PROCESS_NAME_GRAPHICS				"GRAPHICS"
#define PROCESS_NAME_UI_GRAPHICS			"UI GRAPHICS"
#define PROCESS_NAME_INPUT					"INPUT"
#define PROCESS_NAME_LOGIC					"LOGIC"
#define PROCESS_NAME_MESSAGES				"MESSAGES"
#define PROCESS_NAME_NEW_STATE				"NEW STATE"
#define PROCESS_NAME_PHYSICS				"PHYSICS"
#define PROCESS_NAME_STATE_POP				"POP STATE"
#define PROCESS_NAME_STATE_PUSH				"PUSH STATE"
#define PROCESS_NAME_STATE_SWAP				"SWAP STATE"
#define PROCESS_NAME_START_UP				"START UP"
#define PROCESS_NAME_STREAMING				"STREAMING"
#define PROCESS_NAME_TRANSFORMS				"TRANSFORMS"
#define PROCESS_NAME_RENDER					"RENDER"
#define PROCESS_NAME_VRAM_WRITE				"VRAM WRITE"
#define PROCESS_NAME_SOUND_PLAY				"SOUND PLAY"
#define PROCESS_NAME_COMMUNICATIONS			"COMMS"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class VUEngine
///
/// Inherits from ListenerObject
///
/// Implements the game's life cycle.
singleton class VUEngine : ListenerObject
{
	/// @protectedsection

	/// Engine's main state machine
	StateMachine stateMachine;

	/// Global timer
	Clock clock;

	/// Saved data manager
	ListenerObject saveDataManager;

	/// Current process' name
	char* processName;

	/// Flag used to measure the FPS
	volatile bool currentGameCycleEnded;

	/// Flag used to lock the frame rate
	volatile bool gameFrameStarted;
	
	/// If true, the game is paused
	bool isPaused;
	
	/// Flag raised when entering or exiting a tool state
	bool isInToolStateTransition;

	/// If false, the game loop runs unlocked
	bool syncToVIP;

	/// @publicsection

	/// @publicsection
	/// Register an object that will listen for events.
	/// @param listener: ListenerObject that listen for the event
	/// @param callback: EventListener callback for the listener object
	/// @param eventCode: Event's code to listen for
	static void registerEventListener(ListenerObject listener, EventListener callback, uint16 eventCode);

	/// Remove a specific listener object from the listening to a give code with the provided callback.
	/// @param listener: ListenerObject to remove from the list of listeners
	/// @param callback: EventListener callback for the listener object
	/// @param eventCode: Event's code to stop listen for
	static void unregisterEventListener(ListenerObject listener, EventListener callback, uint16 eventCode);

	/// Dispatch a message to the VUEngine instance
	/// @param delay: Milliseconds to wait before dispatching the message
	/// @param sender: Object that sends the message
	/// @param message: Message's code
	/// @param extraInfo: Pointer to any extra data that must accompany the message
	/// @return	Boolean indicating the status of the processing of the message if immediately dispatched
	static bool receieveMessage
	(
		uint32 delay, ListenerObject sender, int32 message, void* extraInfo
	);

	/// Reset the engine's sub components.
	/// @param resetSounds: If false, any playing sounds will keep playing
	static void reset(bool resetSounds);

	/// Reset the engine's main clock.
	static void resetClock();

	/// Start the game with the provided game state.
	/// @param gameState: Game state the engine must enter when starting
	/// @return Return code (0)
	static int32 start(GameState gameState);

	/// Pause the game by pushing the provided game state into the engine's state machine's stack.
	/// @param pauseState: Pause game state
	static void pause(GameState pauseState);

	/// Unpause the game by removing the provided game state from the engine's state machine's stack.
	/// @param pauseState: Pause game state
	static void unpause(GameState pauseState);

	/// Ste the current game state at the top of the engine's state machine's stack.
	/// @param gameState: Game state to set
	static void setState(GameState gameState);

	/// Add a game state to the top of the engine's state machine's stack.
	/// @param state: Game state to push
	static void addState(GameState state);

	/// Swap the game state at the top of the engine's state machine's stack wht the provided one.
	/// @param state: Game state to swap to
	static void changeState(GameState state);

	/// Check if the engine's state machine is in a tool state.
	/// @return True if the engine's state machine is in a tool state
	static bool isInToolState();

	/// Check if the engine's state machine is entering or exiting a tool state.
	/// @return True if the engine's state machine is entering or exiting a tool state
	static bool isInToolStateTransition();

	/// Retrieve the current state.
	/// @return Current game state
	static GameState getCurrentState();

	/// Retrieve the previous state.
	/// @return Previous game state
	static GameState getPreviousState();

	/// Retrieve the current UI container.
	/// @return Current game state's UI container
	static UIContainer getUIContainer();

	/// Retrieve the current stage.
	/// @return Current game state's stage
	static Stage getStage();

	/// Retrieve the current game state's physical world.
	/// @return Current game state's physical world
	static BodyManager getBodyManager();

	/// Retrieve the current game state's collision manager.
	/// @return Current game state's collision manager
	static ColliderManager getColliderManager();

	/// Retrieve the engine's state machine.
	/// @return Engine's state machine
	static StateMachine getStateMachine();

	/// Retrieve the engine's main clock.
	/// @return Engine's main clock
	static Clock getClock();

	/// Retrieve the current game state's logics clock.
	/// @return Current game state's logics clock
	static Clock getLogicsClock();

	/// Retrieve the current game state's messaging clock.
	/// @return Current game state's messaging clock
	static Clock getMessagingClock();

	/// Retrieve the current game state's physics clock.
	/// @return Current game state's physics clock
	static Clock getPhysicsClock();

	/// Retrieve the current process' name.
	/// @return Current process' name
	static char* getProcessName();

	/// Retrieve the duration of game frames.
	/// @return Duration in milliseconds of game frames
	static uint16 getGameFrameDuration();
	
	/// Set the target frame rate.
	/// @param gameFrameRate: New frame rate target
	static void setGameFrameRate(uint16 gameFrameRate);

	/// Lock the frame rate.
	static void lockFrameRate();

	/// Unlock the frame rate.
	static void unlockFrameRate();

	/// Enable user input.
	static void enableKeypad();

	/// Disable user input.
	static void disableKeypad();

	/// Set the saved data manager.
	/// @param saveDataManager:: Save data manager to use
	static void setSaveDataManager(ListenerObject saveDataManager);

	/// Retrieve the saved data manager.
	/// @return Save data manager
	static ListenerObject getSaveDataManager();

	/// Push a post processing effect at the start of the list of effects.
	/// @param postProcessingEffect: Post-processing effect function
	/// @param entity: Post-processing effect function's scope
	static void pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity);
	
	/// Push a post processing effect at the end of the list of effects.
	/// @param postProcessingEffect: Post-processing effect function
	/// @param entity: Post-processing effect function's scope
	static void pushBackPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity);

	/// Remove a post-processing effect from the list of effects.
	/// @param postProcessingEffect: Post-processing effect function
 	/// @param entity: Post-processing effect function's scope
	static void removePostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity);

	/// Called when the VIP reaches FRAMESTART.
	/// @param gameFrameDuration: Time in milliseconds that each game frame lasts.
	static void frameStarted(uint16 gameFrameDuration);

	/// Called when the VIP reaches GAMESTART.
	/// @param gameFrameDuration: Time in milliseconds that each game frame lasts.
	static void gameFrameStarted(uint16 gameFrameDuration);

	/// Check if the game is paused.
	/// @return True if the game is paused; false otherwise
	static bool isPaused();

	/// Halt the game by the provided time.
	/// @param milliSeconds: Time to halt the game
	static void wait(uint32 milliSeconds);

	/// Start profiling the game.
	static void startProfiling();

	/// Receive and process a Telegram.
	/// @param telegram: Received telegram to process
	/// @return True if the telegram was processed
	override bool handleMessage(Telegram telegram);
}

#endif
