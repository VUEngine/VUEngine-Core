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
#include <VBJaESplashScreenState.h>
#include <DefaultScreenConfig.h>


//---------------------------------------------------------------------------------------------------------
// 												DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern StageROMDef VBJAENGINE_SPLASH_SCREEN_ST;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaESplashScreenState_destructor(VBJaESplashScreenState this);
static void VBJaESplashScreenState_constructor(VBJaESplashScreenState this);
static void VBJaESplashScreenState_execute(VBJaESplashScreenState this, void* owner);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaESplashScreenState, SplashScreenState);
__SINGLETON_DYNAMIC(VBJaESplashScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaESplashScreenState_constructor(VBJaESplashScreenState this)
{
	__CONSTRUCT_BASE();

	SplashScreenState_setNextstate(__UPCAST(SplashScreenState, this), __UPCAST(GameState, __VBJAENGINE_SPLASH_SCREEN_NEXT_STATE()));
	this->stageDefinition = (StageDefinition*)&VBJAENGINE_SPLASH_SCREEN_ST;
}

// class's destructor
static void VBJaESplashScreenState_destructor(VBJaESplashScreenState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

// state's execute
static void VBJaESplashScreenState_execute(VBJaESplashScreenState this, void* owner)
{
    VBVec3D translation =
    {
        ITOFIX19_13(1),
        ITOFIX19_13(0),
        ITOFIX19_13(0)
    };

    Screen_move(Screen_getInstance(), translation, false);

 	// call base
    SplashScreenState_execute(__UPCAST(SplashScreenState, this), owner);
}
