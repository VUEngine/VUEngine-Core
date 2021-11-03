/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TEXT_OBJECT_SPRITE_H_
#define TEXT_OBJECT_SPRITE_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ObjectSprite.h>


//---------------------------------------------------------------------------------------------------------
//											 MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

/**
 * A TextObjectSprite spec
 *
 * @memberof TextObjectSprite
 */
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

/**
 * A TextObjectSprite spec that is stored in ROM
 *
 * @memberof TextObjectSprite
 */
typedef const TextObjectSpriteSpec TextObjectSpriteROMSpec;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-sprites-object
class TextObjectSprite : ObjectSprite
{
	const char* text;
	const char* font;
	uint16 palette;
	bool printed;

	/// @publicsection
	void constructor(const TextObjectSpriteSpec* textObjectSpriteSpec, Object owner);

	override int16 doRender(int16 index, bool evenFrame);
}


#endif
