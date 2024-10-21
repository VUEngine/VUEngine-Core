/**
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

#include "ObjectTexture.h"


//=========================================================================================================
// CLASS' PUBLIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
void ObjectTexture::constructor(ObjectTextureSpec* objectTextureSpec, uint16 id)
{
	// construct base object
	Base::constructor((TextureSpec*)objectTextureSpec, id);
}
//---------------------------------------------------------------------------------------------------------
void ObjectTexture::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
//---------------------------------------------------------------------------------------------------------
