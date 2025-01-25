/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef M_BGMAP_SPRITE_H_
#define M_BGMAP_SPRITE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <BgmapSprite.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A MBgmapSprite spec
/// @memberof MBgmapSprite
typedef struct MBgmapSpriteSpec
{
	BgmapSpriteSpec bgmapSpriteSpec;

	/// Texture to use with the sprite
	TextureSpec** textureSpecs;

	/// SCX/SCY value
	uint32 scValue;

	/// Flag to loop the x axis
	int32 xLoop;

	/// Flag to loop the y axis
	int32 yLoop;

	/// Bounds the sprite's width to provide culling; if 0, the value is inferred from the texture
	int32 width;

	/// Bounds the sprite's width to provide culling; if 0, the value is inferred from the texture
	int32 height;

} MBgmapSpriteSpec;

/// A MBgmapSprite spec that is stored in ROM
/// @memberof MBgmapSprite
typedef const MBgmapSpriteSpec MBgmapSpriteROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class MBgmapSprite
///
/// Inherits from MBgmapSprite
///
/// Can display various textures across multiple BGMAP memory segments.
class MBgmapSprite : BgmapSprite
{
	/// @protectedsection

	/// List of textures to display
	VirtualList textures;

	/// Cache of the first texture x offest to speed up rendering
	uint16 textureXOffset;

	/// Cache of the first texture x offest to speed up rendering
	uint16 textureYOffset;

	/// @publicsection

	/// Class' constructor
	/// @param owner: Entity to which the sprite attaches to
	/// @param mBgmapSpriteSpec: Specification that determines how to configure the sprite
	void constructor(Entity owner, const MBgmapSpriteSpec* mBgmapSpriteSpec);

	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @return The index that determines the region of DRAM that this sprite manages
	override int16 doRender(int16 index);

	/// Set the current multiframe.
	/// @param frame: Current animation frame 
	override void setMultiframe(uint16 frame);
}

#endif
