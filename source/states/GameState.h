/*
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
#include <KeypadManager.h>
#include <Stage.h>
#include <UIContainer.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

class Clock;
class CollisionManager;
class Entity;
class PhysicalWorld;
class Telegram;
class VirtualList;

/// @ingroup states
class GameState : State
{
	// a pointer to the game's stage
	PhysicalWorld physicalWorld;
	// a pointer to the game's stage
	CollisionManager collisionManager;
	// a pointer to the game's stage
	Stage stage;
	// the ui container
	UIContainer uiContainer;
	// clock for messaging
	Clock messagingClock;
	// clock for update cycle
	Clock updateClock;
	// clock for physics
	Clock physicsClock;
	// Flags to disable some processes
	bool stream;
	bool transform;
	bool updatePhysics;
	bool processCollisions;

	/// @publicsection
	void constructor();

	override void enter(void* owner);
	override void execute(void* owner);
	override void exit(void* owner);
	override void suspend(void* owner);
	override void resume(void* owner);
	override bool handleMessage(Telegram telegram);
	override bool processMessage(void* owner, Telegram telegram);

	CollisionManager getCollisionManager();
	Clock getMessagingClock();
	PhysicalWorld getPhysicalWorld();
	Clock getPhysicsClock();
	Stage getStage();
	UIContainer getUIContainer();
	Clock getUpdateClock();
	void loadStage(StageSpec* stageSpec, VirtualList positionedEntitiesToIgnore);
	void pauseAnimations(bool pause);
	void pauseClocks();
	void pauseMessagingClock(bool pause);
	void transform();
	void transformUI();
	uint32 processCollisions();
	void pausePhysics(bool pause);
	void pauseMessaging(bool pause);
	int32 propagateMessage(int32 message);
	int32 propagateString(const char* string);
	void resumeClocks();
	void startAnimations();
	void startClocks();
	void startDispatchingDelayedMessages();
	void startPhysics();
	void stopClocks();
	void updatePhysics();
	void streamAll();
	void streamInAll();
	void streamOutAll();
	Clock getClock();
	Entity getEntityByName(const char* entityName);
	void hideEntityWithName(const char* entityName);
	void showEntityWithName(const char* entityName);
	void changeFrameRate(int16 targetFPS, int32 duration);

	/// Process the provided user input.
	/// @param userInput: Struct with the current user input information
	virtual void processUserInput(const UserInput*  userInput);
	virtual bool processUserInputRegardlessOfInput();
	virtual bool isVersusMode();

	/// Stream in or out the stage entities within or outside the camera's range.
	/// @return True if at least some entity was streamed in or out
	virtual bool stream();

}


#endif
