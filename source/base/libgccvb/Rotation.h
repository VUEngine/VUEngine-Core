/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef ROTATION_H_
#define ROTATION_H_

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
static class Rotation : ListenerObject
{
	/// @publicsection
	static inline Rotation zero();
	static inline Rotation invert(Rotation rotation);
	static inline Rotation clamp(fix10_6_ext x, fix10_6_ext y, fix10_6_ext z);
	static inline fix10_6 getShortestDifferce(fix10_6 angleFrom, fix10_6 angleTo);
	static inline Rotation sum(Rotation a, Rotation b);
	static inline Rotation sub(Rotation a, Rotation b);
	static inline Rotation intermediate(Rotation a, Rotation b);
	static inline Rotation scalarProduct(Rotation rotation, int16 scalar);
	static inline Rotation scalarDivision(Rotation rotation, int16 scalar);
	static inline Rotation getRelativeToCamera(Rotation rotation);
	static inline Rotation getFromPixelRotation(PixelRotation pixelRotation);
	static inline bool areEqual(Rotation a, Rotation b);
	static inline void print(Rotation rotation, int32 x, int32 y);
}

//---------------------------------------------------------------------------------------------------------
//											IMPLEMENTATIONS
//---------------------------------------------------------------------------------------------------------

static inline Rotation Rotation::zero()
{
	return (Rotation){0, 0, 0};
}

static inline Rotation Rotation::invert(Rotation rotation)
{
	return Rotation::clamp(__FULL_ROTATION_DEGREES - rotation.x, __FULL_ROTATION_DEGREES - rotation.y, __FULL_ROTATION_DEGREES - rotation.z);
}

static inline Rotation Rotation::clamp(fix10_6_ext x, fix10_6_ext y, fix10_6_ext z)
{
	if(0 > x)
	{
		x += __FULL_ROTATION_DEGREES;
	}
	else if(__FULL_ROTATION_DEGREES <= x)
	{
		x -= __FULL_ROTATION_DEGREES;
	}

	if(0 > y)
	{
		y += __FULL_ROTATION_DEGREES;
	}
	else if(__FULL_ROTATION_DEGREES <= y)
	{
		y -= __FULL_ROTATION_DEGREES;
	}

	if(0 > z)
	{
		z += __FULL_ROTATION_DEGREES;
	}
	else if(__FULL_ROTATION_DEGREES <= z)
	{
		z -= __FULL_ROTATION_DEGREES;
	}

	return (Rotation){__FIX10_6_EXT_TO_FIX10_6(x), __FIX10_6_EXT_TO_FIX10_6(y), __FIX10_6_EXT_TO_FIX10_6(z)};
}

static inline fix10_6 Rotation::getShortestDifferce(fix10_6 angleFrom, fix10_6 angleTo)
{
	int32 rotationDifference = (__FIX10_6_TO_I(angleTo) - __FIX10_6_TO_I(angleFrom) + 256 ) % 512 - 256;
	return __I_TO_FIX10_6(-256 > rotationDifference ? rotationDifference + 512 : rotationDifference);
}

static inline Rotation Rotation::sum(Rotation a, Rotation b)
{
	return Rotation::clamp(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Rotation Rotation::sub(Rotation a, Rotation b)
{
	return Rotation::clamp(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Rotation Rotation::intermediate(Rotation a, Rotation b)
{
	return (Rotation)
	{
		(a.x + b.x) >> 1,
		(a.y + b.y) >> 1,
		(a.z + b.z) >> 1
	};
}

static inline Rotation Rotation::scalarProduct(Rotation rotation, int16 scalar)
{
	return Rotation::clamp(__FIX10_6_EXT_MULT(rotation.x, scalar), __FIX10_6_EXT_MULT(rotation.y, scalar), __FIX10_6_EXT_MULT(rotation.z, scalar));
}

static inline Rotation Rotation::scalarDivision(Rotation rotation, int16 scalar)
{
	if(0 != scalar)
	{
		return Rotation::clamp(__FIX10_6_EXT_DIV(rotation.x, scalar), __FIX10_6_EXT_DIV(rotation.y, scalar), __FIX10_6_EXT_DIV(rotation.z, scalar));
	}

	return Rotation::zero();
}

static inline Rotation Rotation::getRelativeToCamera(Rotation rotation)
{
	extern const Rotation* _cameraRotation;

	return Rotation::clamp(rotation.x - _cameraRotation->x, rotation.y - _cameraRotation->y, rotation.z - _cameraRotation->z);
}

static inline Rotation Rotation::getFromPixelRotation(PixelRotation pixelRotation)
{
	return Rotation::clamp(__I_TO_FIX10_6_EXT(pixelRotation.x), __I_TO_FIX10_6_EXT(pixelRotation.y), __I_TO_FIX10_6_EXT(pixelRotation.z));
}

static inline bool Rotation::areEqual(Rotation a, Rotation b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

static inline void Rotation::print(Rotation rotation, int32 x, int32 y)
{
	PRINT_TEXT("x:    ", x, y);
	PRINT_TEXT("y:    ", x, y + 1);
	PRINT_TEXT("z:    ", x, y + 2);

	PRINT_FLOAT(__FIX10_6_TO_F(rotation.x), x + 2, y);
	PRINT_FLOAT(__FIX10_6_TO_F(rotation.y), x + 2, y + 1);
	PRINT_FLOAT(__FIX10_6_TO_F(rotation.z), x + 2, y + 2);
}


#endif
