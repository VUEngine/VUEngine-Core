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

#include <Printing.h>

#include "Rotation.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static inline void Rotation::print(Rotation rotation, int32 x, int32 y)
{
	PRINT_TEXT("x:    ", x, y);
	PRINT_TEXT("y:    ", x, y + 1);
	PRINT_TEXT("z:    ", x, y + 2);

	PRINT_FLOAT(__FIXED_TO_F(rotation.x), x + 2, y);
	PRINT_FLOAT(__FIXED_TO_F(rotation.y), x + 2, y + 1);
	PRINT_FLOAT(__FIXED_TO_F(rotation.z), x + 2, y + 2);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

