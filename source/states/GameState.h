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

#ifndef GAME_STATE_H_
#define GAME_STATE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <State.h>
#include <Telegram.h>
#include <Stage.h>
#include <Clock.h>
#include <PhysicalWorld.h>
#include <CollisionManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
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
		State_ATTRIBUTES																				\
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
		/* clock for messaging */																		\
		Clock messagingClock;																			\
		/* clock for update cycle */																	\
		Clock updateClock;																				\
		/* clock for physics */																			\
		Clock physicsClock;																				\
		/* previous update time */																		\
		u32 previousUpdateTime;																			\

__CLASS(GameState);


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

void GameState_constructor(GameState this);
void GameState_destructor(GameState this);
void GameState_enter(GameState this, void* owner);
void GameState_execute(GameState this, void* owner);
void GameState_exit(GameState this, void* owner);
void GameState_suspend(GameState this, void* owner);
void GameState_resume(GameState this, void* owner);
bool GameState_processMessage(GameState this, void* owner, Telegram telegram);
int GameState_propagateMessage(GameState this, int message);
void GameState_stream(GameState this);
void GameState_transform(GameState this);
void GameState_updateVisuals(GameState this);
void GameState_updatePhysics(GameState this);
u32 GameState_processCollisions(GameState this);
void GameState_loadStage(GameState this, StageDefinition* stageDefinition, VirtualList positionedEntitiesToIgnore, bool overrideScreenPosition);
Stage GameState_getStage(GameState this);
Clock GameState_getMessagingClock(GameState this);
Clock GameState_getUpdateClock(GameState this);
Clock GameState_getPhysicsClock(GameState this);
void GameState_startClocks(GameState this);
void GameState_stopClocks(GameState this);
void GameState_pauseClocks(GameState this);
void GameState_resumeClocks(GameState this);
void GameState_startDispatchingDelayedMessages(GameState this);
void GameState_startAnimations(GameState this);
void GameState_startPhysics(GameState this);
void GameState_pauseMessagingClock(GameState this, bool pause);
void GameState_pauseAnimations(GameState this, bool pause);
void GameState_pausePhysics(GameState this, bool pause);
PhysicalWorld GameState_getPhysicalWorld(GameState this);
CollisionManager GameState_getCollisionManager(GameState this);


#endif
