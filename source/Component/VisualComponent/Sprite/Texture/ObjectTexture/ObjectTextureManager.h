/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OBJECT_TEXTURE_MANAGER_H_
#define OBJECT_TEXTURE_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>
#include <ObjectTexture.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class BgmapTextureManager
///
/// Inherits from Texture
///
/// Manages textures for OBJECT space.
singleton class ObjectTextureManager : Object
{
	// List of textures with BGMAP space allocated for them
	VirtualList objectTextures;

	/// @publicsection

	/// Reset the manager's state.
	void reset();

	/// Update texture pending rewriting of data in DRAM.
	/// @param maximumTextureRowsToWrite: Number of texture rows to write during this call
	/// @param defer: If true, the texture data is written overtime; otherwise
	/// all is written in a single pass
	void updateTextures(int16 maximumTextureRowsToWrite, bool defer);

	/// Retrieve a texture initialized with the provided spec.
	/// @param objectTextureSpec: Spec to use to initilize the desired texture
	/// @return Texture initialized with the provided spec
	ObjectTexture getTexture(ObjectTextureSpec* objectTextureSpec);

	/// Release a texture.
	/// @param objectTexture: Texture to release
	void releaseTexture(ObjectTexture objectTexture);
}

#endif
