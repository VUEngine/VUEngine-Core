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

#include <stdio.h>

#include <Hardware.h>

#include "Mem.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// It doesn't need to be inlined since it is not used in performance critical places
/// and it produces graphical glitches when inlined.
static void Mem::clear(uint8* destination, uint32 numberOfBYTES)
{
	uint32 i;

	for(i = 0; i < numberOfBYTES; i++)
	{
		*destination++ = 0;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
