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
#include <Printing.h>
#include <MessageDispatcher.h>
#include <PhysicalWorld.h>
#include <VBJaEAutoPauseScreenState.h>

extern StageROMDef EMPTY_ST;


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void VBJaEAutoPauseScreenState_destructor(VBJaEAutoPauseScreenState this);
static void VBJaEAutoPauseScreenState_constructor(VBJaEAutoPauseScreenState this);
static void VBJaEAutoPauseScreenState_enter(VBJaEAutoPauseScreenState this, void* owner);
static void VBJaEAutoPauseScreenState_exit(VBJaEAutoPauseScreenState this, void* owner);
static bool VBJaEAutoPauseScreenState_handleMessage(VBJaEAutoPauseScreenState this, void* owner, Telegram telegram);


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(VBJaEAutoPauseScreenState, GameState);
__SINGLETON(VBJaEAutoPauseScreenState);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
static void VBJaEAutoPauseScreenState_constructor(VBJaEAutoPauseScreenState this)
{
	__CONSTRUCT_BASE();
}

// class's destructor
static void VBJaEAutoPauseScreenState_destructor(VBJaEAutoPauseScreenState this)
{
	// destroy base
	__SINGLETON_DESTROY;
}

// state's enter
static void VBJaEAutoPauseScreenState_enter(VBJaEAutoPauseScreenState this, void* owner)
{
	Optical optical = Game_getOptical(Game_getInstance());
	optical.verticalViewPointCenter = ITOFIX19_13(112 + 112/2);
	Game_setOptical(Game_getInstance(), optical);

	//load stage
	GameState_loadStage(__UPCAST(GameState, this), (StageDefinition*)&EMPTY_ST, true, true);

    Printing_text(Printing_getInstance(), "                                                ", 0, 26, NULL);
    Printing_text(Printing_getInstance(), "REST FOR A WHILE!", ((__SCREEN_WIDTH >> 3) >> 1) - 3, 20, NULL);

	Screen_FXFadeIn(Screen_getInstance(), 16 >> 1);
}

// state's exit
static void VBJaEAutoPauseScreenState_exit(VBJaEAutoPauseScreenState this, void* owner)
{
	// make a fade out
	Screen_FXFadeOut(Screen_getInstance(), 16 >> 1);
}

// state's on message
static bool VBJaEAutoPauseScreenState_handleMessage(VBJaEAutoPauseScreenState this, void* owner, Telegram telegram)
{
	// process message
	switch (Telegram_getMessage(telegram))
    {
		case kKeyUp:
			{
				u16 releasedKey = *((u16*)Telegram_getExtraInfo(telegram));
		
				// check direction
				if (K_STA & releasedKey)
				{
					Game_unpause(Game_getInstance(), __UPCAST(GameState, this));
				}
			}
			return true;
			break;
	}

	return false;
}