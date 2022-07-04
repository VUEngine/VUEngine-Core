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
	this->interlaced = true;
	this->bufferIndex = 0;
	this->show = __SHOW_NEXT_FRAME;

	WireframeManager::register(WireframeManager::getInstance(), this);
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
}

/**
 * Get vertices
 */
VirtualList Wireframe::getVertices()
{
	return NULL;
}

void Wireframe::setupRenderingMode(fixed_ext_t distanceToCamera)
{
	if(NULL == this->wireframeSpec)
	{
		this->color = __COLOR_BLACK;
	}
	else
	{
		this->color = this->wireframeSpec->color;
	}

	if(__COLOR_BLACK != this->color)
	{
		this->interlaced = false;

		if(__FIXED_EXT_MULT(__DIRECT_DRAW_INTERLACED_THRESHOLD, __DIRECT_DRAW_INTERLACED_THRESHOLD) < distanceToCamera)
		{
			this->interlaced = true;
		}
	}
	else
	{
		if(__FIXED_EXT_MULT(__DIRECT_DRAW_INTERLACED_THRESHOLD << 1, __DIRECT_DRAW_INTERLACED_THRESHOLD << 1) < distanceToCamera)
		{
			this->interlaced = true;
			this->color = __COLOR_DARK_RED;
		}
		else if(__FIXED_EXT_MULT((__DIRECT_DRAW_INTERLACED_THRESHOLD << 1) - (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 2), (__DIRECT_DRAW_INTERLACED_THRESHOLD << 1) - (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 2)) < distanceToCamera)
		{
			this->interlaced = false;
			this->color = __COLOR_DARK_RED;
		}
		else if(__FIXED_EXT_MULT(__DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 1), __DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 1)) < distanceToCamera)
		{
			this->interlaced = true;
			this->color = __COLOR_MEDIUM_RED;
		}
		else if(__FIXED_EXT_MULT(__DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 2), __DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 2)) < distanceToCamera)
		{
			this->interlaced = true;
			this->color = __COLOR_BRIGHT_RED;
		}
		else if(__FIXED_EXT_MULT(__DIRECT_DRAW_INTERLACED_THRESHOLD, __DIRECT_DRAW_INTERLACED_THRESHOLD) < distanceToCamera)
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
}