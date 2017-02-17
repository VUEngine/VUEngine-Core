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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Screen.h>
#include <Optics.h>
#include <Game.h>
#include <ScreenMovementManager.h>
#include <ScreenEffectManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Screen
 * @extends Object
 * @ingroup screen
 */
__CLASS_DEFINITION(Screen, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Screen_constructor(Screen this);


//---------------------------------------------------------------------------------------------------------
//												GLOBALS
//---------------------------------------------------------------------------------------------------------

const Optical* _optical = NULL;
const VBVec3D* _screenPosition = NULL;
const VBVec3D* _screenDisplacement = NULL;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// it's a singleton
__SINGLETON(Screen);

// class's constructor
static void __attribute__ ((noinline)) Screen_constructor(Screen this)
{
	ASSERT(this, "Screen::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

	// initialize world's screen's position
	this->position.x = 0;
	this->position.y = 0;
	this->position.z = 0;

	this->positionBackup = this->position;

	// set the default screen movement manager
	this->screenMovementManager = ScreenMovementManager_getInstance();

	// set the default screen effect manager
	this->screenEffectManager = ScreenEffectManager_getInstance();

	this->focusEntityPositionDisplacement.x = 0;
	this->focusEntityPositionDisplacement.y = 0;
	this->focusEntityPositionDisplacement.z = 0;

	// clear focus actor pointer
	this->focusInGameEntity = NULL;
	this->focusInGameEntityPosition = NULL;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;

	// accounts for the physical (real) space between the eyes and
	// the VB's screens, whose virtual representation is the Screen instance
	this->optical.distanceEyeScreen = ITOFIX19_13(__DISTANCE_EYE_SCREEN);

	// maximum distance from the _SC to the infinite
	this->optical.maximumViewDistancePower = __MAXIMUM_VIEW_DISTANCE_POWER;

	// distance from left to right eye (depth sensation)
	this->optical.baseDistance = ITOFIX19_13(__BASE_FACTOR);

	// horizontal view point center
	this->optical.horizontalViewPointCenter = ITOFIX19_13(__HORIZONTAL_VIEW_POINT_CENTER);

	// vertical view point center
	this->optical.verticalViewPointCenter = ITOFIX19_13(__VERTICAL_VIEW_POINT_CENTER);

	// set global pointer to improve access to critical values
	_optical = &this->optical;
	_screenPosition = &this->position;
	_screenDisplacement = &this->lastDisplacement;
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

	if(this->screenMovementManager != screenMovementManager)
	{
		if(this->screenMovementManager)
		{
			__DELETE(this->screenMovementManager);
		}

		this->screenMovementManager = screenMovementManager;
	}
}

// set the effect manager
void Screen_setScreenEffectManager(Screen this, ScreenEffectManager screenEffectManager)
{
	ASSERT(this, "Screen::setScreenEffectManager: null this");

	if(this->screenEffectManager != screenEffectManager)
	{
		if(this->screenEffectManager)
		{
			__DELETE(this->screenEffectManager);
		}

		this->screenEffectManager = screenEffectManager;
	}
}

// center world's screen in function of focus actor's position
void Screen_focus(Screen this, u32 checkIfFocusEntityIsMoving)
{
	ASSERT(this, "Screen::focus: null this");
	ASSERT(this->screenMovementManager, "Screen::focus: null screenMovementManager");

#ifdef __DEBUG_TOOLS
	if(!Game_isInSpecialMode(Game_getInstance()))
#endif
#ifdef __STAGE_EDITOR
	if(!Game_isInSpecialMode(Game_getInstance()))
#endif
#ifdef __ANIMATION_EDITOR
	if(!Game_isInSpecialMode(Game_getInstance()))
#endif

	__VIRTUAL_CALL(ScreenMovementManager, focus, this->screenMovementManager, checkIfFocusEntityIsMoving);
}

// set the focus entity
void Screen_setFocusInGameEntity(Screen this, InGameEntity focusInGameEntity)
{
	ASSERT(this, "Screen::setFocusInGameEntity: null this");

	this->focusInGameEntity = focusInGameEntity;
	this->focusInGameEntityPosition = NULL;

	if(focusInGameEntity)
	{
		this->focusInGameEntityPosition = __VIRTUAL_CALL(SpatialObject, getPosition, this->focusInGameEntity);

		// focus now
		Screen_focus(this, false);
	}
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

	if(this->focusInGameEntity == actor)
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

	if(cap)
	{
		Screen_capPosition(this);
	}
}

// translate screen
void Screen_capPosition(Screen this)
{
	ASSERT(this, "Screen::capPosition: null this");

	if(this->position.x < 0)
	{
		this->position.x = 0;
	}

	if(this->position.x + ITOFIX19_13(__SCREEN_WIDTH) > ITOFIX19_13(this->stageSize.x))
	{
		this->position.x = ITOFIX19_13(this->stageSize.x - __SCREEN_WIDTH);
	}

	if(this->position.y < 0)
	{
		this->position.y = 0;
	}

	if(this->position.y + ITOFIX19_13(__SCREEN_HEIGHT) > ITOFIX19_13(this->stageSize.y))
	{
		this->position.y = ITOFIX19_13(this->stageSize.y - __SCREEN_HEIGHT);
	}

	if(this->position.z < 0)
	{
		this->position.z = 0;
	}

	if(this->position.z > ITOFIX19_13(this->stageSize.z))
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

	this->lastDisplacement.x = __1I_FIX19_13;
	this->lastDisplacement.y = __1I_FIX19_13;
	this->lastDisplacement.z = __1I_FIX19_13;

	Screen_capPosition(this);
}

// set screen's position for UI transformation
void Screen_prepareForUITransform(Screen this)
{
	ASSERT(this, "Screen::prepareForUITransform: null this");

	this->positionBackup = this->position;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;
}

// set screen's position after UI transformation
void Screen_doneUITransform(Screen this)
{
	ASSERT(this, "Screen::doneUITransform: null this");

	this->position = this->positionBackup;

	this->lastDisplacement.x = 0;
	this->lastDisplacement.y = 0;
	this->lastDisplacement.z = 0;
}

// retrieve optical config structure
Optical Screen_getOptical(Screen this)
{
	ASSERT(this, "Screen::getOptical: null this");

	return this->optical;
}

// set optical config structure
void Screen_setOptical(Screen this, Optical optical)
{
	ASSERT(this, "Screen::setOptical: null this");

	this->optical = optical;
}

// set screen's position displacement
void Screen_setFocusEntityPositionDisplacement(Screen this, VBVec3D focusEntityPositionDisplacement)
{
	ASSERT(this, "Screen::setPosition: null this");

	this->focusEntityPositionDisplacement = focusEntityPositionDisplacement;

	// focus now
	Screen_focus(this, false);

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

	this->lastDisplacement.x = flag ? __1I_FIX19_13 : 0;
	this->lastDisplacement.y = flag ? __1I_FIX19_13 : 0;
	this->lastDisplacement.z = flag ? __1I_FIX19_13 : 0;
}

void Screen_startEffect(Screen this, int effect, ...)
{
	ASSERT(this, "Screen::startEffect: null this");

	va_list args;
	va_start(args, effect);
	__VIRTUAL_CALL(ScreenEffectManager, startEffect, this->screenEffectManager, effect, args);
	va_end(args);
}

void Screen_stopEffect(Screen this, int effect)
{
	ASSERT(this, "Screen::stopEffect: null this");

	__VIRTUAL_CALL(ScreenEffectManager, stopEffect, this->screenEffectManager, effect);
}
