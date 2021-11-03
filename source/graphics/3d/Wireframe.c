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
void Wireframe::constructor(uint8 color)
{
	// construct base object
	Base::constructor();

	this->color = color;
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
