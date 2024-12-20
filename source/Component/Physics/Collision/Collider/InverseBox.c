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

#include "InverseBox.h"


//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class Box;


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void InverseBox::constructor(SpatialObject owner, const ColliderSpec* colliderSpec)
{
	Base::constructor(owner, colliderSpec);

	this->classIndex = kColliderInverseBoxIndex;
}
//---------------------------------------------------------------------------------------------------------
 void InverseBox::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
