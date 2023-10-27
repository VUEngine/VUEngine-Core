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

#include <Utilities.h>


//---------------------------------------------------------------------------------------------------------
//											DEFINITIONS
//---------------------------------------------------------------------------------------------------------

Clock _gameClock = NULL;
KeypadManager _keypadManager = NULL;
char _itoaArray[__CHAR_HOLDER_SIZE] __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = {0};
uint32 _seed __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE = 7; /* Seed value */
const char _itoaNumbers[17] = "0123456789ABCDEF";

static void Utilities::setClock(Clock clock)
{
	_gameClock = clock;
}

static void Utilities::setKeypadManager(KeypadManager keypadManager)
{
	_keypadManager = keypadManager;
}

static void Utilities::resetRandomSeed()
{
	_seed = 7; /* Seed value */
}

