/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_TEXTURE_H_
#define OBJECT_TEXTURE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Texture.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

class ObjectSprite;

/**
 * A ObjectTexture spec
 *
 * @memberof ObjectTexture
 */
typedef const TextureSpec ObjectTextureSpec;

/**
 * A ObjectTexture spec that is stored in ROM
 *
 * @memberof ObjectTexture
 */
typedef const ObjectTextureSpec ObjectTextureROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-textures-object
class ObjectTexture : Texture
{
	/// @publicsection
	void constructor(ObjectTextureSpec* objectTextureSpec, uint16 id);
}


#endif
