/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OPTICAL_H_
#define OPTICAL_H_

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Object.h>
#include <Camera.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/// Class Optical
///
/// Inherits from Object
///
/// Computes the values that determine the results from 3D to 2D projection.
static class Optical : Object
{
	/// @publicsection

	/// Converts the provided optical parameters in pixel units to optical values in meters.
	/// @param pixelOptical: Struct that holds optical parameters in pixel units
	/// @param cameraFrustum: Camera's frustum configuration parameters
	/// @return Optical configuration parameters in meters
	static inline Optical getFromPixelOptical(PixelOptical pixelOptical, CameraFrustum cameraFrustum);

	/// Applies the provided camera frustum configuration to the provided optical configuration parameters.
	/// @param optical: Struct that holds optical parameters in meters
	/// @param cameraFrustum: Camera's frustum configuration parameters
	/// @return Optical configuration parameters in meters
	static inline Optical applyCameraFrustum(Optical optical, CameraFrustum cameraFrustum);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Optical Optical::getFromPixelOptical(PixelOptical pixelOptical, CameraFrustum cameraFrustum)
{
	int16 maximumXViewDistancePower = -__PIXELS_PER_METER_2_POWER;
	int16 maximumYViewDistancePower = -__PIXELS_PER_METER_2_POWER;

	while(pixelOptical.maximumXViewDistance)
	{
		pixelOptical.maximumXViewDistance >>= 1;
		maximumXViewDistancePower++;
	}

	while(pixelOptical.maximumYViewDistance)
	{
		pixelOptical.maximumYViewDistance >>= 1;
		maximumYViewDistancePower++;
	}

	Optical optical;

	optical.maximumXViewDistancePower = 
		__MINIMUM_X_VIEW_DISTANCE_POWER > maximumXViewDistancePower ? __MINIMUM_X_VIEW_DISTANCE_POWER : maximumXViewDistancePower;
	optical.maximumYViewDistancePower = 
		__MINIMUM_Y_VIEW_DISTANCE_POWER > maximumYViewDistancePower ? __MINIMUM_Y_VIEW_DISTANCE_POWER : maximumYViewDistancePower;
	optical.cameraNearPlane = __PIXELS_TO_METERS(pixelOptical.cameraNearPlane);
	optical.baseDistance = __PIXELS_TO_METERS(pixelOptical.baseDistance);
	optical.horizontalViewPointCenter = __PIXELS_TO_METERS(pixelOptical.horizontalViewPointCenter);
	optical.verticalViewPointCenter = __PIXELS_TO_METERS(pixelOptical.verticalViewPointCenter);
	optical.scalingFactor = __F_TO_FIXED(pixelOptical.scalingFactor);

	return Optical::applyCameraFrustum(optical, cameraFrustum);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static inline Optical Optical::applyCameraFrustum(Optical optical, CameraFrustum cameraFrustum)
{
	Optical result = optical;

	result.halfWidth = __PIXELS_TO_METERS((cameraFrustum.x1 - cameraFrustum.x0) >> 1);
	result.halfHeight = __PIXELS_TO_METERS((cameraFrustum.y1 - cameraFrustum.y0) >> 1);
	result.aspectRatio = __FIXED_EXT_DIV(result.halfWidth, result.halfHeight);
	// this assumes a fov of 90 degrees (__CAMERA_FOV_DEGREES fix7_9) to speed up computations
	// fov = 1 / tan(angle / 2)
	result.fov = 
		__FIXED_EXT_DIV
		(
			__I_TO_FIXED_EXT(1), 
			__FIX7_9_TO_FIXED(__FIX7_9_DIV(__SIN(__CAMERA_FOV_DEGREES >> 1), __COS(__CAMERA_FOV_DEGREES >> 1)))
		);
	result.aspectRatioXfov = __FIXED_MULT(result.aspectRatio, result.fov);
	// farRatio1Near = // (far + near) / (far - near)
	result.farRatio1Near = __FIXED_EXT_DIV(cameraFrustum.z1 + cameraFrustum.z0, cameraFrustum.z1 - cameraFrustum.z0);
	// farRatio2Near = // (2 * far * near) / (near - far)
	result.farRatio2Near = __FIXED_EXT_DIV(__FIXED_EXT_MULT(cameraFrustum.z1, cameraFrustum.z0) << 1, cameraFrustum.z0 - cameraFrustum.z1);
	result.projectionMultiplierHelper = __FIXED_EXT_MULT(result.halfWidth, result.aspectRatioXfov);
	result.scalingMultiplier = __FIXED_EXT_MULT(result.halfWidth, result.scalingFactor);

	if(0 == result.cameraNearPlane)
	{
		result.cameraNearPlane = result.projectionMultiplierHelper;
	}

	return result;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif
