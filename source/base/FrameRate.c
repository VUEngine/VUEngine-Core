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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <FrameRate.h>
#include <VirtualList.h>
#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define FrameRate_ATTRIBUTES																			\
        /* super's attributes */																		\
        Object_ATTRIBUTES																				\
        /*  frames per second */																		\
        u16 FPS;																						\

// define the FrameRate
__CLASS_DEFINITION(FrameRate, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void FrameRate_constructor(FrameRate this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__SINGLETON(FrameRate);

// class's constructor
static void __attribute__ ((noinline)) FrameRate_constructor(FrameRate this)
{
	__CONSTRUCT_BASE(Object);

	this->FPS = 0;
}

// class's destructor
void FrameRate_destructor(FrameRate this)
{
	ASSERT(this, "FrameRate::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

// reset internal values
void FrameRate_reset(FrameRate this)
{
	ASSERT(this, "FrameRate::reset: null this");

	this->FPS = 0;
}

// retrieve FPS
u16 FrameRate_getFPS(FrameRate this)
{
	ASSERT(this, "FrameRate::getFPS: null this");

	return this->FPS;
}


// increase the  FPS count
void FrameRate_increaseFPS(FrameRate this)
{
	ASSERT(this, "FrameRate::increaseFPS: null this");

	this->FPS++;
}

// print renderFPS
void FrameRate_print(FrameRate this, int col, int row)
{
	ASSERT(this, "FrameRate::print: null this");

	Printing printing = Printing_getInstance();
	Printing_text(printing, "FPS:", col, row, NULL);
	Printing_int(printing, this->FPS, col + 4, row++, NULL);
}
