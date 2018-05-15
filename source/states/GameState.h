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

class GameState : State
{
	/**
	* @var PhysicalWorld 		physicalWorld
	* @brief					a pointer to the game's stage
	* @memberof				GameState
	*/
	PhysicalWorld physicalWorld;
	/**
	* @var CollisionManager 	collisionManager
	* @brief					a pointer to the game's stage
	* @memberof				GameState
	*/
	CollisionManager collisionManager;
	/**
	* @var Stage 				stage
	* @brief					a pointer to the game's stage
	* @memberof				GameState
	*/
	Stage stage;
	/**
	* @var int 				canStream
	* @brief					flag to allow streaming
	* @memberof				GameState
	*/
	int canStream;
	/**
	* @var Vector3D 			cameraPosition
	* @brief					must save to allow pause
	* @memberof				GameState
	*/
	Vector3D cameraPosition;
	/**
	* @var Clock 				messagingClock
	* @brief					clock for messaging
	* @memberof				GameState
	*/
	Clock messagingClock;
	/**
	* @var Clock 				updateClock
	* @brief					clock for update cycle
	* @memberof				GameState
	*/
	Clock updateClock;
	/**
	* @var Clock 				physicsClock
	* @brief					clock for physics
	* @memberof				GameState
	*/
	Clock physicsClock;
	/**
	* @var u32 				previousUpdateTime
	* @brief					previous update time
	* @memberof				GameState
	*/
	u32 previousUpdateTime;

	void constructor(GameState this);
	CollisionManager getCollisionManager(GameState this);
	Clock getMessagingClock(GameState this);
	PhysicalWorld getPhysicalWorld(GameState this);
	Clock getPhysicsClock(GameState this);
	Stage getStage(GameState this);
	Clock getUpdateClock(GameState this);
	void loadStage(GameState this, StageDefinition* stageDefinition, VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition);
	void pauseAnimations(GameState this, bool pause);
	void pauseClocks(GameState this);
	void pauseMessagingClock(GameState this, bool pause);
	u32 processCollisions(GameState this);
	void pausePhysics(GameState this, bool pause);
	int propagateMessage(GameState this, int message);
	void resumeClocks(GameState this);
	void startAnimations(GameState this);
	void startClocks(GameState this);
	void startDispatchingDelayedMessages(GameState this);
	void startPhysics(GameState this);
	void stopClocks(GameState this);
	bool stream(GameState this);
	void updatePhysics(GameState this);
	void synchronizeGraphics(GameState this);
	virtual void processUserInput(GameState this, UserInput userInput);
	virtual void transform(GameState this);
	override void enter(GameState this, void* owner);
	override void execute(GameState this, void* owner);
	override void exit(GameState this, void* owner);
	override void suspend(GameState this, void* owner);
	override void resume(GameState this, void* owner);
	override bool processMessage(GameState this, void* owner, Telegram telegram);
}


#endif
