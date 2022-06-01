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
static class Rotation : Object
{
	/// @publicsection
	static inline Rotation zero();
	static inline Rotation clamp(Rotation rotation);
	static inline Rotation sum(Rotation a, Rotation b);
	static inline Rotation sub(Rotation a, Rotation b);
	static inline Rotation intermediate(Rotation a, Rotation b);
	static inline Rotation scalarProduct(Rotation rotation, int16 scalar);
	static inline Rotation scalarDivision(Rotation rotation, int16 scalar);
	static inline Rotation getRelativeToCamera(Rotation rotation);
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

static inline Rotation Rotation::clamp(Rotation rotation)
{
	rotation.x = __MODULO(rotation.x, 512);
	rotation.y = __MODULO(rotation.y, 512);
	rotation.z = __MODULO(rotation.z, 512);

	if(0 > rotation.x)
	{
		rotation.x += 512;
	}

	if(0 > rotation.y)
	{
		rotation.y += 512;
	}

	if(0 > rotation.z)
	{
		rotation.z += 512;
	}

	return rotation;
}

static inline Rotation Rotation::sum(Rotation a, Rotation b)
{
	return Rotation::clamp((Rotation){a.x + b.x, a.y + b.y, a.z + b.z});
}

static inline Rotation Rotation::sub(Rotation a, Rotation b)
{
	return Rotation::clamp((Rotation){a.x - b.x, a.y - b.y, a.z - b.z});
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
	return Rotation::clamp((Rotation){rotation.x * scalar, rotation.y * scalar, rotation.z * scalar});
}

static inline Rotation Rotation::scalarDivision(Rotation rotation, int16 scalar)
{
	if(0 != scalar)
	{
		return Rotation::clamp((Rotation){rotation.x / scalar, rotation.y / scalar, rotation.z / scalar});
	}

	return Rotation::zero();
}

static inline Rotation Rotation::getRelativeToCamera(Rotation rotation)
{
	extern const Rotation* _cameraRotation;

	return Rotation::clamp(Rotation::sub(rotation, *_cameraRotation));
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

	PRINT_INT(rotation.x, x + 2, y);
	PRINT_INT(rotation.y, x + 2, y + 1);
	PRINT_INT(rotation.z, x + 2, y + 2);
}


#endif
