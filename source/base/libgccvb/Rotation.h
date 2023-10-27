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

#include <Object.h>
#include <Camera.h>
#include <Math.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

extern const Rotation* _cameraRotation __INITIALIZED_GLOBAL_DATA_SECTION_ATTRIBUTE;

/// @ingroup base-libgccvb
static class Rotation : Object
{
	/// @publicsection
	static inline Rotation zero();
	static inline Rotation invert(Rotation rotation);
	static inline Rotation clamp(fixed_ext_t x, fixed_ext_t y, fixed_ext_t z);
	static inline fixed_t getShortestDifferce(fixed_t angleFrom, fixed_t angleTo);
	static inline Rotation sum(Rotation a, Rotation b);
	static inline Rotation sub(Rotation a, Rotation b);
	static inline Rotation intermediate(Rotation a, Rotation b);
	static inline Rotation scalarProduct(Rotation rotation, int16 scalar);
	static inline Rotation scalarDivision(Rotation rotation, int16 scalar);
	static inline Rotation getRelativeToCamera(Rotation rotation);
	static inline Rotation getFromPixelRotation(PixelRotation pixelRotation);
	static inline bool areEqual(Rotation a, Rotation b);
	static void print(Rotation rotation, int32 x, int32 y);
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

static inline Rotation Rotation::clamp(fixed_ext_t x, fixed_ext_t y, fixed_ext_t z)
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

	return (Rotation){__FIXED_EXT_TO_FIXED(x), __FIXED_EXT_TO_FIXED(y), __FIXED_EXT_TO_FIXED(z)};
}

static inline fixed_t Rotation::getShortestDifferce(fixed_t angleFrom, fixed_t angleTo)
{
	int32 rotationDifference = (__FIXED_TO_I(angleTo) - __FIXED_TO_I(angleFrom) + 256 ) % 512 - 256;
	return __I_TO_FIXED(-256 > rotationDifference ? rotationDifference + 512 : rotationDifference);
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
	return Rotation::clamp(__FIXED_EXT_MULT(rotation.x, scalar), __FIXED_EXT_MULT(rotation.y, scalar), __FIXED_EXT_MULT(rotation.z, scalar));
}

static inline Rotation Rotation::scalarDivision(Rotation rotation, int16 scalar)
{
	if(0 != scalar)
	{
		return Rotation::clamp(__FIXED_EXT_DIV(rotation.x, scalar), __FIXED_EXT_DIV(rotation.y, scalar), __FIXED_EXT_DIV(rotation.z, scalar));
	}

	return Rotation::zero();
}

static inline Rotation Rotation::getRelativeToCamera(Rotation rotation)
{
	return Rotation::clamp(rotation.x - _cameraRotation->x, rotation.y - _cameraRotation->y, rotation.z - _cameraRotation->z);
}

static inline Rotation Rotation::getFromPixelRotation(PixelRotation pixelRotation)
{
	return Rotation::clamp(__I_TO_FIXED_EXT(pixelRotation.x), __I_TO_FIXED_EXT(pixelRotation.y), __I_TO_FIXED_EXT(pixelRotation.z));
}

static inline bool Rotation::areEqual(Rotation a, Rotation b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}


#endif
