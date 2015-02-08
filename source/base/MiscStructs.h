/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef MISCSTRUCTS_H_
#define MISCSTRUCTS_H_


//---------------------------------------------------------------------------------------------------------
// 											 DEFINITIONS
//---------------------------------------------------------------------------------------------------------

#include <libgccvb.h>

// spacial size
typedef struct Size
{
	u16 x;
	u16 y;
	u16 z;

} Size;


// spacial position
typedef struct VBVec3DReal
{
	// fps increases a lot in hardware with ints
	float x;
	float y;
	float z;

} VBVec3DReal;

// spacial position
typedef struct VBVec3D
{
	// fps increases a lot in hardware with ints
	fix19_13 x;
	fix19_13 y;
	fix19_13 z;

} VBVec3D;

typedef struct IntegralPosition
{
	// fps increases a lot in hardware with ints
	int x;
	int y;
	int z;

} IntegralPosition;

// spacial movement vector
typedef struct Velocity
{
	// fps increases a lot in hardware with ints
	fix19_13 x;
	fix19_13 y;
	fix19_13 z;
	fix19_13 V;

} Velocity;


// spacial velocity variation vector
typedef struct Acceleration
{
	fix19_13 x;
	fix19_13 y;
	fix19_13 z;

} Acceleration;

// spacial velocity variation vector
typedef struct Force
{
	fix19_13 x;
	fix19_13 y;
	fix19_13 z;

} Force;

// movement type flag vector
typedef struct MovementType
{
	s8 x: 2;
	s8 y: 2;
	s8 z: 2;

} MovementType;

// alignement flag vector
typedef struct Alignement
{
	u8 x;
	u8 y;
	u8 z;

} Alignement;

// alignement flag vector
typedef struct GravitySensibleAxis
{
	u8 x;
	u8 y;
	u8 z;

} GravitySensibleAxis;

typedef struct Angle
{
	//angle between vector and XZ plane
	//int degree;
	//fix7_9 sin;
	fix7_9 cos;
	//fix7_9 sin;
	//fix7_9 tan;

} Angle;

// spacial direction vector
typedef struct Direction
{
	int x: 2;
	int y: 2;
	int z: 2;

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
	fix19_13 distanceEyeScreen;
	fix19_13 maximunViewDistance;//maximun distance from the screen to the infinite
	fix19_13 baseDistance;//distance from left to right eye (deep sensation)
	fix19_13 verticalViewPointCenter;//vertical View point center
	fix19_13 horizontalViewPointCenter;//horizontal View point center

} Optical;

// define a 2d point
typedef struct Point
{
	s16 x;
	s16 y;

} Point;
/*
// define a vector
typedef struct Vector
{
	//vector's components
	int i;
	int j;
	int k;

} Vector;
*/
// scaling factor
typedef struct Scale
{
	fix7_9 x;
	fix7_9 y;

} Scale;

// collision detection gap space
typedef struct Gap
{
	int up:8;
	int down:8;
	int left:8;
	int right:8;

} Gap;

/*
// used to represent a screen position
typedef struct Vec2D
{
	int x;
	int y;

} Vec2D;
*/

// used to represent a screen position with parallax info
typedef struct VBVec2D
{
	fix19_13 x;
	fix19_13 y;

	// since parallax may be the same given different z positions
	// it's needed to have this value to being able to ordern WORLD layers
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

// a spatial description
typedef struct DrawSpec
{
	// spatial position	with parallax info
	VBVec2D position;

	// bgmap's source coordinates
	TextureSource textureSource;
	
	// scale
	Scale scale;

} DrawSpec;

// a spatial description
typedef struct Transformation
{
	// spatial local position
	VBVec3D localPosition;

	// spatial global position
	VBVec3D globalPosition;

	// scale
	Scale scale;

	// rotation
	struct rotation
	{
		// arount x axis
		fix7_9 x;

		// arount y axis
		fix7_9 y;

		// arount z axis
		fix7_9 z;

	} rotation;

} Transformation;

typedef struct Rightcuboid
{
	/* left upper corner */
	fix19_13 x0;
	fix19_13 y0;
	fix19_13 z0;

	/* right down corner */
	fix19_13 x1;
	fix19_13 y1;
	fix19_13 z1;

} Rightcuboid;

typedef struct SmallRightcuboid
{
	/* left upper corner */
	s16 x0;
	s16 y0;
	s16 z0;

	/* right down corner */
	s16 x1;
	s16 y1;
	s16 z1;

} SmallRightcuboid;

#endif