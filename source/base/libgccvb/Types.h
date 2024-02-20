/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef TYPES_H_
#define TYPES_H_


//---------------------------------------------------------------------------------------------------------
//											 DECLARATIONS
//---------------------------------------------------------------------------------------------------------

// quick, easy types
typedef unsigned char 		uint8;
typedef unsigned short 		uint16;
typedef unsigned int 		uint32;
typedef unsigned long long	uint64;

typedef signed char 		int8;
typedef signed short 		int16;
typedef signed int	 		int32;
typedef signed long long 	int64;

typedef uint8		 		BYTE;
typedef uint16		 		HWORD;
typedef uint32		 		WORD;

// define of boolean type
typedef uint8				bool;
enum { false, true };


// fixed point macros
#define fix7_9											int16
#define fix7_9_ext										int32
#define fix13_3											int16
#define fix13_3											int16
#define fix10_6											int16
#define fix10_6_ext										int32
#define fix19_13										int32
#define fix17_15										int32


#define fixed_t											fix10_6
#define fixed_ext_t										fix10_6_ext

#if __FIXED_POINT_TYPE == 13
#undef fixed_t
#undef fixed_ext_t
#define fixed_t											fix19_13
#define fixed_ext_t										fix19_13
#endif

#if __FIXED_POINT_TYPE == 6
#undef fixed_t
#undef fixed_ext_t
#define fixed_t											fix10_6
#define fixed_ext_t										fix10_6_ext
#endif

#if __FIXED_POINT_TYPE == 9
#undef fixed_t
#undef fixed_ext_t
#define fixed_t											fix7_9
#define fixed_ext_t										fix7_9_ext
#endif

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
	fixed_t x;
	fixed_t y;
	fixed_t z;

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
	fixed_t x;
	fixed_t y;
	fixed_t z;

} Vector3D;

// spatial position
typedef struct Vector2D
{
	fixed_t x;
	fixed_t y;

} Vector2D;

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
	fixed_t x;

	// rotation around y axis
	fixed_t y;

	// rotation around z axis
	fixed_t z;

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
typedef struct NormalizedDirection
{
	int8 x;
	int8 y;
	int8 z;

} NormalizedDirection;

// engine's optical values structure
typedef struct Optical
{
	uint16 maximumXViewDistancePower;		// maximum distance from the screen to the infinite
	uint16 maximumYViewDistancePower;		// maximum distance from the screen to the infinite
	fixed_t cameraNearPlane;				// logical distance between the eyes and the screen
	fixed_t baseDistance;					// distance from left to right eye (depth perception)
	fixed_t horizontalViewPointCenter;		// horizontal View point center
	fixed_t verticalViewPointCenter;		// vertical View point center
	fixed_t scalingFactor;					// vertical View point center
	fixed_t halfWidth;						// screen width
	fixed_t halfHeight;						// screen height
	fixed_t aspectRatio;					// screen's width / screen's height
	fixed_t fov;							// 1 / tan (angle / 2)
	fixed_t aspectRatioXfov;				// aspectRatio * fov
	fixed_t farRatio1Near;	 				// (far + near) / (far - near)
	fixed_t farRatio2Near;	 				// (2 * far * near) / (near - far)
	fixed_ext_t projectionMultiplierHelper;	// helper to speed up projection operations 
	fixed_ext_t scalingMultiplier;			// scaling multiplier
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
typedef struct BgmapTextureSource
{
	int16 mx;
	int16 mp;
	int16 my;

} BgmapTextureSource;

typedef struct ObjectTextureSource
{
	int32 displacement;

} ObjectTextureSource;


typedef struct TexturePadding
{
	uint8 cols;
	uint8 rows;

} TexturePadding;

// a spatial description
typedef struct Transformation
{
	// spatial position
	Vector3D position;

	// spatial rotation
	Rotation rotation;

	// spatial scale
	Scale scale;

	// validity flag
	uint8 invalid;

} Transformation;

typedef struct RightBox
{
	/* left upper corner */
	fixed_t x0;
	fixed_t y0;
	fixed_t z0;

	/* right down corner */
	fixed_t x1;
	fixed_t y1;
	fixed_t z1;

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


/**
 * Camera frustum
 *
 * @memberof 	Camera
 */
typedef struct CameraFrustum
{
	/// x0 frustum
	int16 x0;
	/// y0 frustum
	int16 y0;
	/// z0 frustum
	int16 z0;
	/// x1 frustum
	int16 x1;
	/// y1 frustum
	int16 y1;
	/// z1 frustum
	int16 z1;

} CameraFrustum;


#endif
