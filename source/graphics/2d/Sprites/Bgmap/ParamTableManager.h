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

#ifndef PARAMTABLE_H_
#define PARAMTABLE_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <BgmapSprite.h>

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#define __PARAM_TABLE_PADDING	1

//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

// declare the virtual methods
#define ParamTableManager_METHODS												\
		Object_METHODS															\

// declare the virtual methods which are redefined
#define ParamTableManager_SET_VTABLE(ClassName)									\
		Object_SET_VTABLE(ClassName)											\

// declare a Sprite, which holds a texture and a drawing specification
__CLASS(ParamTableManager);


//---------------------------------------------------------------------------------------------------------
// 										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

ParamTableManager ParamTableManager_getInstance();

void ParamTableManager_destructor(ParamTableManager this);
void ParamTableManager_reset(ParamTableManager this);
int ParamTableManager_allocate(ParamTableManager this, BgmapSprite bsprite);
void ParamTableManager_free(ParamTableManager this, BgmapSprite bsprite);
bool ParamTableManager_processRemovedSprites(ParamTableManager this);
void ParamTableManager_print(ParamTableManager this,int x, int y);


#endif