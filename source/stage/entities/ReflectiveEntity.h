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

#ifndef REFLECTIVE_ENTITY_H_
#define REFLECTIVE_ENTITY_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <InGameEntity.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

#define ReflectiveEntity_METHODS(ClassName)																\
		InGameEntity_METHODS(ClassName)																	\
		__VIRTUAL_DEC(ClassName, void, applyReflection, u32 currentDrawingFrameBufferSet);				\

#define ReflectiveEntity_SET_VTABLE(ClassName)															\
		InGameEntity_SET_VTABLE(ClassName)																\
		__VIRTUAL_SET(ClassName, ReflectiveEntity, ready);												\
		__VIRTUAL_SET(ClassName, ReflectiveEntity, suspend);											\
		__VIRTUAL_SET(ClassName, ReflectiveEntity, resume);												\
		__VIRTUAL_SET(ClassName, ReflectiveEntity, applyReflection);									\

__CLASS(ReflectiveEntity);

#define ReflectiveEntity_ATTRIBUTES																		\
		/* it is derived from */																		\
		InGameEntity_ATTRIBUTES																			\
		fix19_13 waveLutIndex;																			\
		fix19_13 waveLutIndexIncrement;																	\

typedef struct ReflectiveEntityDefinition
{
	InGameEntityDefinition inGameEntityDefinition;

	// the starting point from where start to reflect data
	// relative to my position
	VBVec2D sourceDisplacement;

	// the starting point from where start to draw data
	// relative to my position
	VBVec2D outputDisplacement;

	// width and height of the reflection
	u16 width;
	u16 height;

	// mask to apply to the mirrored image
	u32 reflectionMask;

	// mask to apply to the image behind the reflection
	u32 backgroundMask;

	// transparent
	bool transparent;

	// reflect parallax info
	bool reflectParallax;

	// axis for image reversing
	u8 axisForReversing;

	// pointer to table of vertical displacements
	// if no displacement, leave as NULL
	const u8* waveLut;

	// number of wave lut entries
	u16 numberOfWaveLutEntries;

	// fix19_13 throttle for the waving
	fix19_13 waveLutThrottleFactor;

	// parallax displacement applied to the reflection
	s16 parallaxDisplacement;

	// flatten up/down
	bool flattenTop;
	bool flattenBottom;

} ReflectiveEntityDefinition;

// defines a Scrolling background in ROM memory
typedef const ReflectiveEntityDefinition ReflectiveEntityROMDef;


//---------------------------------------------------------------------------------------------------------
//										PUBLIC INTERFACE
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DECLARE(ReflectiveEntity, ReflectiveEntityDefinition* mirrorDefinition, s16 id, s16 internalId, const char* const name);

void ReflectiveEntity_constructor(ReflectiveEntity this, ReflectiveEntityDefinition* mirrorDefinition, s16 id, s16 internalId, const char* const name);
void ReflectiveEntity_destructor(ReflectiveEntity this);
void ReflectiveEntity_ready(ReflectiveEntity this, bool recursive);
void ReflectiveEntity_suspend(ReflectiveEntity this);
void ReflectiveEntity_resume(ReflectiveEntity this);
void ReflectiveEntity_applyReflection(ReflectiveEntity this, u32 currentDrawingFrameBufferSet);
void ReflectiveEntity_drawReflection(ReflectiveEntity this, u32 currentDrawingFrameBufferSet,
								s16 xSourceStart, s16 xSourceEnd,
								s16 ySourceStart, s16 ySourceEnd,
								s16 xOutputStart, s16 yOutputStart,
								u32 reflectionMask, u32 backgroundMask,
								u16 axisForReversing, bool transparent, bool reflectParallax,
								s16 parallaxDisplacement,
								const u8 waveLut[], int numberOfWaveLutEntries,
								bool flattenTop, bool flattenBottom);

#endif
