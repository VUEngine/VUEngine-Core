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

Vector3D _cameraDirection = {0, 0, 0};
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
	this->evenFrame = __TRANSPARENCY_EVEN;
	this->disabled = false;

	VIPManager::addEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), (EventListener)WireframeManager::onVIPManagerXPENDDuringGAMESTART, kEventVIPManagerXPENDDuringGAMESTART);
	VIPManager::addEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), (EventListener)WireframeManager::onVIPManagerGAMESTARTDuringXPEND, kEventVIPManagerGAMESTARTDuringXPEND);

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

	VIPManager::removeEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), (EventListener)WireframeManager::onVIPManagerXPENDDuringGAMESTART, kEventVIPManagerXPENDDuringGAMESTART);
	VIPManager::removeEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), (EventListener)WireframeManager::onVIPManagerGAMESTARTDuringXPEND, kEventVIPManagerGAMESTARTDuringXPEND);

	// allow a new construct
	Base::destructor();
}

void WireframeManager::onVIPManagerXPENDDuringGAMESTART(ListenerObject eventFirer __attribute__ ((unused)))
{
	this->stopRendering = true;
}

void WireframeManager::onVIPManagerGAMESTARTDuringXPEND(ListenerObject eventFirer __attribute__ ((unused)))
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
	this->disabled = false;
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

		// check if z positions are swapped
		if(nextWireframe->squaredDistanceToCamera < wireframe->squaredDistanceToCamera)
		{
			// swap nodes' data
			node->data = nextWireframe;
			nextNode->data = wireframe;

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
	if(NULL == this->wireframes->head)
	{
		return;
	}

	this->stopRendering = false;

	_cameraDirection = Vector3D::rotate((Vector3D){0, 0, __1I_FIXED}, *_cameraRotation);

	this->evenFrame = __TRANSPARENCY_EVEN == this->evenFrame ? __TRANSPARENCY_ODD : __TRANSPARENCY_EVEN;

#ifdef __PROFILE_WIREFRAMES
	uint16 wireframes = 0;
	uint16 renderedWireframes = 0;
#endif

	// check the shapes
	for(VirtualNode node = this->wireframes->head; node && !this->stopRendering; node = node->next)
	{
#ifdef __PROFILE_WIREFRAMES
		wireframes++;
#endif

		Wireframe wireframe = Wireframe::safeCast(node->data);

		if(__HIDE == wireframe->show)
		{
			wireframe->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
			continue;
		}

		if(wireframe->transparent & this->evenFrame)
		{
			wireframe->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
			continue;
		}

		Wireframe::render(wireframe);

#ifdef __PROFILE_WIREFRAMES
		if(__COLOR_BLACK != wireframe->color)
		{
			renderedWireframes++;
		}
#endif
	}

#ifdef __PROFILE_WIREFRAMES
	PRINT_TEXT("Wireframes: ", 1, 1);
	PRINT_TEXT("Rendered: ", 1, 2);
	PRINT_INT(wireframes, 15, 1);
	PRINT_INT(renderedWireframes, 15, 2);
#endif

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
	if(this->disabled || NULL == this->wireframes->head)
	{
		return;
	}

	DirectDraw::startDrawing(_directDraw);

	this->stopDrawing = false;

#ifdef __PROFILE_WIREFRAMES
	uint16 drawnWireframes = 0;
#endif

	// check the shapes
	for(VirtualNode node = this->wireframes->head; !this->stopDrawing && node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		if(__HIDE == wireframe->show)
		{
			continue;
		}

		if(__SHOW_NEXT_FRAME == wireframe->show)
		{
			wireframe->show = __SHOW;
			continue;
		}

		if(__COLOR_BLACK == wireframe->color)
		{
			continue;
		}
		
		if(wireframe->transparent & this->evenFrame)
		{
			continue;
		}

		Wireframe::draw(wireframe);

#ifdef __PROFILE_WIREFRAMES
		drawnWireframes++;
#endif

	}

#ifdef __PROFILE_WIREFRAMES
	PRINT_TEXT("Rendered: ", 1, 3);
	PRINT_INT(drawnWireframes, 15, 3);
#endif
}

void WireframeManager::enable()
{
	this->disabled = false;
}

void WireframeManager::disable()
{
	this->disabled = true;
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

