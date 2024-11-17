/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_TEXTURE_H_
#define OBJECT_TEXTURE_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Texture.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class ObjectSprite;


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// A ObjectTexture spec
/// @memberof ObjectTexture
typedef const TextureSpec ObjectTextureSpec;

/// A ObjectTexture spec that is stored in ROM
/// @memberof ObjectTexture
typedef const ObjectTextureSpec ObjectTextureROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class BgmapTexture
///
/// Inherits from Texture
///
/// A texture allocated in OBJECT memory.
/// @ingroup graphics-2d-textures-object
class ObjectTexture : Texture
{
	/// @publicsection

	/// Class' constructor
	/// @param objectTextureSpec: Specification that determines how to configure the texture
	/// @param id: Texture's identificator
	void constructor(ObjectTextureSpec* objectTextureSpec, uint16 id);
}


#endif
