/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef GAME_H_
#define GAME_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Clock.h>
#include <Stage.h>
#include <GameState.h>
#include <ToolState.h>
#include <StateMachine.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <VIPManager.h>
#include <CommunicationManager.h>
#include <SoundManager.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------

#define PROCESS_NAME_CAMERA					"CAMERA"
#define PROCESS_NAME_COLLISIONS				"COLLISIONS"
#define PROCESS_NAME_END_FRAME				"END FRAME"
#define PROCESS_NAME_GRAPHICS				"GRAPHICS"
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

/// @ingroup base
singleton class Game : ListenerObject
{
	// game's state machine
	StateMachine stateMachine;
	// game's current state
	GameState currentState;
	// engine's global timer
	Clock clock;
	// managers
	ClockManager clockManager;
	//
	KeypadManager keypadManager;
	//
	VIPManager vipManager;
	//
	TimerManager timerManager;
	//
	CommunicationManager communicationManager;
	//
	SoundManager soundManager;
	// current save data manager
	ListenerObject saveDataManager;
	//
	FrameRate frameRate;
	//
	Camera camera;
	// game's next state
	GameState nextState;
	// game's next state operation
	int32 nextStateOperation;
	// last process' name
	char* lastProcessName;
	// frame flags
	volatile bool currentFrameEnded;
	volatile bool nextFrameStarted;
	// random seed
	uint32 randomSeed;
	// game paused flag
	bool isPaused;

	/// @publicsection
	static Game getInstance();
	static bool isConstructed();
	void pushFrontProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void pushBackProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void addState(GameState state);
	void changeState(GameState state);
	void cleanAndChangeState(GameState state);
	void disableKeypad();
	void enableKeypad();
	Clock getClock();
	CollisionManager getCollisionManager();
	char* getLastProcessName();
	Clock getMessagingClock();
	Optical getOptical();
	Clock getPhysicsClock();
	PhysicalWorld getPhysicalWorld();
	uint32 getTime();
	StateMachine getStateMachine();
	Stage getStage();
	GameState getCurrentState();
	Clock getUpdateClock();
	bool isEnteringSpecialMode();
	bool isExitingSpecialMode();
	bool isPaused();
	bool isInSpecialMode();
	void pause(GameState pauseState);
	void printClassSizes(int32 x, int32 y);
	void removePostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void reset(bool resetSounds);
	void resetProfiling();
	void setOptical(Optical optical);
	void start(GameState state);
	void unpause(GameState pauseState);
	void wait(uint32 milliSeconds);
	void setLastProcessName(char* processName);
	bool isInDebugMode();
	bool isInStageEditor();
	bool isInAnimationInspector();
	bool isInSoundTest();
	void openTool(ToolState toolState);
	void nextFrameStarted();
	bool hasCurrentFrameEnded();
	void saveProcessNameDuringFRAMESTART();
	void saveProcessNameDuringXPEND();
	override bool handleMessage(Telegram telegram);
	void registerSaveDataManager(ListenerObject saveDataManager);
	ListenerObject getSaveDataManager();
	long getRandomSeed();
	void startProfiling();
}


extern uint32 _gameRandomSeed;

#endif
