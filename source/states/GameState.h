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
#include <KeyPadManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define GameState_METHODS(ClassName)																	\
		State_METHODS(ClassName)																		\
		__VIRTUAL_DEC(ClassName, void, processUserInput, UserInput userInput);							\
		__VIRTUAL_DEC(ClassName, void, transform);														\

// declare the virtual methods which are redefined
#define GameState_SET_VTABLE(ClassName)																	\
		State_SET_VTABLE(ClassName)																		\
		__VIRTUAL_SET(ClassName, GameState, enter);														\
		__VIRTUAL_SET(ClassName, GameState, execute);													\
		__VIRTUAL_SET(ClassName, GameState, exit);														\
		__VIRTUAL_SET(ClassName, GameState, suspend);													\
		__VIRTUAL_SET(ClassName, GameState, resume);													\
		__VIRTUAL_SET(ClassName, GameState, processUserInput);											\
		__VIRTUAL_SET(ClassName, GameState, processMessage);											\
		__VIRTUAL_SET(ClassName, GameState, transform);													\

#define GameState_ATTRIBUTES																			\
		State_ATTRIBUTES																				\
		/**
		 * @var PhysicalWorld 		physicalWorld
		 * @brief					a pointer to the game's stage
		 * @memberof				GameState
		 */																								\
		PhysicalWorld physicalWorld;																	\
		/**
		 * @var CollisionManager 	collisionManager
		 * @brief					a pointer to the game's stage
		 * @memberof				GameState
		 */																								\
		CollisionManager collisionManager;																\
		/**
		 * @var Stage 				stage
		 * @brief					a pointer to the game's stage
		 * @memberof				GameState
		 */																								\
		Stage stage;																					\
		/**
		 * @var int 				canStream
		 * @brief					flag to allow streaming
		 * @memberof				GameState
		 */																								\
		int canStream;																					\
		/**
		 * @var VBVec3D 			screenPosition
		 * @brief					must save to allow pause
		 * @memberof				GameState
		 */																								\
		VBVec3D screenPosition;																			\
		/**
		 * @var Clock 				messagingClock
		 * @brief					clock for messaging
		 * @memberof				GameState
		 */																								\
		Clock messagingClock;																			\
		/**
		 * @var Clock 				updateClock
		 * @brief					clock for update cycle
		 * @memberof				GameState
		 */																								\
		Clock updateClock;																				\
		/**
		 * @var Clock 				physicsClock
		 * @brief					clock for physics
		 * @memberof				GameState
		 */																								\
		Clock physicsClock;																				\
		/**
		 * @var u32 				previousUpdateTime
		 * @brief					previous update time
		 * @memberof				GameState
		 */																								\
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
CollisionManager GameState_getCollisionManager(GameState this);
Clock GameState_getMessagingClock(GameState this);
PhysicalWorld GameState_getPhysicalWorld(GameState this);
Clock GameState_getPhysicsClock(GameState this);
Stage GameState_getStage(GameState this);
Clock GameState_getUpdateClock(GameState this);
void GameState_loadStage(GameState this, StageDefinition* stageDefinition, VirtualList positionedEntitiesToIgnore, bool overrideScreenPosition);
void GameState_pauseAnimations(GameState this, bool pause);
void GameState_pauseClocks(GameState this);
void GameState_pauseMessagingClock(GameState this, bool pause);
u32 GameState_processCollisions(GameState this);
void GameState_processUserInput(GameState this, UserInput userInput);
bool GameState_processMessage(GameState this, void* owner, Telegram telegram);
void GameState_pausePhysics(GameState this, bool pause);
int GameState_propagateMessage(GameState this, int message);
void GameState_resume(GameState this, void* owner);
void GameState_resumeClocks(GameState this);
void GameState_startAnimations(GameState this);
void GameState_startClocks(GameState this);
void GameState_startDispatchingDelayedMessages(GameState this);
void GameState_startPhysics(GameState this);
void GameState_stopClocks(GameState this);
bool GameState_stream(GameState this);
void GameState_suspend(GameState this, void* owner);
void GameState_transform(GameState this);
void GameState_updatePhysics(GameState this);
void GameState_synchronizeGraphics(GameState this);


#endif
