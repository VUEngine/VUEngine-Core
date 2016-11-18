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
        Object_ATTRIBUTES																				\
        /**
         * @var u16     fps
         * @brief       Frames per second
         * @memberof    FrameRate
		 */																								\
        u16 fps;																						\

/**
 * @class   FrameRate
 * @extends Object
 */
__CLASS_DEFINITION(FrameRate, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void FrameRate_constructor(FrameRate this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn          FrameRate_getInstance()
 * @memberof    FrameRate
 * @public
 *
 * @return      FrameRate instance
 */
__SINGLETON(FrameRate);

/**
 * Class constructor
 *
 * @memberof    FrameRate
 * @private
 *
 * @param this  Function scope
 */
static void __attribute__ ((noinline)) FrameRate_constructor(FrameRate this)
{
	__CONSTRUCT_BASE(Object);

	this->fps = 0;
}

/**
 * Class destructor
 *
 * @memberof    FrameRate
 * @public
 *
 * @param this  Function scope
 */
void FrameRate_destructor(FrameRate this)
{
	ASSERT(this, "FrameRate::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Reset internal values
 *
 * @memberof    FrameRate
 * @public
 *
 * @param this  Function scope
 */
void FrameRate_reset(FrameRate this)
{
	ASSERT(this, "FrameRate::reset: null this");

	this->fps = 0;
}

/**
 * Retrieve FPS
 *
 * @memberof    FrameRate
 * @public
 *
 * @param this  Function scope
 */
u16 FrameRate_getFps(FrameRate this)
{
	ASSERT(this, "FrameRate::getFps: null this");

	return this->fps;
}

/**
 * Increase the FPS count
 *
 * @memberof    FrameRate
 * @public
 *
 * @param this  Function scope
 */
void FrameRate_increaseFps(FrameRate this)
{
	ASSERT(this, "FrameRate::increaseFps: null this");

	this->fps++;
}

/**
 * Print FPS
 *
 * @memberof    FrameRate
 * @public
 *
 * @param this  Function scope
 * @param col   Column to start printing at
 * @param row   Row to start printing at
 */
void FrameRate_print(FrameRate this, int col, int row)
{
	ASSERT(this, "FrameRate::print: null this");

	Printing printing = Printing_getInstance();
	Printing_text(printing, "FPS:", col, row, NULL);
	Printing_int(printing, this->fps, col + 4, row++, NULL);
}
