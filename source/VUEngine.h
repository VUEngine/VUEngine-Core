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

extern VUEngine _vuEngine __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;

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

	/// Method to retrieve the singleton instance
	/// @return VUEngine singleton
	static VUEngine getInstance();

	/// Receive and process a Telegram.
	/// @param telegram: Received telegram to process
	/// @return True if the telegram was processed
	override bool handleMessage(Telegram telegram);

	/// Reset the engine's sub components.
	/// @param resetSounds: If false, any playing sounds will keep playing
	void reset(bool resetSounds);

	/// Reset the engine's main clock.
	void resetClock();

	/// Start the game with the provided game state.
	/// @param gameState: Game state the engine must enter when starting
	/// @return Return code (0)
	int32 start(GameState gameState);

	/// Pause the game by pushing the provided game state into the engine's state machine's stack.
	/// @param pauseState: Pause game state
	void pause(GameState pauseState);

	/// Unpause the game by removing the provided game state from the engine's state machine's stack.
	/// @param pauseState: Pause game state
	void unpause(GameState pauseState);

	/// Ste the current game state at the top of the engine's state machine's stack.
	/// @param gameState: Game state to set
	void setState(GameState gameState);

	/// Add a game state to the top of the engine's state machine's stack.
	/// @param state: Game state to push
	void addState(GameState state);

	/// Swap the game state at the top of the engine's state machine's stack wht the provided one.
	/// @param state: Game state to swap to
	void changeState(GameState state);

	/// Check if the engine's state machine is in a tool state.
	/// @return True if the engine's state machine is in a tool state
	bool isInToolState();

	/// Check if the engine's state machine is entering or exiting a tool state.
	/// @return True if the engine's state machine is entering or exiting a tool state
	bool isInToolStateTransition();

	/// Retrieve the current state.
	/// @return Current game state
	GameState getCurrentState();

	/// Retrieve the previous state.
	/// @return Previous game state
	GameState getPreviousState();

	/// Retrieve the current UI container.
	/// @return Current game state's UI container
	UIContainer getUIContainer();

	/// Retrieve the current stage.
	/// @return Current game state's stage
	Stage getStage();

	/// Retrieve the current game state's physical world.
	/// @return Current game state's physical world
	BodyManager getBodyManager();

	/// Retrieve the current game state's collision manager.
	/// @return Current game state's collision manager
	ColliderManager getColliderManager();

	/// Retrieve the engine's state machine.
	/// @return Engine's state machine
	StateMachine getStateMachine();

	/// Retrieve the engine's main clock.
	/// @return Engine's main clock
	Clock getClock();

	/// Retrieve the current game state's logics clock.
	/// @return Current game state's logics clock
	Clock getLogicsClock();

	/// Retrieve the current game state's messaging clock.
	/// @return Current game state's messaging clock
	Clock getMessagingClock();

	/// Retrieve the current game state's physics clock.
	/// @return Current game state's physics clock
	Clock getPhysicsClock();

	/// Retrieve the current process' name.
	/// @return Current process' name
	char* getProcessName();

	/// Retrieve the duration of game frames.
	/// @return Duration in milliseconds of game frames
	uint16 getGameFrameDuration();
	
	/// Set the target frame rate.
	/// @param gameFrameRate: New frame rate target
	void setGameFrameRate(uint16 gameFrameRate);

	/// Lock the frame rate.
	void lockFrameRate();

	/// Unlock the frame rate.
	void unlockFrameRate();

	/// Enable user input.
	void enableKeypad();

	/// Disable user input.
	void disableKeypad();

	/// Set the saved data manager.
	/// @param saveDataManager:: Save data manager to use
	void setSaveDataManager(ListenerObject saveDataManager);

	/// Retrieve the saved data manager.
	/// @return Save data manager
	ListenerObject getSaveDataManager();

	/// Push a post processing effect at the start of the list of effects.
	/// @param postProcessingEffect: Post-processing effect function
	/// @param entity: Post-processing effect function's scope
	void pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity);
	
	/// Push a post processing effect at the end of the list of effects.
	/// @param postProcessingEffect: Post-processing effect function
	/// @param entity: Post-processing effect function's scope
	void pushBackPostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity);

	/// Remove a post-processing effect from the list of effects.
	/// @param postProcessingEffect: Post-processing effect function
 	/// @param entity: Post-processing effect function's scope
	void removePostProcessingEffect(PostProcessingEffect postProcessingEffect, Entity entity);

	/// Called when the VIP reaches FRAMESTART.
	/// @param gameFrameDuration: Time in milliseconds that each game frame lasts.
	void frameStarted(uint16 gameFrameDuration);

	/// Called when the VIP reaches GAMESTART.
	/// @param gameFrameDuration: Time in milliseconds that each game frame lasts.
	void gameFrameStarted(uint16 gameFrameDuration);

	/// Check if the game is paused.
	/// @return True if the game is paused; false otherwise
	bool isPaused();

	/// Halt the game by the provided time.
	/// @param milliSeconds: Time to halt the game
	void wait(uint32 milliSeconds);

	/// Force the complete initialization of all graphics.
	void prepareGraphics();

	/// Start profiling the game.
	void startProfiling();

}

#endif
