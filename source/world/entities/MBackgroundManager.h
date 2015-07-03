/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef MBACKGROUND_MANAGER_H_
#define MBACKGROUND_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <MBackground.h>
#include <Texture.h>

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define MBackgroundManager_METHODS												\
	Object_METHODS;																\
	
#define MBackgroundManager_SET_VTABLE(ClassName)								\
	Object_SET_VTABLE(ClassName);												\

__CLASS(MBackgroundManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

MBackgroundManager MBackgroundManager_getInstance();

void MBackgroundManager_destructor(MBackgroundManager this);
void MBackgroundManager_registerTexture(MBackgroundManager this, TextureDefinition* textureDefinition);
void MBackgroundManager_removeTexture(MBackgroundManager this, Texture texture);
void MBackgroundManager_reset(MBackgroundManager this);

#endif