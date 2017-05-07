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
//											CLASS' MACROS
//---------------------------------------------------------------------------------------------------------

#define POINTER_TYPE			u32
#define Y_SHIFT					4
// sizeof(POINTER_TYPE) << 2
#define Y_STEP_SIZE				16
#define Y_STEP_SIZE_2_EXP		4
// sizeof(POINTER_TYPE) << 3
#define BITS_PER_STEP 			32

#define MODULO(n, m)			(n & (m - 1))



//---------------------------------------------------------------------------------------------------------
//											CLASS' DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(ReflectiveEntity, InGameEntity);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void ReflectiveEntity_reflect(u32 currentDrawingFrameBufferSet, SpatialObject spatialObject);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ReflectiveEntity, ReflectiveEntityDefinition* mirrorDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(ReflectiveEntity, mirrorDefinition, id, internalId, name);

// class's constructor
void ReflectiveEntity_constructor(ReflectiveEntity this, ReflectiveEntityDefinition* mirrorDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "ReflectiveEntity::constructor: null this");

	// construct base
	__CONSTRUCT_BASE(InGameEntity, &mirrorDefinition->inGameEntityDefinition, id, internalId, name);

	this->waveLutIndex = 0;
	this->waveLutIndexIncrement = FIX19_13_MULT(mirrorDefinition->waveLutThrottleFactor, FIX19_13_DIV(ITOFIX19_13(mirrorDefinition->numberOfWaveLutEntries), ITOFIX19_13(mirrorDefinition->width)));
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
	__CALL_BASE_METHOD(InGameEntity, ready, this, recursive);

	// add post processing effect to make key emit rhombuses
	Game_pushFrontProcessingEffect(Game_getInstance(), ReflectiveEntity_reflect, __SAFE_CAST(SpatialObject, this));
}

void ReflectiveEntity_suspend(ReflectiveEntity this)
{
	ASSERT(this, "ReflectiveEntity::suspend: null this");

	__CALL_BASE_METHOD(InGameEntity, suspend, this);

	// remove post processing effect
	Game_removePostProcessingEffect(Game_getInstance(), ReflectiveEntity_reflect, __SAFE_CAST(SpatialObject, this));
}

