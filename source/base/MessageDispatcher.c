/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <MessageDispatcher.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void MessageDispatcher_constructor(MessageDispatcher this);

// class's destructor
static void MessageDispatcher_destructor(MessageDispatcher this);




/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// text box based on bgmaps
#define MessageDispatcher_ATTRIBUTES			\
												\
	/* super's attributes */					\
	Object_ATTRIBUTES;							\
												\
	/* a clock to point to the in game clock */	\
	/* for delayed messages (TODO feature) */	\
	//Clock clock;


__CLASS_DEFINITION(MessageDispatcher);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// a singleton
__SINGLETON(MessageDispatcher);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void MessageDispatcher_constructor(MessageDispatcher this){
	
	__CONSTRUCT_BASE(Object);
	{	
		//this->clock = Game_getInGameClock(Game_getInstance());
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void MessageDispatcher_destructor(MessageDispatcher this){

	// allow a new construct
	__SINGLETON_DESTROY(Object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dispatch a telegram
/*
static int MessageDispatcher_discharge(StateMachine receiver, Telegram telegram){

	return StateMachine_handleMessage(receiver, telegram);
}
*/


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MessageDispatcher_dispatchMessage(u32 delay, Object sender, 
									Object receiver, int message, void* extraInfo){
  
	//make sure the receiver is valid
	ASSERT(receiver);
  
	{
		//create the telegram
		Telegram telegram = __NEW(Telegram, __ARGUMENTS(0, sender, receiver, message, extraInfo));

		//send the telegram to the recipient
		int result = __VIRTUAL_CALL(int, Object, handleMessage, receiver, __ARGUMENTS(telegram));
		
		__DELETE(telegram);
		return result;
	}
	return false;
}

