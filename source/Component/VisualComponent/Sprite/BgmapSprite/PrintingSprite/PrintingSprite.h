/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PRINTING_SPRITE_H_
#define PRINTING_SPRITE_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <BgmapSprite.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DATA
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// A PrintingSprite spec
/// @memberof PrintingSprite
typedef struct PrintingSpriteSpec
{
	/// it has a Sprite spec at the beginning
	BgmapSpriteSpec bgmapSpriteSpec;

} PrintingSpriteSpec;

/// A PrintingSprite spec that is stored in ROM
/// @memberof PrintingSprite
typedef const PrintingSpriteSpec PrintingSpriteROMSpec;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class BgmapTexture
///
/// Inherits from Texture
///
/// Enables the Printer class to output text to the screen.
class PrintingSprite : BgmapSprite
{
	/// BGMAP segment used for printing
	int8 printingBgmapSegment;
	
	/// @publicsection
	void constructor(Entity owner, const PrintingSpriteSpec* printingSpriteSpec);
	
	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @return The index that determines the region of DRAM that this sprite manages
	override int16 doRender(int16 index);

	/// Reset the sprite's rendering configuration.
	void reset();

	/// Set the BGMAP segment used for printing.
	/// @param printingBgmapSegment: BGMAP segment to use for printing
	void setPrintingBgmapSegment(int8 printingBgmapSegment);

	/// Set the G values to be written to the WORLD's entry mananged by the sprite.
	/// @param gx: GX coordinate of the WORLD used to display the text
	/// @param gy: GY coordinate of the WORLD used to display the text
	/// @param gp: GP coordinate of the WORLD used to display the text
	void setGValues(int16 gx, int16 gy, int16 gp);

	/// Set the M values to be written to the WORLD's entry mananged by the sprite.
	/// @param mx: MX coordinate of BGMAP area to be used to display the text
	/// @param my: MY coordinate of BGMAP area to be used to display the text
	/// @param mp: MP coordinate of BGMAP area to be used to display the text
	void setMValues(int16 mx, int16 my, int16 mp);

	/// Set the size of the WORLD to by used to display the text
	/// @param width: Width of the WORLD to by used to display the text
	/// @param height: Height of the WORLD to by used to display the text
	void setSize(uint16 width, uint16 height);
}

#endif
