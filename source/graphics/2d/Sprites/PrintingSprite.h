/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef PRINTING_SPRITE_H_
#define PRINTING_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <BgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * A PrintingSprite spec
 *
 * @memberof PrintingSprite
 */
typedef struct PrintingSpriteSpec
{
	/// it has a Sprite spec at the beginning
	BgmapSpriteSpec bgmapSpriteSpec;

} PrintingSpriteSpec;

/**
 * A PrintingSprite spec that is stored in ROM
 *
 * @memberof PrintingSprite
 */
typedef const PrintingSpriteSpec PrintingSpriteROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// Sprite which holds a texture and a drawing specification.
/// @ingroup graphics-2d-sprites-bgmap
class PrintingSprite : BgmapSprite
{
	// print WORLD's width
	uint16 w;
	// print WORLD's height
	uint16 h;
	// printing segment
	int8 printingBgmapSegment;
	
	/// @publicsection
	void constructor(SpatialObject owner, const PrintingSpriteSpec* printingSpriteSpec);
	void reset();

	void setPrintingBgmapSegment(int8 printingBgmapSegment);
	void setGValues(int16 gx, int16 gy, int16 gp);
	void setMValues(int16 mx, int16 my, int16 mp);
	void setSize(uint16 w, uint16 h);
	void setSize(uint16 w, uint16 h);

	int16 getGX();
	int16 getGY();
	int16 getGP();

	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @return The index that determines the regio of DRAM that this sprite configured
	override int16 doRender(int16 index);
}


#endif
