/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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
	uint8 darkRed;
	uint8 mediumRed;
	uint8 brightRed;

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
	uint16 x;
	uint16 y;
	uint16 z;

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
	int16 x;
	int16 y;
	int16 z;
	int16 parallax;

} PixelVector;

// used to represent a screen position with z sorting displacement info
typedef struct ScreenPixelVector
{
	int16 x;
	int16 y;
	// it is used for WORLD sorting and having micromanagement allows for easier sorting
	int16 z;
	int16 zDisplacement;

} ScreenPixelVector;


typedef struct Vector3DFlag
{
	bool x;
	bool y;
	bool z;

} Vector3DFlag;

// spatial movement vector
typedef struct Vector3D Velocity;

// spatial velocity variation vector
typedef struct Vector3D Acceleration;

// spatial velocity variation vector
typedef struct Vector3D Force;

// spatial direction variation vector
typedef struct Vector3D Direction3D;

// movement type flag vector
typedef struct MovementType
{
	int8 x;
	int8 y;
	int8 z;

} MovementType;

typedef struct Rotation
{
	// rotation around x axis
	fix10_6 x;

	// rotation around y axis
	fix10_6 y;

	// rotation around z axis
	fix10_6 z;

} Rotation;

typedef struct PixelRotation
{
	// rotation around x axis
	int16 x;

	// rotation around y axis
	int16 y;

	// rotation around z axis
	int16 z;

} PixelRotation;

// spatial direction vector
typedef struct Direction
{
	int8 x;
	int8 y;
	int8 z;

} Direction;

// engine's optical values structure
typedef struct Optical
{
	uint16 maximumXViewDistancePower;		// maximum distance from the screen to the infinite
	uint16 maximumYViewDistancePower;		// maximum distance from the screen to the infinite
	fix10_6 cameraNearPlane;				// logical distance between the eyes and the screen
	fix10_6 baseDistance;					// distance from left to right eye (depth perception)
	fix10_6 horizontalViewPointCenter;		// horizontal View point center
	fix10_6 verticalViewPointCenter;		// vertical View point center
	fix10_6 scalingFactor;					// vertical View point center
	fix10_6 halfWidth;						// screen width
	fix10_6 halfHeight;						// screen height
	fix10_6 aspectRatio;					// screen's width / screen's height
	fix10_6 fov;							// 1 / tan (angle / 2)
	fix10_6 aspectRatioXfov;				// aspectRatio * fov
	fix10_6 farRatio1Near;	 				// (far + near) / (far - near)
	fix10_6 farRatio2Near;	 				// (2 * far * near) / (near - far)
	fix10_6_ext projectionMultiplierHelper;	// helper to speed up projection operations 
	fix10_6_ext scalingMultiplier;			// scaling multiplier
} Optical;

// engine's optical values structure
typedef struct PixelOptical
{
	uint16 maximumXViewDistance;		// maximum distance from the screen to the infinite
	uint16 maximumYViewDistance;		// maximum distance from the screen to the infinite
	uint16 cameraNearPlane;			// logical distance between the eyes and the screen
	uint16 baseDistance;				// distance from left to right eye (depth perception)
	int16 horizontalViewPointCenter;	// horizontal View point center
	int16 verticalViewPointCenter;		// vertical View point center
	float scalingFactor;				// scaling factor for sprite resizing

} PixelOptical;


// define a 2d point
typedef struct Point
{
	int16 x;
	int16 y;

} Point;

// define a 2d point in screen space
typedef struct Pixel
{
	uint16 x;
	uint16 y;

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
	int16 mx;
	int16 mp;
	int16 my;

} TextureSource;


typedef struct TexturePadding
{
	uint8 cols;
	uint8 rows;

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
	int16 x0;
	int16 y0;
	int16 z0;

	/* right down corner */
	int16 x1;
	int16 y1;
	int16 z1;

} PixelRightBox;


#endif
