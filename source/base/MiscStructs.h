/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#ifndef MISC_STRUCTS_H_
#define MISC_STRUCTS_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Types.h>


//---------------------------------------------------------------------------------------------------------
// 											 DEFINITIONS
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

// spatial position
typedef struct VBVec3DReal
{
	// fps increases a lot in hardware with ints
	float x;
	float y;
	float z;

} VBVec3DReal;

// spatial position
typedef struct VBVec3D
{
	// fps increases a lot in hardware with ints
	fix19_13 x;
	fix19_13 y;
	fix19_13 z;

} VBVec3D;

typedef struct VBVec3DFlag
{
	int x: 2;
	int y: 2;
	int z: 2;

} VBVec3DFlag;

typedef struct IntegralPosition
{
	// fps increases a lot in hardware with ints
	int x;
	int y;
	int z;

} IntegralPosition;

// spatial movement vector
typedef struct Velocity
{
	// fps increases a lot in hardware with ints
	fix19_13 x;
	fix19_13 y;
	fix19_13 z;

} Velocity;

// spatial velocity variation vector
typedef struct Acceleration
{
	fix19_13 x;
	fix19_13 y;
	fix19_13 z;

} Acceleration;

// spatial velocity variation vector
typedef struct Force
{
	fix19_13 x;
	fix19_13 y;
	fix19_13 z;

} Force;

// movement type flag vector
typedef struct MovementType
{
	s8 x;
	s8 y;
	s8 z;

} MovementType;

typedef struct Rotation
{
	// arount x axis
	s16 x;

	// arount y axis
	s16 y;

	// arount z axis
	s16 z;

} Rotation;

// spatial direction vector
typedef struct Direction
{
	s8 x;
	s8 y;
	s8 z;

	/*
	//angle between vector and X axis
	Angle alpha;
	//angle between vector and Y axis
	Angle betha;
	//angle between vector and Z axis
	Angle tetha;
	*/

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

// scaling factor
typedef struct Scale
{
	fix7_9 x;
	fix7_9 y;

} Scale;

// collision detection gap space
typedef struct Gap
{
	s8 up;
	s8 down;
	s8 left;
	s8 right;

} Gap;

// used to represent a screen position with parallax info
typedef struct VBVec2D
{
	fix19_13 x;
	fix19_13 y;

	// since parallax may be the same given different z positions
	// it's needed to have this value to being able to order WORLD layers
	fix19_13 z;
	int parallax;

} VBVec2D;

// used to representthe m coordinates of the bgmaps
typedef struct TextureSource
{
	s16 mx;
	s16 mp;
	s16 my;

}TextureSource;


typedef struct TexturePadding
{
	u8 cols;
	u8 rows;

}TexturePadding;

// a spatial description
typedef struct DrawSpec
{
	// spatial position	with parallax info
	VBVec2D position;

	// bgmap's source coordinates
	TextureSource textureSource;

	// scale
	Scale scale;

	// angle with respect to each axis (indexes for the SINLUT array)
	Rotation rotation;

} DrawSpec;

// a spatial description
typedef struct Transformation
{
	// spatial local position
	VBVec3D localPosition;

	// spatial global position
	VBVec3D globalPosition;

	// local rotation
	Rotation localRotation;

	// global rotation
	Rotation globalRotation;

	// scale
	Scale localScale;

	// scale
	Scale globalScale;

} Transformation;

typedef struct RightCuboid
{
	/* left upper corner */
	fix19_13 x0;
	fix19_13 y0;
	fix19_13 z0;

	/* right down corner */
	fix19_13 x1;
	fix19_13 y1;
	fix19_13 z1;

} RightCuboid;

typedef struct SmallRightCuboid
{
	/* left upper corner */
	s16 x0;
	s16 y0;
	s16 z0;

	/* right down corner */
	s16 x1;
	s16 y1;
	s16 z1;

} SmallRightCuboid;

// spatial state vector
typedef struct GeneralAxisFlag
{
	s8 x;
	s8 y;
	s8 z;

} GeneralAxisFlag;


#endif
