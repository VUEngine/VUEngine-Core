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

#include <InverseBox.h>
#include <Optics.h>
#include <Polyhedron.h>
#include <Math.h>
#include <CollisionHelper.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Box;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @param owner
 */
void InverseBox::constructor(SpatialObject owner)
{
	Base::constructor(owner);
}

/**
 * Class destructor
 */
 void InverseBox::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
