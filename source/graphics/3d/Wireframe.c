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
	this->color = wireframeSpec->color;
	this->position = NULL;
	this->rotation = NULL;
	this->interlaced = true;
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
 * Start being rendered
 */
void Wireframe::show()
{
	WireframeManager::register(WireframeManager::getInstance(), this);
}

/**
 * Stop being rendered
 */
void Wireframe::hide()
{
	WireframeManager::remove(WireframeManager::getInstance(), this);
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
}

/**
 * Get vertices
 */
VirtualList Wireframe::getVertices()
{
	return NULL;
}

void Wireframe::setupRenderingMode(fix10_6_ext distanceToCamera)
{
	if(__COLOR_BLACK != this->wireframeSpec->color)
	{
		this->interlaced = false;
		this->color = this->wireframeSpec->color;

		if(__FIX10_6_EXT_MULT(__DIRECT_DRAW_INTERLACED_THRESHOLD, __DIRECT_DRAW_INTERLACED_THRESHOLD) < distanceToCamera)
		{
			this->interlaced = true;
		}
	}
	else
	{
		if(__FIX10_6_EXT_MULT(__DIRECT_DRAW_INTERLACED_THRESHOLD << 1, __DIRECT_DRAW_INTERLACED_THRESHOLD << 1) < distanceToCamera)
		{
			this->interlaced = true;
			this->color = __COLOR_DARK_RED;
		}
		else if(__FIX10_6_EXT_MULT((__DIRECT_DRAW_INTERLACED_THRESHOLD << 1) - (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 2), (__DIRECT_DRAW_INTERLACED_THRESHOLD << 1) - (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 2)) < distanceToCamera)
		{
			this->interlaced = false;
			this->color = __COLOR_DARK_RED;
		}
		else if(__FIX10_6_EXT_MULT(__DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 1), __DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 1)) < distanceToCamera)
		{
			this->interlaced = true;
			this->color = __COLOR_MEDIUM_RED;
		}
		else if(__FIX10_6_EXT_MULT(__DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 2), __DIRECT_DRAW_INTERLACED_THRESHOLD + (__DIRECT_DRAW_INTERLACED_THRESHOLD >> 2)) < distanceToCamera)
		{
			this->interlaced = true;
			this->color = __COLOR_BRIGHT_RED;
		}
		else if(__FIX10_6_EXT_MULT(__DIRECT_DRAW_INTERLACED_THRESHOLD, __DIRECT_DRAW_INTERLACED_THRESHOLD) < distanceToCamera)
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