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
#include <KeypadManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup states
class GameState : State
{
	// a pointer to the game's stage
	PhysicalWorld physicalWorld;
	// a pointer to the game's stage
	CollisionManager collisionManager;
	// a pointer to the game's stage
	Stage stage;
	// flag to allow streaming
	int canStream;
	// must save to allow pause
	Vector3D cameraPosition;
	// clock for messaging
	Clock messagingClock;
	// clock for update cycle
	Clock updateClock;
	// clock for physics
	Clock physicsClock;
	// previous update time
	u32 previousUpdateTime;

	/// @publicsection
	void constructor();
	CollisionManager getCollisionManager();
	Clock getMessagingClock();
	PhysicalWorld getPhysicalWorld();
	Clock getPhysicsClock();
	Stage getStage();
	Clock getUpdateClock();
	void loadStage(StageDefinition* stageDefinition, VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition);
	void pauseAnimations(bool pause);
	void pauseClocks();
	void pauseMessagingClock(bool pause);
	u32 processCollisions();
	void pausePhysics(bool pause);
	int propagateMessage(int message);
	void resumeClocks();
	void startAnimations();
	void startClocks();
	void startDispatchingDelayedMessages();
	void startPhysics();
	void stopClocks();
	bool stream();
	void updatePhysics();
	void synchronizeGraphics();
	virtual void processUserInput(UserInput userInput);
	virtual bool processUserInputRegardlessOfInput();
	virtual void transform();
	virtual bool isVersusMode();
	override void enter(void* owner);
	override void execute(void* owner);
	override void exit(void* owner);
	override void suspend(void* owner);
	override void resume(void* owner);
	override bool processMessage(void* owner, Telegram telegram);
}


#endif
