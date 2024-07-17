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
#include <VirtualList.h>
#include <VirtualNode.h>
#include <WireframeManager.h>

#include "Wireframe.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 */
void Wireframe::constructor(SpatialObject owner, WireframeSpec* wireframeSpec)
{
	// construct base object
	Base::constructor(owner, wireframeSpec);

	this->color = NULL == wireframeSpec ? __COLOR_BRIGHT_RED : wireframeSpec->color;
	this->displacement = ((WireframeSpec*)this->componentSpec)->displacement;
	this->interlaced = ((WireframeSpec*)this->componentSpec)->interlaced;
	this->bufferIndex = 0;
	this->transparent = wireframeSpec->transparent;
	this->squaredDistanceToCamera = 0;
	this->rendered = false;
	this->drawn = false;
}

/**
 * Class destructor
 */
void Wireframe::destructor()
{
	Wireframe::hide(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Rendered
 */
bool Wireframe::render()
{
	return true;
}

/**
 * Get vertices
 */
VirtualList Wireframe::getVertices()
{
	return NULL;
}

/**
 * Get pixel right box
 */
PixelRightBox Wireframe::getPixelRightBox()
{
	return (PixelRightBox){0, 0, 0, 0, 0, 0};
}

void Wireframe::setupRenderingMode(const Vector3D* relativePosition)
{
	if(NULL == ((WireframeSpec*)this->componentSpec) || __COLOR_BLACK != ((WireframeSpec*)this->componentSpec)->color)
	{
		this->color = ((WireframeSpec*)this->componentSpec)->color;
		this->interlaced = ((WireframeSpec*)this->componentSpec)->interlaced;
#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
		this->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
#endif
		return;
	}

	NM_ASSERT(NULL != ((WireframeSpec*)this->componentSpec), "Wireframe::setupRenderingMode: NULL wireframeSpec");

	if(0 > Vector3D::dotProduct(*relativePosition, _cameraDirection))
	{
		this->interlaced = ((WireframeSpec*)this->componentSpec)->interlaced;
		this->color = __COLOR_BLACK;
#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
		this->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
#endif
		return;
	}

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

		if(0 == distanceToCamera)
		{
			this->color = __COLOR_BLACK;
#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
			this->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
#endif
			return;
		}

		if(__FIXED_SQUARE(__PIXELS_TO_METERS(__SCREEN_WIDTH << 1)) > distanceToCamera)
		{
			// This is a hack. The correct operation should use a fixed div and a conversion to int
			// but this is way faster and works accurately enough
			cameraViewingAngle += (__FIXED_SQUARE(__PIXELS_TO_METERS(__SCREEN_WIDTH << 2)) / distanceToCamera);
		}
		
		if(__COS(cameraViewingAngle) > __FIXED_EXT_TO_FIX7_9(Vector3D::dotProduct(Vector3D::normalize(*relativePosition), _cameraDirection)))
		{
#ifdef __WIREFRAME_MANAGER_SORT_FOR_DRAWING
			this->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
#endif
			this->color = __COLOR_BLACK;
			return;
		}

		if(__FIXED_SQUARE((__DIRECT_DRAW_INTERLACED_THRESHOLD << 1) < distanceToCamera))
		{
			this->color = __COLOR_BLACK;
			this->interlaced = true;
			return;
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
}

void Wireframe::setDisplacement(const Vector3D* displacement)
{
	if(NULL != displacement)
	{
		this->displacement = *displacement;
	}
}

/**
 * Is visible?
 *
 * @return visibility
 */
bool Wireframe::isVisible()
{
	return this->drawn && __SHOW == this->show;
}