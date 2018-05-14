/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#include <Wireframe.h>
#include <WireframeManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Wireframe
 * @extends Object
 * @ingroup graphics-3d
 */
__CLASS_DEFINITION(Wireframe, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @memberof	Wireframe
 * @public
 *
 * @param this	Function scope
 */
void Wireframe::constructor(Wireframe this)
{
	ASSERT(this, "Wireframe::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);
}

/**
 * Class destructor
 *
 * @memberof	Wireframe
 * @public
 *
 * @param this	Function scope
 */
void Wireframe::destructor(Wireframe this)
{
	ASSERT(this, "Wireframe::destructor: null this");

	Wireframe::hide(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Start being rendered
 *
 * @memberof	Wireframe
 * @public
 *
 * @param this	Function scope
 */
void Wireframe::show(Wireframe this)
{
	ASSERT(this, "Wireframe::show: null this");

	WireframeManager::register(WireframeManager::getInstance(), this);
}

/**
 * Stop being rendered
 *
 * @memberof	Wireframe
 * @public
 *
 * @param this	Function scope
 */
void Wireframe::hide(Wireframe this)
{
	ASSERT(this, "Wireframe::hide: null this");

	WireframeManager::remove(WireframeManager::getInstance(), this);
}
