/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
#include <Profiler.h>
#include <FrameRate.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base
singleton class Game : Object
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
	// current auto pause state
	GameState autoPauseState;
	// current save data manager
	Object saveDataManager;
	//
	FrameRate frameRate;
	//
	Profiler profiler;
	//
	Camera camera;
	// game's next state
	GameState nextState;
	// game's next state operation
	int nextStateOperation;
	// last process' name
	char* lastProcessName;
	// elapsed time in current 50hz cycle
	u32 gameFrameTotalTime;
	// frame flags
	volatile bool currentFrameEnded;
	volatile bool nextFrameStarted;
	// random seed
	long randomSeed;
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
	u32 getTime();
	StateMachine getStateMachine();
	Stage getStage();
	GameState getCurrentState();
	Clock getUpdateClock();
	bool isEnteringSpecialMode();
	bool isExitingSpecialMode();
	bool isPaused();
	bool isInSpecialMode();
	void pause(GameState pauseState);
	void printClassSizes(int x, int y);
	void removePostProcessingEffect(PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void reset();
	void resetProfiling();
	void setOptical(Optical optical);
	void showLastGameFrameProfiling(int x, int y);
	void start(GameState state);
	void unpause(GameState pauseState);
	void wait(u32 milliSeconds);
	void setLastProcessName(char* processName);
	bool isInDebugMode();
	bool isInStageEditor();
	bool isInAnimationInspector();
	bool isInSoundTest();
	void openTool(ToolState toolState);
	void nextFrameStarted(u16 gameFrameDuration);
	bool hasCurrentFrameEnded();
	void increaseGameFrameDuration(u32 gameFrameDuration);
	void saveProcessNameDuringFRAMESTART();
	void saveProcessNameDuringXPEND();
	override bool handleMessage(Telegram telegram);
	void registerAutoPauseState(GameState autoPauseState);
	GameState getAutoPauseState();
	void registerSaveDataManager(Object saveDataManager);
	Object getSaveDataManager();
	long getRandomSeed();
	void startProfiling();
}


#endif