void ReflectiveEntity_resume(ReflectiveEntity this)
{
	ASSERT(this, "ReflectiveEntity::resume: null this");

	__CALL_BASE_METHOD(InGameEntity, resume, this);

	// add post processing effect to make key emit rhombuses
	Game_pushFrontProcessingEffect(Game_getInstance(), ReflectiveEntity_reflect, __SAFE_CAST(SpatialObject, this));
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

	ReflectiveEntityDefinition* mirrorDefinition = (ReflectiveEntityDefinition*)this->entityDefinition;

	VBVec3D position3D = this->transform.globalPosition;
	__OPTICS_NORMALIZE(position3D);

	VBVec2D position2D;
	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, position2D);

	ReflectiveEntity_drawReflection(this, currentDrawingFrameBufferSet,
								FIX19_13TOI(position2D.x + mirrorDefinition->sourceDisplacement.x),
								mirrorDefinition->width,
								FIX19_13TOI(position2D.y + mirrorDefinition->sourceDisplacement.y),
								mirrorDefinition->height,
								FIX19_13TOI(position2D.x + mirrorDefinition->outputDisplacement.x),
								FIX19_13TOI(position2D.y + mirrorDefinition->outputDisplacement.y),
								mirrorDefinition->reflectionMask,
								mirrorDefinition->backgroundMask,
								mirrorDefinition->axisForReversing,
								mirrorDefinition->transparent,
								mirrorDefinition->reflectParallax,
								mirrorDefinition->parallaxDisplacement,
								mirrorDefinition->waveLut,
								mirrorDefinition->numberOfWaveLutEntries,
								mirrorDefinition->flattenTop, mirrorDefinition->flattenBottom);
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
								s16 xSourceStart, s16 width,
								s16 ySourceStart, s16 height,
								s16 xOutputStart, s16 yOutputStart,
								u32 reflectionMask, u32 backgroundMask,
								u16 axisForReversing, bool transparent, bool reflectParallax,
								s16 parallaxDisplacement,
								const u8 waveLut[], int numberOfWaveLutEntries,
								bool flattenTop __attribute__ ((unused)), bool flattenBottom)
{
	ASSERT(this, "ReflectiveEntity::drawReflection: null this");

	CACHE_DISABLE;
	CACHE_ENABLE;

	fix19_13 fixedNumberOfWaveLutEntries = ITOFIX19_13(numberOfWaveLutEntries);

	u32 transparentMask = transparent ? 0xFFFFFFFF : 0;

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

	int xClamping = 0;
	int xTotalClamping = 0;

	// clamp values to not write out of the screen
	if(xSourceStart < _cameraFrustum->x0)
	{
		xClamping = _cameraFrustum->x0 - xSourceStart;
		xTotalClamping += xClamping;
		xOutputStart += xClamping;
		xSourceStart = _cameraFrustum->x0;
	}

	if(xSourceEnd > _cameraFrustum->x1 - 1)
	{
		xClamping = xSourceEnd - _cameraFrustum->x1 + 1;
		xTotalClamping += xClamping;
		xOutputEnd -= xClamping;
		xSourceEnd = _cameraFrustum->x1 - 1;
	}

	ySourceStart = (ySourceStart < _cameraFrustum->y0) ? _cameraFrustum->y0 : ySourceStart;
	ySourceEnd = (ySourceEnd > _cameraFrustum->y1) ? _cameraFrustum->y1 : ySourceEnd;

	// must clamp the output too, but moving the wave lut index accordingly
	if(xOutputStart < _cameraFrustum->x0)
	{
		xClamping = _cameraFrustum->x0 - xOutputStart;
		xTotalClamping += xClamping;
		xSourceStart += xClamping;
		xOutputStart = _cameraFrustum->x0;
	}

	if(xOutputEnd > _cameraFrustum->x1)
	{
		xClamping = xOutputEnd - _cameraFrustum->x1 + 1;
		xTotalClamping += xClamping;

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

	this->waveLutIndex += FIX19_13_MULT(this->waveLutIndexIncrement, ITOFIX19_13(xTotalClamping));

	yOutputStart = (yOutputStart < _cameraFrustum->y0) ? _cameraFrustum->y0 : yOutputStart;

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
		(VBVec2D) {ITOFIX19_13(10),ITOFIX19_13(yOutputStart / Y_STEP_SIZE * Y_STEP_SIZE - 1),0,0},
		(VBVec2D) {ITOFIX19_13(50),ITOFIX19_13(yOutputStart / Y_STEP_SIZE * Y_STEP_SIZE - 1),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(10),ITOFIX19_13(yOutputEnd / Y_STEP_SIZE * Y_STEP_SIZE),0,0},
		(VBVec2D) {ITOFIX19_13(50),ITOFIX19_13(yOutputEnd / Y_STEP_SIZE * Y_STEP_SIZE),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(150),ITOFIX19_13(yOutputStart-1),0,0},
		(VBVec2D) {ITOFIX19_13(250),ITOFIX19_13(yOutputStart-1),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(150),ITOFIX19_13(yOutputEnd),0,0},
		(VBVec2D) {ITOFIX19_13(250),ITOFIX19_13(yOutputEnd),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(150),ITOFIX19_13(ySourceStart-1),0,0},
		(VBVec2D) {ITOFIX19_13(250),ITOFIX19_13(ySourceStart-1),0,0},
		__COLOR_BRIGHT_RED
	);

	DirectDraw_drawLine(
		DirectDraw_getInstance(),
		(VBVec2D) {ITOFIX19_13(150),ITOFIX19_13(ySourceEnd),0,0},
		(VBVec2D) {ITOFIX19_13(250),ITOFIX19_13(ySourceEnd),0,0},
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
		s16 temp = ySourceEnd;
		ySourceEnd = ySourceStart;
		ySourceStart = temp;
		ySourceIncrement = -1;
	}

	u32 appliedBackgroundMask = transparentMask & backgroundMask;

    int ySourceLimit = ySourceEnd >> Y_STEP_SIZE_2_EXP;
    int ySourceStartHelper = ySourceStart >> Y_STEP_SIZE_2_EXP;

	int xSourceDistance = abs(xSourceEnd - xSourceStart);
	int xOutputDistance = abs(xOutput - xOutputLimit);
	int xTotal = xOutputDistance > xSourceDistance ? xSourceDistance : xOutputDistance;

	for(;xTotal--; xOutput += xOutputIncrement, xSource +=xOutputIncrement)
	{
		int leftColumn = xOutput;
		int rightColumn = xOutput;

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

		this->waveLutIndex += waveLutIndexIncrement;

		if(this->waveLutIndex >= fixedNumberOfWaveLutEntries)
		{
			this->waveLutIndex = 0;
		}

		int waveLutPixelDisplacement = waveLut[FIX19_13TOI(this->waveLutIndex)];

        int ySource = ySourceStartHelper;
        int yOutput = (yOutputStart + waveLutPixelDisplacement) >> Y_STEP_SIZE_2_EXP;

		POINTER_TYPE remainderLeftValue = 0;
		POINTER_TYPE remainderRightValue = 0;

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

		u32 effectiveBackgroundMask = 0xFFFFFFFF << (MODULO((yOutputStart + waveLutPixelDisplacement), Y_STEP_SIZE) << 1);

		POINTER_TYPE* columnSourcePointerLeft = (POINTER_TYPE*) (currentDrawingFrameBufferSet) + (xSource << Y_SHIFT) + ySource;
		POINTER_TYPE* columnSourcePointerRight = (POINTER_TYPE*) (currentDrawingFrameBufferSet | 0x00010000) + (xSource << Y_SHIFT) + ySource;
		POINTER_TYPE* columnOutputPointerLeft = (POINTER_TYPE*) (currentDrawingFrameBufferSet) + (leftColumn << Y_SHIFT) + yOutput;
		POINTER_TYPE* columnOutputPointerRight = (POINTER_TYPE*) (currentDrawingFrameBufferSet | 0x00010000) + (rightColumn << Y_SHIFT) + yOutput;

		POINTER_TYPE sourcePreviousValueLeft = *columnSourcePointerLeft;
		POINTER_TYPE sourceNextValueLeft = sourcePreviousValueLeft;

		int columnSourcePointerLeftIncrement = ySourceIncrement;
		int columnSourcePointerRightIncrement = ySourceIncrement;

		if(!reflectParallax)
		{
			columnSourcePointerRight = &sourceNextValueLeft;
			columnSourcePointerRightIncrement = 0;
		}

		POINTER_TYPE sourcePreviousValueRight = *columnSourcePointerRight;
		POINTER_TYPE sourceNextValueRight = sourcePreviousValueRight;

		POINTER_TYPE outputValueLeft = *columnOutputPointerLeft;
		POINTER_TYPE outputValueRight = *columnOutputPointerRight;

		if(__YAXIS & axisForReversing)
		{
			sourcePreviousValueLeft = Utilities_reverse(sourcePreviousValueLeft, BITS_PER_STEP);
			sourcePreviousValueRight = Utilities_reverse(sourcePreviousValueRight, BITS_PER_STEP);
		}

		int yOutputRemainder = 0;

		waveLutPixelDisplacement = flattenBottom? 0 : waveLutPixelDisplacement;
		yOutputRemainder = MODULO((yOutputEnd - waveLutPixelDisplacement), Y_STEP_SIZE) << 1;

		ReflectiveEntity_shiftPixels(pixelShift, &sourcePreviousValueLeft, sourceNextValueLeft, &remainderLeftValue, reflectionMask);
		ReflectiveEntity_shiftPixels(pixelShift, &sourcePreviousValueRight, sourceNextValueRight, &remainderRightValue, reflectionMask);

		if(effectiveBackgroundMask)
		{
			sourcePreviousValueLeft |= appliedBackgroundMask & outputValueLeft;
			sourcePreviousValueRight |= appliedBackgroundMask & outputValueRight;
			sourcePreviousValueLeft &= effectiveBackgroundMask;
			sourcePreviousValueRight &= effectiveBackgroundMask;
			sourcePreviousValueLeft |= (outputValueLeft & ~effectiveBackgroundMask);
			sourcePreviousValueRight |= (outputValueRight & ~effectiveBackgroundMask);
		}

		POINTER_TYPE sourceValueLeft = sourcePreviousValueLeft;
		POINTER_TYPE sourceValueRight = sourcePreviousValueRight;

		s16 yOutputLimit = (yOutputEnd - waveLutPixelDisplacement) >> Y_STEP_SIZE_2_EXP;

		int yCounter = 0;
		int ySourceDistance = abs(ySource - ySourceLimit);
		int yOutputDistance = abs(yOutput - yOutputLimit);
		int yTotal = yOutputDistance > ySourceDistance ? ySourceDistance : yOutputDistance;

		for(; yTotal--; yCounter++, yOutput++, ySource += ySourceIncrement)
		{
			columnSourcePointerLeft += columnSourcePointerLeftIncrement;
			columnSourcePointerRight += columnSourcePointerRightIncrement;

			sourceValueLeft |= appliedBackgroundMask & outputValueLeft;
			sourceValueRight |= appliedBackgroundMask & outputValueRight;

			*columnOutputPointerLeft = sourceValueLeft;
			*columnOutputPointerRight = sourceValueRight;

			sourceNextValueLeft = *columnSourcePointerLeft;
			sourceNextValueRight = *columnSourcePointerRight;

			if(__YAXIS & axisForReversing)
			{
				sourceNextValueLeft = Utilities_reverse(sourceNextValueLeft, BITS_PER_STEP);
				sourceNextValueRight = Utilities_reverse(sourceNextValueRight, BITS_PER_STEP);
			}

			sourcePreviousValueLeft = sourceNextValueLeft;
			sourcePreviousValueRight = sourceNextValueRight;
			columnOutputPointerLeft++;
			columnOutputPointerRight++;

			sourceValueLeft = sourcePreviousValueLeft;
			sourceValueRight = sourcePreviousValueRight;

			ReflectiveEntity_shiftPixels(pixelShift, &sourceValueLeft, sourceNextValueLeft, &remainderLeftValue, reflectionMask);
			ReflectiveEntity_shiftPixels(pixelShift, &sourceValueRight, sourceNextValueRight, &remainderRightValue, reflectionMask);

			if(transparent)
			{
				outputValueLeft = *columnOutputPointerLeft;
				outputValueRight = *columnOutputPointerRight;
			}
		}

		if(yOutputRemainder)
		{
			effectiveBackgroundMask = 0xFFFFFFFF >> (BITS_PER_STEP - yOutputRemainder);

			if(!transparent)
			{
				outputValueLeft = *columnOutputPointerLeft;
				outputValueRight = *columnOutputPointerRight;
			}

			remainderLeftValue |= (sourcePreviousValueLeft << pixelShift);
			remainderRightValue |= (sourcePreviousValueRight << pixelShift);

			remainderLeftValue &= reflectionMask;
			remainderRightValue &= reflectionMask;

			remainderLeftValue |= appliedBackgroundMask & outputValueLeft;
			remainderRightValue |= appliedBackgroundMask & outputValueRight;

			*columnOutputPointerLeft = (outputValueLeft & ~effectiveBackgroundMask) | (remainderLeftValue & effectiveBackgroundMask);
			*columnOutputPointerRight = (outputValueRight & ~effectiveBackgroundMask) | (remainderRightValue & effectiveBackgroundMask);
		}
	}
}

