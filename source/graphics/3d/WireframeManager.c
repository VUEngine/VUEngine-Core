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
#include <Camera.h>
#include <DirectDraw.h>
#include <debugConfig.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;

Vector3D _cameraRealPosition = {0, 0, 0};
Rotation _cameraRealRotation = {0, 0, 0};


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
}

/**
 * Reset manager's state
 */
void WireframeManager::reset()
{
	VirtualList::clear(this->wireframes);
}

/**
 * Render the wireframes
 */
void WireframeManager::render()
{
	_cameraRealPosition = Vector3D::sum(*_cameraPosition, (Vector3D){__HALF_SCREEN_WIDTH_METERS, __HALF_SCREEN_HEIGHT_METERS, 0});

	_cameraRealRotation = (Rotation)
	{
		512 -_cameraRotation->x,
		512 - _cameraRotation->y,
		512 - _cameraRotation->z
	};

	// comparing against the other shapes
	VirtualNode node = this->wireframes->head;

	// check the shapes
	for(; node; node = node->next)
	{
		Wireframe::render(Wireframe::safeCast(node->data));
	}
}

/**
 * Draw the wireframes to the frame buffers
 */
static void WireframeManager::drawWireframes()
{
	DirectDraw::reset(DirectDraw::getInstance());

	WireframeManager this = WireframeManager::getInstance();

	// comparing against the other shapes
	VirtualNode node = this->wireframes->head;

	CACHE_DISABLE;
	CACHE_CLEAR;

	// check the shapes
	for(; node; node = node->next)
	{
		Wireframe::draw(node->data, true);
	}

	CACHE_ENABLE;

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

