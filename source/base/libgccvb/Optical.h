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


//---------------------------------------------------------------------------------------------------------
//											PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static class Optical : Object
{
	static inline Optical getFromPixelOptical(PixelOptical pixelOptical);
}

//---------------------------------------------------------------------------------------------------------
//											IMPLEMENTATIONS
//---------------------------------------------------------------------------------------------------------

static inline Optical Optical::getFromPixelOptical(PixelOptical pixelOptical)
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

	return (Optical)
	{
		__MINIMUM_X_VIEW_DISTANCE_POWER > maximumXViewDistancePower ? __MINIMUM_X_VIEW_DISTANCE_POWER : maximumXViewDistancePower,
		__MINIMUM_Y_VIEW_DISTANCE_POWER > maximumYViewDistancePower ? __MINIMUM_Y_VIEW_DISTANCE_POWER : maximumYViewDistancePower,
		__PIXELS_TO_METERS(pixelOptical.distanceEyeScreen),
		__PIXELS_TO_METERS(pixelOptical.baseDistance),
		__PIXELS_TO_METERS(pixelOptical.horizontalViewPointCenter),
		__PIXELS_TO_METERS(pixelOptical.verticalViewPointCenter),
		__F_TO_FIX10_6(pixelOptical.scalingFactor) << maximumXViewDistancePower
	};
}

#endif
