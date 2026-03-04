/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

/* This is based on Thunderstrucks' Rumble Pak library */

#ifndef __TEXTURE_MANAGER_H_
#define __TEXTURE_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>
#include <Texture.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class TextureManager
///
/// Inherits from Object
///
/// Manages rumble effects.
static class TextureManager : Object
{
	/// @publicsection

	/// Reset the manager's state.
	static void reset();

	/// Get a texture configured with the provided spec.
	/// @param textureClass: Class of texture to instantiate
	/// @param textureSpec: Spec used to select or initialize a texture with
	/// @return Texture initialized with the provided spec
	static Texture get(ClassPointer textureClass, const TextureSpec* textureSpec);

	/// Release a texture.
	/// @param texture: Texture to release
	static void release(Texture texture);

	/// Update texture pending rewriting of data in DRAM.
	/// @param maximumTextureRowsToWrite: Number of texture rows to write during this call
	/// @param defer: If true, the texture data is written overtime; otherwise
	/// all is written in a single pass
	static void updateTextures(int16 maximumTextureRowsToWrite, bool defer);

	/// Load textures in function of the provided array of specs.
	/// @param textureSpecs: Array of texture specs in function of which to load textures 
	static void loadTextures(const TextureSpec** textureSpecs);

	/// Erase the contents of graphics memory space.
	static void clearGraphicMemory();
}

#endif
