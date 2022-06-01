/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SCALE_H_
#define SCALE_H_

//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Math.h>
#include <MiscStructs.h>
#include <Constants.h>
#include <Printing.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup base-libgccvb
static class Scale : Object
{
	/// @publicsection
	static inline Scale zero();
	static inline Scale unit();
	static inline Scale sum(Scale a, Scale b);
	static inline Scale sub(Scale a, Scale b);
	static inline Scale intermediate(Scale a, Scale b);
	static inline Scale product(Scale a, Scale b);
	static inline Scale division(Scale a, Scale b);
	static inline Scale scalarProduct(Scale scale, int16 scalar);
	static inline Scale scalarDivision(Scale scale, int16 scalar);
	static inline bool areEqual(Scale a, Scale b);
	static inline void print(Scale scale, int32 x, int32 y);
}

//---------------------------------------------------------------------------------------------------------
//											IMPLEMENTATIONS
//---------------------------------------------------------------------------------------------------------

static inline Scale Scale::zero()
{
	return (Scale){0, 0, 0};
}

static inline Scale Scale::unit()
{
	return (Scale){__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9};
}

static inline Scale Scale::sum(Scale a, Scale b)
{
	return (Scale){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Scale Scale::sub(Scale a, Scale b)
{
	return (Scale){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline Scale Scale::intermediate(Scale a, Scale b)
{
	return (Scale)
	{
		(a.x + b.x) >> 1,
		(a.y + b.y) >> 1,
		(a.z + b.z) >> 1
	};
}

static inline Scale Scale::product(Scale a, Scale b)
{
	return (Scale){__FIX7_9_MULT(a.x, b.x), __FIX7_9_MULT(a.y, b.y), __FIX7_9_MULT(a.z, a.z)};
}

static inline Scale Scale::division(Scale a, Scale b)
{
	return (Scale){0 == b.x ? 0 : __FIX7_9_DIV(a.x, b.x), 0 == b.y ? 0 : __FIX7_9_DIV(a.y, b.y), 0 == b.z ? 0 : __FIX7_9_DIV(a.z, a.z)};
}

static inline Scale Scale::scalarProduct(Scale scale, fix7_9 scalar)
{
	return (Scale){__FIX7_9_MULT(scale.x, scalar), __FIX7_9_MULT(scale.y, scalar), __FIX7_9_MULT(scale.z, scalar)};
}

static inline Scale Scale::scalarDivision(Scale scale, fix7_9 scalar)
{
	if(0 != scalar)
	{
		return (Scale){__FIX7_9_DIV(scale.x, scalar), __FIX7_9_DIV(scale.y, scalar), __FIX7_9_DIV(scale.z, scalar)};
	}

	return Scale::zero();
}

static inline bool Scale::areEqual(Scale a, Scale b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

static inline void Scale::print(Scale scale, int32 x, int32 y)
{
	PRINT_TEXT("x:    ", x, y);
	PRINT_TEXT("y:    ", x, y + 1);
	PRINT_TEXT("z:    ", x, y + 2);

	PRINT_INT(scale.x, x + 2, y);
	PRINT_INT(scale.y, x + 2, y + 1);
	PRINT_INT(scale.z, x + 2, y + 2);
}


#endif
