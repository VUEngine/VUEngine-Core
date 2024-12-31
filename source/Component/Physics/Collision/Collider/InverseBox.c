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
void InverseBox::constructor(GameObject owner, const ColliderSpec* colliderSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, colliderSpec);

	this->classIndex = kColliderInverseBoxIndex;
}
//---------------------------------------------------------------------------------------------------------
 void InverseBox::destructor()
{
	// Always explicitly call the base's destructor 
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
