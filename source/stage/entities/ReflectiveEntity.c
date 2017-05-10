/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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
#include <Screen.h>
#include <Utilities.h>
#include <DirectDraw.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS' DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(ReflectiveEntity, InanimatedInGameEntity);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void ReflectiveEntity_reflect(u32 currentDrawingFrameBufferSet, SpatialObject spatialObject);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ReflectiveEntity, ReflectiveEntityDefinition* reflectiveEntityDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(ReflectiveEntity, reflectiveEntityDefinition, id, internalId, name);

// class's constructor
void ReflectiveEntity_constructor(ReflectiveEntity this, ReflectiveEntityDefinition* reflectiveEntityDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "ReflectiveEntity::constructor: null this");

	// construct base
	__CONSTRUCT_BASE(InanimatedInGameEntity, &reflectiveEntityDefinition->inGameEntityDefinition, id, internalId, name);

	this->waveLutIndex = 0;
	this->waveLutIndexIncrement = FIX19_13_MULT(reflectiveEntityDefinition->waveLutThrottleFactor, FIX19_13_DIV(ITOFIX19_13(reflectiveEntityDefinition->numberOfWaveLutEntries), ITOFIX19_13(reflectiveEntityDefinition->width)));
	this->nextFramePosition2D = this->position2D = (Point){_cameraFrustum->x1 + 1, _cameraFrustum->y1 + 1};
}

// class's destructor
void ReflectiveEntity_destructor(ReflectiveEntity this)
{
	ASSERT(this, "ReflectiveEntity::destructor: null this");

	// remove post processing effect
	Game_removePostProcessingEffect(Game_getInstance(), ReflectiveEntity_reflect, __SAFE_CAST(SpatialObject, this));

	// delete the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

void ReflectiveEntity_ready(ReflectiveEntity this, bool recursive)
{
	ASSERT(this, "ReflectiveEntity::ready: null this");

	// call base
	__CALL_BASE_METHOD(InanimatedInGameEntity, ready, this, recursive);

	// add post processing effect to make key emit rhombuses
	Game_pushFrontProcessingEffect(Game_getInstance(), ReflectiveEntity_reflect, __SAFE_CAST(SpatialObject, this));
}

void ReflectiveEntity_suspend(ReflectiveEntity this)
{
	ASSERT(this, "ReflectiveEntity::suspend: null this");

	__CALL_BASE_METHOD(InanimatedInGameEntity, suspend, this);

	// remove post processing effect
	Game_removePostProcessingEffect(Game_getInstance(), ReflectiveEntity_reflect, __SAFE_CAST(SpatialObject, this));
}

void ReflectiveEntity_resume(ReflectiveEntity this)
{
	ASSERT(this, "ReflectiveEntity::resume: null this");

	__CALL_BASE_METHOD(InanimatedInGameEntity, resume, this);

	// add post processing effect to make key emit rhombuses
	Game_pushFrontProcessingEffect(Game_getInstance(), ReflectiveEntity_reflect, __SAFE_CAST(SpatialObject, this));
}

void ReflectiveEntity_transform(ReflectiveEntity this, const Transformation* environmentTransform __attribute__ ((unused)))
{
	ASSERT(this, "ReflectiveEntity::transform: null this");

	VBVec3D position3D = this->transform.globalPosition;
	__OPTICS_NORMALIZE(position3D);

	VBVec2D position2D;
	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, position2D);

	this->position2D = this->nextFramePosition2D;
	this->nextFramePosition2D.x = FIX19_13TOI(position2D.x);
	this->nextFramePosition2D.y = FIX19_13TOI(position2D.y);
}
static void ReflectiveEntity_reflect(u32 currentDrawingFrameBufferSet, SpatialObject spatialObject)
{
	ASSERT(spatialObject, "ReflectiveEntity::reflect: null this");

	if(!__IS_OBJECT_ALIVE(spatialObject))
	{
		return;
	}

	ReflectiveEntity this = __SAFE_CAST(ReflectiveEntity, spatialObject);

	__VIRTUAL_CALL(ReflectiveEntity, applyReflection, this, currentDrawingFrameBufferSet);
}

