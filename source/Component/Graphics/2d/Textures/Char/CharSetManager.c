/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with charSetManager source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <CharSet.h>
#include <Printing.h>
#include <VirtualList.h>

#include "CharSetManager.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// FORWARD DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;
friend class CharSet;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CharSetManager::reset()
{
	CharSetManager charSetManager = CharSetManager::getInstance(NULL);

	if(charSetManager->charSets)
	{
		VirtualList::deleteData(charSetManager->charSets);
	}

	charSetManager->freedOffset = 1;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CharSetManager::loadCharSets(const CharSetSpec** charSetSpecs)
{
	if(NULL != charSetSpecs)
	{
		for(int16 i = 0; charSetSpecs[i]; i++)
		{
			if(charSetSpecs[i]->shared)
			{
				CharSetManager::getCharSet((CharSetSpec*)charSetSpecs[i]);
			}
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static CharSet CharSetManager::getCharSet(CharSetSpec* charSetSpec)
{
	if(NULL == charSetSpec)
	{
		return NULL;
	}

	CharSet charSet = NULL;

	if(!charSetSpec->shared)
	{
		// Ask for allocation
		charSet = CharSetManager::allocateCharSet(charSetSpec);
	}
	else
	{
		// First try to find an already created charset
		charSet = CharSetManager::findCharSet(charSetSpec);

		if(NULL == charSet)
		{
			charSet = CharSetManager::allocateCharSet(charSetSpec);
		}
		else
		{
			CharSet::increaseUsageCount(charSet);
		}
	}

	return charSet;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static bool CharSetManager::releaseCharSet(CharSet charSet)
{
	CharSetManager charSetManager = CharSetManager::getInstance(NULL);

	if(isDeleted(charSet))
	{
		return false;
	}

	if(CharSet::decreaseUsageCount(charSet))
	{
		VirtualList::removeData(charSetManager->charSets, charSet);

		uint32 offset = CharSet::getOffset(charSet);

		if(1 == charSetManager->freedOffset || offset < charSetManager->freedOffset)
		{
			charSetManager->freedOffset = offset;
		}

		delete charSet;

		return true;
	}

	return false;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CharSetManager::defragment(bool deferred)
{
	CharSetManager charSetManager = CharSetManager::getInstance(NULL);

	if(1 < charSetManager->freedOffset)
	{
		do
		{
			VirtualNode node = charSetManager->charSets->head;

			for(; NULL != node; node = node->next)
			{
				CharSet charSet = CharSet::safeCast(node->data);

				if(isDeleted(charSet))
				{
					continue;
				}

				uint32 offset = CharSet::getOffset(charSet);

				if(charSetManager->freedOffset < offset)
				{
					uint16 newOffset = charSetManager->freedOffset;
					charSetManager->freedOffset += CharSet::getNumberOfChars(charSet);
					CharSet::setOffset(charSet, newOffset);
					break;
				}
				else if(charSetManager->freedOffset == offset)
				{
					charSetManager->freedOffset += CharSet::getNumberOfChars(charSet);
				}
			}

			if(NULL == node)
			{
				charSetManager->freedOffset = 1;
			}
		}
		while(!deferred && 1 < charSetManager->freedOffset);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int32 CharSetManager::getTotalUsedChars()
{
	CharSetManager charSetManager = CharSetManager::getInstance(NULL);

	ASSERT(charSetManager->charSets, "CharSetManager::getTotalFreeChars: null charSets list");

	CharSet lastCharSet = VirtualList::back(charSetManager->charSets);
	return (int32)CharSet::getOffset(lastCharSet) + CharSet::getNumberOfChars(lastCharSet);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int32 CharSetManager::getTotalFreeChars()
{
	CharSetManager charSetManager = CharSetManager::getInstance(NULL);

	return __CHAR_MEMORY_TOTAL_CHARS - CharSetManager::getTotalUsedChars(charSetManager);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static int32 CharSetManager::getTotalCharSets()
{
	CharSetManager charSetManager = CharSetManager::getInstance(NULL);

	return VirtualList::getCount(charSetManager->charSets);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static void CharSetManager::print(int32 x, int32 y)
{
	CharSetManager charSetManager = CharSetManager::getInstance(NULL);

	Printing::text("CHAR MEMORY USAGE", x, y++, NULL);

	Printing::text("Total CharSets:        ", x, ++y, NULL);
	Printing::int32(VirtualList::getCount(charSetManager->charSets), x + 18, y, NULL);
	Printing::text("Total used chars:      ", x, ++y, NULL);
	Printing::int32(CharSetManager::getTotalUsedChars(charSetManager), x + 18, y, NULL);
	Printing::text("Total free chars:      ", x, ++y, NULL);
	Printing::int32(CharSetManager::getTotalFreeChars(charSetManager), x + 18, y, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static CharSet CharSetManager::findCharSet(CharSetSpec* charSetSpec)
{
	CharSetManager charSetManager = CharSetManager::getInstance(NULL);

	CACHE_RESET;

	for(VirtualNode node = charSetManager->charSets->head; NULL != node; node = node->next)
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

static CharSet CharSetManager::allocateCharSet(CharSetSpec* charSetSpec)
{
	CharSetManager charSetManager = CharSetManager::getInstance(NULL);

	NM_ASSERT(charSetManager->charSets, "CharSetManager::allocateCharSet: null charSetManager");
	NM_ASSERT(charSetSpec, "CharSetManager::allocateCharSet: null charSetSpec");
	NM_ASSERT(charSetSpec->numberOfChars > 0, "CharSetManager::allocateCharSet: number of chars < 0");
	NM_ASSERT(charSetSpec->numberOfChars < __CHAR_MEMORY_TOTAL_CHARS, "CharSetManager::allocateCharSet: too many chars in spec");

	uint16 offset = NULL != charSetManager->charSets->head ? 0 : 1;

	if(NULL != charSetManager->charSets->head)
	{
		CharSet lastCharSet = CharSet::safeCast(VirtualList::back(charSetManager->charSets));
		offset += CharSet::getOffset(lastCharSet) + CharSet::getNumberOfChars(lastCharSet);
	}

	if((unsigned)offset + charSetSpec->numberOfChars < __CHAR_MEMORY_TOTAL_CHARS)
	{
		CharSet charSet = new CharSet(charSetSpec, offset);

		VirtualList::pushBack(charSetManager->charSets, charSet);

		return charSet;
	}

// TODO: implement __CHAR_FORCE_LOADING in Config.h file
#ifdef __CHAR_FORCE_LOADING
	else
	{
		CharSet charSet = new CharSet(charSetSpec, __CHAR_MEMORY_TOTAL_CHARS - charSetSpec->numberOfChars);

		VirtualList::pushBack(charSetManager->charSets, charSet);

		return charSet;		
	}
#endif

#ifndef __SHIPPING
	Printing::setDebugMode();
	Printing::clear();

	// If there isn't enough memory thrown an exception
	NM_ASSERT(false, "CharSetManager::allocateCharSet: CHAR mem depleted");
#endif

	return NULL;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void CharSetManager::writeCharSets()
{
	CharSetManager charSetManager = CharSetManager::getInstance(NULL);

	CharSetManager::defragment(false);

	for(VirtualNode node = charSetManager->charSets->head; NULL != node; node = node->next)
	{
		CharSet::write(node->data);
	}
}

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
	CharSetManager::reset();

	delete this->charSets;
	this->charSets = NULL;

	// Allow a new construct
	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
