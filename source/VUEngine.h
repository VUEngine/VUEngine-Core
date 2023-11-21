/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef VUENGINE_H_
#define VUENGINE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ListenerObject.h>
#include <VIPManager.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

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
#define PROCESS_NAME_SOUND_SETUP			"SOUND SET"
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


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class VUEngine;
class Clock;
class Stage;
class GameState;
class ToolState;
class StateMachine;
class CollisionManager;
class PhysicalWorld;
class CommunicationManager;
class SpriteManager;
class WireframeManager;
class SoundManager;
class SpatialObject;

extern VUEngine _vuEngine __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;

/// @ingroup base
singleton class VUEngine : ListenerObject
{
	// game's state machine
	StateMachine stateMachine;
	// game's current state
	GameState currentState;
	// engine's global timer
	Clock clock;
	// managers
	ClockManager clockManager;
	KeypadManager keypadManager;
	VIPManager vipManager;
	WireframeManager wireframeManager;
	SpriteManager spriteManager;
	TimerManager timerManager;
	CommunicationManager communicationManager;
	SoundManager soundManager;
	FrameRate frameRate;
	Camera camera;
	// current save data manager
	ListenerObject saveDataManager;
	// game's next state
	GameState nextState;
	// game's next state operation
	int32 nextStateOperation;
	// last process' name
	char* lastProcessName;
	// frame flags
	volatile bool currentGameCycleEnded;
	volatile bool nextGameCycleStarted;
	// random seed
	uint32 randomSeed;
	// game paused flag
	bool isPaused;

	/// @publicsection
	static VUEngine getInstance();
	static bool isConstructed();
	void pushFrontPostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void pushBackPostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void removePostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void addState(GameState state);
	void changeState(GameState state);
	void cleanAndChangeState(GameState state);
	void disableKeypad();
	void enableKeypad();
	void resetClock();
	Clock getClock();
	Clock getUpdateClock();
	Clock getMessagingClock();
	Clock getPhysicsClock();
	CollisionManager getCollisionManager();
	char* getLastProcessName();
	void setLastProcessName(char* processName);
	PhysicalWorld getPhysicalWorld();
	uint32 getTime();
	StateMachine getStateMachine();
	Stage getStage();
	GameState getCurrentState();
	uint16 getGameFrameDuration();
	void setGameFrameRate(uint16 gameFrameRate);
	bool isEnteringSpecialMode();
	bool isExitingSpecialMode();
	bool isPaused();
	bool isInSpecialMode();
	void pause(GameState pauseState);
	void printClassSizes(int32 x, int32 y);
	void reset(bool resetSounds);
	void start(GameState state);
	void unpause(GameState pauseState);
	void wait(uint32 milliSeconds);
	void startProfiling();
	void resetProfiling();
	void openTool(ToolState toolState);
	void nextFrameStarted(uint16 gameFrameDuration);
	void nextGameCycleStarted(uint16 gameFrameDuration);
	ListenerObject getSaveDataManager();
	void setSaveDataManager(ListenerObject saveDataManager);
	long getRandomSeed();
	
	override bool handleMessage(Telegram telegram);
}


extern uint32 _gameRandomSeed __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;

#endif
