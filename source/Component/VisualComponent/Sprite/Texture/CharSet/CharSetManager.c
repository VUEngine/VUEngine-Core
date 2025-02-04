/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <CharSet.h>
#include <Mem.h>
#include <Printer.h>
#include <Singleton.h>
#include <VirtualList.h>

#include "CharSetManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;
friend class CharSet;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void CharSetManager::reset()
{
	if(this->charSets)
	{
		VirtualList::deleteData(this->charSets);
	}

	this->freedOffset = 1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void CharSetManager::clearDRAM()
{
	Mem::clear((BYTE*) __CHAR_SPACE_BASE_ADDRESS, 8192 * 4);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void CharSetManager::loadCharSets(const CharSetSpec** charSetSpecs)
{
	if(NULL != charSetSpecs)
	{
		for(int16 i = 0; charSetSpecs[i]; i++)
		{
			if(charSetSpecs[i]->shared)
			{
				CharSetManager::getCharSet(this, (CharSetSpec*)charSetSpecs[i]);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

CharSet CharSetManager::getCharSet(const CharSetSpec* charSetSpec)
{
	NM_ASSERT(NULL != charSetSpec, "CharSetManager::getCharSet: NULL charSetSpec");

	if(NULL == charSetSpec)
	{
		return NULL;
	}

	CharSet charSet = NULL;

	if(!charSetSpec->shared)
	{
		// Ask for allocation
		charSet = CharSetManager::allocateCharSet(this, charSetSpec);
	}
	else
	{
		// First try to find an already created charset
		charSet = CharSetManager::findCharSet(this, charSetSpec);

		if(NULL == charSet)
		{
			charSet = CharSetManager::allocateCharSet(this, charSetSpec);
		}
		else
		{
			CharSet::increaseUsageCount(charSet);
		}
	}

	return charSet;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure bool CharSetManager::releaseCharSet(CharSet charSet)
{
	if(isDeleted(charSet))
	{
		return false;
	}

	if(CharSet::decreaseUsageCount(charSet))
	{
		VirtualList::removeData(this->charSets, charSet);

		uint32 offset = CharSet::getOffset(charSet);

		if(1 == this->freedOffset || offset < this->freedOffset)
		{
			this->freedOffset = offset;
		}

		delete charSet;

		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void CharSetManager::defragment(bool deferred)
{
	if(1 < this->freedOffset)
	{
		do
		{
			VirtualNode node = this->charSets->head;

			for(; NULL != node; node = node->next)
			{
				CharSet charSet = CharSet::safeCast(node->data);

				NM_ASSERT(!isDeleted(charSet), "CharSetManager::defragment: deleted charset");

				uint32 offset = CharSet::getOffset(charSet);

				if(this->freedOffset < offset)
				{
					CharSet::setOffset(charSet, this->freedOffset);
					this->freedOffset += CharSet::getNumberOfChars(charSet);
					break;
				}
				else if(this->freedOffset == offset)
				{
					this->freedOffset += CharSet::getNumberOfChars(charSet);
				}
			}

			if(NULL == node)
			{
				this->freedOffset = 1;
			}
		}
		while(!deferred && 1 < this->freedOffset);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 CharSetManager::getTotalUsedChars()
{
	ASSERT(this->charSets, "CharSetManager::getTotalFreeChars: null charSets list");

	CharSet lastCharSet = VirtualList::back(this->charSets);
	return (int32)CharSet::getOffset(lastCharSet) + CharSet::getNumberOfChars(lastCharSet);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 CharSetManager::getTotalFreeChars()
{
	return __CHAR_MEMORY_TOTAL_CHARS - CharSetManager::getTotalUsedChars(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int32 CharSetManager::getTotalCharSets()
{
	return VirtualList::getCount(this->charSets);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void CharSetManager::print(int32 x, int32 y)
{
	Printer::text("CHAR MEMORY USAGE", x, y++, NULL);

	Printer::text("Total CharSets:        ", x, ++y, NULL);
	Printer::int32(VirtualList::getCount(this->charSets), x + 18, y, NULL);
	Printer::text("Total used chars:      ", x, ++y, NULL);
	Printer::int32(CharSetManager::getTotalUsedChars(this), x + 18, y, NULL);
	Printer::text("Total free chars:      ", x, ++y, NULL);
	Printer::int32(CharSetManager::getTotalFreeChars(this), x + 18, y, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CharSetManager::constructor()
{
	// Always explicitly call the base's constructor 
	Base::constructor();

	this->charSets = new VirtualList();
	this->freedOffset = 1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void CharSetManager::destructor()
{
	CharSetManager::reset(this);

	delete this->charSets;
	this->charSets = NULL;


	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

CharSet CharSetManager::findCharSet(const CharSetSpec* charSetSpec)
{
	CACHE_RESET;

	for(VirtualNode node = this->charSets->head; NULL != node; node = node->next)
	{
		CharSet charSet = CharSet::safeCast(node->data);

		if(!isDeleted(charSet) && charSet->charSetSpec->tiles == charSetSpec->tiles && charSet->charSetSpec->shared == charSetSpec->shared)
		{
			return charSet;
		}
	}

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

CharSet CharSetManager::allocateCharSet(const CharSetSpec* charSetSpec)
{
	NM_ASSERT(!isDeleted(this->charSets), "CharSetManager::allocateCharSet: null charSets");
	NM_ASSERT(charSetSpec, "CharSetManager::allocateCharSet: null charSetSpec");
	NM_ASSERT(charSetSpec->numberOfChars > 0, "CharSetManager::allocateCharSet: number of chars < 0");
	NM_ASSERT(charSetSpec->numberOfChars < __CHAR_MEMORY_TOTAL_CHARS, "CharSetManager::allocateCharSet: too many chars in spec");

	uint16 offset = NULL != this->charSets->head ? 0 : 1;

	if(NULL != this->charSets->head)
	{
		CharSet lastCharSet = CharSet::safeCast(VirtualList::back(this->charSets));
		offset += CharSet::getOffset(lastCharSet) + CharSet::getNumberOfChars(lastCharSet);
	}

	if((unsigned)offset + charSetSpec->numberOfChars < __CHAR_MEMORY_TOTAL_CHARS)
	{
		CharSet charSet = new CharSet(charSetSpec, offset);

		VirtualList::pushBack(this->charSets, charSet);

		return charSet;
	}

// TODO: implement __CHAR_FORCE_LOADING in Config.h file
#ifdef __CHAR_FORCE_LOADING
	else
	{
		CharSet charSet = new CharSet(charSetSpec, __CHAR_MEMORY_TOTAL_CHARS - charSetSpec->numberOfChars);

		VirtualList::pushBack(this->charSets, charSet);

		return charSet;		
	}
#endif

#ifndef __SHIPPING
	Printer::setDebugMode();
	Printer::clear();

	// If there isn't enough memory thrown an exception
	NM_ASSERT(false, "CharSetManager::allocateCharSet: CHAR mem depleted");
#endif

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

secure void CharSetManager::writeCharSets()
{
	CharSetManager::defragment(this, false);

	for(VirtualNode node = this->charSets->head; NULL != node; node = node->next)
	{
		CharSet::write(node->data);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
