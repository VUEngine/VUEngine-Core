/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <CharSet.h>
#include <Printing.h>
#include <VirtualList.h>

#include "CharSetManager.h"


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;
friend class CharSet;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			CharSetManager::getInstance()
 * @memberof	CharSetManager
 * @public
 * @return		CharSetManager instance
 */


/**
 * Class constructor
 *
 * @private
 */
void CharSetManager::constructor()
{
	Base::constructor();

	this->charSets = new VirtualList();
	this->freedOffset = 1;
}

/**
 * Class destructor
 */
void CharSetManager::destructor()
{
	CharSetManager::reset(this);

	delete this->charSets;
	this->charSets = NULL;

	// allow a new construct
	Base::destructor();
}

/**
 * Reset manager's state
 */
void CharSetManager::reset()
{
	if(this->charSets)
	{
		VirtualList::deleteData(this->charSets);
	}

	this->freedOffset = 1;
}


/**
 * Load charSets from array of CharSetPecs
 *
 * @param charSets				NULL terminaed array of pointers to charSetSpec
 */
void CharSetManager::loadCharSets(const CharSetSpec** charSetSpecs)
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

/**
 * Find a previously registered CharSet with the given spec
 *
 * @private
 * @param charSetSpec		CharSet spec
 */
CharSet CharSetManager::findCharSet(CharSetSpec* charSetSpec)
{
	// try to find a charset with the same char spec
	VirtualNode node = this->charSets->head;

	CACHE_RESET;

	for(; NULL != node; node = node->next)
	{
		CharSet charSet = CharSet::safeCast(node->data);

		if(!isDeleted(charSet) && charSet->charSetSpec->tiles == charSetSpec->tiles && charSet->charSetSpec->shared == charSetSpec->shared)
		{
			return charSet;
		}
	}

	return NULL;
}

/**
 * Retrieve a CharSet
 *
 * @private
 * @param charSetSpec				CharSet spec to find o allocate a CharSet
 * @return 								Allocated CharSet
 */
CharSet CharSetManager::getCharSet(CharSetSpec* charSetSpec)
{
	if(NULL == charSetSpec)
	{
		return NULL;
	}

	CharSet charSet = NULL;

	if(!charSetSpec->shared)
	{
		// ask for allocation
		charSet = CharSetManager::allocateCharSet(this, charSetSpec);
	}
	else
	{
		// first try to find an already created charset
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

/**
 * Release a previously allocated CharSet
 *
 * @param charSet			CharSet to release
 */
bool CharSetManager::releaseCharSet(CharSet charSet)
{
	if(isDeleted(charSet))
	{
		return true;
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

/**
 * Try to allocate a CHAR memory space for a new CharSet
 *
 * @private
 * @param charSetSpec		CharSet spec to allocate space for
 * @return 						Allocated CharSet
 */
CharSet CharSetManager::allocateCharSet(CharSetSpec* charSetSpec)
{
	NM_ASSERT(this->charSets, "CharSetManager::allocateCharSet: null this");
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
	Printing::setDebugMode(Printing::getInstance());
	Printing::clear(Printing::getInstance());
	CharSetManager::print(this, 1, 10);
	Printing::text(Printing::getInstance(), "CharSet ", 1, 19, NULL);
	Printing::text(Printing::getInstance(), "    Address: ", 1, 21, NULL);
	Printing::hex(Printing::getInstance(), (WORD)charSetSpec, 14, 21, 8, NULL);
	Printing::text(Printing::getInstance(), "    Size: ", 1, 22, NULL);
	Printing::int32(Printing::getInstance(), charSetSpec->numberOfChars, 14, 22, NULL);

	// if there isn't enough memory thrown an exception
	NM_ASSERT(false, "CharSetManager::allocateCharSet: CHAR mem depleted");
#endif

	return NULL;
}

/**
 * Write char sets pending writing
 */
void CharSetManager::writeCharSets()
{
	CharSetManager::defragment(this);

	VirtualNode node = this->charSets->head;

	for(; NULL != node; node = node->next)
	{
		CharSet::write(node->data);
	}
}

/**
 * Deframent CHAR memory
 */
void CharSetManager::defragment()
{
	while(1 < this->freedOffset)
	{
		CharSetManager::defragmentProgressively(this);
	}
}

/**
 * Deframent CHAR memory progressively
 */
bool CharSetManager::defragmentProgressively()
{
	if(1 < this->freedOffset)
	{
		VirtualNode node = this->charSets->head;

		for(; NULL != node; node = node->next)
		{
			CharSet charSet = CharSet::safeCast(node->data);

			if(isDeleted(charSet))
			{
				continue;
			}

			uint32 offset = CharSet::getOffset(charSet);

			if(this->freedOffset < offset)
			{
				uint16 newOffset = this->freedOffset;
				this->freedOffset += CharSet::getNumberOfChars(charSet);
				CharSet::setOffset(charSet, newOffset);
				return true;
			}
			else if(this->freedOffset == offset)
			{
				this->freedOffset += CharSet::getNumberOfChars(charSet);
			}
		}

		this->freedOffset = 1;
	}

	return false;
}

/**
 * Retrieve the total number of used CHARs
 *
 * @return 				Total number of used CHARs
 */
int32 CharSetManager::getTotalUsedChars()
{
	ASSERT(this->charSets, "CharSetManager::getTotalFreeChars: null charSets list");

	CharSet lastCharSet = VirtualList::back(this->charSets);
	return (int32)CharSet::getOffset(lastCharSet) + CharSet::getNumberOfChars(lastCharSet);
}

/**
 * Retrieve the total number of free CHARs
 *
 * @return 				Total number of free CHARs
 */
int32 CharSetManager::getTotalFreeChars()
{
	return __CHAR_MEMORY_TOTAL_CHARS - CharSetManager::getTotalUsedChars(this);
}

/**
 * Retrieve the total number of registered char sets
 *
 * @return 				Total number of registered char sets
 */
int32 CharSetManager::getTotalCharSets()
{
	return VirtualList::getCount(this->charSets);
}

/**
 * Print manager's state
 *
 * @param x				Camera's x coordinate
 * @param y				Camera's y coordinate
 */
#ifndef __SHIPPING
void CharSetManager::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "CHAR MEMORY USAGE", x, y++, NULL);

	Printing::text(Printing::getInstance(), "Total CharSets:        ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), VirtualList::getCount(this->charSets), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Total used chars:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), CharSetManager::getTotalUsedChars(this), x + 18, y, NULL);
	Printing::text(Printing::getInstance(), "Total free chars:      ", x, ++y, NULL);
	Printing::int32(Printing::getInstance(), CharSetManager::getTotalFreeChars(this), x + 18, y, NULL);
}
#endif
