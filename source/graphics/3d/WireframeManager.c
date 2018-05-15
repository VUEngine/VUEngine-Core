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

#include <WireframeManager.h>
#include <VirtualList.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	WireframeManager
 * @extends Object
 * @ingroup graphics-3d
 */

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

void WireframeManager::constructor(WireframeManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			WireframeManager::getInstance()
 * @memberof	WireframeManager
 * @public
 *
 * @return		WireframeManager instance
 */


/**
 * Class constructor
 *
 * @memberof	WireframeManager
 * @private
 *
 * @param this	Function scope
 */
void WireframeManager::constructor(WireframeManager this)
{
	ASSERT(this, "WireframeManager::constructor: null this");

	Base::constructor();

	this->wireframes = __NEW(VirtualList);
}

/**
 * Class destructor
 *
 * @memberof	WireframeManager
 * @public
 *
 * @param this	Function scope
 */
void WireframeManager::destructor(WireframeManager this)
{
	ASSERT(this, "WireframeManager::destructor: null this");
	ASSERT(this->wireframes, "WireframeManager::destructor: null wireframes");

	VirtualNode node = this->wireframes->head;

	for(; node; node = node->next)
	{
		__DELETE_BASIC(node->next);
	}

	__DELETE(this->wireframes);

	this->wireframes = NULL;

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Register a Wireframe to be rendered
 *
 * @memberof		WireframeManager
 * @public
 *
 * @param this		Function scope
 * @param wireframe	Wireframe to register
 */
void WireframeManager::register(WireframeManager this, Wireframe wireframe)
{
	ASSERT(this, "WireframeManager::register: null this");
	ASSERT(wireframe, "WireframeManager::register: null wireframe");

	if(!VirtualList::find(this->wireframes, wireframe))
	{
		VirtualList::pushBack(this->wireframes, wireframe);
	}
}

/**
 * Remove a registered Wireframe
 *
 * @memberof		WireframeManager
 * @public
 *
 * @param this		Function scope
 * @param wireframe	Wireframe to remove
 */
void WireframeManager::remove(WireframeManager this, Wireframe wireframe)
{
	ASSERT(this, "WireframeManager::remove: null this");
	ASSERT(wireframe, "WireframeManager::remove: null wireframe");

	VirtualList::removeElement(this->wireframes, wireframe);
}

/**
 * Reset manager's state
 *
 * @memberof	WireframeManager
 * @public
 *
 * @param this	Function scope
 */
void WireframeManager::reset(WireframeManager this)
{
	ASSERT(this, "WireframeManager::reset: null this");

	VirtualList::clear(this->wireframes);
}

/**
 * Draw the wireframes to the frame buffers
 *
 * @memberof	WireframeManager
 * @public
 *
 * @param this	Function scope
 */
void WireframeManager::drawWireframes(WireframeManager this)
{
	ASSERT(this, "WireframeManager::draw: null this");

	// comparing against the other shapes
	VirtualNode node = this->wireframes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		 Wireframe::draw(node->data, true);
	}
}

/**
 * Print manager's state
 *
 * @memberof	WireframeManager
 * @public
 *
 * @param this	Function scope
 * @param x		Camera's x coordinate
 * @param y		Camera's y coordinate
 */
void WireframeManager::print(WireframeManager this, int x, int y)
{
	Printing::text(Printing::getInstance(), "WireframeManager's status", x, y++, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Entries: ", x, y, NULL);
	Printing::int(Printing::getInstance(), VirtualList::getSize(this->wireframes), x + 17, y++, NULL);
}

