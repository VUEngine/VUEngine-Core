/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_TEXTURE_MANAGER_H_
#define OBJECT_TEXTURE_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <ObjectTexture.h>


//---------------------------------------------------------------------------------------------------------
//												MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-textures-bgmap
singleton class ObjectTextureManager : Object
{
	VirtualList objectTextures;

	/// @publicsection
	static ObjectTextureManager getInstance();
	ObjectTexture getTexture(ObjectTextureSpec* objectTextureSpec);
	void releaseTexture(ObjectTexture bgmapTexture);
	void reset();
	void updateTextures();
}


#endif
