#ifndef G3d_H_
#define G3d_H_
/*******************************
Fixed point math defines
*******************************/
#define FIXED_SHIFT 3
#define F_NUM_UP(X) ((X)<<FIXED_SHIFT)
#define F_NUM_DN(X) ((X)>>FIXED_SHIFT)
#define F_MUL(X,Y) F_NUM_DN(((X)*(Y)))
#define F_ADD(X,Y) ((X)+(Y))
#define F_SUB(X,Y) ((X)-(Y))
#define F_DIV(X,Y) (F_NUM_UP(X)/(Y))
//#define F_DIV(X,Y) ((X)/F_NUM_DN(Y))
#define FIXED_SHIFT_PRECISION 8
#define F_PRECISION_UP(X) ((X)<<FIXED_SHIFT_PRECISION)
#define F_PRECISION_DN(X) ((X)>>FIXED_SHIFT_PRECISION)
#define F_PRECISION_MUL(X,Y) F_PRECISION_DN(((X)*(Y)))
#define F_PRECISION_DIV(X,Y) (F_PRECISION_UP(X)/(Y))

//Definitions
#define SCREEN_HEIGHT 128
#define SCREEN_WIDTH 320
#define FAR_Z F_NUM_UP(8192)
#define _CacheEnable asm("mov 2,r15 \n ldsr r15,sr24":::"r15");
#define _CacheDisable asm("ldsr r0,sr24");
#define PARALLAX_MAX 20
#define PARALLAX_SHIFT 8


/**********************
Type definitions used in
the game functions
**********************/
typedef struct
{
	s32 x; //fixed point
	s32 y; //fixed point
	s32 z; //fixed point
	s32 sx; //screen x
	s32 sy; //screen y

} vector3d;//represents a vector or coordinate. 20 bytes size

typedef struct
{
	s32 vertexSize;//Size of vertex array portion
	s32 lineSize;//Size of line data portion
	s32 faceSize;//Number of points per "face". Since we use only lines for wireframe this should always be 2
	const s32 data[];//Distinct vertices and line point index data

} objectData;

typedef struct
{
	s32 width;
	s32 height;
	s32 depth;

}collisionCube; //This can be used for collision detection 12 bytes size

typedef struct
{
	u32 visible; //Is this object visible
	u32 clip;//This is whether the object is clipped or not
	u32 detectCollision; //Do we perform collision detection
	u32 lineColor;
	u32 state;//State byte to be used for anything
	collisionCube hitCube; //Cube data for collision detection

} objectProperties;//32 bytes size

typedef struct object
{
	vector3d worldPosition;//Actual position inside the world //20 bytes
	vector3d moveTo;//worldPosition to move the object to //20 bytes
	vector3d worldRotation;//Not Fixed Point (x,y,z) rotation in degrees //20 bytes
	vector3d rotation;//Not Fixed Point (x,y,z) incremental rotation in degrees //20 bytes
	vector3d worldSpeed;//Vector to move the object //20 bytes
	vector3d speed;//Vector for increasing or decreasing velocity //20 bytes
	vector3d worldScale;//Scale factor of object [Maximum value for each axis is 255] //20 bytes
	vector3d scale;//Incremental Scale factor to apply each frame [Maximum value for each axis is 255] //20 bytes
	objectProperties properties; //45 bytes
	objectData* objData; //4 bytes
	struct object* parent;//Used to chain objects together //4bytes

} object;

typedef struct
{
	vector3d worldPosition; //20 bytes
	vector3d worldSpeed; //20 bytes
	vector3d speed; //20 bytes
	vector3d moveTo; //20 bytes
	vector3d target; //20 bytes
	vector3d worldRotation;//Not Fixed Point //20 bytes
	vector3d rotation;//Not Fixed Point // 20 bytes
	s32 d; // 4

} camera;




/*********************************************************
3d Calculations/Functions
*********************************************************/
void  G3d_copyVector3d(vector3d* from, vector3d* to);
void  G3d_scale(vector3d* factor, vector3d* v, vector3d* o);
void  G3d_rotateXAxis(s32 degrees, vector3d* v, vector3d* o);
void  G3d_rotateYAxis(s32 degrees, vector3d* v, vector3d* o);
void  G3d_rotateZAxis(s32 degrees, vector3d* v, vector3d* o);
void  G3d_rotateAllAxis(s32 rx, s32 ry, s32 rz, vector3d* v, vector3d* o);
void  G3d_translate(s32 x, s32 y, s32 z, vector3d* v, vector3d* o);
void  G3d_cameraRotateAllAxis(s32 rx, s32 ry, s32 rz, vector3d* v, vector3d* o);
void  G3d_cameraTranslate(s32 x, s32 y, s32 z, vector3d* v, vector3d* o);
void  G3d_initObject(object* o, objectData* objData);
void  G3d_moveObject(object* o);
void  G3d_moveCamera(camera* c);
void  G3d_calculateProjection(vector3d* o);
void  G3d_clipZAxis(vector3d* v1, vector3d* v2);
void  G3d_clipObject(object* o);
void  G3d_detectCollision(vector3d* position1, collisionCube* c1, vector3d* position2, collisionCube* c2, u32* flag);
/********************************************************/
/********************************************************
Core Drawing Functions
*********************************************************/
void G3d_drawPoint(s32 x, s32 y, u8 color, s32 p);
void G3d_drawLine(vector3d* v1, vector3d* v2, u8 color);
void G3d_drawObject(object* o);
void G3d_renderVector3d(object* obj, vector3d* v, vector3d* o, u8 initHitCube);
void G3d_renderObject(object* o);
/********************************************************/

#endif