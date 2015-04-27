/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Screen.h>
#include <Optics.h>
#include <Game.h>
#include <MessageDispatcher.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------


// define the Screen
__CLASS_DEFINITION(Screen, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
const VBVec3D* _screenDisplacement = NULL;

static void Screen_constructor(Screen this);
bool Screen_handleMessage(Screen this, Telegram telegram);
static void Screen_capPosition(Screen this);
void Screen_onScreenShake(Screen this);


//---------------------------------------------------------------------------------------------------------
// 												GLOBALS
//---------------------------------------------------------------------------------------------------------

const VBVec3D* _screenPosition = NULL;


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// it's a singleton
__SINGLETON(Screen);

// class's constructor
static void Screen_constructor(Screen this)
{
	ASSERT(this, "Screen::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE();

	// initialize world's screen's position
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;
	
	this->screenMovementManager = NULL;

	this->focusEntityPositionDisplacement.x = 0;
	this->focusEntityPositionDisplacement.y = 0;
	this->focusEntityPositionDisplacement.z = 0;

	// clear focus actor pointer
	this->focusInGameEntity = NULL;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;

	_screenDisplacement = &this->lastDisplacement;
	_screenPosition = &this->position;
}

// class's destructor
void Screen_destructor(Screen this)
{
	ASSERT(this, "Screen::destructor: null this");

	// destroy base
	__SINGLETON_DESTROY;
}

// set the movement manager
void Screen_setScreenMovementManager(Screen this, ScreenMovementManager screenMovementManager)
{
	ASSERT(this, "Screen::setScreenMovementManager: null this");
	
	if(this->screenMovementManager)
	{
		__DELETE(this->screenMovementManager);
	}
	
	this->screenMovementManager = screenMovementManager;

}

// center world's screen in function of focus actor's position
void Screen_positione(Screen this, u8 checkIfFocusEntityIsMoving)
{
	ASSERT(this, "Screen::update: null this");
	ASSERT(this->screenMovementManager, "Screen::update: null screenMovementManager");

#ifdef __DEBUG_TOOLS
	if (!Game_isInSpecialMode(Game_getInstance()))
#endif
#ifdef __STAGE_EDITOR
	if (!Game_isInSpecialMode(Game_getInstance()))
#endif
#ifdef __ANIMATION_EDITOR
	if (!Game_isInSpecialMode(Game_getInstance()))
#endif

	__VIRTUAL_CALL(void, ScreenMovementManager, positione, this->screenMovementManager, checkIfFocusEntityIsMoving);
}

// set the focus entity
void Screen_setFocusInGameEntity(Screen this, InGameEntity focusInGameEntity)
{
	ASSERT(this, "Screen::setFocusInGameEntity: null this");

	this->focusInGameEntity = focusInGameEntity;

	// make sure that any other entity knows about the change
	Screen_forceDisplacement(this, true);
}

// unset the focus entity
void Screen_unsetFocusInGameEntity(Screen this)
{
	ASSERT(this, "Screen::unsetFocusInGameEntity: null this");

	this->focusInGameEntity = NULL;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;
}

// retrieve focus entity
InGameEntity Screen_getFocusInGameEntity(Screen this)
{
	ASSERT(this, "Screen::getFocusInGameEntity: null this");

	return this->focusInGameEntity;
}

// an actor has been deleted
void Screen_onFocusEntityDeleted(Screen this, InGameEntity actor)
{
	ASSERT(this, "Screen::focusEntityDeleted: null this");

	if (this->focusInGameEntity == actor)
	{
		Screen_unsetFocusInGameEntity(this);
	}
}


// translate screen
void Screen_move(Screen this, VBVec3D translation, int cap)
{
	ASSERT(this, "Screen::move: null this");

	this->lastDisplacement = translation;

	this->position.x += translation.x;
	this->position.y += translation.y;
	this->position.z += translation.z;

	if (cap)
	{
		Screen_capPosition(this);
	}
}

// translate screen
static void Screen_capPosition(Screen this)
{
	ASSERT(this, "Screen::capPosition: null this");

	if (this->position.x < 0)
	{
		this->position.x = 0;
	}

	if (this->position.x + ITOFIX19_13(__SCREEN_WIDTH) > ITOFIX19_13(this->stageSize.x))
	{
		this->position.x = ITOFIX19_13(this->stageSize.x - __SCREEN_WIDTH);
	}

	if (this->position.y < 0)
	{
		this->position.y = 0;
	}

	if (this->position.y + ITOFIX19_13(__SCREEN_HEIGHT) > ITOFIX19_13(this->stageSize.y))
	{
		this->position.y = ITOFIX19_13(this->stageSize.y - __SCREEN_HEIGHT);
	}

	if (this->position.z < 0)
	{
		this->position.z = 0;
	}

	if (this->position.z > ITOFIX19_13(this->stageSize.z))
	{
		this->position.z = ITOFIX19_13(this->stageSize.z);
	}
}

// get screen's position
VBVec3D Screen_getPosition(Screen this)
{
	ASSERT(this, "Screen::getPosition: null this");

	return this->position;
}

// set screen's position
void Screen_setPosition(Screen this, VBVec3D position)
{
	ASSERT(this, "Screen::setPosition: null this");

	this->position = position;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;

	Screen_capPosition(this);
}

// set screen's position displacement
void Screen_setFocusEntityPositionDisplacement(Screen this, VBVec3D focusEntityPositionDisplacement)
{
	ASSERT(this, "Screen::setPosition: null this");

	this->focusEntityPositionDisplacement = focusEntityPositionDisplacement;

	// make sure that any other entity knows about the change
	Screen_forceDisplacement(this, true);
}

// retrieve last displacement
VBVec3D Screen_getLastDisplacement(Screen this)
{
	ASSERT(this, "Screen::getLastDisplacement: null this");

	return this->lastDisplacement;
}

// get current stage's size
Size Screen_getStageSize(Screen this)
{
	ASSERT(this, "Screen::getStageSize: null this");

	return this->stageSize;
}

// set current stage's size
void Screen_setStageSize(Screen this, Size size)
{
	ASSERT(this, "Screen::setStageSize: null this");

	this->stageSize = size;
}

// force values as if screen is moving
void Screen_forceDisplacement(Screen this, int flag)
{
	ASSERT(this, "Screen::forceDisplacement: null this");

	this->lastDisplacement.x = flag ? 1 : 0;
	this->lastDisplacement.y = flag ? 1 : 0;
	this->lastDisplacement.z = flag ? 1 : 0;
}

// state's on message
bool Screen_handleMessage(Screen this, Telegram telegram)
{
	switch (Telegram_getMessage(telegram))
	{
		case kScreenShake:
            Screen_onScreenShake(this);
            break;
	}

	return false;
}

// fade in the screen
void Screen_FXFadeIn(Screen this, int wait)
{
	ASSERT(this, "Screen::FXFadeIn: null this");

	int i = 0;
	// create the delay
	for (; i <= 32; i += 2)
	{
		if (wait)
		{
			// create time delay
			Clock_delay(Game_getClock(Game_getInstance()), wait);
		}

		// increase brightness
		SET_BRIGHT(i, i << 1, i);
	}
}

// fade out the screen
void Screen_FXFadeOut(Screen this, int wait)
{
	ASSERT(this, "Screen::FXFadeOut: null this");

	int i = 32;

	// create the delay
	for (; i >= 0; i-=2)
	{
		if (wait)
		{
			// create time delay
			Clock_delay(Game_getClock(Game_getInstance()), wait);
		}
		// decrease brightness
		SET_BRIGHT(i, i << 1, i);
	}
}

// start shaking the screen
void Screen_FXShakeStart(Screen this, u16 duration)
{
	ASSERT(this, "Screen::FXShakeStart: null this");

    // set desired fx duration
    this->shakeTimeLeft = duration;

    // discard pending screen shake messages from previously started shake fx
    MessageDispatcher_discardDelayedMessages(MessageDispatcher_getInstance(), kScreenShake);

    // instantly send shake message to myself to start fx
    MessageDispatcher_dispatchMessage(0, __UPCAST(Object, this), __UPCAST(Object, this), kScreenShake, NULL);
}

// stop shaking the screen
void Screen_FXShakeStop(Screen this)
{
	ASSERT(this, "Screen::FXShakeStop: null this");

    this->shakeTimeLeft = 0;
}

// shake the screen
void Screen_onScreenShake(Screen this)
{
	ASSERT(this, "Screen::onScreenShake: null this");

    // stop if no shaking time left
    if (this->shakeTimeLeft == 0)
    {
        // if needed, undo last offset
        if (this->lastShakeOffset.x != 0 || this->lastShakeOffset.y != 0)
        {
            Screen_setFocusInGameEntity(this, this->tempFocusInGameEntity);
            this->lastShakeOffset.x = 0;
            GameState_transform(__UPCAST(GameState, StateMachine_getCurrentState(Game_getStateMachine(Game_getInstance()))));
        }

        return;
    }

    // substract time until next shake
    this->shakeTimeLeft = (this->shakeTimeLeft <= SHAKE_DELAY) ? 0 : this->shakeTimeLeft - SHAKE_DELAY;

    if (this->lastShakeOffset.x == 0 && this->lastShakeOffset.y == 0)
    {
        // new offset
        // TODO: use random number(s) or pre-defined shaking pattern
        this->lastShakeOffset.x = ITOFIX19_13(2);

        this->tempFocusInGameEntity = Screen_getFocusInGameEntity(this);
		Screen_unsetFocusInGameEntity(this);

        // move screen a bit
        Screen_move(this, this->lastShakeOffset, false);
    }
    else
    {
        // undo last offset
        Screen_setFocusInGameEntity(this, this->tempFocusInGameEntity);
        this->lastShakeOffset.x = 0;
    }

    // apply screen offset
    GameState_transform(__UPCAST(GameState, StateMachine_getCurrentState(Game_getStateMachine(Game_getInstance()))));

    // send message for next screen movement
	MessageDispatcher_dispatchMessage(SHAKE_DELAY, __UPCAST(Object, this), __UPCAST(Object, this), kScreenShake, NULL);
}