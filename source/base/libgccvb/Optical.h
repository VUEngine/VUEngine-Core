/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef OPTICAL_H_
#define OPTICAL_H_

//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Math.h>
#include <MiscStructs.h>
#include <Constants.h>
#include <Camera.h>


//---------------------------------------------------------------------------------------------------------
//											PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static class Optical : Object
{
	static inline Optical getFromPixelOptical(PixelOptical pixelOptical, CameraFrustum cameraFrustum);
}

//---------------------------------------------------------------------------------------------------------
//											IMPLEMENTATIONS
//---------------------------------------------------------------------------------------------------------

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

	optical.maximumXViewDistancePower = __MINIMUM_X_VIEW_DISTANCE_POWER > maximumXViewDistancePower ? __MINIMUM_X_VIEW_DISTANCE_POWER : maximumXViewDistancePower;
	optical.maximumYViewDistancePower = __MINIMUM_Y_VIEW_DISTANCE_POWER > maximumYViewDistancePower ? __MINIMUM_Y_VIEW_DISTANCE_POWER : maximumYViewDistancePower;
	optical.baseDistance = __PIXELS_TO_METERS(pixelOptical.baseDistance);
	optical.horizontalViewPointCenter = __PIXELS_TO_METERS(pixelOptical.horizontalViewPointCenter);
	optical.verticalViewPointCenter = __PIXELS_TO_METERS(pixelOptical.verticalViewPointCenter);
	optical.scalingFactor = __F_TO_FIX10_6(pixelOptical.scalingFactor);
	optical.halfWidth = __PIXELS_TO_METERS((cameraFrustum.x1 - cameraFrustum.x0) >> 1);
	optical.halfHeight = __PIXELS_TO_METERS((cameraFrustum.y1 - cameraFrustum.y0) >> 1);
	optical.aspectRatio = __FIX10_6_EXT_DIV(optical.halfWidth, optical.halfHeight);
	// this assumes and fov of 90 degrees (128 fix7_9) to speed up computations
	// fov = 1 / tan(angle / 2)
	optical.fov = __FIX10_6_EXT_DIV(__I_TO_FIX10_6_EXT(1), __FIX7_9_TO_FIX10_6(__FIX7_9_DIV(__SIN(__CAMERA_FOV_DEGREES >> 1), __COS(__CAMERA_FOV_DEGREES >> 1))));
	optical.aspectRatioXfov = __FIX10_6_MULT(optical.aspectRatio, optical.fov);
	// farRatio1Near = // (far + near) / (far - near)
	optical.farRatio1Near = __FIX10_6_EXT_DIV(cameraFrustum.z1 + cameraFrustum.z0, cameraFrustum.z1 - cameraFrustum.z0);
	// farRatio2Near = // (2 * far * near) / (near - far)
	optical.farRatio2Near = __FIX10_6_EXT_DIV(__FIX10_6_EXT_MULT(cameraFrustum.z1, cameraFrustum.z0) << 1, cameraFrustum.z0 - cameraFrustum.z1);
	optical.scalingReferenceCoordinate = __FIX10_6_EXT_DIV(__FIX10_6_EXT_MULT(optical.halfWidth << 1, optical.halfHeight), optical.halfWidth << 1);

	return optical;
}

static inline Optical Optical::updateWithCameraFrustum(Optical optical, CameraFrustum cameraFrustum)
{
	Optical result = optical;

	result.halfWidth = __PIXELS_TO_METERS((cameraFrustum.x1 - cameraFrustum.x0) >> 1);
	result.halfHeight = __PIXELS_TO_METERS((cameraFrustum.y1 - cameraFrustum.y0) >> 1);
	result.aspectRatio = __FIX10_6_EXT_DIV(result.halfWidth, result.halfHeight);
	// this assumes and fov of 90 degrees (__CAMERA_FOV_DEGREES fix7_9) to speed up computations
	// fov = 1 / tan(angle / 2)
	result.fov = __FIX10_6_EXT_DIV(__I_TO_FIX10_6_EXT(1), __FIX7_9_TO_FIX10_6(__FIX7_9_DIV(__SIN(__CAMERA_FOV_DEGREES >> 1), __COS(__CAMERA_FOV_DEGREES >> 1))));
	result.aspectRatioXfov = __FIX10_6_MULT(result.aspectRatio, result.fov);
	// farRatio1Near = // (far + near) / (far - near)
	result.farRatio1Near = __FIX10_6_EXT_DIV(cameraFrustum.z1 + cameraFrustum.z0, cameraFrustum.z1 - cameraFrustum.z0);
	// farRatio2Near = // (2 * far * near) / (near - far)
	result.farRatio2Near = __FIX10_6_EXT_DIV(__FIX10_6_EXT_MULT(cameraFrustum.z1, cameraFrustum.z0) << 1, cameraFrustum.z0 - cameraFrustum.z1);
	result.scalingReferenceCoordinate = __FIX10_6_EXT_DIV(__FIX10_6_EXT_MULT(result.halfWidth << 1, result.halfHeight), optical.halfWidth << 1);
	
	return result;
}

#endif
