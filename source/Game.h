/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

// declare the virtual methods
#define Game_METHODS(ClassName)																			\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define Game_SET_VTABLE(ClassName)																		\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Game, handleMessage);													\

__CLASS(Game);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

Game Game_getInstance();
void Game_destructor(Game this);

void Game_addPostProcessingEffect(Game this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
void Game_addState(Game this, GameState state);
void Game_changeState(Game this, GameState state);
void Game_cleanAndChangeState(Game this, GameState state);
void Game_disableHardwareInterrupts(Game this);
void Game_disableKeypad(Game this);
void Game_enableHardwareInterrupts(Game this);
void Game_enableKeypad(Game this);
GameState Game_getAutomaticPauseState(Game this);
Clock Game_getClock(Game this);
CollisionManager Game_getCollisionManager(Game this);
char* Game_getLastProcessName(Game this);
Clock Game_getMessagingClock(Game this);
Optical Game_getOptical(Game this);
Clock Game_getPhysicsClock(Game this);
PhysicalWorld Game_getPhysicalWorld(Game this);
u32 Game_getTime(Game this);
StateMachine Game_getStateMachine(Game this);
Stage Game_getStage(Game this);
GameState Game_getCurrentState(Game this);
Clock Game_getUpdateClock(Game this);
bool Game_handleMessage(Game this, Telegram telegram);
bool Game_isEnteringSpecialMode(Game this);
bool Game_isExitingSpecialMode(Game this);
bool Game_isInSpecialMode(Game this);
void Game_pause(Game this, GameState pauseState);
void Game_printClassSizes(int x, int y);
void Game_removePostProcessingEffect(Game this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
void Game_reset(Game this);
void Game_resetProfiling(Game this);
void Game_setAutomaticPauseState(Game this, GameState automaticPauseState);
void Game_setOptical(Game this, Optical optical);
void Game_showLastGameFrameProfiling(Game this, int x, int y);
void Game_start(Game this, GameState state);
void Game_unpause(Game this, GameState pauseState);
void Game_wait(Game this, u32 milliSeconds);

#ifdef __DEBUG_TOOLS
bool Game_isInDebugMode(Game this);
#endif
#ifdef __STAGE_EDITOR
bool Game_isInStageEditor(Game this);
#endif
#ifdef __ANIMATION_EDITOR
bool Game_isInAnimationEditor(Game this);
#endif


#endif
