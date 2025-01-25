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

#include <Printer.h>

#include "PixelVector.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static inline void PixelVector::print(PixelVector vector, int32 x, int32 y)
{
	PRINT_TEXT("x:    ", x, y);
	PRINT_TEXT("y:    ", x, y + 1);
	PRINT_TEXT("z:    ", x, y + 2);
	PRINT_TEXT("p:    ", x, y + 3);

	PRINT_INT(vector.x, x + 2, y);
	PRINT_INT(vector.y, x + 2, y + 1);
	PRINT_INT(vector.z, x + 2, y + 2);
	PRINT_INT(vector.parallax, x + 2, y + 3);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
