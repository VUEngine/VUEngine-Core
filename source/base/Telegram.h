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

#ifndef TELEGRAM_H_
#define TELEGRAM_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define Telegram_METHODS														\
		Object_METHODS															\

// declare the virtual methods which are redefined
#define Telegram_SET_VTABLE(ClassName)											\
		Object_SET_VTABLE(ClassName)											\

// declare a Telegram
__CLASS(Telegram);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(Telegram, u32 delay, void* sender, void* receiver, int message, void* extraInfo);

void Telegram_destructor(Telegram this);
void* Telegram_getSender(Telegram this);
void* Telegram_getReceiver(Telegram this);
int Telegram_getMessage(Telegram this);
void* Telegram_getExtraInfo(Telegram this);
u32 Telegram_getDelay(Telegram this);

#endif