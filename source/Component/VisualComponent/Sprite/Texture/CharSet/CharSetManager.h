/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef CHARSET_MANAGER_H_
#define CHARSET_MANAGER_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>
#include <CharSet.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class CharSetManager
///
/// Inherits from Object
///
/// Manages char sets and CHAR memory allocation.
singleton class CharSetManager : Object
{
	/// @protectedsection

	/// Allocated char sets with a block in CHAR memory allocated to them
	VirtualList charSets;

	/// Start offset in CHAR space when free memory starts
	uint16 freedOffset;

	/// @publicsection
	
	/// Print the manager's status.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(int32 x, int32 y);

	/// Empties internal virtual list of registered fonts
	void reset();

	/// Erase the contents of CHAR memory space.
	void clearDRAM();

	/// Load char sets in function of the provided array of specs.
	/// @param charSetSpecs: Array of char set specs in function of which to load char sets 
	void loadCharSets(const CharSetSpec** charSetSpecs);

	/// Retrieve a char set initialized with the provided spec.
	/// @param charSetSpec: Spec to use to initilize the desired char set
	/// @return Char set initialized with the provided spec
	CharSet getCharSet(const CharSetSpec* charSetSpec);

	/// Release a char set.
	/// @param charSet: Char set to release
	/// @return True if the char set is successfully deleted; false otherwise
	bool releaseCharSet(CharSet charSet);

	/// Write graphical data to VRAM.
	void writeCharSets();

	/// Defragment CHAR space.
	/// @return True if memory was defragmented; false otherwise
	void defragment(bool deferred);

	/// Return the total number of used CHARs in CHAR space.
	/// @return Total number of used CHARs in CHAR space
	int32 getTotalUsedChars();

	/// Return the total number of free CHARs in CHAR space.
	/// @return Total number of free CHARs in CHAR space
	int32 getTotalFreeChars();

	/// Return the total number of char sets.
	/// @return Total number of char sets
	int32 getTotalCharSets();
}

#endif
