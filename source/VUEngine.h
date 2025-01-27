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
class GameState;
class Entity;
class StateMachine;
class Stage;
class ToolState;
class UIContainer;
class VUEngine;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define PROCESS_NAME_BEHAVIORS				"BEHAVIORS"
#define PROCESS_NAME_CAMERA					"CAMERA"
#define PROCESS_NAME_COLLISIONS				"COLLISIONS"
#define PROCESS_NAME_END_FRAME				"END FRAME"
#define PROCESS_NAME_GRAPHICS				"GRAPHICS"
#define PROCESS_NAME_UI_GRAPHICS			"UI GRAPHICS"
#define PROCESS_NAME_INPUT					"INPUT"
#define PROCESS_NAME_EXECUTE_STATE			"EXECUTE"
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

	/// Current game state that the engine is in
	GameState currentGameState;

	/// Engine's main state machine
	StateMachine stateMachine;

	/// Saved data manager
	ListenerObject saveDataManager;

	/// Flag raised when the game loop is completed
	volatile bool currentGameCycleEnded;

	/// Flag raised upon VIP's GAMESTART
	volatile bool gameFrameStarted;
	
	/// If true, the game is paused
	bool isPaused;
	
	/// Flag raised when entering or exiting a tool state
	bool isInToolStateTransition;

	/// If false, the game loop runs unlocked
	bool lockFrameRate;

	/// @publicsection

	/// Check if the next game frame has started.
	/// @return True if the game frame has started
	static bool hasGameFrameStarted();

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

	/// Check if the engine's state machine is entering or exiting a tool state.
	/// @return True if the engine's state machine is entering or exiting a tool state
	static bool isInToolStateTransition();

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

	/// Set the saved data manager.
	/// @param saveDataManager:: Save data manager to use
	static void setSaveDataManager(ListenerObject saveDataManager);

	/// Retrieve the saved data manager.
	/// @return Save data manager
	static ListenerObject getSaveDataManager();

	/// Check if the game is paused.
	/// @return True if the game is paused; false otherwise
	static bool isPaused();

	/// Halt the game by the provided time.
	/// @param milliSeconds: Time to halt the game
	static void wait(uint32 milliSeconds);

	/// Start profiling the game.
	static void startProfiling();

	/// Process an event that the instance is listen for.
	/// @param eventFirer: ListenerObject that signals the event
	/// @param eventCode: Code of the firing event
	/// @return False if the listener has to be removed; true to keep it
	override bool onEvent(ListenerObject eventFirer, uint16 eventCode);

	/// Receive and process a Telegram.
	/// @param telegram: Received telegram to process
	/// @return True if the telegram was processed
	override bool handleMessage(Telegram telegram);
}

#endif
