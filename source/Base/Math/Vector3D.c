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

#include "Vector3D.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static inline void Vector3D::print(Vector3D vector, int32 x, int32 y)
{
	PRINT_TEXT("x:    ", x, y);
	PRINT_TEXT("y:    ", x, y + 1);
	PRINT_TEXT("z:    ", x, y + 2);

	PRINT_FLOAT(__FIXED_TO_F(vector.x), x + 2, y);
	PRINT_FLOAT(__FIXED_TO_F(vector.y), x + 2, y + 1);
	PRINT_FLOAT(__FIXED_TO_F(vector.z), x + 2, y + 2);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
static inline void Vector3D::printRaw(Vector3D vector, int32 x, int32 y)
{
	PRINT_TEXT("x:    ", x, y);
	PRINT_TEXT("y:    ", x, y + 1);
	PRINT_TEXT("z:    ", x, y + 2);

	PRINT_INT((vector.x), x + 2, y);
	PRINT_INT((vector.y), x + 2, y + 1);
	PRINT_INT((vector.z), x + 2, y + 2);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

