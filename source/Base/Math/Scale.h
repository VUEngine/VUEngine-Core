/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef SCALE_H_
#define SCALE_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>
#include <Math.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class Scale
///
/// Inherits from Object
///
/// Implements methods to operate on Scale structs.
static class Scale : Object
{
	/// @publicsection

	/// Get a scale with all its members initialized to zero.
	/// @return Scale with all its members initialized to zero
	static inline Scale zero();

	/// Get a unit scale with all its members are initialized to 1.
	/// @return Unit scale
	static inline Scale unit();

	/// Compute the addition scale between other two provided scales.
	/// @param a: First scale
	/// @param b: Second scale
	/// @return Addition scale between the provided scales
	static inline Scale sum(Scale a, Scale b);

	/// Compute the difference scale between two provided scales.
	/// @param a: Minuend scale
	/// @param b: Substraend scale
	/// @return Difference scale between the provided scales
	static inline Scale sub(Scale a, Scale b);

	/// Compute the intermediate scale between two provided scales.
	/// @param a: First scale
	/// @param b: Second scale
	/// @return Intermediate scale between the provided scales
	static inline Scale intermediate(Scale a, Scale b);

	/// Compute the product scale between two provided scales.
	/// @param a: First scale
	/// @param b: Second scale
	/// @return Product scale between the provided scales
	static inline Scale product(Scale a, Scale b);

	/// Compute the division scale between two provided scales.
	/// @param a: Dividend scale
	/// @param b: Divisor scale
	/// @return Division scale between the provided scales
	static inline Scale division(Scale a, Scale b);

	/// Apply a scalar product over the scale's components
	/// @param scale: Scale to scale
	/// @param scalar: Scalar to multiply
	/// @return Scaled scale
	static inline Scale scalarProduct(Scale scale, int16 scalar);

	/// Apply a scalar product over the scale's components
	/// @param scale: Scale to scale
	/// @param scalar: Scalar divisor
	/// @return Scaled scale
	static inline Scale scalarDivision(Scale scale, int16 scalar);

	/// Transform the provided scale in screen coordinates into a normal 3D scale.
	/// @param screenPixelScale: Scale to transform
	/// @return 3D scale
	static inline Scale getFromScreenPixelScale(ScreenPixelScale screenPixelScale);

	/// Test if two scales are equal.
	/// @param a: First scale
	/// @param b: Second scale
	/// @return True if all the components of both scales are equal; false otherwise
	static inline bool areEqual(Scale a, Scale b);

	/// Print the scale's components.
	/// @param scale: Scale to print
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	static void print(Scale scale, int32 x, int32 y);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Scale Scale::zero()
{
	return (Scale){0, 0, 0};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Scale Scale::unit()
{
	return (Scale){__1I_FIX7_9, __1I_FIX7_9, __1I_FIX7_9};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Scale Scale::sum(Scale a, Scale b)
{
	return (Scale){a.x + b.x, a.y + b.y, a.z + b.z};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Scale Scale::sub(Scale a, Scale b)
{
	return (Scale){a.x - b.x, a.y - b.y, a.z - b.z};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Scale Scale::intermediate(Scale a, Scale b)
{
	return (Scale)
	{
		(a.x + b.x) >> 1,
		(a.y + b.y) >> 1,
		(a.z + b.z) >> 1
	};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Scale Scale::product(Scale a, Scale b)
{
	return (Scale){__FIX7_9_MULT(a.x, b.x), __FIX7_9_MULT(a.y, b.y), __FIX7_9_MULT(a.z, b.z)};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Scale Scale::division(Scale a, Scale b)
{
	return (Scale){0 == b.x ? 0 : __FIX7_9_DIV(a.x, b.x), 0 == b.y ? 0 : __FIX7_9_DIV(a.y, b.y), 0 == b.z ? 0 : __FIX7_9_DIV(a.z, a.z)};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Scale Scale::scalarProduct(Scale scale, fix7_9 scalar)
{
	return (Scale){__FIX7_9_MULT(scale.x, scalar), __FIX7_9_MULT(scale.y, scalar), __FIX7_9_MULT(scale.z, scalar)};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Scale Scale::scalarDivision(Scale scale, fix7_9 scalar)
{
	if(0 != scalar)
	{
		return (Scale){__FIX7_9_DIV(scale.x, scalar), __FIX7_9_DIV(scale.y, scalar), __FIX7_9_DIV(scale.z, scalar)};
	}

	return Scale::zero();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Scale Scale::getFromScreenPixelScale(ScreenPixelScale screenPixelScale)
{
	return (Scale){__F_TO_FIX7_9(screenPixelScale.x), __F_TO_FIX7_9(screenPixelScale.y), __F_TO_FIX7_9(screenPixelScale.z)};
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline bool Scale::areEqual(Scale a, Scale b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


#endif
