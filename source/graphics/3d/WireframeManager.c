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

#include <Camera.h>
#include <DebugConfig.h>
#include <DebugUtilities.h>
#include <DirectDraw.h>
#include <Printing.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <Wireframe.h>

#include "WireframeManager.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;
friend class Wireframe;

Vector3D _cameraDirection __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = {0, 0, 0};
Vector3D _previousCameraPosition __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = {0, 0, 0};
Vector3D _previousCameraPositionBuffer __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = {0, 0, 0};
Rotation _previousCameraInvertedRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = {0, 0, 0};
Rotation _previousCameraInvertedRotationBuffer __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = {0, 0, 0};


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
	this->lockWireframeList = false;
	this->renderedWireframes = 0;
	this->drawnWireframes = 00;

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
 * Create a Wireframe to be rendered
 *
 * @param wireframeSpec	Wireframe spec
 */
Wireframe WireframeManager::createWireframe(WireframeSpec* wireframeSpec, SpatialObject owner)
{
	if(NULL == wireframeSpec)
	{
		return NULL;
	}

	Wireframe wireframe = ((Wireframe (*)(SpatialObject, WireframeSpec*))wireframeSpec->allocator)(owner, wireframeSpec);

	if(WireframeManager::registerWireframe(this, wireframe) == wireframe)
	{
		return wireframe;
	}

	delete wireframe;

	return NULL;
}

/**
 * Register a Wireframe to be rendered
 *
 * @param wireframeSpec	Wireframe spec
 */
Wireframe WireframeManager::registerWireframe(Wireframe wireframe)
{
	NM_ASSERT(!isDeleted(wireframe), "WireframeManager::createWireframe: coudln't create wireframe");

	if(isDeleted(wireframe))
	{
		return NULL;
	}

	if(!VirtualList::find(this->wireframes, wireframe))
	{
		this->lockWireframeList = true;

		VirtualList::pushBack(this->wireframes, wireframe);

		this->lockWireframeList = false;

		return wireframe;
	}

	return NULL;
}

/**
 * Destroy a Wireframe
 *
 * @param wireframe	Wireframe to remove
 */
void WireframeManager::destroyWireframe(Wireframe wireframe)
{
	NM_ASSERT(!isDeleted(wireframe), "WireframeManager::destroyWirefram: trying to dispose dead wireframe");
	ASSERT(__GET_CAST(Wireframe, wireframe), "WireframeManager::destroyWirefram: trying to dispose non wireframe");

	if(isDeleted(wireframe))
	{
		return;
	}

	Wireframe::hide(wireframe);

	if(wireframe == WireframeManager::unregisterWireframe(this, wireframe))
	{
		delete wireframe;
	}
}

/**
 * Register a Wireframe to be rendered
 *
 * @param wireframeSpec	Wireframe spec
 */
Wireframe WireframeManager::unregisterWireframe(Wireframe wireframe)
{
	NM_ASSERT(!isDeleted(wireframe), "WireframeManager::createWireframe: coudln't create wireframe");

	if(isDeleted(wireframe))
	{
		return NULL;
	}

	this->lockWireframeList = true;
	bool result = VirtualList::removeElement(this->wireframes, wireframe);
	this->lockWireframeList = false;

	return result ? wireframe : NULL;
}

/**
 * Reset manager's state
 */
void WireframeManager::reset()
{
	WireframeManager::enable(this);
	
	VirtualList::clear(this->wireframes);
	this->disabled = false;
	this->lockWireframeList = false;

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

	for(VirtualNode node = this->wireframes->head; NULL != node && NULL != node->next; node = node->next)
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

	this->stopRendering = false;

	_cameraDirection = Vector3D::rotate((Vector3D){0, 0, __1I_FIXED}, *_cameraRotation);

#ifdef __PROFILE_WIREFRAMES
	this->renderedWireframes = 0;
#endif

	// check the colliders
	for(VirtualNode node = this->wireframes->head; NULL != node && !this->stopRendering; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		wireframe->rendered = false;

		if((__HIDE == wireframe->show) || (wireframe->transparent & this->evenFrame) || __NON_TRANSFORMED == wireframe->transformation->invalid)
		{
#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
			wireframe->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
#endif

			continue;
		}

		Wireframe::render(wireframe);

		wireframe->rendered = true;

#ifdef __PROFILE_WIREFRAMES
		if(__COLOR_BLACK != wireframe->color)
		{
			this->renderedWireframes++;
		}
#endif
	}

#ifdef __SHOW_WIREFRAMES_PROFILING
	WireframeManager::print(this, 1, 1);
#endif

#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
	if(!this->lockWireframeList)
	{
		WireframeManager::sortProgressively(this);
	}
#endif

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

	this->stopDrawing = false;

#ifdef __PROFILE_WIREFRAMES
	this->drawnWireframes = 0;
#endif

	this->evenFrame = __TRANSPARENCY_EVEN == this->evenFrame ? __TRANSPARENCY_ODD : __TRANSPARENCY_EVEN;

	// check the colliders
	for(VirtualNode node = this->wireframes->head; !this->stopDrawing && NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		if(!wireframe->rendered || __COLOR_BLACK == wireframe->color)
		{
			continue;
		}

		if((__HIDE == wireframe->show) || (wireframe->transparent & this->evenFrame))
		{
			continue;
		}

		Wireframe::draw(wireframe);

#ifdef __PROFILE_WIREFRAMES
		this->drawnWireframes++;
#endif
	}
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
	Printing::text(Printing::getInstance(), "WIREFRAME MANAGER", x, y++, NULL);
	y++;
	Printing::text(Printing::getInstance(), "Wireframes: ", x, y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getSize(this->wireframes), x + 17, y++, NULL);
	Printing::text(Printing::getInstance(), "Rendered: ", x, y, NULL);
	Printing::int32(Printing::getInstance(), this->renderedWireframes, x + 17, y++, NULL);
	Printing::text(Printing::getInstance(), "Drawn: ", x, y, NULL);
	Printing::int32(Printing::getInstance(), this->drawnWireframes, x + 17, y++, NULL);
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
