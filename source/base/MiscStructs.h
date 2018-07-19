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

#ifndef MISC_STRUCTS_H_
#define MISC_STRUCTS_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Types.h>


//---------------------------------------------------------------------------------------------------------
//											 DEFINITIONS
//---------------------------------------------------------------------------------------------------------

// brightness setting
typedef struct Brightness
{
	u8 darkRed;
	u8 mediumRed;
	u8 brightRed;

} Brightness;

// spatial size
typedef struct Size
{
	fix10_6 x;
	fix10_6 y;
	fix10_6 z;

} Size;

// size in pixels
typedef struct PixelSize
{
	u16 x;
	u16 y;
	u16 z;

} PixelSize;


// spatial position
typedef struct Vector3D
{
	fix10_6 x;
	fix10_6 y;
	fix10_6 z;

} Vector3D;

// used to represent a screen position with parallax info
typedef struct PixelVector
{
	s16 x;
	s16 y;
	s16 z;
	s16 parallax;

} PixelVector;

// used to represent a screen position with z sorting displacement info
typedef struct ScreenPixelVector
{
	s16 x;
	s16 y;
	// it is used for WORLD sorting and having micromanagement allows for easier sorting
	s16 z;
	s16 zDisplacement;

} ScreenPixelVector;


typedef struct Vector3DFlag
{
	u16 x: 2;
	u16 y: 2;
	u16 z: 2;

} Vector3DFlag;

// spatial movement vector
typedef struct Vector3D Velocity;

// spatial velocity variation vector
typedef struct Vector3D Acceleration;

// spatial velocity variation vector
typedef struct Vector3D Force;

// movement type flag vector
typedef struct MovementType
{
	s8 x;
	s8 y;
	s8 z;

} MovementType;

typedef struct Rotation
{
	// rotation around x axis
	s16 x;

	// rotation around y axis
	s16 y;

	// rotation around z axis
	s16 z;

} Rotation;

// spatial direction vector
typedef struct Direction
{
	s8 x;
	s8 y;
	s8 z;

} Direction;

// engine's optical values structure
typedef struct Optical
{
	u16 maximumXViewDistancePower;		// maximum distance from the screen to the infinite
	u16 maximumYViewDistancePower;		// maximum distance from the screen to the infinite
	fix10_6 distanceEyeScreen;
	fix10_6 baseDistance;				// distance from left to right eye (depth perception)
	fix10_6 horizontalViewPointCenter;	// horizontal View point center
	fix10_6 verticalViewPointCenter;	// vertical View point center
	fix10_6 scalingFactor;	// vertical View point center

} Optical;

// engine's optical values structure
typedef struct PixelOptical
{
	u16 maximumXViewDistance;		// maximum distance from the screen to the infinite
	u16 maximumYViewDistance;		// maximum distance from the screen to the infinite
	u16 distanceEyeScreen;
	u16 baseDistance;					// distance from left to right eye (depth perception)
	s16 horizontalViewPointCenter;		// horizontal View point center
	s16 verticalViewPointCenter;		// vertical View point center
	float scalingFactor;				// scaling factor for sprite resizing

} PixelOptical;


// define a 2d point
typedef struct Point
{
	s16 x;
	s16 y;

} Point;

// define a 2d point in screen space
typedef struct Pixel
{
	u16 x;
	u16 y;

} Pixel;

// scaling factor
typedef struct Scale
{
	fix7_9 x;
	fix7_9 y;
	fix7_9 z;

} Scale;

// used to represent the m coordinates of the bgmaps
typedef struct TextureSource
{
	s16 mx;
	s16 mp;
	s16 my;

} TextureSource;


typedef struct TexturePadding
{
	u8 cols;
	u8 rows;

} TexturePadding;

// a spatial description
typedef struct DrawSpec
{
	// bgmap's source coordinates
	TextureSource textureSource;

	// angle with respect to each axis (indexes for the _sinLut array)
	Rotation rotation;

	// scale
	Scale scale;

} DrawSpec;

// a spatial description
typedef struct Transformation
{
	// spatial local position
	Vector3D localPosition;

	// spatial global position
	Vector3D globalPosition;

	// local rotation
	Rotation localRotation;

	// global rotation
	Rotation globalRotation;

	// scale
	Scale localScale;

	// scale
	Scale globalScale;

} Transformation;

typedef struct RightBox
{
	/* left upper corner */
	fix10_6 x0;
	fix10_6 y0;
	fix10_6 z0;

	/* right down corner */
	fix10_6 x1;
	fix10_6 y1;
	fix10_6 z1;

} RightBox;

typedef struct PixelRightBox
{
	/* left upper corner */
	s16 x0;
	s16 y0;
	s16 z0;

	/* right down corner */
	s16 x1;
	s16 y1;
	s16 z1;

} PixelRightBox;


#endif