void ReflectiveEntity_applyReflection(ReflectiveEntity this, u32 currentDrawingFrameBufferSet)
{
	ASSERT(this, "ReflectiveEntity::applyReflection: null this");

	ReflectiveEntityDefinition* reflectiveEntityDefinition = (ReflectiveEntityDefinition*)this->entityDefinition;

/*
	static fix19_13 index = 0;

	const s16 displ[] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -1,
		-2, -3, -4, -5, -6, -7, -8, -9, -10,
		-9, -8, -7, -6, -5, -4, -3, -2, -1,
	};

	index += this->waveLutIndexIncrement;

	if(FIX19_13TOI(index) >= sizeof(displ) / sizeof(s16))
	{
		index = 0;
	}
*/

	ReflectiveEntity_drawReflection(this, currentDrawingFrameBufferSet,
								this->position2D.x + reflectiveEntityDefinition->sourceDisplacement.x,
								this->position2D.y + reflectiveEntityDefinition->sourceDisplacement.y,
								this->position2D.x + reflectiveEntityDefinition->outputDisplacement.x,
								this->position2D.y + reflectiveEntityDefinition->outputDisplacement.y,
								reflectiveEntityDefinition->width,
								reflectiveEntityDefinition->height,
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

inline void ReflectiveEntity_shiftPixels(int pixelShift, POINTER_TYPE* sourceValue, u32 nextSourceValue, POINTER_TYPE* remainderValue, u32 reflectionMask)
{
	*sourceValue &= reflectionMask;
	*remainderValue &= reflectionMask;

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

void ReflectiveEntity_drawReflection(ReflectiveEntity this, u32 currentDrawingFrameBufferSet,
								s16 xSourceStart, s16 ySourceStart,
								s16 xOutputStart, s16 yOutputStart,
								s16 width, s16 height,
								u32 reflectionMask, u32 backgroundMask,
								u16 axisForReversing, bool transparent, bool reflectParallax,
								s16 parallaxDisplacement,
								const u8 waveLut[], int numberOfWaveLutEntries, fix19_13 waveLutThrottleFactor,
								bool flattenTop __attribute__ ((unused)), bool flattenBottom,
								u32 topBorderMask,
								u32 bottomBorderMask,
								u32 leftBorderMask __attribute__ ((unused)),
								u32 rightBorderMask __attribute__ ((unused)))
{
	ASSERT(this, "ReflectiveEntity::drawReflection: null this");

    s16 xSourceEnd = xSourceStart + width;
    s16 ySourceEnd = ySourceStart + height;
	s16 xOutputEnd = xOutputStart + width;
	s16 yOutputEnd = yOutputStart + height;

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

	fix19_13 fixedNumberOfWaveLutEntries = FIX19_13_MULT(waveLutThrottleFactor, ITOFIX19_13(numberOfWaveLutEntries));

	u32 transparentMask = transparent ? 0xFFFFFFFF : 0;

	int xClamping = 0;
	int xOutputStartSave = xOutputStart;

	// clamp values to not write out of the screen
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

		if(__XAXIS & axisForReversing)
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
		if(__YAXIS & axisForReversing)
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
		if(__YAXIS & axisForReversing)
		{
			ySourceStart += (yOutputEnd - _cameraFrustum->y1);
		}

		yOutputEnd = _cameraFrustum->y1;
	}

	int xSource = xSourceStart;
	int xOutput = xOutputStart;
	int xOutputLimit = xOutputEnd;
	int xOutputIncrement = 1;

	if(__XAXIS & axisForReversing)
	{
		xOutput = xOutputEnd;
		xOutputLimit = xOutputStart;
		xOutputIncrement = -1;
	}

/*
	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(1),ITOFIX19_13((1+yOutputStart / Y_STEP_SIZE) * Y_STEP_SIZE - 1),0,0},
		(VBVec2D) {ITOFIX19_13(50),ITOFIX19_13((1+yOutputStart / Y_STEP_SIZE) * Y_STEP_SIZE - 1),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(1),ITOFIX19_13(yOutputEnd / Y_STEP_SIZE * Y_STEP_SIZE),0,0},
		(VBVec2D) {ITOFIX19_13(50),ITOFIX19_13(yOutputEnd / Y_STEP_SIZE * Y_STEP_SIZE),0,0},
		__COLOR_MEDIUM_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(1),ITOFIX19_13((yOutputEnd / Y_STEP_SIZE -1)* Y_STEP_SIZE),0,0},
		(VBVec2D) {ITOFIX19_13(50),ITOFIX19_13((yOutputEnd / Y_STEP_SIZE -1)* Y_STEP_SIZE),0,0},
		__COLOR_MEDIUM_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(10),ITOFIX19_13(yOutputStart-1),0,0},
		(VBVec2D) {ITOFIX19_13(100),ITOFIX19_13(yOutputStart-1),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(10),ITOFIX19_13(yOutputEnd),0,0},
		(VBVec2D) {ITOFIX19_13(100),ITOFIX19_13(yOutputEnd),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(1),ITOFIX19_13(ySourceStart / Y_STEP_SIZE * Y_STEP_SIZE - 1),0,0},
		(VBVec2D) {ITOFIX19_13(50),ITOFIX19_13(ySourceStart / Y_STEP_SIZE * Y_STEP_SIZE - 1),0,0},
		__COLOR_MEDIUM_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(1),ITOFIX19_13(ySourceEnd / Y_STEP_SIZE * Y_STEP_SIZE),0,0},
		(VBVec2D) {ITOFIX19_13(50),ITOFIX19_13(ySourceEnd / Y_STEP_SIZE * Y_STEP_SIZE),0,0},
		__COLOR_MEDIUM_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(50),ITOFIX19_13(ySourceStart-1),0,0},
		(VBVec2D) {ITOFIX19_13(100),ITOFIX19_13(ySourceStart-1),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(50),ITOFIX19_13(ySourceEnd),0,0},
		(VBVec2D) {ITOFIX19_13(100),ITOFIX19_13(ySourceEnd),0,0},
		__COLOR_BRIGHT_RED
	);
*/

	u32 reflectionMaskSave = reflectionMask;

	u8 dummyWaveLut[] =
	{
		0
	};

	fix19_13 waveLutIndexIncrement = this->waveLutIndexIncrement;

	if(!waveLut)
	{
		waveLutIndexIncrement = 0;
		this->waveLutIndex = 0;
		waveLut = dummyWaveLut;
	}

	int ySourceIncrement = 1;

	if(__YAXIS & axisForReversing)
	{
		s16 temp = ySourceEnd - Y_STEP_SIZE;
		ySourceEnd = ySourceStart;
		ySourceStart = temp;
		ySourceIncrement = -1;
	}

	u32 appliedBackgroundMask = transparentMask & backgroundMask;

    int ySourceStartHelper = ySourceStart >> Y_STEP_SIZE_2_EXP;

	int xSourceDistance = abs(xSourceEnd - xSourceStart);
	int xOutputDistance = abs(xOutput - xOutputLimit);
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
		CACHE_DISABLE;
		CACHE_ENABLE;

		for(; xTotal--; xOutput += xOutputIncrement, xSource +=xOutputIncrement, xCounter++)
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
				leftColumn -= parallaxDisplacement;
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

			int xRelativeCoordinate = (xCounter % width) + FIX19_13TOI(this->waveLutIndex);
			int xIndex = (numberOfWaveLutEntries * xRelativeCoordinate) / width;
			int waveLutPixelDisplacement = waveLut[xIndex];

			int ySource = ySourceStartHelper;
			int yOutput = (yOutputStart + waveLutPixelDisplacement) >> Y_STEP_SIZE_2_EXP;

			int pixelShift = (MODULO((yOutputStart + waveLutPixelDisplacement), Y_STEP_SIZE) - MODULO(ySourceStart, Y_STEP_SIZE)) << 1;

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

			u32 effectiveContentMaskDisplacement = (MODULO((yOutputStart + (flattenTop? 0 : waveLutPixelDisplacement)), Y_STEP_SIZE) << 1);
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

			if(__YAXIS & axisForReversing)
			{
				sourceCurrentValueLeft = Utilities_reverse(sourceCurrentValueLeft, BITS_PER_STEP);
				sourceCurrentValueRight = Utilities_reverse(sourceCurrentValueRight, BITS_PER_STEP);
				sourceNextValueLeft = Utilities_reverse(sourceNextValueLeft, BITS_PER_STEP);
				sourceNextValueRight = Utilities_reverse(sourceNextValueRight, BITS_PER_STEP);
			}

			waveLutPixelDisplacement =  flattenBottom ? 0 : waveLutPixelDisplacement;

			int yOutputRemainder = MODULO((yOutputEnd + waveLutPixelDisplacement), Y_STEP_SIZE) << 1;

			POINTER_TYPE remainderLeftValue = 0;
			POINTER_TYPE remainderRightValue = 0;

			int yOutputLimit = (yOutputEnd + waveLutPixelDisplacement) >> Y_STEP_SIZE_2_EXP;

			for(; yOutput < yOutputLimit; yOutput++, ySource += ySourceIncrement)
			{
				ReflectiveEntity_shiftPixels(pixelShift, &sourceCurrentValueLeft, sourceNextValueLeft, &remainderLeftValue, reflectionMask);
				ReflectiveEntity_shiftPixels(pixelShift, &sourceCurrentValueRight, sourceNextValueRight, &remainderRightValue, reflectionMask);

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

				if(__YAXIS & axisForReversing)
				{
					sourceNextValueLeft = Utilities_reverse(sourceNextValueLeft, BITS_PER_STEP);
					sourceNextValueRight = Utilities_reverse(sourceNextValueRight, BITS_PER_STEP);
				}
			}

			if(yOutputRemainder)
			{
				u32 maskDisplacement = (BITS_PER_STEP - yOutputRemainder);
				effectiveContentMask = 0xFFFFFFFF >> maskDisplacement;
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

				*columnOutputPointerLeft = (outputValueLeft & ~effectiveContentMask) | (remainderLeftValue & effectiveContentMask);
				*columnOutputPointerRight = (outputValueRight & ~effectiveContentMask) | (remainderRightValue & effectiveContentMask);
			}
		}
	}
	else
	{
		CACHE_DISABLE;
		CACHE_ENABLE;

		for(; xTotal--; xOutput += xOutputIncrement, xSource +=xOutputIncrement, xCounter++)
		{
			int leftColumn = xOutput;
			int rightColumn = xOutput;

			if(parallaxDisplacement)
			{
				leftColumn -= parallaxDisplacement;
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

			int xRelativeCoordinate = (xCounter % width) + FIX19_13TOI(this->waveLutIndex);
			int xIndex = (numberOfWaveLutEntries * xRelativeCoordinate) / width;
			int waveLutPixelDisplacement = waveLut[xIndex];

			int ySource = ySourceStartHelper;
			int yOutput = (yOutputStart + waveLutPixelDisplacement) >> Y_STEP_SIZE_2_EXP;

			int pixelShift = (MODULO((yOutputStart + waveLutPixelDisplacement), Y_STEP_SIZE) - MODULO(ySourceStart, Y_STEP_SIZE)) << 1;

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

			u32 effectiveContentMaskDisplacement = (MODULO((yOutputStart + (flattenTop? 0 : waveLutPixelDisplacement)), Y_STEP_SIZE) << 1);
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

			if(__YAXIS & axisForReversing)
			{
				sourceCurrentValueLeft = Utilities_reverse(sourceCurrentValueLeft, BITS_PER_STEP);
				sourceNextValueLeft = Utilities_reverse(sourceNextValueLeft, BITS_PER_STEP);
			}

			waveLutPixelDisplacement =  flattenBottom ? 0 : waveLutPixelDisplacement;

			int yOutputRemainder = MODULO((yOutputEnd + waveLutPixelDisplacement), Y_STEP_SIZE) << 1;

			POINTER_TYPE remainderLeftValue = 0;

			int yOutputLimit = (yOutputEnd + waveLutPixelDisplacement) >> Y_STEP_SIZE_2_EXP;

			for(; yOutput < yOutputLimit; yOutput++, ySource += ySourceIncrement)
			{
				ReflectiveEntity_shiftPixels(pixelShift, &sourceCurrentValueLeft, sourceNextValueLeft, &remainderLeftValue, reflectionMask);

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

				if(__YAXIS & axisForReversing)
				{
					sourceNextValueLeft = Utilities_reverse(sourceNextValueLeft, BITS_PER_STEP);
				}
			}

			if(yOutputRemainder)
			{
				u32 maskDisplacement = (BITS_PER_STEP - yOutputRemainder);
				effectiveContentMask = 0xFFFFFFFF >> maskDisplacement;
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

				*columnOutputPointerLeft = (outputValueLeft & ~effectiveContentMask) | (remainderLeftValue & effectiveContentMask);
				*columnOutputPointerRight = (outputValueLeft & ~effectiveContentMask) | (remainderLeftValue & effectiveContentMask);
			}
		}
	}
}

