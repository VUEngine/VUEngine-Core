/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with wireframeManager source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Camera.h>
#include <DebugConfig.h>
#include <DirectDraw.h>
#include <Printing.h>
#include <VirtualList.h>
#include <VIPManager.h>
#include <Wireframe.h>

#include "WireframeManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;
friend class Wireframe;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' ATTRIBUTES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Vector3D _cameraDirection __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = {0, 0, 0};
Vector3D _previousCameraPosition __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = {0, 0, 0};
Vector3D _previousCameraPositionBuffer __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = {0, 0, 0};
Rotation _previousCameraInvertedRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = {0, 0, 0};
Rotation _previousCameraInvertedRotationBuffer __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = {0, 0, 0};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool WireframeManager::isAnyVisible(Entity owner)
{
	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		if(owner == wireframe->owner && Wireframe::isVisible(wireframe))
		{
			return true;
		}
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Wireframe WireframeManager::instantiateComponent(Entity owner, const WireframeSpec* wireframeSpec)
{
	if(NULL == wireframeSpec)
	{
		return NULL;
	}

	Base::instantiateComponent(this, owner, (ComponentSpec*)wireframeSpec);

	return WireframeManager::createWireframe(owner, wireframeSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::deinstantiateComponent(Entity owner, Wireframe wireframe) 
{
	if(isDeleted(wireframe))
	{
		return;
	}

	Base::deinstantiateComponent(this, owner, Component::safeCast(wireframe));

	WireframeManager::destroyWireframe(wireframe);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void WireframeManager::reset()
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	WireframeManager::enable(wireframeManager);
	
	VirtualList::clear(wireframeManager->components);
	wireframeManager->disabled = false;

	_previousCameraPosition = *_cameraPosition;
	_previousCameraPositionBuffer = _previousCameraPosition;

	_previousCameraInvertedRotation = *_cameraInvertedRotation;
	_previousCameraInvertedRotationBuffer = _previousCameraInvertedRotation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void WireframeManager::enable()
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	wireframeManager->disabled = false;

	VIPManager::registerEventListener
	(
		ListenerObject::safeCast(wireframeManager), (EventListener)WireframeManager::onVIPGAMESTART, 
		kEventVIPManagerGAMESTART
	);

	VIPManager::registerEventListener
	(
		ListenerObject::safeCast(wireframeManager), (EventListener)WireframeManager::onVIPXPEND, 
		kEventVIPManagerXPEND
	);

	VIPManager::registerEventListener
	(
		ListenerObject::safeCast(wireframeManager), (EventListener)WireframeManager::onVIPManagerGAMESTARTDuringXPEND, 
		kEventVIPManagerGAMESTARTDuringXPEND
	);

	VIPManager::registerEventListener
	(
		ListenerObject::safeCast(wireframeManager), (EventListener)WireframeManager::onVIPManagerXPENDDuringGAMESTART, 
		kEventVIPManagerXPENDDuringGAMESTART
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void WireframeManager::disable()
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	wireframeManager->disabled = true;

	VIPManager::unregisterEventListener
	(
		ListenerObject::safeCast(wireframeManager), (EventListener)WireframeManager::onVIPGAMESTART, 
		kEventVIPManagerGAMESTARTDuringXPEND
	);

	VIPManager::unregisterEventListener
	(
		ListenerObject::safeCast(wireframeManager), (EventListener)WireframeManager::onVIPXPEND, 
		kEventVIPManagerGAMESTARTDuringXPEND
	);

	VIPManager::unregisterEventListener
	(
		ListenerObject::safeCast(wireframeManager), (EventListener)WireframeManager::onVIPManagerGAMESTARTDuringXPEND, 
		kEventVIPManagerXPENDDuringGAMESTART
	);

	VIPManager::unregisterEventListener
	(
		ListenerObject::safeCast(wireframeManager), (EventListener)WireframeManager::onVIPManagerXPENDDuringGAMESTART, 
		kEventVIPManagerGAMESTARTDuringXPEND
	);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static Wireframe WireframeManager::createWireframe(Entity owner, const WireframeSpec* wireframeSpec)
{
	if(NULL == wireframeSpec)
	{
		return NULL;
	}

	Wireframe wireframe = 
		((Wireframe (*)(Entity, const WireframeSpec*))((ComponentSpec*)wireframeSpec)->allocator)(owner, wireframeSpec);

	if(!isDeleted(wireframe) && WireframeManager::registerWireframe( wireframe))
	{
		return wireframe;
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void WireframeManager::destroyWireframe(Wireframe wireframe)
{
	NM_ASSERT(!isDeleted(wireframe), "WireframeManager::destroyWireframe: trying to dispose dead wireframe");
	ASSERT(__GET_CAST(Wireframe, wireframe), "WireframeManager::destroyWireframe: trying to dispose non wireframe");

	if(isDeleted(wireframe))
	{
		return;
	}

	if(WireframeManager::unregisterWireframe(wireframe))
	{
		Wireframe::hide(wireframe);
		delete wireframe;
	}
	else
	{
		NM_ASSERT(false, "WireframeManager::destroyWireframe: destroying a wireframe that I don't manage");
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool WireframeManager::registerWireframe(Wireframe wireframe)
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	NM_ASSERT(!isDeleted(wireframe), "WireframeManager::registerWireframe: coudln't create wireframe");

	if(isDeleted(wireframe))
	{
		return false;
	}

	if(!VirtualList::find(wireframeManager->components, wireframe))
	{
		VirtualList::pushBack(wireframeManager->components, wireframe);
	}
	else
	{
		NM_ASSERT(false, "WireframeManager::registerWireframe: already wireframe");
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool WireframeManager::unregisterWireframe(Wireframe wireframe)
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	NM_ASSERT(!isDeleted(wireframe), "WireframeManager::createWireframe: coudln't create wireframe");

	if(isDeleted(wireframe))
	{
		return false;
	}

	return VirtualList::removeData(wireframeManager->components, wireframe);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void WireframeManager::render()
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	if(NULL == wireframeManager->components->head)
	{
		return;
	}

	_cameraDirection = Vector3D::rotate((Vector3D){0, 0, __1I_FIXED}, *_cameraRotation);

#ifdef __PROFILE_WIREFRAMES
	wireframeManager->renderedWireframes = 0;
#endif

	for(VirtualNode node = wireframeManager->components->head; !wireframeManager->stopRendering &&  NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		if
		(
			__HIDE == wireframe->show || (wireframe->transparency & wireframeManager->evenFrame) || 
			__NON_TRANSFORMED == wireframe->transformation->invalid
		)
		{
#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
			wireframe->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
#endif

			continue;
		}

		Vector3D relativePosition = Vector3D::zero();

		if(!Wireframe::prepareForRender(wireframe, &relativePosition))
		{
			continue;
		}

		Wireframe::render(wireframe, relativePosition);
		wireframe->rendered = true;

#ifdef __PROFILE_WIREFRAMES
		wireframeManager->renderedWireframes++;
#endif
	}

#ifdef __PROFILE_WIREFRAMES
	WireframeManager::print(wireframeManager, 1, 1);
#endif

#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
	WireframeManager::sortProgressively(wireframeManager);
#endif

	_previousCameraPosition = _previousCameraPositionBuffer;
	_previousCameraPositionBuffer = *_cameraPosition;

	_previousCameraInvertedRotation = _previousCameraInvertedRotationBuffer;
	_previousCameraInvertedRotationBuffer = *_cameraInvertedRotation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void WireframeManager::draw()
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	if(wireframeManager->disabled || NULL == wireframeManager->components->head)
	{
		return;
	}

#ifdef __PROFILE_WIREFRAMES
	wireframeManager->drawnWireframes = 0;
#endif

	for(VirtualNode node = wireframeManager->components->head; !wireframeManager->stopDrawing && NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		wireframe->drawn = false;

		if(__COLOR_BLACK == wireframe->color)
		{
			continue;
		}

		if((__HIDE == wireframe->show) || (wireframe->transparency & wireframeManager->evenFrame))
		{
			continue;
		}

		wireframe->drawn = Wireframe::draw(wireframe);

#ifdef __PROFILE_WIREFRAMES
		wireframeManager->drawnWireframes++;
#endif
	}

	wireframeManager->evenFrame = __TRANSPARENCY_EVEN == wireframeManager->evenFrame ? __TRANSPARENCY_ODD : __TRANSPARENCY_EVEN;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void WireframeManager::showWireframes(Entity owner)
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	for(VirtualNode node = wireframeManager->components->head; NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		if(owner == wireframe->owner)
		{
			Wireframe::show(wireframe);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void WireframeManager::hideWireframes(Entity owner)
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	for(VirtualNode node = wireframeManager->components->head; NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		if(owner == wireframe->owner)
		{
			Wireframe::hide(wireframe);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void WireframeManager::showAllWireframes()
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	for(VirtualNode node = wireframeManager->components->head; NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		Wireframe::show(wireframe);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void WireframeManager::hideAllWireframes()
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	for(VirtualNode node = wireframeManager->components->head; NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		Wireframe::hide(wireframe);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool WireframeManager::hasWireframes()
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	return NULL != wireframeManager->components && NULL != wireframeManager->components->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static void WireframeManager::print(int32 x, int32 y)
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	Printing::text("WIREFRAME MANAGER", x, y++, NULL);
	y++;
	Printing::text("Wireframes:   ", x, y, NULL);
	Printing::int32(VirtualList::getCount(wireframeManager->components), x + 12, y++, NULL);
	Printing::text("Rendered:     ", x, y, NULL);
	Printing::int32(wireframeManager->renderedWireframes, x + 12, y++, NULL);
	Printing::text("Drawn:        ", x, y, NULL);
	Printing::int32(wireframeManager->drawnWireframes, x + 12, y++, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void WireframeManager::sort()
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	while(WireframeManager::sortProgressively(wireframeManager));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool WireframeManager::sortProgressively()
{
	WireframeManager wireframeManager = WireframeManager::getInstance();

	bool swapped = false;

	for(VirtualNode node = wireframeManager->components->head; NULL != node && NULL != node->next; node = node->next)
	{
		VirtualNode nextNode = node->next;

		NM_ASSERT(!isDeleted(node->data), "WireframeManager::sortProgressively: NULL node's data");
		ASSERT(__GET_CAST(Wireframe, nextNode->data), "WireframeManager::sortProgressively: NULL node's data cast");

		Wireframe wireframe = Wireframe::safeCast(node->data);

		NM_ASSERT(!isDeleted(nextNode->data), "WireframeManager::sortProgressively: NULL nextNode's data");
		ASSERT(__GET_CAST(Wireframe, nextNode->data), "WireframeManager::sortProgressively: NULL nextNode's data cast");

		Wireframe nextWireframe = Wireframe::safeCast(nextNode->data);

		// Check if z positions are swapped
		if(nextWireframe->squaredDistanceToCamera < wireframe->squaredDistanceToCamera)
		{
			// Swap nodes' data
			node->data = nextWireframe;
			nextNode->data = wireframe;

			node = nextNode;

			swapped = true;
			break;
		}
	}

	return swapped;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->stopRendering = false;
	this->stopDrawing = false;
	this->evenFrame = __TRANSPARENCY_EVEN;
	this->disabled = false;
	this->renderedWireframes = 0;
	this->drawnWireframes = 00;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::destructor()
{
	ASSERT(this->components, "WireframeManager::destructor: null wireframes");

	if(!isDeleted(this->components))
	{
		VirtualList::deleteData(this->components);
	}

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool WireframeManager::onVIPGAMESTART(ListenerObject eventFirer __attribute__ ((unused)))
{
	this->stopRendering = false;
	this->stopDrawing = kVIPManagerFavorPerformance == VIPManager::getDrawingStrategy();

	WireframeManager::render();

	this->stopDrawing = false;

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool WireframeManager::onVIPXPEND(ListenerObject eventFirer __attribute__ ((unused)))
{
	this->stopDrawing = false;
	this->stopRendering = kVIPManagerFavorPerformance == VIPManager::getDrawingStrategy();

	DirectDraw::preparteToDraw();
	WireframeManager::draw();

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool WireframeManager::onVIPManagerGAMESTARTDuringXPEND(ListenerObject eventFirer __attribute__ ((unused)))
{
	this->stopDrawing = kVIPManagerFavorPerformance == VIPManager::getDrawingStrategy();

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool WireframeManager::onVIPManagerXPENDDuringGAMESTART(ListenerObject eventFirer __attribute__ ((unused)))
{
	this->stopRendering = kVIPManagerFavorPerformance == VIPManager::getDrawingStrategy();

	DirectDraw::preparteToDraw();
	WireframeManager::draw();

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
