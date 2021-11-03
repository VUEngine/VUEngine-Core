/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
	int32 canStream;
	// must save to allow pause
	Vector3D cameraPosition;
	// clock for messaging
	Clock messagingClock;
	// clock for update cycle
	Clock updateClock;
	// clock for physics
	Clock physicsClock;
	// previous update time
	uint32 previousUpdateTime;
	// Flags to disable some processes
	bool stream;
	bool transform;
	bool synchronizeGraphics;
	bool updatePhysics;
	bool processCollisions;

	/// @publicsection
	void constructor();
	CollisionManager getCollisionManager();
	Clock getMessagingClock();
	PhysicalWorld getPhysicalWorld();
	Clock getPhysicsClock();
	Stage getStage();
	Clock getUpdateClock();
	void loadStage(StageSpec* stageSpec, VirtualList positionedEntitiesToIgnore, bool overrideCameraPosition, bool forceNoPopIn);
	void pauseAnimations(bool pause);
	void pauseClocks();
	void pauseMessagingClock(bool pause);
	uint32 processCollisions();
	void pausePhysics(bool pause);
	int32 propagateMessage(int32 message);
	void resumeClocks();
	void startAnimations();
	void startClocks();
	void startDispatchingDelayedMessages();
	void startPhysics();
	void stopClocks();
	void updatePhysics();
	void streamAll();
	void streamOutAll();

	virtual void synchronizeGraphics();
	virtual void processUserInput(UserInput userInput);
	virtual bool processUserInputRegardlessOfInput();
	virtual void transform();
	virtual bool isVersusMode();
	virtual Clock getClock();
	virtual bool stream();

	override void enter(void* owner);
	override void execute(void* owner);
	override void exit(void* owner);
	override void suspend(void* owner);
	override void resume(void* owner);
	override bool processMessage(void* owner, Telegram telegram);
}


#endif
