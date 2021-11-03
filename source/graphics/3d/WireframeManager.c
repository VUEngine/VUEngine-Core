/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <WireframeManager.h>
#include <VirtualList.h>
#include <SpatialObject.h>
#include <Game.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			WireframeManager::getInstance()
 * @memberof	WireframeManager
 * @public
 * @return		WireframeManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void WireframeManager::constructor()
{
	Base::constructor();

	this->wireframes = new VirtualList();
}

/**
 * Class destructor
 */
void WireframeManager::destructor()
{
	ASSERT(this->wireframes, "WireframeManager::destructor: null wireframes");

	VirtualNode node = this->wireframes->head;

	for(; node; node = node->next)
	{
		delete node->next;
	}

	delete this->wireframes;

	this->wireframes = NULL;

	// allow a new construct
	Base::destructor();
}

/**
 * Register a Wireframe to be rendered
 *
 * @param wireframe	Wireframe to register
 */
void WireframeManager::register(Wireframe wireframe)
{
	ASSERT(wireframe, "WireframeManager::register: null wireframe");

	if(!VirtualList::find(this->wireframes, wireframe))
	{
		VirtualList::pushBack(this->wireframes, wireframe);

		Game::removePostProcessingEffect(Game::getInstance(), WireframeManager::drawWireframes, NULL);
		Game::pushBackProcessingEffect(Game::getInstance(), WireframeManager::drawWireframes, NULL);
	}
}

/**
 * Remove a registered Wireframe
 *
 * @param wireframe	Wireframe to remove
 */
void WireframeManager::remove(Wireframe wireframe)
{
	ASSERT(wireframe, "WireframeManager::remove: null wireframe");

	VirtualList::removeElement(this->wireframes, wireframe);

	if(0 == VirtualList::getSize(this->wireframes))
	{
		Game::removePostProcessingEffect(Game::getInstance(), WireframeManager::drawWireframes, NULL);
	}
}

/**
 * Reset manager's state
 */
void WireframeManager::reset()
{
	VirtualList::clear(this->wireframes);

	Game::removePostProcessingEffect(Game::getInstance(), WireframeManager::drawWireframes, NULL);
}

/**
 * Draw the wireframes to the frame buffers
 */
static void WireframeManager::drawWireframes(uint32 currentDrawingFrameBufferSet __attribute__ ((unused)), SpatialObject spatialObject __attribute__ ((unused)))
{
	WireframeManager this = WireframeManager::getInstance();

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
 * @param x		Camera's x coordinate
 * @param y		Camera's y coordinate
 */
void WireframeManager::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "WireframeManager's status", x, y++, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Entries: ", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->wireframes), x + 17, y++, NULL);
}

