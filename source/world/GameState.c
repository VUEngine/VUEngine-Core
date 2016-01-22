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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <GameState.h>
#include <Game.h>
#include <Screen.h>
#include <SpriteManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(GameState, State);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void GameState_constructor(GameState this)
{
	ASSERT(this, "GameState::constructor: null this");

	__CONSTRUCT_BASE();

	this->stage = NULL;
	
	// clocks
	this->inGameClock = __NEW(Clock);
	this->animationsClock = __NEW(Clock);
	this->physicsClock = __NEW(Clock);

	// construct the physical world and collision manager
	this->physicalWorld = __NEW(PhysicalWorld);
	this->collisionManager = __NEW(CollisionManager);

	// by default can stream
	this->canStream = true;
	
	this->screenPosition.x = 0;
	this->screenPosition.y = 0;
	this->screenPosition.z = 0;
}

// class's destructor
void GameState_destructor(GameState this)
{
	ASSERT(this, "GameState::destructor: null this");

	__DELETE(this->inGameClock);
	__DELETE(this->animationsClock);
	__DELETE(this->physicsClock);
	
	__DELETE(this->physicalWorld);
	__DELETE(this->collisionManager);

	// destroy the stage
	if(this->stage)
	{
		// destroy the stage
		__DELETE(this->stage);

		this->stage = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// state's enter
void GameState_enter(GameState this, void* owner)
{
	ASSERT(this, "GameState::enter: null this");
	
	Clock_start(this->inGameClock);
}

// state's execute
void GameState_execute(GameState this, void* owner)
{
	ASSERT(this, "GameState::execute: null this");
	ASSERT(this->stage, "GameState::execute: null stage");

	// update the stage
	__VIRTUAL_CALL(void, Container, update, this->stage);
	
	// stream level
	Stage_stream(this->stage);
}

// state's exit
void GameState_exit(GameState this, void* owner)
{
	ASSERT(this, "GameState::exit: null this");

	// make sure to free the memory
	if(this->stage)
	{
		// destroy the stage
		__DELETE(this->stage);
	}

	this->stage = NULL;
	
	Clock_stop(this->inGameClock);
}

// state's suspend
void GameState_suspend(GameState this, void* owner)
{
	ASSERT(this, "GameState::suspend: null this");

	Clock_pause(this->inGameClock, true);
	
#ifdef __DEBUG_TOOLS
	if(!Game_isEnteringSpecialMode(Game_getInstance()))
	{
#endif
#ifdef __STAGE_EDITOR
	if(!Game_isEnteringSpecialMode(Game_getInstance()))
	{
#endif
#ifdef __ANIMATION_EDITOR
	if(!Game_isEnteringSpecialMode(Game_getInstance()))
	{
#endif
	
	// save the screen position for resume reconfiguration
	this->screenPosition = Screen_getPosition(Screen_getInstance());

	if(this->stage)
	{
		__VIRTUAL_CALL(void, Container, suspend, this->stage);
	}
	
#ifdef __DEBUG_TOOLS
	}
#endif
#ifdef __STAGE_EDITOR
	}
#endif
#ifdef __ANIMATION_EDITOR
	}
#endif
}

// state's execute
void GameState_resume(GameState this, void* owner)
{
	ASSERT(this, "GameState::resume: null this");
	NM_ASSERT(this->stage, "GameState::resume: null stage");

#ifdef __DEBUG_TOOLS
	if(!Game_isExitingSpecialMode(Game_getInstance()))
	{
#endif
#ifdef __STAGE_EDITOR
	if(!Game_isExitingSpecialMode(Game_getInstance()))
	{
#endif
#ifdef __ANIMATION_EDITOR
	if(!Game_isExitingSpecialMode(Game_getInstance()))
	{
#endif

	// set screen to its previous position
	Screen_setStageSize(Screen_getInstance(), Stage_getSize(this->stage));
	Screen_setPosition(Screen_getInstance(), this->screenPosition);

	if(this->stage)
	{
		Game_reset(Game_getInstance());
		
		// update the stage
		__VIRTUAL_CALL(void, Container, resume, this->stage);
	}

	// move the screen to its previous position
	Screen_position(Screen_getInstance(), false);

	// transform everything before showing up
	GameState_transform(this);

	// sort all sprites' layers
	SpriteManager_sortLayers(SpriteManager_getInstance(), false);

	// render sprites as soon as possible
	SpriteManager_render(SpriteManager_getInstance());
#ifdef __DEBUG_TOOLS
	}
#endif
#ifdef __STAGE_EDITOR
	}
#endif
#ifdef __ANIMATION_EDITOR
	}
#endif
	
	// unpause clock
	Clock_pause(this->inGameClock, false);
}

// state's on message
bool GameState_handleMessage(GameState this, void* owner, Telegram telegram)
{
	ASSERT(this, "GameState::handleMessage: null this");

	return Container_propagateMessage(__SAFE_CAST(Container, this->stage), Container_onPropagatedMessage, Telegram_getMessage(telegram));
}

// update level entities' positions
void GameState_transform(GameState this)
{
	ASSERT(this, "GameState::transform: null this");
	ASSERT(this->stage, "GameState::transform: null stage");

	// static to avoid call to memcpy
	static Transformation environmentTransform =
	{
			// local position
			{0, 0, 0},
			// global position
			{0, 0, 0},
			// local rotation
			{0, 0, 0},
			// global rotation
			{0, 0, 0},
			// local scale
			{ITOFIX7_9(1), ITOFIX7_9(1)},
			// global scale
			{ITOFIX7_9(1), ITOFIX7_9(1)},
	};

	// then transform loaded entities
	__VIRTUAL_CALL(void, Container, transform, this->stage, &environmentTransform);
}

// propagate message to all entities in the level
int GameState_propagateMessage(GameState this, int message)
{
	return Container_propagateMessage(__SAFE_CAST(Container, this->stage), Container_onPropagatedMessage, message);
}

// process user input
void GameState_onPropagatedMessage(GameState this, int message)
{
	ASSERT(this, "GameState::onPropagatedMessage: null this");

}
// load a stage
void GameState_loadStage(GameState this, StageDefinition* stageDefinition, VirtualList entityNamesToIgnore, bool overrideScreenPosition)
{
	ASSERT(this, "GameState::loadStage: null this");
	ASSERT(stageDefinition, "GameState::loadStage: null stageDefinition");

	// disable hardware interrupts
	Game_disableHardwareInterrupts(Game_getInstance());

	if(this->stage)
	{
		// destroy the stage
		__DELETE(this->stage);
	}

	// reset the engine state
	Game_reset(Game_getInstance());

	// construct the stage
	this->stage = __NEW(Stage);

	ASSERT(this->stage, "GameState::loadStage: null stage");

	// make sure no entity is set as focus for the screen
	Screen_setFocusInGameEntity(Screen_getInstance(), NULL);

	//load world entities
	Stage_load(this->stage, stageDefinition, entityNamesToIgnore, overrideScreenPosition);

	// move the screen to its position
	Screen_position(Screen_getInstance(), false);

	// transform everything
	GameState_transform(this);

	// sort all sprites' layers
	SpriteManager_sortLayers(SpriteManager_getInstance(), false);

	// render sprites as soon as possible
	SpriteManager_render(SpriteManager_getInstance());
}

// set streaming flag
void GameState_setCanStream(GameState this, int canStream)
{
	ASSERT(this, "GameState::loadStage: null this");

	this->canStream = canStream;
}

// get streaming flag
bool GameState_canStream(GameState this)
{
	ASSERT(this, "GameState::canStream: null this");

	return this->canStream;
}

// retrieve stage
Stage GameState_getStage(GameState this)
{
	ASSERT(this, "GameState::getStage: null this");
	ASSERT(this->stage, "GameState::getStage: null stage");

	return this->stage;
}

const Clock GameState_getInGameClock(GameState this)
{
	ASSERT(this, "GameState::getInGameClock: null this");

	return this->inGameClock;
}

const Clock GameState_getAnimationsClock(GameState this)
{
	ASSERT(this, "GameState::getAnimationsClock: null this");

	return this->animationsClock;
}

const Clock GameState_getPhysicsClock(GameState this)
{
	ASSERT(this, "GameState::getAnimationsClock: null this");

	return this->physicsClock;
}

void GameState_startClocks(GameState this)
{
	ASSERT(this, "GameState::startClocks: null this");

	Clock_start(this->inGameClock);
	Clock_start(this->animationsClock);
	Clock_start(this->physicsClock);
}

void GameState_pauseClocks(GameState this)
{
	ASSERT(this, "GameState::pauseClocks: null this");

	Clock_pause(this->inGameClock, true);
	Clock_pause(this->animationsClock, true);
	Clock_pause(this->physicsClock, true);
}

void GameState_resumeClocks(GameState this)
{
	ASSERT(this, "GameState::resumeClocks: null this");

	Clock_pause(this->inGameClock, false);
	Clock_pause(this->animationsClock, false);
	Clock_pause(this->physicsClock, false);
}

void GameState_startInGameClock(GameState this)
{
	ASSERT(this, "GameState::startInGameClock: null this");

	Clock_start(this->inGameClock);
}

void GameState_startAnimations(GameState this)
{
	ASSERT(this, "GameState::startAnimations: null this");

	Clock_start(this->animationsClock);
}

void GameState_startPhysics(GameState this)
{
	ASSERT(this, "GameState::startPhysics: null this");

	Clock_start(this->physicsClock);
}

void GameState_pauseInGameClock(GameState this, bool pause)
{
	ASSERT(this, "GameState::pauseInGameClock: null this");

	Clock_pause(this->inGameClock, pause);
}

void GameState_pauseAnimations(GameState this, bool pause)
{
	ASSERT(this, "GameState::pauseAnimations: null this");

	Clock_pause(this->animationsClock, pause);
}

void GameState_pausePhysics(GameState this, bool pause)
{
	ASSERT(this, "GameState::pausePhysics: null this");
	
	Clock_pause(this->physicsClock, pause);
}

void GameState_updatePhysics(GameState this)
{
	ASSERT(this, "GameState::updatePhysics: null this");
	
	if(!Clock_isPaused(this->physicsClock))
	{
		PhysicalWorld_update(this->physicalWorld, this->physicsClock);
	}
}

PhysicalWorld GameState_getPhysicalWorld(GameState this)
{
	ASSERT(this, "GameState::getPhysicalWorld: null this");

	return this->physicalWorld;
}

void GameState_processCollisions(GameState this)
{
	ASSERT(this, "GameState::processCollisions: null this");
	
	if(!Clock_isPaused(this->physicsClock))
	{
		CollisionManager_update(this->collisionManager);
	}
}

CollisionManager GameState_getCollisionManager(GameState this)
{
	ASSERT(this, "GameState::getCollisionManager: null this");

	return this->collisionManager;
}

