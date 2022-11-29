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

	VIPManager::addEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), (EventListener)WireframeManager::onVIPManagerGAMESTARTDuringXPEND, kEventVIPManagerGAMESTARTDuringXPEND);
}

/**
 * Class destructor
 */
void WireframeManager::destructor()
{
	ASSERT(this->wireframes, "WireframeManager::destructor: null wireframes");

	if(!isDeleted(this->wireframes))
	{
		VirtualList::deleteData(this->wireframes);
		delete this->wireframes;
		this->wireframes = NULL;
	}

	VIPManager::removeEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), (EventListener)WireframeManager::onVIPManagerGAMESTARTDuringXPEND, kEventVIPManagerGAMESTARTDuringXPEND);

	// allow a new construct
	Base::destructor();
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
	WireframeManager::enable(this);
	
	VirtualList::clear(this->wireframes);
	this->disabled = false;

	_previousCameraPosition = *_cameraPosition;
	_previousCameraPositionBuffer = _previousCameraPosition;

	_previousCameraInvertedRotation = *_cameraInvertedRotation;
	_previousCameraInvertedRotationBuffer = _previousCameraInvertedRotation;
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
		ASSERT(__GET_CAST(Wireframe, nextNode->data), "WireframeManager::sortProgressively: NULL node's data cast");

		Wireframe wireframe = Wireframe::safeCast(node->data);

		NM_ASSERT(!isDeleted(nextNode->data), "WireframeManager::sortProgressively: NULL nextNode's data");
		ASSERT(__GET_CAST(Wireframe, nextNode->data), "WireframeManager::sortProgressively: NULL nextNode's data cast");

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

	// UI graphics synchronization involves moving the camera
	// which can mess rendering if the VIP's XPEND interrupt 
	// happens when the camera is modified
	Camera::suspendUIGraphicsSynchronization(Camera::getInstance());

	this->stopRendering = false;

	_cameraDirection = Vector3D::rotate((Vector3D){0, 0, __1I_FIXED}, *_cameraRotation);

#ifdef __PROFILE_WIREFRAMES
	uint16 wireframes = 0;
	uint16 renderedWireframes = 0;
#endif

	// check the shapes
	for(VirtualNode node = this->wireframes->head; NULL != node && !this->stopRendering; node = node->next)
	{
#ifdef __PROFILE_WIREFRAMES
		wireframes++;
#endif

		Wireframe wireframe = Wireframe::safeCast(node->data);

		if((__HIDE == wireframe->show) || (wireframe->transparent & this->evenFrame))
		{
#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
			wireframe->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
#endif
			continue;
		}

		Wireframe::render(wireframe);

		wireframe->draw = true;

#ifdef __PROFILE_WIREFRAMES
		if(__COLOR_BLACK != wireframe->color)
		{
			renderedWireframes++;
		}
#endif
	}

#ifdef __PROFILE_WIREFRAMES
	PRINT_TEXT("Wireframes: ", 1, 5);
	PRINT_TEXT("Rendered: ", 1, 6);
	PRINT_INT(wireframes, 15, 5);
	PRINT_INT(renderedWireframes, 15, 6);
#endif

#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
	WireframeManager::sortProgressively(this);
#endif

	_previousCameraPosition = _previousCameraPositionBuffer;
	_previousCameraPositionBuffer = *_cameraPosition;

	_previousCameraInvertedRotation = _previousCameraInvertedRotationBuffer;
	_previousCameraInvertedRotationBuffer = *_cameraInvertedRotation;


	Camera::resumeUIGraphicsSynchronization(Camera::getInstance());
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

	DirectDraw::startDrawing(DirectDraw::getInstance());

	this->stopDrawing = false;

#ifdef __PROFILE_WIREFRAMES
	uint16 drawnWireframes = 0;
#endif

	this->evenFrame = __TRANSPARENCY_EVEN == this->evenFrame ? __TRANSPARENCY_ODD : __TRANSPARENCY_EVEN;

	// check the shapes
	for(VirtualNode node = this->wireframes->head; !this->stopDrawing && NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		if(!wireframe->draw || __COLOR_BLACK == wireframe->color)
		{
			continue;
		}

		if((__HIDE == wireframe->show) || (wireframe->transparent & this->evenFrame))
		{
			continue;
		}

		Wireframe::draw(wireframe);

#ifdef __PROFILE_WIREFRAMES
		drawnWireframes++;
#endif
	}

#ifdef __PROFILE_WIREFRAMES
	PRINT_TEXT("Drawn: ", 1, 7);
	PRINT_INT(drawnWireframes, 15, 7);
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

void WireframeManager::hideWireframes()
{
	for(VirtualNode node = this->wireframes->head; NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		Wireframe::hide(wireframe);
	}
}

void WireframeManager::showWireframes()
{
	for(VirtualNode node = this->wireframes->head; NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		Wireframe::show(wireframe);
	}
}
