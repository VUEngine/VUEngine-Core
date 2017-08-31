/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <FrameRate.h>
#include <VirtualList.h>
#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define FrameRate_ATTRIBUTES																			\
		Object_ATTRIBUTES																				\
		/**
		 * @var u16	 	fps
		 * @brief		Frames per second
		 * @memberof	FrameRate
		 */																								\
		u16 fps;																						\

/**
 * @class	FrameRate
 * @extends Object
 * @ingroup base
 */
__CLASS_DEFINITION(FrameRate, Object);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void FrameRate_constructor(FrameRate this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			FrameRate_getInstance()
 * @memberof	FrameRate
 * @public
 *
 * @return		FrameRate instance
 */
__SINGLETON(FrameRate);

/**
 * Class constructor
 *
 * @memberof	FrameRate
 * @private
 *
 * @param this	Function scope
 */
static void __attribute__ ((noinline)) FrameRate_constructor(FrameRate this)
{
	ASSERT(this, "FrameRate::constructor: null this");

	__CONSTRUCT_BASE(Object);

	this->fps = 0;
}

/**
 * Class destructor
 *
 * @memberof	FrameRate
 * @public
 *
 * @param this	Function scope
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
 * @memberof	FrameRate
 * @public
 *
 * @param this	Function scope
 */
void FrameRate_reset(FrameRate this)
{
	ASSERT(this, "FrameRate::reset: null this");

	this->fps = 0;
}

/**
 * Retrieve FPS
 *
 * @memberof	FrameRate
 * @public
 *
 * @param this	Function scope
 */
u16 FrameRate_getFps(FrameRate this)
{
	ASSERT(this, "FrameRate::getFps: null this");

	return this->fps;
}

/**
 * Increase the FPS count
 *
 * @memberof	FrameRate
 * @public
 *
 * @param this	Function scope
 */
void FrameRate_increaseFps(FrameRate this)
{
	ASSERT(this, "FrameRate::increaseFps: null this");

	this->fps++;
}

/**
 * Print FPS
 *
 * @memberof	FrameRate
 * @public
 *
 * @param this	Function scope
 * @param col	Column to start printing at
 * @param row	Row to start printing at
 */
void FrameRate_print(FrameRate this, int col, int row)
{
	ASSERT(this, "FrameRate::print: null this");

	Printing printing = Printing_getInstance();
	Printing_text(printing, "FPS      ", col, row, NULL);
	Printing_int(printing, this->fps, col + 4, row++, NULL);
}
