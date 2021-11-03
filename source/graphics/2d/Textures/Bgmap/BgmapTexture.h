/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef BGMAP_TEXTURE_H_
#define BGMAP_TEXTURE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Texture.h>
#include <CharSet.h>
#include <Telegram.h>


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * A BgmapTexture spec
 *
 * @memberof BgmapTexture
 */
typedef const TextureSpec BgmapTextureSpec;

/**
 * A BgmapTexture spec that is stored in ROM
 *
 * @memberof BgmapTexture
 */
typedef const BgmapTextureSpec BgmapTextureROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// A texture which has the logic to be allocated in graphic memory
/// @ingroup graphics-2d-textures-bgmap
class BgmapTexture : Texture
{
	// Segment
	int8 segment;
	// Remaining rows to be written
	int8 remainingRowsToBeWritten;
	// flip flag
	bool horizontalFlip : 1;
	bool verticalFlip : 1;

	/// @publicsection
	static void addHWORD(HWORD* destination, const HWORD* source, uint32 numberOfHWORDS, uint32 offset, uint16 flip, bool backward);

	void constructor(BgmapTextureSpec* bgmapTextureSpec, uint16 id);
	int8 getRemainingRowsToBeWritten();
	int16 getXOffset();
	int16 getYOffset();
	int8 getSegment();
	void setSegment(int8 segment);
	void setHorizontalFlip(bool value);
	void setVerticalFlip(bool value);
	override bool write();
	override void rewrite();
}


#endif
