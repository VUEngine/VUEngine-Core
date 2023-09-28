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

#include <ObjectTexture.h>

#include <ObjectSprite.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 * @param objectTextureSpec		Texture spec
 * @param id							Identifier
 */
void ObjectTexture::constructor(ObjectTextureSpec* objectTextureSpec, uint16 id)
{
	// construct base object
	Base::constructor((TextureSpec*)objectTextureSpec, id);
}

/**
 * Class destructor
 */
void ObjectTexture::destructor()
{
	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}
