/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TEXT_OBJECT_SPRITE_H_
#define TEXT_OBJECT_SPRITE_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ObjectSprite.h>


//=========================================================================================================
// CLASS' DATA
//=========================================================================================================

/// A TextObjectSprite spec
/// @memberof TextObjectSprite
typedef struct TextObjectSpriteSpec
{
	/// it has a ObjectSprite spec at the beginning
	ObjectSpriteSpec objectSpriteSpec;

	/// Text
	const char* text;

	/// Font
	const char* font;

	/// Palette
	uint8 palette;

} TextObjectSpriteSpec;

/// A TextObjectSprite spec that is stored in ROM
/// @memberof TextObjectSprite
typedef const TextObjectSpriteSpec TextObjectSpriteROMSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class TextObjectSprite
///
/// Inherits from Sprite
///
/// Displays a text in OBJECT space.
class TextObjectSprite : ObjectSprite
{
	/// @protectedsection

	/// Pointer to the text to display
	const char* text;

	/// Pointer to the font name to use
	const char* font;

	/// Palette to apply when rendering the text
	uint16 palette;

	/// Flag to avoid printing the text every frame cycle if not necessary
	bool printed;

	/// @publicsection

	/// Class' constructor
	/// @param owner: GameObject to which the sprite attaches to
	/// @param textObjectSpriteSpec: Specification that determines how to configure the sprite
	void constructor(GameObject owner, const TextObjectSpriteSpec* textObjectSpriteSpec);

	/// Render the sprite by configuring the DRAM assigned to it by means of the provided index.
	/// @param index: Determines the region of DRAM that this sprite is allowed to configure
	/// @return The index that determines the region of DRAM that this sprite manages
	override int16 doRender(int16 index);
}


#endif
