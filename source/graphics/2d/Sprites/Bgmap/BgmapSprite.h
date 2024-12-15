/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BGMAP_SPRITE_H_
#define BGMAP_SPRITE_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Sprite.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class BgmapSprite;


//=========================================================================================================
// CLASS' MACROS
//=========================================================================================================

#define __WORLD_SIZE_DISPLACEMENT					1
#define __G_DISPLACEMENT_BECAUSE_WH_0_EQUALS_1		1
#define __GX_LIMIT									511
#define __GY_LIMIT									223
#define __MINIMUM_BGMAP_SPRITE_WIDTH				0
#define __MINIMUM_BGMAP_SPRITE_HEIGHT				7


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

typedef int16 (*ParamTableEffectMethod)(BgmapSprite);


/// A BgmapSprite spec
/// @memberof BgmapSprite
typedef struct BgmapSpriteSpec
{
	SpriteSpec spriteSpec;

	/// The display mode (BGMAP, AFFINE, H-BIAS)
	uint16 bgmapMode;

	/// Pointer to affine/hbias manipulation function
	ParamTableEffectMethod applyParamTableEffect;

	/// Flag to indicate in which display to show the bg texture
	uint16 display;

} BgmapSpriteSpec;

/// A BgmapSprite spec that is stored in ROM
/// @memberof BgmapSprite
typedef const BgmapSpriteSpec BgmapSpriteROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class BgmapSprite
///
/// Inherits from Sprite
///
/// Displays a texture that is allocated in BGMAP space.
class BgmapSprite : Sprite
{
	/// @protectedsection

	/// Offset that keeps track of where to continue writing in param table 
	int16 paramTableRow;

	/// Offset within param table space that determines the area where this sprite
	/// is allowed to write
	uint32 param;

	/// Pointer to function that implements the param table based effects
	ParamTableEffectMethod applyParamTableEffect;

	/// Texture's configuration in BGMAP space
	BgmapTextureSource bgmapTextureSource;

	/// @publicsection

	/// Class' constructor
	/// @param owner: SpatialObject to which the sprite attaches to
	/// @param bgmapSpriteSpec: Specification that determines how to configure the sprite
	void constructor(SpatialObject owner, const BgmapSpriteSpec* bgmapSpriteSpec);
	
	/// Register this sprite with the appropriate sprites manager.
	override void registerWithManager();

	/// Unegister this sprite with the appropriate sprites manager.	
	override void unregisterWithManager();

	/// Check if the sprite has affine or hbias effects.
	/// @return True if the sprite's mode of display is (__WORLD_AFFINE or __WORLD_HBIAS)
	override bool hasSpecialEffects();
	
	/// Process affine and hbias effects
	override void processEffects();

	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @return The index that determines the region of DRAM that this sprite manages
	override int16 doRender(int16 index);

	/// Set the current multiframe
	/// @param frame: Current animation frame 
	override void setMultiframe(uint16 frame);

	/// Set the sprite's rotation.
	/// @param rotation: Rotation to apply to the sprite 
	override void setRotation(const Rotation* rotation);

	/// Set the sprite's scale.
	/// @param scale: Scale to apply to the sprite 
	override void setScale(const PixelScale* scale);

	/// Retrieve the sprite's total number of pixels actually displayed.
	/// @return Sprite's total number of pixels actually displayed
	override int32 getTotalPixels();

	/// Configure the sprite's texture.
	void configureTexture();
	
	/// Configure the displays on which to show the sprite and how it will be displayed
	/// @param display: Displays on which to show the sprite (__WORLD_ON, __WORLD_LON or __WORLD_RON)
	/// @param mode: The mode to use to display the sprite (__WORLD_BGMAP | __WORLD_AFFINE | __WORLD_HBIAS)
	void setMode(uint16 display, uint16 mode);

	/// Set the offset within param table space that determines the area where 
	/// this sprite is allowed to write.
	/// @param param: Offset within param table space
	void setParam(uint32 param);

	/// Retrieve the offset within param table space that determines the area where 
	/// this sprite is allowed to write
	/// @return Offset within param table space
	uint32 getParam();

	/// Retrieve the offset that keeps track of where to continue writing in param table
	/// @return Offset within param table space
	int16 getParamTableRow();

	/// Force the rewrite of the sprite's param table during the next rendering cycle
	void invalidateParamTable();

	/// Callback for when the sprite's texture is rewriten.
	/// @param eventFirer: The rewrite texture
	/// @return True if the listener must be kept; false to remove after the current call
	bool onTextureRewritten(ListenerObject eventFirer);

	/// Start rewriting the sprite's param table for affine transformations.
	void applyAffineTransformations();

	/// Start rewriting the sprite's param table for hbias effects.
	void applyHbiasEffects();
}


#endif
