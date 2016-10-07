/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef GAME_H_
#define GAME_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Clock.h>
#include <Stage.h>
#include <GameState.h>
#include <StateMachine.h>
#include <CollisionManager.h>
#include <PhysicalWorld.h>
#include <VIPManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Game_METHODS(ClassName)																			\
		Object_METHODS(ClassName)																		\

// declare the virtual methods which are redefined
#define Game_SET_VTABLE(ClassName)																		\
		Object_SET_VTABLE(ClassName)																	\
		__VIRTUAL_SET(ClassName, Game, handleMessage);              									\

__CLASS(Game);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

Game Game_getInstance();

void Game_destructor(Game this);
void Game_start(Game this, GameState state);
void Game_changeState(Game this, GameState state);
void Game_addState(Game this, GameState state);
void Game_disableHardwareInterrupts(Game this);
void Game_enableHardwareInterrupts(Game this);
void Game_reset(Game this);
bool Game_handleMessage(Game this, Telegram telegram);
u32 Game_getTime(Game this);
Clock Game_getClock(Game this);
Clock Game_getMessagingClock(Game this);
Clock Game_getUpdateClock(Game this);
Clock Game_getPhysicsClock(Game this);
void Game_printClassSizes(int x, int y);
char* Game_getLastProcessName(Game this);
Optical Game_getOptical(Game this);
void Game_setOptical(Game this, Optical optical);
bool Game_isInSpecialMode(Game this);
bool Game_isEnteringSpecialMode(Game this);
bool Game_isExitingSpecialMode(Game this);
StateMachine Game_getStateMachine(Game this);
Stage Game_getStage(Game this);
GameState Game_getCurrentState(Game this);
void Game_pause(Game this, GameState pauseState);
void Game_unpause(Game this, GameState pauseState);
void Game_setAutomaticPauseState(Game this, GameState automaticPauseState);
GameState Game_getAutomaticPauseState(Game this);
void Game_disableKeypad(Game this);
void Game_enableKeypad(Game this);
void Game_addPostProcessingEffect(Game this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
void Game_removePostProcessingEffect(Game this, PostProcessingEffect postProcessingEffect, SpatialObject spatialObject);
CollisionManager Game_getCollisionManager(Game this);
PhysicalWorld Game_getPhysicalWorld(Game this);
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

#ifdef __PROFILE_GAME
void Game_showProfiling(Game this);
#endif

#endif
