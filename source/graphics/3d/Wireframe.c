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

#include <Wireframe.h>
#include <WireframeManager.h>
#include <debugConfig.h>


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
void Wireframe::constructor(WireframeSpec* wireframeSpec)
{
	// construct base object
	Base::constructor();

	this->wireframeSpec = wireframeSpec;
	this->color = NULL == wireframeSpec ? __COLOR_BRIGHT_RED : wireframeSpec->color;
	this->position = NULL;
	this->rotation = NULL;
	this->interlaced = false;
	this->bufferIndex = 0;
	this->show = __SHOW_NEXT_FRAME;
	this->transparent = wireframeSpec->transparent;
	this->squaredDistanceToCamera = 0;
}

/**
 * Class destructor
 */
void Wireframe::destructor()
{
	Wireframe::hide(this);

	WireframeManager::remove(WireframeManager::getInstance(), this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Set transparent flag
 */
void Wireframe::setTransparent(bool transparent)
{
	this->transparent = transparent;
}

/**
 * Start being rendered
 */
void Wireframe::show()
{
	this->show = __SHOW_NEXT_FRAME;
}

/**
 * Stop being rendered
 */
void Wireframe::hide()
{
	this->show = __HIDE;
}

/**
 * Rendered
 */
void Wireframe::render()
{
}

/**
 * Position
 */
void Wireframe::setup(const Vector3D* position __attribute__((unused)), const Rotation* rotation __attribute__((unused)), const Scale* scale __attribute__((unused)))
{
	this->position = position;
	this->rotation = rotation;
	this->scale = scale;

	WireframeManager::register(WireframeManager::getInstance(), this);
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
	if(NULL == this->wireframeSpec)
	{
		this->color = __COLOR_BRIGHT_RED;
		this->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
		return;
	}

	NM_ASSERT(NULL != this->wireframeSpec, "Wireframe::setupRenderingMode: NULL wireframeSpec");

	extern Vector3D _cameraDirection;

	if(0 > Vector3D::dotProduct(*relativePosition, _cameraDirection))
	{
		this->interlaced = this->wireframeSpec->interlaced;
		this->color = __COLOR_BLACK;
		this->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
		return;
	}

	this->color = this->wireframeSpec->color;

	fixed_ext_t distanceToCamera = Vector3D::squareLength(*relativePosition);

	if(__COLOR_BLACK != this->color)
	{
		this->interlaced = false;

		if(__FIXED_SQUARE(__DIRECT_DRAW_INTERLACED_THRESHOLD) < distanceToCamera)
		{
			this->interlaced = true;
		}
	}
	else
	{
		if(__COS(__CAMERA_VIEWING_ANGLE) > __FIXED_EXT_TO_FIX7_9(Vector3D::dotProduct(Vector3D::normalize(*relativePosition), _cameraDirection)))
		{
			this->color = __COLOR_BLACK;
			this->squaredDistanceToCamera = __WIREFRAME_MAXIMUM_SQUARE_DISTANCE_TO_CAMERA;
			return;
		}

		if(__FIXED_SQUARE((__DIRECT_DRAW_INTERLACED_THRESHOLD << 1) < distanceToCamera))
		{
			this->interlaced = true;
			this->color = __COLOR_BLACK;
		}
		else if(__FIXED_SQUARE(__DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 1)) < distanceToCamera)
		{
			this->interlaced = true;
			this->color = __COLOR_DARK_RED;
		}
		else if(__FIXED_SQUARE(__DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 2)) < distanceToCamera)
		{
			this->interlaced = false;
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
			this->interlaced = false;
			this->color = __COLOR_MEDIUM_RED;
		}
		else
		{
			this->interlaced = false;
			this->color = __COLOR_BRIGHT_RED;
		}
	}

	this->interlaced += this->wireframeSpec->interlaced;

	this->squaredDistanceToCamera = distanceToCamera;
}