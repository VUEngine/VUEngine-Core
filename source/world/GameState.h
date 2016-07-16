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

#ifndef GAME_STATE_H_
#define GAME_STATE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <State.h>
#include <Telegram.h>
#include <Stage.h>
#include <Clock.h>
#include <PhysicalWorld.h>
#include <CollisionManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define GameState_METHODS(ClassName)																	\
		State_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, void, transform);														\

// declare the virtual methods which are redefined
#define GameState_SET_VTABLE(ClassName)																	\
		State_SET_VTABLE(ClassName)																		\
		__VIRTUAL_SET(ClassName, GameState, enter);														\
		__VIRTUAL_SET(ClassName, GameState, execute);													\
		__VIRTUAL_SET(ClassName, GameState, exit);														\
		__VIRTUAL_SET(ClassName, GameState, suspend);													\
		__VIRTUAL_SET(ClassName, GameState, resume);													\
		__VIRTUAL_SET(ClassName, GameState, processMessage);											\
		__VIRTUAL_SET(ClassName, GameState, transform);													\

#define GameState_ATTRIBUTES																			\
        /* super's attributes */																		\
        State_ATTRIBUTES;																				\
        /* a pointer to the game's stage */																\
        PhysicalWorld physicalWorld;																	\
        /* a pointer to the game's stage */																\
        CollisionManager collisionManager;																\
        /* a pointer to the game's stage */																\
        Stage stage;																					\
        /* flag to allow streaming */																	\
        int canStream;																					\
        /* must save to allow pause */																	\
        VBVec3D screenPosition;																			\
        /* timer to use in game */																		\
        Clock inGameClock;																				\
        /* timer to use in game for animations */														\
        Clock animationsClock;																			\
        /* timer to use for physics */																	\
        Clock physicsClock;																				\

__CLASS(GameState);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void GameState_constructor(GameState this);
void GameState_destructor(GameState this);
void GameState_enter(GameState this, void* owner);
void GameState_execute(GameState this, void* owner);
void GameState_exit(GameState this, void* owner);
void GameState_suspend(GameState this, void* owner);
void GameState_resume(GameState this, void* owner);
bool GameState_processMessage(GameState this, void* owner, Telegram telegram);
void GameState_transform(GameState this);
void GameState_updateVisuals(GameState this);
int GameState_propagateMessage(GameState this, int message);
void GameState_loadStage(GameState this, StageDefinition* stageDefinition, VirtualList entityNamesToIgnore, bool overrideScreenPosition);
void GameState_setCanStream(GameState this, int canStream);
bool GameState_canStream(GameState this);
Stage GameState_getStage(GameState this);
const Clock GameState_getInGameClock(GameState this);
const Clock GameState_getAnimationsClock(GameState this);
const Clock GameState_getPhysicsClock(GameState this);
void GameState_startClocks(GameState this);
void GameState_pauseClocks(GameState this);
void GameState_resumeClocks(GameState this);
void GameState_startInGameClock(GameState this);
void GameState_startAnimations(GameState this);
void GameState_startPhysics(GameState this);
void GameState_pauseInGameClock(GameState this, bool pause);
void GameState_pauseAnimations(GameState this, bool pause);
void GameState_pausePhysics(GameState this, bool pause);

void GameState_updatePhysics(GameState this);
PhysicalWorld GameState_getPhysicalWorld(GameState this);
void GameState_processCollisions(GameState this);
CollisionManager GameState_getCollisionManager(GameState this);

#endif
