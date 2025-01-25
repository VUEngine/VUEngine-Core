/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Camera.h>
#include <DebugConfig.h>
#include <Printer.h>
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

void WireframeManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->stopRendering = false;
	this->stopDrawing = false;
	this->evenFrame = __TRANSPARENCY_EVEN;
	this->renderedWireframes = 0;
	this->drawnWireframes = 00;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::destructor()
{
	WireframeManager::stopListeningForVIP(this);

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool WireframeManager::onEvent(ListenerObject eventFirer __attribute__((unused)), uint16 eventCode)
{
	switch(eventCode)
	{
		case kEventVIPManagerGAMESTART:
		{
			this->stopRendering = false;
			this->stopDrawing = kVIPManagerFavorPerformance == VIPManager::getDrawingStrategy(eventFirer);

			WireframeManager::render(this);

			this->stopDrawing = false;

			return true;
		}

		case kEventVIPManagerXPEND:
		{
			this->stopDrawing = false;
			this->stopRendering = kVIPManagerFavorPerformance == VIPManager::getDrawingStrategy(eventFirer);

			WireframeManager::draw(this);

			return true;
		}

		case kEventVIPManagerGAMESTARTDuringXPEND:
		{
			this->stopDrawing = kVIPManagerFavorPerformance == VIPManager::getDrawingStrategy(eventFirer);

			return true;
		}

		case kEventVIPManagerXPENDDuringGAMESTART:
		{
			this->stopRendering = kVIPManagerFavorPerformance == VIPManager::getDrawingStrategy(eventFirer);

			WireframeManager::draw(this);

			return true;
		}
	}

	return Base::onEvent(this, eventFirer, eventCode);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32 WireframeManager::getType()
{
	return kWireframeComponent;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::enable()
{
	Base::enable(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::disable()
{
	WireframeManager::stopListeningForVIP(this);

	Base::disable(this);

	WireframeManager::destroyAllComponents(this);

	_previousCameraPosition = *_cameraPosition;
	_previousCameraPositionBuffer = _previousCameraPosition;

	_previousCameraInvertedRotation = *_cameraInvertedRotation;
	_previousCameraInvertedRotationBuffer = _previousCameraInvertedRotation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Wireframe WireframeManager::instantiateComponent(Entity owner, const WireframeSpec* wireframeSpec)
{
	if(NULL == wireframeSpec)
	{
		return NULL;
	}

	Base::instantiateComponent(this, owner, (ComponentSpec*)wireframeSpec);

	return WireframeManager::createWireframe(this, owner, wireframeSpec);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::deinstantiateComponent(Entity owner, Wireframe wireframe) 
{
	if(isDeleted(wireframe))
	{
		return;
	}

	Base::deinstantiateComponent(this, owner, Component::safeCast(wireframe));

	WireframeManager::destroyWireframe(this, wireframe);
}

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

Wireframe WireframeManager::createWireframe(Entity owner, const WireframeSpec* wireframeSpec)
{
	if(NULL == wireframeSpec)
	{
		return NULL;
	}

	Wireframe wireframe = 
		((Wireframe (*)(Entity, const WireframeSpec*))((ComponentSpec*)wireframeSpec)->allocator)(owner, wireframeSpec);
	
	if(!isDeleted(wireframe) && WireframeManager::registerWireframe(this, wireframe))
	{
		return wireframe;
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::destroyWireframe(Wireframe wireframe)
{
	NM_ASSERT(!isDeleted(wireframe), "WireframeManager::destroyWireframe: trying to dispose dead wireframe");
	ASSERT(__GET_CAST(Wireframe, wireframe), "WireframeManager::destroyWireframe: trying to dispose non wireframe");

	if(isDeleted(wireframe))
	{
		return;
	}

	if(WireframeManager::unregisterWireframe(this, wireframe))
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

bool WireframeManager::registerWireframe(Wireframe wireframe)
{
	NM_ASSERT(!isDeleted(wireframe), "WireframeManager::registerWireframe: coudln't create wireframe");

	if(isDeleted(wireframe))
	{
		return false;
	}

	if(!VirtualList::find(this->components, wireframe))
	{
		if(NULL == this->components->head)
		{
			WireframeManager::startListeningForVIP(this);			
		}

		VirtualList::pushBack(this->components, wireframe);
	}
	else
	{
		NM_ASSERT(false, "WireframeManager::registerWireframe: already wireframe");
	}

	return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool WireframeManager::unregisterWireframe(Wireframe wireframe)
{
	NM_ASSERT(!isDeleted(wireframe), "WireframeManager::createWireframe: coudln't create wireframe");

	if(isDeleted(wireframe))
	{
		return false;
	}

	bool removed = VirtualList::removeData(this->components, wireframe);

	if(NULL == this->components->head)
	{
		WireframeManager::stopListeningForVIP(this);			
	}

	return removed;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// secure
void WireframeManager::render()
{
	if(NULL == this->components->head)
	{
		return;
	}

	_cameraDirection = Vector3D::rotate((Vector3D){0, 0, __1I_FIXED}, *_cameraRotation);

#ifdef __PROFILE_WIREFRAMES
	this->renderedWireframes = 0;
#endif

	for(VirtualNode node = this->components->head; !this->stopRendering &&  NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		if
		(
			__HIDE == wireframe->show || (wireframe->transparency & this->evenFrame) || 
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
		this->renderedWireframes++;
#endif
	}

#ifdef __PROFILE_WIREFRAMES
	WireframeManager::print(this, 1, 1);
#endif

#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
	WireframeManager::sortProgressively(this);
#endif

	_previousCameraPosition = _previousCameraPositionBuffer;
	_previousCameraPositionBuffer = *_cameraPosition;

	_previousCameraInvertedRotation = _previousCameraInvertedRotationBuffer;
	_previousCameraInvertedRotationBuffer = *_cameraInvertedRotation;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// secure
void WireframeManager::draw()
{
	if(NULL == this->components->head)
	{
		return;
	}

#ifdef __PROFILE_WIREFRAMES
	this->drawnWireframes = 0;
#endif

	for(VirtualNode node = this->components->head; !this->stopDrawing && NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		wireframe->drawn = false;

		if(__COLOR_BLACK == wireframe->color)
		{
			continue;
		}

		if((__HIDE == wireframe->show) || (wireframe->transparency & this->evenFrame))
		{
			continue;
		}

		wireframe->drawn = Wireframe::draw(wireframe);

#ifdef __PROFILE_WIREFRAMES
		this->drawnWireframes++;
#endif
	}

	this->evenFrame = __TRANSPARENCY_EVEN == this->evenFrame ? __TRANSPARENCY_ODD : __TRANSPARENCY_EVEN;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::showWireframes(Entity owner)
{
	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		if(owner == wireframe->owner)
		{
			Wireframe::show(wireframe);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::hideWireframes(Entity owner)
{
	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		if(owner == wireframe->owner)
		{
			Wireframe::hide(wireframe);
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::showAllWireframes()
{
	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		Wireframe::show(wireframe);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::hideAllWireframes()
{
	for(VirtualNode node = this->components->head; NULL != node; node = node->next)
	{
		Wireframe wireframe = Wireframe::safeCast(node->data);

		Wireframe::hide(wireframe);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool WireframeManager::hasWireframes()
{
	return NULL != this->components && NULL != this->components->head;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void WireframeManager::print(int32 x, int32 y)
{
	Printer::text("WIREFRAME MANAGER", x, y++, NULL);
	y++;
	Printer::text("Wireframes:   ", x, y, NULL);
	Printer::int32(VirtualList::getCount(this->components), x + 12, y++, NULL);
	Printer::text("Rendered:     ", x, y, NULL);
	Printer::int32(this->renderedWireframes, x + 12, y++, NULL);
	Printer::text("Drawn:        ", x, y, NULL);
	Printer::int32(this->drawnWireframes, x + 12, y++, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::startListeningForVIP()
{
	HardwareManager::suspendInterrupts();

	VIPManager::addEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerGAMESTART);
	VIPManager::addEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerXPEND);
	VIPManager::addEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerGAMESTARTDuringXPEND);
	VIPManager::addEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerXPENDDuringGAMESTART);

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void WireframeManager::stopListeningForVIP()
{
	HardwareManager::suspendInterrupts();

	VIPManager::removeEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerGAMESTART);
	VIPManager::removeEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerXPEND);
	VIPManager::removeEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerGAMESTARTDuringXPEND);
	VIPManager::removeEventListener(VIPManager::getInstance(), ListenerObject::safeCast(this), kEventVIPManagerXPENDDuringGAMESTART);

	HardwareManager::resumeInterrupts();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool WireframeManager::sortProgressively()
{
	bool swapped = false;

	for(VirtualNode node = this->components->head; NULL != node && NULL != node->next; node = node->next)
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
