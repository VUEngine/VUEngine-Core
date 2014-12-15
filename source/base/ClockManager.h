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

#ifndef CLOCKMANAGER_H_
#define CLOCKMANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Clock.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define ClockManager_METHODS													\
		Object_METHODS															\

// declare the virtual methods which are redefined
#define ClockManager_SET_VTABLE(ClassName)										\
		Object_SET_VTABLE(ClassName)											\

__CLASS(ClockManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

ClockManager ClockManager_getInstance();
void ClockManager_destructor(ClockManager this);
void ClockManager_register(ClockManager this, Clock clock);
void ClockManager_unregister(ClockManager this, Clock clock);
void ClockManager_update(ClockManager this, u32 ticksElapsed);
void ClockManager_reset(ClockManager this);


#endif