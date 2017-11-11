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

#include <WireframeManager.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define WireframeManager_ATTRIBUTES																	\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* wireframes */																				\
		VirtualList wireframes;																		\

/**
 * @class	WireframeManager
 * @extends Object
 * @ingroup graphics-3d
 */
__CLASS_DEFINITION(WireframeManager, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void WireframeManager_constructor(WireframeManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			WireframeManager_getInstance()
 * @memberof	WireframeManager
 * @public
 *
 * @return		WireframeManager instance
 */
__SINGLETON(WireframeManager);

/**
 * Class constructor
 *
 * @memberof	WireframeManager
 * @private
 *
 * @param this	Function scope
 */
static void WireframeManager_constructor(WireframeManager this)
{
	ASSERT(this, "WireframeManager::constructor: null this");

	__CONSTRUCT_BASE(Object);

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
void WireframeManager_destructor(WireframeManager this)
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
 * @memberof			WireframeManager
 * @public
 *
 * @param this			Function scope
 * @param wireframe	Wireframe to register
 */
void WireframeManager_register(WireframeManager this, Wireframe wireframe)
{
	ASSERT(this, "WireframeManager::register: null this");
	ASSERT(wireframe, "WireframeManager::register: null wireframe");

	if(!VirtualList_find(this->wireframes, wireframe))
	{
		VirtualList_pushBack(this->wireframes, wireframe);
	}
}

/**
 * Remove a registered Wireframe
 *
 * @memberof			WireframeManager
 * @public
 *
 * @param this			Function scope
 * @param wireframe	Wireframe to remove
 */
void WireframeManager_remove(WireframeManager this, Wireframe wireframe)
{
	ASSERT(this, "WireframeManager::remove: null this");
	ASSERT(wireframe, "WireframeManager::remove: null wireframe");

	VirtualList_removeElement(this->wireframes, wireframe);
}

/**
 * Reset manager's state
 *
 * @memberof			WireframeManager
 * @public
 *
 * @param this			Function scope
 */
void WireframeManager_reset(WireframeManager this)
{
	ASSERT(this, "WireframeManager::reset: null this");

	VirtualList_clear(this->wireframes);
}

/**
 * Draw the wireframes to the frame buffers
 *
 * @memberof			WireframeManager
 * @public
 *
 * @param this			Function scope
 */
void WireframeManager_drawWireframes(WireframeManager this)
{
	ASSERT(this, "WireframeManager::draw: null this");

	// comparing against the other shapes
	VirtualNode node = this->wireframes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		__VIRTUAL_CALL(Wireframe, draw, node->data, true);
	}
}

/**
 * Print manager's state
 *
 * @memberof			WireframeManager
 * @public
 *
 * @param this			Function scope
 * @param x				Screen's x coordinate
 * @param y				Screen's y coordinate
 */
void WireframeManager_print(WireframeManager this, int x, int y)
{
	Printing_text(Printing_getInstance(), "WireframeManager's status", x, y++, NULL);
	y++;
	Printing_text(Printing_getInstance(), "Entries: ", x, y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->wireframes), x + 17, y++, NULL);
}

