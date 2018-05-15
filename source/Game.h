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
#include <StateMachine.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <VIPManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

singleton class Game : Object
{
	/**
	 * @var StateMachine	stateMachine
	 * @brief				game's state machine
	 * @memberof			Game
	 */
	StateMachine stateMachine;
	/**
	 * @var GameState		currentState
	 * @brief				game's current state
	 * @memberof			Game
	 */
	GameState currentState;
	/**
	 * @var Clock			clock
	 * @brief				engine's global timer
	 * @memberof			Game
	 */
	Clock clock;
	/**
	 * @var ClockManager	clockManager
	 * @brief				managers
	 * @memberof			Game
	 */
	ClockManager clockManager;
	/**
	 * @var KeypadManager 	keypadManager
	 * @brief
	 * @memberof			Game
	 */
	KeypadManager keypadManager;
	/**
	 * @var VIPManager 		vipManager
	 * @brief
	 * @memberof			Game
	 */
	VIPManager vipManager;
	/**
	 * @var TimerManager 	timerManager
	 * @brief
	 * @memberof			Game
	 */
	TimerManager timerManager;
	/**
	 * @var Camera 			camera
	 * @brief
	 * @memberof			Game
	 */
	Camera camera;
	/**
	 * @var GameState		nextState
	 * @brief				game's next state
	 * @memberof			Game
	 */
	GameState nextState;
	/**
	 * @var int				nextStateOperation
	 * @brief				game's next state operation
	 * @memberof			Game
	 */
	int nextStateOperation;
	/**
	 * @var char*			lastProcessName
	 * @brief				last process' name
	 * @memberof			Game
	 */
	char* lastProcessName;
	/**
	 * @var GameState		automaticPauseState
	 * @brief				auto pause state
	 * @memberof			Game
	 */
	GameState automaticPauseState;
	/**
	 * @var u32				lastAutoPauseCheckTime
	 * @brief				auto pause last checked time
	 * @memberof			Game
	 */
	u32 lastAutoPauseCheckTime;
	/**
	 * @var u32				gameFrameTotalTime
	 * @brief				elapsed time in current 50hz cycle
	 * @memberof			Game
	 */
	u32 gameFrameTotalTime;
	/**
	 * @var bool			isShowingLowBatteryIndicator
	 * @brief				low battery indicator showing flag
	 * @memberof			Game
	 */
	bool isShowingLowBatteryIndicator;
	/**
	 * @var bool			currentFrameEnded
	 * @brief				frame ended flag
	 * @memberof			Game
	 */
	volatile bool currentFrameEnded;

	static Game getInstance();
	void pushFrontProcessingEffect(Game this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void pushBackProcessingEffect(Game this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void addState(Game this, GameState state);
	void changeState(Game this, GameState state);
	void cleanAndChangeState(Game this, GameState state);
	void disableHardwareInterrupts(Game this);
	void disableKeypad(Game this);
	void enableHardwareInterrupts(Game this);
	void enableKeypad(Game this);
	GameState getAutomaticPauseState(Game this);
	Clock getClock(Game this);
	CollisionManager getCollisionManager(Game this);
	char* getLastProcessName(Game this);
	Clock getMessagingClock(Game this);
	Optical getOptical(Game this);
	Clock getPhysicsClock(Game this);
	PhysicalWorld getPhysicalWorld(Game this);
	u32 getTime(Game this);
	StateMachine getStateMachine(Game this);
	Stage getStage(Game this);
	GameState getCurrentState(Game this);
	Clock getUpdateClock(Game this);
	bool isEnteringSpecialMode(Game this);
	bool isExitingSpecialMode(Game this);
	bool isInSpecialMode(Game this);
	void pause(Game this, GameState pauseState);
	void printClassSizes(Game this, int x, int y);
	void removePostProcessingEffect(Game this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
	void reset(Game this);
	void resetProfiling(Game this);
	void setAutomaticPauseState(Game this, GameState automaticPauseState);
	void setOptical(Game this, Optical optical);
	void showLastGameFrameProfiling(Game this, int x, int y);
	void start(Game this, GameState state);
	void unpause(Game this, GameState pauseState);
	void wait(Game this, u32 milliSeconds);
	void setLastProcessName(Game this, char* processName);
	bool isInDebugMode(Game this);
	bool isInStageEditor(Game this);
	bool isInAnimationInspector(Game this);
	override bool handleMessage(Game this, Telegram telegram);
}


#endif
