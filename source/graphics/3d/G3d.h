#ifdef __3D_ENGINE

#ifndef G3D_H_
#define G3D_H_


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <Oop.h>
#include <Types.h>


//---------------------------------------------------------------------------------------------------------
//												DEFINES
//---------------------------------------------------------------------------------------------------------

// Fixed point math
#define __G3D_FIXED_SHIFT 			3
#define __G3D_FIXED_SHIFT_PRECISION 8
#define __G3D_F_NUM_UP(X) 			((X)<<__G3D_FIXED_SHIFT)
#define __G3D_F_NUM_DN(X) 			((X)>>__G3D_FIXED_SHIFT)
#define __G3D_F_MUL(X,Y) 			__G3D_F_NUM_DN(((X)*(Y)))
#define __G3D_F_ADD(X,Y) 			((X)+(Y))
#define __G3D_F_SUB(X,Y) 			((X)-(Y))
#define __G3D_F_DIV(X,Y) 			(__G3D_F_NUM_UP(X)/(Y))
//#define __G3D_F_DIV(X,Y) 			((X)/__G3D_F_NUM_DN(Y))
#define __G3D_F_PRECISION_UP(X) 	((X)<<__G3D_FIXED_SHIFT_PRECISION)
#define __G3D_F_PRECISION_DN(X) 	((X)>>__G3D_FIXED_SHIFT_PRECISION)
#define __G3D_F_PRECISION_MUL(X,Y) 	__G3D_F_PRECISION_DN(((X)*(Y)))
#define __G3D_F_PRECISION_DIV(X,Y) 	(__G3D_F_PRECISION_UP(X)/(Y))

// Specs
#define __G3D_SCREEN_HEIGHT 		128
#define __G3D_SCREEN_WIDTH 			320
#define __G3D_FAR_Z 				__G3D_F_NUM_UP(8192)
#define __G3D_CACHE_ENABLE 			asm("mov 2,r15 \n ldsr r15,sr24":::"r15");
#define __G3D_CACHE_DISABLE 		asm("ldsr r0,sr24");
#define __G3D_PARALLAX_MAX 			20
#define __G3D_PARALLAX_SHIFT 		8

// Enable ASM code
#define __G3D_ASM_CODE


//---------------------------------------------------------------------------------------------------------
// 											TYPE DEFINITIONS
//---------------------------------------------------------------------------------------------------------

// Represents a vector or coordinate. 20 bytes size
typedef struct g3dVector3D
{
	// fixed point
	s32 x; 
	// fixed point
	s32 y; 
	// fixed point
	s32 z; 
	// screen x
	s32 sx; 
	// screen y
	s32 sy; 

} g3dVector3D;

typedef struct g3dObjectData
{
	// Size of vertex array portion
	s32 vertexSize; 
	// Size of line data portion
	s32 lineSize; 
	// Number of points per "face". Since we use only lines for wireframe this should always be 2
	s32 faceSize; 
	// Distinct vertices and line point index data
	const s32 data[]; 

} g3dObjectData;

// This can be used for collision detection 12 bytes size
typedef struct g3dCollisionCube
{
	s32 width;
	s32 height;
	s32 depth;

} g3dCollisionCube; 

typedef struct g3dObjectProperties
{
	// Is this g3dObject visible
	u32 visible; 
	// This is whether the g3dObject is clipped or not
	u32 clip;
	// Do we perform collision detection
	u32 detectCollision; 
	// Color of the line
	u32 lineColor;
	// State byte to be used for anything
	u32 state;
	// Cube data for collision detection
	g3dCollisionCube hitCube; 

} g3dObjectProperties; //32 bytes size

typedef struct g3dObject
{
	// Actual position inside the world (20 bytes)
	g3dVector3D worldPosition; 
	// worldPosition to move the g3dObject to (20 bytes)
	g3dVector3D moveTo; 
	// Not Fixed Point (x,y,z) rotation in degrees (20 bytes)
	g3dVector3D worldRotation; 
	// Not Fixed Point (x,y,z) incremental rotation in degrees (20 bytes)
	g3dVector3D rotation; 
	// Vector to move the g3dObject (20 bytes)
	g3dVector3D worldSpeed; 
	// Vector for increasing or decreasing velocity (20 bytes)
	g3dVector3D speed; 
	// Scale factor of g3dObject [Maximum value for each axis is 255] (20 bytes)
	g3dVector3D worldScale; 
	// Incremental Scale factor to apply each frame [Maximum value for each axis is 255] (20 bytes)
	g3dVector3D scale; 
	// (45 bytes)
	g3dObjectProperties properties; 
	// (4 bytes)
	g3dObjectData* objData; 
	// Used to chain g3dObjects together (4 bytes)
	struct g3dObject* parent; 

} g3dObject;

typedef struct g3dCamera
{
	// (20 bytes)
	g3dVector3D worldPosition; 
	// (20 bytes)
	g3dVector3D worldSpeed; 
	// (20 bytes)
	g3dVector3D speed; 
	// (20 bytes)
	g3dVector3D moveTo; 
	// (20 bytes)
	g3dVector3D target; 
	// Not Fixed Point (20 bytes)
	g3dVector3D worldRotation; 
	// Not Fixed Point (20 bytes)
	g3dVector3D rotation; 
	// (4 bytes)
	s32 d; 

} g3dCamera;


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

singleton class G3d : Object
{
	static G3d getInstance();

	// 3D Calculations/Functions
	void copyVector3d(g3dVector3D* from, g3dVector3D* to);
	void scale(g3dVector3D* factor, g3dVector3D* v, g3dVector3D* o);
	void rotateXAxis(s32 degrees, g3dVector3D* v, g3dVector3D* o);
	void rotateYAxis(s32 degrees, g3dVector3D* v, g3dVector3D* o);
	void rotateZAxis(s32 degrees, g3dVector3D* v, g3dVector3D* o);
	void rotateAllAxis(s32 rx, s32 ry, s32 rz, g3dVector3D* v, g3dVector3D* o);
	void translate(s32 x, s32 y, s32 z, g3dVector3D* v, g3dVector3D* o);
	void cameraRotateAllAxis(s32 rx, s32 ry, s32 rz, g3dVector3D* v, g3dVector3D* o);
	void cameraTranslate(s32 x, s32 y, s32 z, g3dVector3D* v, g3dVector3D* o);
	void initObject(g3dObject* o, g3dObjectData* objData);
	void moveObject(g3dObject* o);
	void moveCamera(g3dCamera* c);
	void calculateProjection(g3dVector3D* o);
	void clipZAxis(g3dVector3D* v1, g3dVector3D* v2);
	void clipObject(g3dObject* o);
	void detectCollision(g3dVector3D* position1, g3dCollisionCube* c1, g3dVector3D* position2, g3dCollisionCube* c2, u32* flag);

	// Core Drawing Functions
	void drawPoint(s32 x, s32 y, u8 color, s32 p);
	void drawLine(g3dVector3D* v1, g3dVector3D* v2, u8 color);
	void drawObject(g3dObject* o);
	void renderVector3d(g3dObject* obj, g3dVector3D* v, g3dVector3D* o, u8 initHitCube);
	void renderObject(g3dObject* o);
}


#endif

#endif