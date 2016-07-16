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

#define MBackgroundManager_METHODS(ClassName)																		\
	Object_METHODS(ClassName);																						\

#define MBackgroundManager_SET_VTABLE(ClassName)														\
	Object_SET_VTABLE(ClassName);																		\

__CLASS(MBackgroundManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

MBackgroundManager MBackgroundManager_getInstance();

void MBackgroundManager_destructor(MBackgroundManager this);
Texture MBackgroundManager_registerTexture(MBackgroundManager this, TextureDefinition* textureDefinition);
void MBackgroundManager_removeTexture(MBackgroundManager this, Texture texture);
void MBackgroundManager_reset(MBackgroundManager this);


#endif
