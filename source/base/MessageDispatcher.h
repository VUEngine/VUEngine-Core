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

#ifndef MESSAGE_DISPATCHER_H_
#define MESSAGE_DISPATCHER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <StateMachine.h>
#include <Telegram.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define MessageDispatcher_METHODS(ClassName)																		\
		Object_METHODS(ClassName)																					\

// declare the virtual methods which are redefined
#define MessageDispatcher_SET_VTABLE(ClassName)															\
		Object_SET_VTABLE(ClassName)																	\

__CLASS(MessageDispatcher);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

MessageDispatcher MessageDispatcher_getInstance();
bool MessageDispatcher_dispatchMessage(u32 delay, Object sender, Object receiver, int message, void* extraInfo);
u32 MessageDispatcher_dispatchDelayedMessages(MessageDispatcher this);
void MessageDispatcher_discardDelayedMessagesFromSender(MessageDispatcher this, Object sender, int message);
void MessageDispatcher_discardDelayedMessagesWithClock(MessageDispatcher this, Clock clock);


#endif
