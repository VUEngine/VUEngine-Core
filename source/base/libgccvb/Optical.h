/* VUEngine - Virtual Utopia Engine <https://www.vuengine.dev>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>, 2007-2020
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
	int16 maximumXViewDistancePower = 0;
	int16 maximumYViewDistancePower = 0;

	pixelOptical.maximumXViewDistance >>= __PIXELS_PER_METER_2_POWER;
	pixelOptical.maximumYViewDistance >>= __PIXELS_PER_METER_2_POWER;

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
		__MINIMUM_X_VIEW_DISTANCE_POWER <= maximumXViewDistancePower ? maximumXViewDistancePower - 1 : __MINIMUM_X_VIEW_DISTANCE_POWER,
		__MINIMUM_Y_VIEW_DISTANCE_POWER <= maximumYViewDistancePower ? maximumYViewDistancePower - 1 : __MINIMUM_Y_VIEW_DISTANCE_POWER,
		__PIXELS_TO_METERS(pixelOptical.distanceEyeScreen),
		__PIXELS_TO_METERS(pixelOptical.baseDistance),
		__PIXELS_TO_METERS(pixelOptical.horizontalViewPointCenter),
		__PIXELS_TO_METERS(pixelOptical.verticalViewPointCenter),
		__F_TO_FIX10_6(pixelOptical.scalingFactor) << maximumXViewDistancePower
	};
}

#endif
