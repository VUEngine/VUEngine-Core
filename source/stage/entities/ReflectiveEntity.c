/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ReflectiveEntity.h>
#include <Game.h>
#include <Optics.h>
#include <Camera.h>
#include <Utilities.h>
#include <DirectDraw.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS' DEFINITION
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void ReflectiveEntity::constructor(ReflectiveEntityDefinition* reflectiveEntityDefinition, s16 id, s16 internalId, const char* const name)
{
	// construct base
	Base::constructor(&reflectiveEntityDefinition->entityDefinition, id, internalId, name);

	this->waveLutIndex = 0;
	this->waveLutIndexIncrement = __FIX10_6_MULT(reflectiveEntityDefinition->waveLutThrottleFactor, __FIX10_6_DIV(__I_TO_FIX10_6(reflectiveEntityDefinition->numberOfWaveLutEntries), __I_TO_FIX10_6(reflectiveEntityDefinition->width)));
	this->nextFramePosition2D = this->position2D = (Point){_cameraFrustum->x1 + 1, _cameraFrustum->y1 + 1};
}

// class's destructor
void ReflectiveEntity::destructor()
{
	// remove post processing effect
	Game::removePostProcessingEffect(Game::getInstance(), ReflectiveEntity::reflect, SpatialObject::safeCast(this));

	// delete the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void ReflectiveEntity::ready(bool recursive)
{
	// call base
	Base::ready(this, recursive);

	// add post processing effect to make key emit rhombuses
	Game::pushFrontProcessingEffect(Game::getInstance(), ReflectiveEntity::reflect, SpatialObject::safeCast(this));
}

void ReflectiveEntity::suspend()
{
	Base::suspend(this);

	// remove post processing effect
	Game::removePostProcessingEffect(Game::getInstance(), ReflectiveEntity::reflect, SpatialObject::safeCast(this));
}

void ReflectiveEntity::resume()
{
	Base::resume(this);

	// add post processing effect to make key emit rhombuses
	Game::pushFrontProcessingEffect(Game::getInstance(), ReflectiveEntity::reflect, SpatialObject::safeCast(this));
}

void ReflectiveEntity::synchronizeGraphics()
{
	Vector3D position3D = Vector3D::getRelativeToCamera(this->transformation.globalPosition);

	PixelVector position2D = Vector3D::projectToPixelVector(position3D, 0);

	this->position2D = this->nextFramePosition2D;
	this->nextFramePosition2D.x = position2D.x;
	this->nextFramePosition2D.y = position2D.y;
}

static void ReflectiveEntity::reflect(u32 currentDrawingFrameBufferSet, SpatialObject spatialObject)
{
	ASSERT(spatialObject, "ReflectiveEntity::reflect: null this");

	if(isDeleted(spatialObject))
	{
		return;
	}

	ReflectiveEntity this = ReflectiveEntity::safeCast(spatialObject);

	 ReflectiveEntity::applyReflection(this, currentDrawingFrameBufferSet);
}

void ReflectiveEntity::applyReflection(u32 currentDrawingFrameBufferSet)
{
	ReflectiveEntityDefinition* reflectiveEntityDefinition = (ReflectiveEntityDefinition*)this->entityDefinition;

/*
	static fix10_6 index = 0;

	const s16 displ[] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1,
		-2, -3, -4, -5, -6, -7, -8, -9, -10,
		-9, -8, -7, -6, -5, -4, -3, -2, -1,
	};

	index += this->waveLutIndexIncrement;

	if(__FIX10_6_TO_I(index) >= sizeof(displ) / sizeof(s16))
	{
		index = 0;
	}
*/

	ReflectiveEntity::drawReflection(this, currentDrawingFrameBufferSet,
								this->position2D.x + reflectiveEntityDefinition->sourceDisplacement.x,
								this->position2D.y + reflectiveEntityDefinition->sourceDisplacement.y,
								this->position2D.x + reflectiveEntityDefinition->outputDisplacement.x,
								this->position2D.y + reflectiveEntityDefinition->outputDisplacement.y,
								reflectiveEntityDefinition->width,
								reflectiveEntityDefinition->height,
								reflectiveEntityDefinition->overallMask,
								reflectiveEntityDefinition->reflectionMask,
								reflectiveEntityDefinition->backgroundMask,
								reflectiveEntityDefinition->axisForReversing,
								reflectiveEntityDefinition->transparent,
								reflectiveEntityDefinition->reflectParallax,
								reflectiveEntityDefinition->parallaxDisplacement,
								reflectiveEntityDefinition->waveLut,
								reflectiveEntityDefinition->numberOfWaveLutEntries,
								reflectiveEntityDefinition->waveLutThrottleFactor,
								reflectiveEntityDefinition->flattenTop, reflectiveEntityDefinition->flattenBottom,
								reflectiveEntityDefinition->topBorder, reflectiveEntityDefinition->bottomBorder,
								reflectiveEntityDefinition->leftBorder, reflectiveEntityDefinition->rightBorder);
}

static void ReflectiveEntity::shiftPixels(int pixelShift, POINTER_TYPE* sourceValue, u32 nextSourceValue, POINTER_TYPE* remainderValue, u32 overallMask, u32 reflectionMask)
{
	*sourceValue &= reflectionMask;
	*remainderValue &= reflectionMask;

	*sourceValue |= overallMask;
	*remainderValue |= overallMask;

	if(0 < pixelShift)
	{
		POINTER_TYPE remainderValueTemp = *remainderValue;
		*remainderValue = (*sourceValue >> (BITS_PER_STEP - pixelShift));
		*sourceValue <<= pixelShift;
		*sourceValue |= remainderValueTemp;
	}
	else if(0 > pixelShift)
	{
		*sourceValue >>= -pixelShift;
		*sourceValue |= (nextSourceValue << (BITS_PER_STEP + pixelShift));
		*remainderValue = nextSourceValue >> (-pixelShift);
	}
}

void ReflectiveEntity::drawReflection(u32 currentDrawingFrameBufferSet,
								s16 xSourceStart, s16 ySourceStart,
								s16 xOutputStart, s16 yOutputStart,
								s16 width, s16 height,
								u32 overallMask, u32 reflectionMask, u32 backgroundMask,
								u16 axisForReversing, bool transparent, bool reflectParallax,
								s16 parallaxDisplacement,
								const u8 waveLut[], int numberOfWaveLutEntries, fix10_6 waveLutThrottleFactor,
								bool flattenTop __attribute__ ((unused)), bool flattenBottom,
								u32 topBorderMask,
								u32 bottomBorderMask,
								u32 leftBorderMask __attribute__ ((unused)),
								u32 rightBorderMask __attribute__ ((unused)))
{
    s16 xSourceEnd = xSourceStart + width;
    s16 ySourceEnd = ySourceStart + height;
	s16 xOutputEnd = xOutputStart + width;
	s16 yOutputEnd = yOutputStart + height;
/*
    s16 xSourceStartTemp = xSourceStart;
    s16 ySourceStartTemp = ySourceStart;
	s16 xOutputStartTemp = xOutputStart;
	s16 yOutputStartTemp = yOutputStart;

    s16 xSourceEndTemp = xSourceEnd;
    s16 ySourceEndTemp = ySourceEnd;
	s16 xOutputEndTemp = xOutputEnd;
	s16 yOutputEndTemp = yOutputEnd;
*/
	// check if source and destination are not out of bounds
	if((xSourceStart > _cameraFrustum->x1) | (ySourceStart > _cameraFrustum->y1)
		|
		(xSourceEnd < _cameraFrustum->x0) | (ySourceEnd < _cameraFrustum->y0)
		|
		(xOutputStart > _cameraFrustum->x1) | (yOutputStart > _cameraFrustum->y1)
		|
		(xOutputStart > _cameraFrustum->x1) | (yOutputStart > _cameraFrustum->y1))
	{
		return;
	}

	fix10_6 fixedNumberOfWaveLutEntries = __FIX10_6_MULT(waveLutThrottleFactor, __I_TO_FIX10_6(numberOfWaveLutEntries));

	u32 transparentMask = transparent ? 0xFFFFFFFF : 0;

	int xClamping = 0;
	int xOutputStartSave = xOutputStart;

	// clamp values to not write out of the camera
	if(xSourceStart < _cameraFrustum->x0)
	{
		xClamping = _cameraFrustum->x0 - xSourceStart;
		xOutputStart += xClamping;
		xSourceStart = _cameraFrustum->x0;
	}

	if(xSourceEnd > _cameraFrustum->x1 - 1)
	{
		xClamping = xSourceEnd - _cameraFrustum->x1 + 1;
		xOutputEnd -= xClamping;
		xSourceEnd = _cameraFrustum->x1 - 1;
	}

	if(ySourceStart < _cameraFrustum->y0)
	{
		yOutputStart += _cameraFrustum->y0 - ySourceStart;
		ySourceStart = _cameraFrustum->y0;
	}

	if(ySourceEnd > _cameraFrustum->y1)
	{
		yOutputEnd -= ySourceEnd - _cameraFrustum->y1;
		ySourceEnd = _cameraFrustum->y1;
	}

	// must clamp the output too, but moving the wave lut index accordingly
	if(xOutputStart < _cameraFrustum->x0)
	{
		xClamping = _cameraFrustum->x0 - xOutputStart;
		xSourceStart += xClamping;
		xOutputStart = _cameraFrustum->x0;
	}

	if(xOutputEnd > _cameraFrustum->x1)
	{
		xClamping = xOutputEnd - _cameraFrustum->x1 + 1;

		if(__X_AXIS & axisForReversing)
		{
			xOutputStart += xClamping;
		}
		else
		{
			xSourceEnd -= xClamping;
		}

		xOutputEnd = _cameraFrustum->x1 - 1;
	}

	// must clamp the output too, but moving the wave lut index accordingly
	if(yOutputStart < _cameraFrustum->y0)
	{
		if(__Y_AXIS & axisForReversing)
		{
			ySourceEnd -= ((_cameraFrustum->y0 - yOutputStart) - Y_STEP_SIZE);
		}
		else
		{
			ySourceStart += (_cameraFrustum->y0 - yOutputStart);
		}

		yOutputStart = _cameraFrustum->y0;
	}

	if(yOutputEnd > _cameraFrustum->y1)
	{
		if(__Y_AXIS & axisForReversing)
		{
			ySourceStart += (yOutputEnd - _cameraFrustum->y1);
		}

		yOutputEnd = _cameraFrustum->y1;
	}

	int xSource = xSourceStart;
	int xOutput = xOutputStart;
	int xOutputLimit = xOutputEnd;
	int xOutputIncrement = 1;

	if(__X_AXIS & axisForReversing)
	{
		xOutput = xOutputEnd;
		xOutputLimit = xOutputStart;
		xOutputIncrement = -1;
	}

	u32 reflectionMaskSave = reflectionMask;

	u8 dummyWaveLut[] =
	{
		0
	};

	fix10_6 waveLutIndexIncrement = this->waveLutIndexIncrement;

	if(!waveLut)
	{
		waveLutIndexIncrement = 0;
		this->waveLutIndex = 0;
		waveLut = dummyWaveLut;
		numberOfWaveLutEntries = 1;
	}

	int ySourceIncrement = 1;

	if(__Y_AXIS & axisForReversing)
	{
		s16 temp = ySourceEnd - Y_STEP_SIZE;
		ySourceEnd = ySourceStart;
		ySourceStart = temp;
		ySourceIncrement = -1;
	}

	u32 appliedBackgroundMask = transparentMask & backgroundMask;

    int ySourceStartHelper = ySourceStart >> Y_STEP_SIZE_2_EXP;

	int xSourceDistance = __ABS(xSourceEnd - xSourceStart);
	int xOutputDistance = __ABS(xOutput - xOutputLimit);
	int xTotal = xOutputDistance > xSourceDistance ? xSourceDistance : xOutputDistance;

	this->waveLutIndex += waveLutIndexIncrement;

	if(0 > this->waveLutIndex)
	{
		this->waveLutIndex = fixedNumberOfWaveLutEntries - this->waveLutIndex;
	}
	else if(this->waveLutIndex >= fixedNumberOfWaveLutEntries)
	{
		this->waveLutIndex = 0;
	}

	int xCounter = xOutputStart - xOutputStartSave;

	if(reflectParallax)
	{
		for(; xTotal--; xOutput += xOutputIncrement, xSource++, xCounter++)
		{
			this->waveLutIndex += waveLutIndexIncrement;

			if(this->waveLutIndex >= fixedNumberOfWaveLutEntries)
			{
				this->waveLutIndex = 0;
			}

			int leftColumn = xOutput;
			int rightColumn = xOutput;

			if(parallaxDisplacement)
			{
//				leftColumn -= parallaxDisplacement;
				rightColumn += parallaxDisplacement;

				if((unsigned)(leftColumn - _cameraFrustum->x0) >= (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
				{
					continue;
				}

				if((unsigned)(rightColumn - _cameraFrustum->x0) >= (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
				{
					continue;
				}
			}

			int xRelativeCoordinate = (xCounter % width) + __FIX10_6_TO_I(this->waveLutIndex);
			int xIndex = (numberOfWaveLutEntries * xRelativeCoordinate) / width;

			if(xIndex >= numberOfWaveLutEntries)
			{
				xIndex = xIndex % numberOfWaveLutEntries;
			}

			int waveLutPixelDisplacement = waveLut[xIndex];

			int ySource = ySourceStartHelper;
			int yOutput = (yOutputStart + waveLutPixelDisplacement) >> Y_STEP_SIZE_2_EXP;

			int pixelShift = (__MODULO((yOutputStart + waveLutPixelDisplacement), Y_STEP_SIZE) - __MODULO(ySourceStart, Y_STEP_SIZE)) << 1;

			reflectionMask = reflectionMaskSave;

			if(transparent && waveLutPixelDisplacement)
			{
				if(0 < pixelShift)
				{
					reflectionMask <<= pixelShift % waveLutPixelDisplacement;
				}
				else
				{
					reflectionMask >>= -pixelShift % waveLutPixelDisplacement;
				}
			}

			u32 effectiveContentMaskDisplacement = (__MODULO((yOutputStart + (flattenTop? 0 : waveLutPixelDisplacement)), Y_STEP_SIZE) << 1);
			u32 effectiveContentMask = 0xFFFFFFFF << effectiveContentMaskDisplacement;
			u32 effectiveBackgroundMask = ~effectiveContentMask;
			effectiveContentMask &= ~(topBorderMask << effectiveContentMaskDisplacement);

			POINTER_TYPE* columnSourcePointerLeft = (POINTER_TYPE*) (currentDrawingFrameBufferSet) + (xSource << Y_SHIFT) + ySource;
			POINTER_TYPE* columnSourcePointerRight = (POINTER_TYPE*) (currentDrawingFrameBufferSet | 0x00010000) + (xSource << Y_SHIFT) + ySource;
			POINTER_TYPE* columnOutputPointerLeft = (POINTER_TYPE*) (currentDrawingFrameBufferSet) + (leftColumn << Y_SHIFT) + yOutput;
			POINTER_TYPE* columnOutputPointerRight = (POINTER_TYPE*) (currentDrawingFrameBufferSet | 0x00010000) + (rightColumn << Y_SHIFT) + yOutput;

			int columnSourcePointerLeftIncrement = ySourceIncrement;
			int columnSourcePointerRightIncrement = ySourceIncrement;

			POINTER_TYPE sourceCurrentValueLeft = *columnSourcePointerLeft;
			columnSourcePointerLeft += columnSourcePointerLeftIncrement;
			POINTER_TYPE sourceNextValueLeft = *columnSourcePointerLeft;

			POINTER_TYPE sourceCurrentValueRight = *columnSourcePointerRight;
			columnSourcePointerRight += columnSourcePointerRightIncrement;

			POINTER_TYPE sourceNextValueRight = *columnSourcePointerRight;

			POINTER_TYPE outputValueLeft = *columnOutputPointerLeft;
			POINTER_TYPE outputValueRight = *columnOutputPointerRight;

			if(__Y_AXIS & axisForReversing)
			{
				sourceCurrentValueLeft = Utilities::reverse(sourceCurrentValueLeft, BITS_PER_STEP);
				sourceCurrentValueRight = Utilities::reverse(sourceCurrentValueRight, BITS_PER_STEP);
				sourceNextValueLeft = Utilities::reverse(sourceNextValueLeft, BITS_PER_STEP);
				sourceNextValueRight = Utilities::reverse(sourceNextValueRight, BITS_PER_STEP);
			}

			waveLutPixelDisplacement =  flattenBottom ? 0 : waveLutPixelDisplacement;

			int yOutputRemainder = __MODULO((yOutputEnd + waveLutPixelDisplacement), Y_STEP_SIZE) << 1;

			POINTER_TYPE remainderLeftValue = 0;
			POINTER_TYPE remainderRightValue = 0;

			int yOutputLimit = (yOutputEnd + waveLutPixelDisplacement) >> Y_STEP_SIZE_2_EXP;

			for(; yOutput < yOutputLimit; yOutput++, ySource += ySourceIncrement)
			{
				ReflectiveEntity::shiftPixels(pixelShift, &sourceCurrentValueLeft, sourceNextValueLeft, &remainderLeftValue, overallMask, reflectionMask);
				ReflectiveEntity::shiftPixels(pixelShift, &sourceCurrentValueRight, sourceNextValueRight, &remainderRightValue, overallMask, reflectionMask);

				sourceCurrentValueLeft |= appliedBackgroundMask & outputValueLeft;
				sourceCurrentValueRight |= appliedBackgroundMask & outputValueRight;
				sourceCurrentValueLeft &= effectiveContentMask;
				sourceCurrentValueRight &= effectiveContentMask;
				sourceCurrentValueLeft |= (outputValueLeft & effectiveBackgroundMask);
				sourceCurrentValueRight |= (outputValueRight & effectiveBackgroundMask);

				effectiveContentMask = 0xFFFFFFFF;
				effectiveBackgroundMask = 0;

				*columnOutputPointerLeft = sourceCurrentValueLeft;
				*columnOutputPointerRight = sourceCurrentValueRight;

				columnOutputPointerLeft++;
				columnOutputPointerRight++;

				if(transparent)
				{
					outputValueLeft = *columnOutputPointerLeft;
					outputValueRight = *columnOutputPointerRight;
				}

				sourceCurrentValueLeft = sourceNextValueLeft;
				sourceCurrentValueRight = sourceNextValueRight;

				columnSourcePointerLeft += columnSourcePointerLeftIncrement;
				columnSourcePointerRight += columnSourcePointerRightIncrement;

				sourceNextValueLeft = *columnSourcePointerLeft;
				sourceNextValueRight = *columnSourcePointerRight;

				if(__Y_AXIS & axisForReversing)
				{
					sourceNextValueLeft = Utilities::reverse(sourceNextValueLeft, BITS_PER_STEP);
					sourceNextValueRight = Utilities::reverse(sourceNextValueRight, BITS_PER_STEP);
				}
			}

			if(yOutputRemainder)
			{
				u32 maskDisplacement = (BITS_PER_STEP - yOutputRemainder);
				effectiveContentMask = 0xFFFFFFFF << maskDisplacement;
				effectiveContentMask &= ~(bottomBorderMask >> maskDisplacement);

				if(!transparent)
				{
					outputValueLeft = *columnOutputPointerLeft;
					outputValueRight = *columnOutputPointerRight;
				}

				if(0 <= pixelShift)
				{
					remainderLeftValue |= (sourceCurrentValueLeft << pixelShift);
					remainderRightValue |= (sourceCurrentValueRight << pixelShift);
				}

				remainderLeftValue &= reflectionMask;
				remainderRightValue &= reflectionMask;

				remainderLeftValue |= appliedBackgroundMask & outputValueLeft;
				remainderRightValue |= appliedBackgroundMask & outputValueRight;

				*columnOutputPointerLeft = (outputValueLeft & effectiveContentMask) | (remainderLeftValue & ~effectiveContentMask);
				*columnOutputPointerRight = (outputValueLeft & effectiveContentMask) | (remainderLeftValue & ~effectiveContentMask);
			}
		}
	}
	else
	{
		for(; xTotal--; xOutput += xOutputIncrement, xSource++, xCounter++)
		{
			int leftColumn = xOutput;
			int rightColumn = xOutput;

			if(parallaxDisplacement)
			{
//				leftColumn -= parallaxDisplacement;
				rightColumn += parallaxDisplacement;

				if((unsigned)(leftColumn - _cameraFrustum->x0) >= (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
				{
					continue;
				}

				if((unsigned)(rightColumn - _cameraFrustum->x0) >= (unsigned)(_cameraFrustum->x1 - _cameraFrustum->x0))
				{
					continue;
				}
			}

			int xRelativeCoordinate = (xCounter % width) + __FIX10_6_TO_I(this->waveLutIndex);
			int xIndex = (numberOfWaveLutEntries * xRelativeCoordinate) / width;

			if(xIndex >= numberOfWaveLutEntries)
			{
				xIndex = xIndex % numberOfWaveLutEntries;
			}

			int waveLutPixelDisplacement = waveLut[xIndex];

			int ySource = ySourceStartHelper;
			int yOutput = (yOutputStart + waveLutPixelDisplacement) >> Y_STEP_SIZE_2_EXP;

			int pixelShift = (__MODULO((yOutputStart + waveLutPixelDisplacement), Y_STEP_SIZE) - __MODULO(ySourceStart, Y_STEP_SIZE)) << 1;

			reflectionMask = reflectionMaskSave;

			if(transparent && waveLutPixelDisplacement)
			{
				if(0 < pixelShift)
				{
					reflectionMask <<= pixelShift % waveLutPixelDisplacement;
				}
				else
				{
					reflectionMask >>= -pixelShift % waveLutPixelDisplacement;
				}
			}

			u32 effectiveContentMaskDisplacement = (__MODULO((yOutputStart + (flattenTop? 0 : waveLutPixelDisplacement)), Y_STEP_SIZE) << 1);
			u32 effectiveContentMask = 0xFFFFFFFF << effectiveContentMaskDisplacement;
			u32 effectiveBackgroundMask = ~effectiveContentMask;
			effectiveContentMask &= ~(topBorderMask << effectiveContentMaskDisplacement);

			POINTER_TYPE* columnSourcePointerLeft = (POINTER_TYPE*) (currentDrawingFrameBufferSet) + (xSource << Y_SHIFT) + ySource;
			POINTER_TYPE* columnOutputPointerLeft = (POINTER_TYPE*) (currentDrawingFrameBufferSet) + (leftColumn << Y_SHIFT) + yOutput;
			POINTER_TYPE* columnOutputPointerRight = (POINTER_TYPE*) (currentDrawingFrameBufferSet | 0x00010000) + (rightColumn << Y_SHIFT) + yOutput;

			int columnSourcePointerLeftIncrement = ySourceIncrement;

			POINTER_TYPE sourceCurrentValueLeft = *columnSourcePointerLeft;
			columnSourcePointerLeft += columnSourcePointerLeftIncrement;
			POINTER_TYPE sourceNextValueLeft = *columnSourcePointerLeft;

			POINTER_TYPE outputValueLeft = *columnOutputPointerLeft;

			if(__Y_AXIS & axisForReversing)
			{
				sourceCurrentValueLeft = Utilities::reverse(sourceCurrentValueLeft, BITS_PER_STEP);
				sourceNextValueLeft = Utilities::reverse(sourceNextValueLeft, BITS_PER_STEP);
			}

			waveLutPixelDisplacement =  flattenBottom ? 0 : waveLutPixelDisplacement;

			int yOutputRemainder = __MODULO((yOutputEnd + waveLutPixelDisplacement), Y_STEP_SIZE) << 1;

			POINTER_TYPE remainderLeftValue = 0;

			int yOutputLimit = (yOutputEnd + waveLutPixelDisplacement) >> Y_STEP_SIZE_2_EXP;

			for(; yOutput < yOutputLimit; yOutput++, ySource += ySourceIncrement)
			{
				ReflectiveEntity::shiftPixels(pixelShift, &sourceCurrentValueLeft, sourceNextValueLeft, &remainderLeftValue, overallMask, reflectionMask);

				sourceCurrentValueLeft |= appliedBackgroundMask & outputValueLeft;
				sourceCurrentValueLeft &= effectiveContentMask;
				sourceCurrentValueLeft |= (outputValueLeft & effectiveBackgroundMask);

				effectiveContentMask = 0xFFFFFFFF;
				effectiveBackgroundMask = 0;

				*columnOutputPointerLeft = sourceCurrentValueLeft;
				*columnOutputPointerRight = sourceCurrentValueLeft;

				columnOutputPointerLeft++;
				columnOutputPointerRight++;

				if(transparent)
				{
					outputValueLeft = *columnOutputPointerLeft;
				}

				sourceCurrentValueLeft = sourceNextValueLeft;

				columnSourcePointerLeft += columnSourcePointerLeftIncrement;

				sourceNextValueLeft = *columnSourcePointerLeft;

				if(__Y_AXIS & axisForReversing)
				{
					sourceNextValueLeft = Utilities::reverse(sourceNextValueLeft, BITS_PER_STEP);
				}
			}

			if(yOutputRemainder)
			{
				u32 maskDisplacement = (BITS_PER_STEP - yOutputRemainder);
				effectiveContentMask = 0xFFFFFFFF << maskDisplacement;
				effectiveContentMask &= ~(bottomBorderMask >> maskDisplacement);

				if(!transparent)
				{
					outputValueLeft = *columnOutputPointerLeft;
				}

				if(0 <= pixelShift)
				{
					remainderLeftValue |= (sourceCurrentValueLeft << pixelShift);
				}

				remainderLeftValue &= reflectionMask;
				remainderLeftValue |= appliedBackgroundMask & outputValueLeft;

				*columnOutputPointerLeft = (outputValueLeft & effectiveContentMask) | (remainderLeftValue & ~effectiveContentMask);
				*columnOutputPointerRight = (outputValueLeft & effectiveContentMask) | (remainderLeftValue & ~effectiveContentMask);
			}
		}
	}
/*
	DirectDraw::drawLine(
		DirectDraw::getInstance(),
		(PixelVector) {(xOutputStartTemp),((yOutputStartTemp / Y_STEP_SIZE) * Y_STEP_SIZE),0,0},
		(PixelVector) {(xOutputEndTemp),((yOutputStartTemp / Y_STEP_SIZE) * Y_STEP_SIZE),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw::drawLine(
		DirectDraw::getInstance(),
		(PixelVector) {(xOutputStartTemp),((yOutputEndTemp / Y_STEP_SIZE) * Y_STEP_SIZE),0,0},
		(PixelVector) {(xOutputEndTemp),((yOutputEndTemp / Y_STEP_SIZE) * Y_STEP_SIZE),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw::drawLine(
		DirectDraw::getInstance(),
		(PixelVector) {(xSourceStartTemp),((ySourceStartTemp / Y_STEP_SIZE) * Y_STEP_SIZE),0,0},
		(PixelVector) {(xSourceEndTemp),((ySourceStartTemp / Y_STEP_SIZE) * Y_STEP_SIZE),0,0},
		__COLOR_DARK_RED
	);

	DirectDraw::drawLine(
		DirectDraw::getInstance(),
		(PixelVector) {(xSourceStartTemp),((ySourceEndTemp / Y_STEP_SIZE) * Y_STEP_SIZE),0,0},
		(PixelVector) {(xSourceEndTemp),((ySourceEndTemp / Y_STEP_SIZE) * Y_STEP_SIZE),0,0},
		__COLOR_DARK_RED
	);
*/
/*
	DirectDraw::drawLine(
		DirectDraw::getInstance(),
		(PixelVector) {(xOutputStartTemp),(yOutputStartTemp),0,0},
		(PixelVector) {(xOutputEndTemp),(yOutputStartTemp),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw::drawLine(
		DirectDraw::getInstance(),
		(PixelVector) {(xOutputStartTemp),(yOutputEndTemp),0,0},
		(PixelVector) {(xOutputEndTemp),(yOutputEndTemp),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw::drawLine(
		DirectDraw::getInstance(),
		(PixelVector) {(xSourceStartTemp),(ySourceStartTemp),0,0},
		(PixelVector) {(xSourceEndTemp),(ySourceStartTemp),0,0},
		__COLOR_DARK_RED
	);

	DirectDraw::drawLine(
		DirectDraw::getInstance(),
		(PixelVector) {(xSourceStartTemp),(ySourceEndTemp),0,0},
		(PixelVector) {(xSourceEndTemp),(ySourceEndTemp),0,0},
		__COLOR_DARK_RED
	);
*/
}

