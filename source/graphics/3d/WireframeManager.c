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
friend class Wireframe;

static DirectDraw _directDraw = NULL;

Vector3D _previousCameraPosition = {0, 0, 0};
Vector3D _previousCameraPositionBuffer = {0, 0, 0};
Rotation _previousCameraInvertedRotation = {0, 0, 0};
Rotation _previousCameraInvertedRotationBuffer = {0, 0, 0};

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
	this->stopRendering = false;
	this->stopDrawing = false;

	VIPManager::addEventListener(VIPManager::getInstance(), Object::safeCast(this), (EventListener)WireframeManager::onVIPManagerGAMESTARTDuringGAMESTART, kEventVIPManagerGAMESTARTDuringGAMESTART);
	VIPManager::addEventListener(VIPManager::getInstance(), Object::safeCast(this), (EventListener)WireframeManager::onVIPManagerGAMESTARTDuringXPEND, kEventVIPManagerGAMESTARTDuringXPEND);

	_directDraw = DirectDraw::getInstance();
}

/**
 * Class destructor
 */
void WireframeManager::destructor()
{
	ASSERT(this->wireframes, "WireframeManager::destructor: null wireframes");

	VirtualNode node = this->wireframes->head;

	for(; NULL != node; node = node->next)
	{
		delete node->next;
	}

	delete this->wireframes;

	this->wireframes = NULL;

	VIPManager::removeEventListener(VIPManager::getInstance(), Object::safeCast(this), (EventListener)WireframeManager::onVIPManagerGAMESTARTDuringGAMESTART, kEventVIPManagerGAMESTARTDuringGAMESTART);
	VIPManager::removeEventListener(VIPManager::getInstance(), Object::safeCast(this), (EventListener)WireframeManager::onVIPManagerGAMESTARTDuringXPEND, kEventVIPManagerGAMESTARTDuringXPEND);


	// allow a new construct
	Base::destructor();
}

void WireframeManager::onVIPManagerGAMESTARTDuringGAMESTART(Object eventFirer __attribute__ ((unused)))
{
	this->stopRendering = true;
}

void WireframeManager::onVIPManagerGAMESTARTDuringXPEND(Object eventFirer __attribute__ ((unused)))
{
	this->stopDrawing = true;
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
 * Sort wireframes according to their z coordinate
 */
void WireframeManager::sort()
{
	while(WireframeManager::sortProgressively(this));
}

/**
 * Deferred sorting wireframes according to their z coordinate
 */
bool WireframeManager::sortProgressively()
{
	bool swapped = false;

	for(VirtualNode node = this->wireframes->head; node && node->next; node = node->next)
	{
		VirtualNode nextNode = node->next;

		NM_ASSERT(!isDeleted(node->data), "WireframeManager::sortProgressively: NULL node's data");
		NM_ASSERT(__GET_CAST(Wireframe, nextNode->data), "WireframeManager::sortProgressively: NULL node's data cast");

		Wireframe wireframe = Wireframe::safeCast(node->data);

		NM_ASSERT(!isDeleted(nextNode->data), "WireframeManager::sortProgressively: NULL nextNode's data");
		NM_ASSERT(__GET_CAST(Wireframe, nextNode->data), "WireframeManager::sortProgressively: NULL nextNode's data cast");

		Wireframe nextWireframe = Wireframe::safeCast(nextNode->data);

		fix10_6_ext squareDistanceToCamera = Vector3D::squareLength(Vector3D::get(*wireframe->position, *_cameraPosition));
		fix10_6_ext nextSquareDistanceToCamera = Vector3D::squareLength(Vector3D::get(*nextWireframe->position, *_cameraPosition));

		// check if z positions are swapped
		if(nextSquareDistanceToCamera < squareDistanceToCamera)
		{
			VirtualNode::swapData(node, nextNode);

			node = nextNode;

			swapped = true;
			break;
		}
	}

	return swapped;
}

/**
 * Render the wireframes
 */
void WireframeManager::render()
{
	this->stopRendering = false;

	// check the shapes
	for(VirtualNode node = this->wireframes->head; node && !this->stopRendering; node = node->next)
	{
		Wireframe::render(Wireframe::safeCast(node->data));
	}

	WireframeManager::sortProgressively(this);

	_previousCameraPosition = _previousCameraPositionBuffer;
	_previousCameraPositionBuffer = *_cameraPosition;

	_previousCameraInvertedRotation = _previousCameraInvertedRotationBuffer;
	_previousCameraInvertedRotationBuffer = *_cameraInvertedRotation;
}

/**
 * Draw the wireframes to the frame buffers
 */
void WireframeManager::draw()
{
	DirectDraw::startDrawing(_directDraw);

	this->stopDrawing = false;

	// check the shapes
	for(VirtualNode node = this->wireframes->head; !this->stopDrawing && node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		if(wireframe->culled)
		{
			continue;
		}

		Wireframe::draw(wireframe, true);
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

