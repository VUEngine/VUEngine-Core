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

#include <PolyhedronManager.h>
#include <VirtualList.h>
#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define PolyhedronManager_ATTRIBUTES																	\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\
		/* polyhedrons */																				\
		VirtualList polyhedrons;																		\

/**
 * @class	PolyhedronManager
 * @extends Object
 * @ingroup graphics-3d
 */
__CLASS_DEFINITION(PolyhedronManager, Object);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void PolyhedronManager_constructor(PolyhedronManager this);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			PolyhedronManager_getInstance()
 * @memberof	PolyhedronManager
 * @public
 *
 * @return		PolyhedronManager instance
 */
__SINGLETON(PolyhedronManager);

/**
 * Class constructor
 *
 * @memberof	PolyhedronManager
 * @private
 *
 * @param this	Function scope
 */
static void PolyhedronManager_constructor(PolyhedronManager this)
{
	__CONSTRUCT_BASE(Object);

	this->polyhedrons = __NEW(VirtualList);
}

/**
 * Class destructor
 *
 * @memberof	PolyhedronManager
 * @public
 *
 * @param this	Function scope
 */
void PolyhedronManager_destructor(PolyhedronManager this)
{
	ASSERT(this, "PolyhedronManager::destructor: null this");
	ASSERT(this->polyhedrons, "PolyhedronManager::destructor: null polyhedrons");

	VirtualNode node = this->polyhedrons->head;

	for(; node; node = node->next)
	{
		__DELETE_BASIC(node->next);
	}

	__DELETE(this->polyhedrons);

	this->polyhedrons = NULL;

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Register a Polyhedron to be rendered
 *
 * @memberof			PolyhedronManager
 * @public
 *
 * @param this			Function scope
 * @param polyhedron	Polyhedron to register
 */
void PolyhedronManager_register(PolyhedronManager this, Polyhedron polyhedron)
{
	ASSERT(this, "PolyhedronManager::register: null this");
	ASSERT(polyhedron, "PolyhedronManager::register: null polyhedron");

	if(!VirtualList_find(this->polyhedrons, polyhedron))
	{
		VirtualList_pushBack(this->polyhedrons, polyhedron);
	}
}

/**
 * Remove a registered Polyhedron
 *
 * @memberof			PolyhedronManager
 * @public
 *
 * @param this			Function scope
 * @param polyhedron	Polyhedron to remove
 */
void PolyhedronManager_remove(PolyhedronManager this, Polyhedron polyhedron)
{
	ASSERT(this, "PolyhedronManager::remove: null this");
	ASSERT(polyhedron, "PolyhedronManager::remove: null polyhedron");

	VirtualList_removeElement(this->polyhedrons, polyhedron);
}

/**
 * Reset manager's state
 *
 * @memberof			PolyhedronManager
 * @public
 *
 * @param this			Function scope
 */
void PolyhedronManager_reset(PolyhedronManager this)
{
	ASSERT(this, "PolyhedronManager::reset: null this");

	VirtualList_clear(this->polyhedrons);
}

/**
 * Draw the polyhedrons to the frame buffers
 *
 * @memberof			PolyhedronManager
 * @public
 *
 * @param this			Function scope
 */
void PolyhedronManager_drawPolyhedrons(PolyhedronManager this)
{
	ASSERT(this, "PolyhedronManager::draw: null this");

	// comparing against the other shapes
	VirtualNode node = this->polyhedrons->head;

	// check the shapes
	for(; node; node = node->next)
	{
		Polyhedron_draw(__SAFE_CAST(Polyhedron, node->data), true);
	}
}

/**
 * Print manager's state
 *
 * @memberof			PolyhedronManager
 * @public
 *
 * @param this			Function scope
 * @param x				Screen's x coordinate
 * @param y				Screen's y coordinate
 */
void PolyhedronManager_print(PolyhedronManager this, int x, int y)
{
	Printing_text(Printing_getInstance(), "PolyhedronManager's status", x, y++, NULL);
	y++;
	Printing_text(Printing_getInstance(), "Entries: ", x, y, NULL);
	Printing_int(Printing_getInstance(), VirtualList_getSize(this->polyhedrons), x + 17, y++, NULL);
}

