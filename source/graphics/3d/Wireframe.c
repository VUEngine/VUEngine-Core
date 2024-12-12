/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Camera.h>
#include <DebugConfig.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <WireframeManager.h>

#include "Wireframe.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class VirtualNode;
friend class VirtualList;


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void Wireframe::constructor(SpatialObject owner, const WireframeSpec* wireframeSpec)
{
	// construct base object
	Base::constructor(owner, wireframeSpec);

	if(NULL == wireframeSpec)
	{
		this->color = __COLOR_BRIGHT_RED;
		this->displacement = Vector3D::zero();
		this->interlaced = false;
		this->transparency = __TRANSPARENCY_NONE;
	}
	else
	{
		this->color = wireframeSpec->color;
		this->displacement = ((WireframeSpec*)this->componentSpec)->displacement;
		this->interlaced = ((WireframeSpec*)this->componentSpec)->interlaced;
		this->transparency = wireframeSpec->transparency;
	}

	this->bufferIndex = 0;
	this->squaredDistanceToCamera = 0;
	this->rendered = false;
	this->drawn = false;
}
//---------------------------------------------------------------------------------------------------------
void Wireframe::destructor()
{
	Wireframe::hide(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
void Wireframe::setDisplacement(Vector3D displacement)
{
	this->displacement = displacement;
}
//---------------------------------------------------------------------------------------------------------
bool Wireframe::isVisible()
{
	return this->drawn && __SHOW == this->show;
}
//---------------------------------------------------------------------------------------------------------
bool Wireframe::prepareForRender(Vector3D* relativePosition)
{
	Vector3D displacement = Vector3D::rotate(this->displacement, this->transformation->rotation);
	*relativePosition = Vector3D::sub(Vector3D::sum(this->transformation->position, displacement), _previousCameraPosition);

	if(NULL == ((WireframeSpec*)this->componentSpec))
	{
		this->color = __COLOR_BRIGHT_RED;
		this->interlaced = false;
		this->squaredDistanceToCamera = 0;
		return __COLOR_BLACK != this->color;
	}

	if(__COLOR_BLACK != ((WireframeSpec*)this->componentSpec)->color)
	{
		this->color = ((WireframeSpec*)this->componentSpec)->color;
		this->interlaced = ((WireframeSpec*)this->componentSpec)->interlaced;
#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
		this->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
#endif
		return __COLOR_BLACK != this->color;
	}

	NM_ASSERT(NULL != ((WireframeSpec*)this->componentSpec), "Wireframe::setupRenderingMode: NULL wireframeSpec");

	this->color = ((WireframeSpec*)this->componentSpec)->color;

	fixed_ext_t distanceToCamera = Vector3D::squareLength(*relativePosition);

	if(__COLOR_BLACK != this->color)
	{
		this->interlaced = ((WireframeSpec*)this->componentSpec)->interlaced;

		if(__FIXED_SQUARE(__DIRECT_DRAW_INTERLACED_THRESHOLD) < distanceToCamera)
		{
			this->interlaced = true;
		}
	}
	else
	{
		int16 cameraViewingAngle = __CAMERA_VIEWING_ANGLE;

		if(__FIXED_SQUARE(__PIXELS_TO_METERS(__SCREEN_WIDTH << 1)) < distanceToCamera)
		{
			if(__COS(cameraViewingAngle) > __FIXED_EXT_TO_FIX7_9(Vector3D::dotProduct(Vector3D::normalize(*relativePosition), _cameraDirection)))
			{
#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
				this->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
#endif
				this->color = __COLOR_BLACK;
				return __COLOR_BLACK != this->color;
			}
		}
		
		if(__FIXED_SQUARE((__DIRECT_DRAW_INTERLACED_THRESHOLD << 1) < distanceToCamera))
		{
			this->color = __COLOR_BLACK;
			this->interlaced = true;
			return __COLOR_BLACK != this->color;
		}
		else if(__FIXED_SQUARE(__DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 1)) < distanceToCamera)
		{
			this->interlaced = true;
			this->color = __COLOR_DARK_RED;
		}
		else if(__FIXED_SQUARE(__DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 2)) < distanceToCamera)
		{
			this->interlaced = ((WireframeSpec*)this->componentSpec)->interlaced;
			this->color = __COLOR_DARK_RED;
		}
		else if(__FIXED_SQUARE(__DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 3)) < distanceToCamera)
		{
			this->interlaced = true;
			this->color = __COLOR_MEDIUM_RED;
		}
		else if(__FIXED_SQUARE(__DIRECT_DRAW_INTERLACED_THRESHOLD) < distanceToCamera)
		{
			this->interlaced = true;
			this->color = __COLOR_BRIGHT_RED;
		}
		else if(__FIXED_SQUARE(__DIRECT_DRAW_INTERLACED_THRESHOLD - (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 1)) < distanceToCamera)
		{
			this->interlaced = ((WireframeSpec*)this->componentSpec)->interlaced;
			this->color = __COLOR_MEDIUM_RED;
		}
		else
		{
			this->interlaced = ((WireframeSpec*)this->componentSpec)->interlaced;
			this->color = __COLOR_BRIGHT_RED;
		}
	}

	this->interlaced += ((WireframeSpec*)this->componentSpec)->interlaced;

#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
	this->squaredDistanceToCamera = distanceToCamera;
#endif

	return __COLOR_BLACK != this->color;
}
//---------------------------------------------------------------------------------------------------------
RightBox Wireframe::getRightBox()
{
	return (RightBox){0, 0, 0, 0, 0, 0};
}
//---------------------------------------------------------------------------------------------------------
VirtualList Wireframe::getVertices()
{
	return NULL;
}
//---------------------------------------------------------------------------------------------------------
void Wireframe::render(Vector3D relativePosition __attribute__((unused)))
{}
//---------------------------------------------------------------------------------------------------------
