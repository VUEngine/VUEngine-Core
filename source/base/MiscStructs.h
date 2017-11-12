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
	u16 x;
	u16 y;
	u16 z;

} Size;

// used to represent a screen position with parallax info
typedef struct Vector2D
{
	fix19_13 x;
	fix19_13 y;

	// since parallax may be the same given different z positions
	// it's needed to have this value to being able to order WORLD layers
	fix19_13 z;
	int parallax;

} Vector2D;

// spatial position
typedef struct Vector3D
{
	// fps increases a lot in hardware with ints
	fix19_13 x;
	fix19_13 y;
	fix19_13 z;

} Vector3D;

// WORLD vector
typedef struct WorldVector
{
	// fps increases a lot in hardware with ints
	fix19_13 x;
	fix19_13 y;
	fix19_13 z;
	fix19_13 p;

} WorldVector;

typedef struct Vector3DFlag
{
	int x: 2;
	int y: 2;
	int z: 2;

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
	int maximumViewDistancePower;		// maximum distance from the screen to the infinite
	fix19_13 distanceEyeScreen;
	fix19_13 baseDistance;				// distance from left to right eye (depth sensation)
	fix19_13 horizontalViewPointCenter;	// horizontal View point center
	fix19_13 verticalViewPointCenter;	// vertical View point center

} Optical;

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
	// spatial position	with parallax info
	Vector2D position;

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
	fix19_13 x0;
	fix19_13 y0;
	fix19_13 z0;

	/* right down corner */
	fix19_13 x1;
	fix19_13 y1;
	fix19_13 z1;

} RightBox;

typedef struct SmallRightBox
{
	/* left upper corner */
	s16 x0;
	s16 y0;
	s16 z0;

	/* right down corner */
	s16 x1;
	s16 y1;
	s16 z1;

} SmallRightBox;

// spatial state vector
typedef struct GeneralAxisFlag
{
	s8 x;
	s8 y;
	s8 z;

} GeneralAxisFlag;


#endif
