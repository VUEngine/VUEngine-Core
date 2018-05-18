/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

singleton class CharSetManager : Object
{
	/**
	 * @var VirtualList charSets
	 * @brief			Charsets defined
	 * @memberof		CharSetManager
	 */
	VirtualList charSets;
	/**
	 * @var VirtualList charSetsPendingWriting
	 * @brief			Charsets pending writing
	 * @memberof		CharSetManager
	 */
	VirtualList charSetsPendingWriting;
	/**
	 * @var u16 		freedOffset
	 * @brief			Next offset to be reclaimed
	 * @memberof		CharSetManager
	 */
	u16 freedOffset;

	static CharSetManager getInstance();
	void reset();
	CharSet getCharSet(CharSetDefinition* charSetDefinition);
	void releaseCharSet(CharSet charSet);
	void defragment();
	void writeCharSets();
	bool writeCharSetsProgressively();
	int getTotalUsedChars();
	int getTotalFreeChars();
	int getTotalCharSets();
	void print(int x, int y);
}


#endif
