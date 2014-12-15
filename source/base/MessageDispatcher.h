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

#ifndef MESSAGEDISPATCHER_H_
#define MESSAGEDISPATCHER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <StateMachine.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define MessageDispatcher_METHODS												\
		Object_METHODS															\

// declare the virtual methods which are redefined
#define MessageDispatcher_SET_VTABLE(ClassName)									\
		Object_SET_VTABLE(ClassName)											\

__CLASS(MessageDispatcher);

//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

MessageDispatcher MessageDispatcher_getInstance();
int MessageDispatcher_dispatchMessage(u32 delay, Object sender, Object receiver, int message, void* extraInfo);
void MessageDispatcher_dispatchDelayedMessages(MessageDispatcher this);
void MessageDispatcher_discardDelayedMessages(MessageDispatcher this);


#endif