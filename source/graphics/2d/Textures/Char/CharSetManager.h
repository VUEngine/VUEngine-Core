/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CHARSET_MANAGER_H_
#define CHARSET_MANAGER_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <CharSet.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup graphics-2d-textures-char
singleton class CharSetManager : Object
{
	// Charsets defined
	VirtualList charSets;
	// Charsets pending writing
	VirtualList charSetsPendingWriting;
	// Next offset to be reclaimed
	uint16 freedOffset;
	bool preventDefragmentation;

	/// @publicsection
	static CharSetManager getInstance();
	void reset();
	CharSet getCharSet(CharSetSpec* charSetSpec);
	bool releaseCharSet(CharSet charSet);
	void defragment();
	void writeCharSets();
	bool writeCharSetsProgressively();
	int32 getTotalUsedChars();
	int32 getTotalFreeChars();
	int32 getTotalCharSets();
	void print(int32 x, int32 y);
}


#endif
