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

#include <string.h>

#include <Game.h>
#include <Screen.h>
#include <MessageDispatcher.h>
#include <VBJaEAdjustmentScreenState.h>
#include <DefaultScreenConfig.h>


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

extern StageROMDef VBJAENGINE_ADJUSTMENT_SCREEN_ST;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaEAdjustmentScreenState_destructor(VBJaEAdjustmentScreenState this);
static void VBJaEAdjustmentScreenState_constructor(VBJaEAdjustmentScreenState this);
static void VBJaEAdjustmentScreenState_processInput(VBJaEAdjustmentScreenState this, u16 releasedKey);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaEAdjustmentScreenState, SplashScreenState);
__SINGLETON_DYNAMIC(VBJaEAdjustmentScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaEAdjustmentScreenState_constructor(VBJaEAdjustmentScreenState this)
{
	__CONSTRUCT_BASE();

	SplashScreenState_setNextstate(__UPCAST(SplashScreenState, this), __UPCAST(GameState, __ADJUSTMENT_SCREEN_NEXT_STATE));
	this->stageDefinition = (StageDefinition*)&VBJAENGINE_ADJUSTMENT_SCREEN_ST;
}

// class's destructor
static void VBJaEAdjustmentScreenState_destructor(VBJaEAdjustmentScreenState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

static void VBJaEAdjustmentScreenState_processInput(VBJaEAdjustmentScreenState this, u16 releasedKey)
{
    // TODO: replace this ugly hack with a proper Game_isPaused check or something similar
    if (this->nextState == NULL) 
    {
        Game_unpause(Game_getInstance(), __UPCAST(GameState, this));
    } 
    else 
    {
	    Game_changeState(Game_getInstance(), this->nextState);
    }
}
