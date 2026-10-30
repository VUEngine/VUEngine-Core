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
abstract class TextureManager : Object
{
	// List of textures
	VirtualList textures;

	/// @publicsection

	/// Class' constructor
	void constructor();

	/// Register a texture manager to respond to texture requests.
	/// @param textureManager: Instance to register
	static void registerManager(TextureManager textureManager);

	/// Reset the manager's state.
	static void resetManager();

	/// Get a texture configured with the provided spec.
	/// @param textureClass: Class of texture to instantiate
	/// @param textureSpec: Spec used to select or initialize a texture with
	/// @return Texture initialized with the provided spec
	static Texture getTexture(ClassPointer textureClass, const TextureSpec* textureSpec);

	/// Release a texture.
	/// @param texture: Texture to release
	static void releaseTexture(Texture texture);

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

	/// Retrieve the class type of the managed textures.
	/// @return ClassPointer to the type of textures
	virtual ClassPointer getTextureClass() = 0;

	/// Erase the contents of graphics memory space.
	virtual void clear() = 0;

	/// Reset the manager's state.
	virtual void reset() = 0;

	/// Load textures in function of the provided array of specs.
	/// @param textureSpecs: Array of texture specs in function of which to load textures 
	virtual void load(const TextureSpec** textureSpecs) = 0;

	/// Retrieve a texture initialized with the provided spec.
	/// @param textureSpec: Spec to use to initilize the desired texture
	/// @return Texture initialized with the provided spec
	virtual Texture get(const TextureSpec* textureSpec) = 0;

	/// Release a texture.
	/// @param texture: Texture to release
	virtual void release(Texture texture) = 0;
}

#endif
