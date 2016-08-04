#ifdef __3D_ENGINE

#include <G3d.h>

//Game camera
camera cam;

//Vector memory area
vector3d vertexBuffer[200];
u32 numVertices=0;

//Frame Buffers
u32* currentFrameBuffer=(u32*)0x00008000;
u32* nextFrameBuffer=(u32*)0x00000000;

/*********************
Cosine and Sine tables in degrees multiplied by a factor of 8
Example to multiply 25 by cos(8) (degrees)
Take (25*cosine[8])>>3.
*********************/

const s32 cosine[360]=
{
	8,8,8,8,8,8,8,8,
	8,8,8,8,8,8,8,8,
	8,8,8,8,8,7,7,7,
	7,7,7,7,7,7,7,7,
	7,7,7,7,6,6,6,6,
	6,6,6,6,6,6,6,5,
	5,5,5,5,5,5,5,5,
	4,4,4,4,4,4,4,4,
	4,3,3,3,3,3,3,3,
	2,2,2,2,2,2,2,2,
	1,1,1,1,1,1,1,0,
	0,0,0,-0,-0,-0,-1,-1,
	-1,-1,-1,-1,-1,-2,-2,-2,
	-2,-2,-2,-2,-2,-3,-3,-3,
	-3,-3,-3,-3,-4,-4,-4,-4,
	-4,-4,-4,-4,-4,-5,-5,-5,
	-5,-5,-5,-5,-5,-5,-6,-6,
	-6,-6,-6,-6,-6,-6,-6,-6,
	-6,-7,-7,-7,-7,-7,-7,-7,
	-7,-7,-7,-7,-7,-7,-7,-7,
	-8,-8,-8,-8,-8,-8,-8,-8,
	-8,-8,-8,-8,-8,-8,-8,-8,
	-8,-8,-8,-8,-8,-8,-8,-8,
	-8,-8,-8,-8,-8,-8,-8,-8,
	-8,-8,-8,-8,-8,-8,-8,-8,
	-8,-7,-7,-7,-7,-7,-7,-7,
	-7,-7,-7,-7,-7,-7,-7,-7,
	-6,-6,-6,-6,-6,-6,-6,-6,
	-6,-6,-6,-5,-5,-5,-5,-5,
	-5,-5,-5,-5,-4,-4,-4,-4,
	-4,-4,-4,-4,-4,-3,-3,-3,
	-3,-3,-3,-3,-2,-2,-2,-2,
	-2,-2,-2,-2,-1,-1,-1,-1,
	-1,-1,-1,-0,-0,-0,-0,0,
	0,0,1,1,1,1,1,1,
	1,2,2,2,2,2,2,2,
	2,3,3,3,3,3,3,3,
	4,4,4,4,4,4,4,4,
	4,5,5,5,5,5,5,5,
	5,5,6,6,6,6,6,6,
	6,6,6,6,6,7,7,7,
	7,7,7,7,7,7,7,7,
	7,7,7,7,8,8,8,8,
	8,8,8,8,8,8,8,8,
	8,8,8,8,8,8,8,8
};
const s32 sine[360]=
{
	0,0,0,0,1,1,1,1,
	1,1,1,2,2,2,2,2,
	2,2,2,3,3,3,3,3,
	3,3,4,4,4,4,4,4,
	4,4,4,5,5,5,5,5,
	5,5,5,5,6,6,6,6,
	6,6,6,6,6,6,6,7,
	7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,8,8,
	8,8,8,8,8,8,8,8,
	8,8,8,8,8,8,8,8,
	8,8,8,8,8,8,8,8,
	8,8,8,8,8,8,8,8,
	8,8,8,8,8,8,8,7,
	7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,6,6,
	6,6,6,6,6,6,6,6,
	6,5,5,5,5,5,5,5,
	5,5,4,4,4,4,4,4,
	4,4,4,3,3,3,3,3,
	3,3,2,2,2,2,2,2,
	2,2,1,1,1,1,1,1,
	1,0,0,0,0,-0,-0,-0,
	-1,-1,-1,-1,-1,-1,-1,-2,
	-2,-2,-2,-2,-2,-2,-2,-3,
	-3,-3,-3,-3,-3,-3,-4,-4,
	-4,-4,-4,-4,-4,-4,-4,-5,
	-5,-5,-5,-5,-5,-5,-5,-5,
	-6,-6,-6,-6,-6,-6,-6,-6,
	-6,-6,-6,-7,-7,-7,-7,-7,
	-7,-7,-7,-7,-7,-7,-7,-7,
	-7,-7,-8,-8,-8,-8,-8,-8,
	-8,-8,-8,-8,-8,-8,-8,-8,
	-8,-8,-8,-8,-8,-8,-8,-8,
	-8,-8,-8,-8,-8,-8,-8,-8,
	-8,-8,-8,-8,-8,-8,-8,-8,
	-8,-8,-8,-7,-7,-7,-7,-7,
	-7,-7,-7,-7,-7,-7,-7,-7,
	-7,-7,-6,-6,-6,-6,-6,-6,
	-6,-6,-6,-6,-6,-5,-5,-5,
	-5,-5,-5,-5,-5,-5,-4,-4,
	-4,-4,-4,-4,-4,-4,-4,-3,
	-3,-3,-3,-3,-3,-3,-2,-2,
	-2,-2,-2,-2,-2,-2,-1,-1,
	-1,-1,-1,-1,-1,-0,-0,-0
};

/****************************
Copies one vector to another
*****************************/
void inline G3d_copyVector3d(vector3d* from, vector3d* to)
{
#ifndef __ASM_CODE
	to->x = from->x;
	to->y = from->y;
	to->z = from->z;
	to->sx = from->sx;
	to->sy = from->sy;
#else
	asm(
	"ld.w 0x0[%[from]],  r6\n"
	"st.w r6,            0x0[%[to]]\n"
	"ld.w 0x4[%[from]],  r6\n"
	"st.w r6,            0x4[%[to]]\n"
	"ld.w 0x8[%[from]],  r6\n"
	"st.w r6,            0x8[%[to]]\n"
	"ld.w 0xc[%[from]],  r6\n"
	"st.w r6,            0xc[%[to]]\n"
	"ld.w 0x10[%[from]], r6\n"
	"st.w r6,            0x10[%[to]]\n"
	:
	:[from] "r" ((vector3d*)from), [to] "r" ((vector3d*)to)
	:"r6"
	);
#endif
}

/****************************
Scales a vector by a given fixed point vector
*****************************/
void inline G3d_scale(vector3d* factor, vector3d* v, vector3d* o)
{
	if(factor->x == 1 && factor->y == 1 && factor->z == 1)
	{
		G3d_copyVector3d(v,o);
		return;
	}
#ifndef __ASM_CODE
	o->x = F_MUL(v->x,factor->x);
	o->y = F_MUL(v->y,factor->y);
	o->z = F_MUL(v->z,factor->z);
#else
	asm(
	"ld.w 0[%[factor]],    r5\n"
	"ld.w 0[%[v]],         r6\n"
	"mul r5,               r6\n"
	"mov r6,               r7\n"
	"sar %[fshiftm1],      r7\n"
	"andi 0x01,            r7,   r7\n"
	"sar %[fshift],        r6\n"
	"add r6,               r7\n"
	"st.w r7,              0[%[o]]\n"
	"ld.w 4[%[factor]],    r5\n"
	"ld.w 4[%[v]],         r6\n"
	"mul r5,               r6\n"
	"mov r6,               r7\n"
	"sar %[fshiftm1],      r7\n"
	"andi 0x01,            r7,   r7\n"
	"sar %[fshift],        r6\n"
	"add r6,               r7\n"
	"st.w r7,              4[%[o]]\n"
	"ld.w 8[%[factor]],    r5\n"
	"ld.w 8[%[v]],         r6\n"
	"mul r5,               r6\n"
	"mov r6,               r7\n"
	"sar %[fshiftm1],      r7\n"
	"andi 0x01,            r7,  r7\n"
	"sar %[fshift],        r6\n"
	"add r6,               r7\n"
	"st.w r7,              8[%[o]]\n"
	:
	:[factor] "r" ((vector3d*)factor), [v]        "r" ((vector3d*)v)    , [o] "r" ((vector3d*)o),
	 [fshift] "r" (FIXED_SHIFT)      , [fshiftm1] "r" (FIXED_SHIFT-1)
	:"r5","r6","r7"
	);
#endif
}

/***********************************
Rotates a point around the X axis
************************************/
void  G3d_rotateXAxis(s32 degrees, vector3d* v, vector3d* o)
{
	o->x = v->x;
#ifndef __ASM_CODE
	o->z = (F_MUL(v->z, cosine[degrees])) + (F_MUL(sine[degrees], v->y));
	o->y = (F_MUL(v->z, -sine[degrees])) + (F_MUL(cosine[degrees], v->y));
#else
	asm(
	"ld.w %[vy],           r7\n"
	"ld.w %[vz],           r8\n"
	"ld.w %[cos],          r10\n"
	"ld.w %[sin],          r11\n"
	//F_MUL(v->z, cosine[degrees])
	"mov r8,               r12\n"
	"mul r10,              r12\n"
	"sar %[fixShift],      r12\n"
	//+ (F_MUL(sine[degrees],v->y))
	"mov r7,               r13\n"
	"mul r11,              r13\n"
	"sar %[fixShift],      r13\n"
	"add r12,              r13\n"
	"st.w r13,             0x08[%[o]]\n"
	//F_MUL(v->z, -sine[degrees])
	"not r11,              r11\n"
	"addi 1,r11,           r11\n"
	"mov r8,               r12\n"
	"mul r11,              r12\n"
	"sar %[fixShift],      r12\n"
	//+ F_MUL(cosine[degrees], v->y)
	"mov r10,              r13\n"
	"mul r7,               r13\n"
	"sar %[fixShift],      r13\n"
	"add r12,              r13\n"
	"st.w r13,             0x04[%[o]]\n"
	:/*output*/
	:  [vy]  "m" (v->y)           , [vz]  "m" (v->z)         , [o]        "r" ((vector3d*)o)
	 , [cos] "m" (cosine[degrees]), [sin] "m" (sine[degrees]), [fixShift] "i" (FIXED_SHIFT)
	:"r7","r8","r10","r11","r12","r13"
	);
#endif
}

/**********************************
Rotates a point around the Y axis
***********************************/
void  G3d_rotateYAxis(s32 degrees, vector3d* v, vector3d* o)
{
	o->y = v->y;
#ifndef __ASM_CODE
	o->x = (F_MUL(v->x, cosine[degrees])) + (F_MUL(sine[degrees], v->z));
	o->z = (F_MUL(v->x, -sine[degrees])) + (F_MUL(cosine[degrees], v->z));
#else
	asm(
	"ld.w %[vz],                r7\n"
	"ld.w %[vx],                r8\n"
	"ld.w %[cos],               r10\n"
	"ld.w %[sin],               r11\n"
	//F_MUL(v->x, cosine[degrees])
	"mov r8,                    r12\n"
	"mul r10,                   r12\n"
	"sar %[fixShift],           r12\n"
	//+ (F_MUL(sine[degrees],v->z))
	"mov r7,                    r13\n"
	"mul r11,                   r13\n"
	"sar %[fixShift],           r13\n"
	"add r12,                   r13\n"
	"st.w r13,                  0x00[%[o]]\n"
	//F_MUL(v->x, -sine[degrees])
	"not r11,                   r11\n"
	"addi 1,                    r11,       r11\n"
	"mov r8,                    r12\n"
	"mul r11,                   r12\n"
	"sar %[fixShift],           r12\n"
	//+ F_MUL(cosine[degrees], v->z)
	"mov r10,                   r13\n"
	"mul r7,                    r13\n"
	"sar %[fixShift],           r13\n"
	"add r12,                   r13\n"
	"st.w r13,                  0x08[%[o]]\n"
	:/*output*/
	: [vz]  "m" (v->z)            , [vx] "m" (v->x)          , [o]        "r" ((vector3d*)o)
	 ,[cos] "m" (cosine[degrees]), [sin] "m" (sine[degrees]) , [fixShift] "i" (FIXED_SHIFT)
	:"r7","r8","r10","r11","r12","r13"
	);
#endif
}

/**********************************
Rotates a point around the Z axis
***********************************/
void  G3d_rotateZAxis(s32 degrees, vector3d* v, vector3d* o)
{
	o->z = v->z;
#ifndef __ASM_CODE
	o->x = (F_MUL(v->x, cosine[degrees])) + (F_MUL(sine[degrees], v->y));
	o->y = (F_MUL(v->x, -sine[degrees])) + (F_MUL(cosine[degrees], v->y));
#else
	asm(
	"ld.w %[vy],r7\n"
	"ld.w %[vx],r8\n"
	"ld.w %[cos],r10\n"
	"ld.w %[sin],r11\n"
	//F_MUL(v->x, cosine[degrees])
	"mov r8,r12\n"
	"mul r10,r12\n"
	"sar %[fixShift],r12\n"
	//+ (F_MUL(sine[degrees],v->y))
	"mov r7,r13\n"
	"mul r11,r13\n"
	"sar %[fixShift],r13\n"
	"add r12,r13\n"
	"st.w r13, 0x00[%[o]]\n"
	//F_MUL(v->x, -sine[degrees])
	"not r11,r11\n"
	"addi 1,r11,r11\n"
	"mov r8,r12\n"
	"mul r11,r12\n"
	"sar %[fixShift],r12\n"
	//+ F_MUL(cosine[degrees], v->y)
	"mov r10,r13\n"
	"mul r7,r13\n"
	"sar %[fixShift],r13\n"
	"add r12,r13\n"
	"st.w r13,0x04[%[o]]\n"
	:/*output*/
	: [vy]  "m" (v->y)           ,[vx]  "m" (v->x)         , [o]        "r" ((vector3d*)o)
	 ,[cos] "m" (cosine[degrees]),[sin] "m" (sine[degrees]), [fixShift] "i" (FIXED_SHIFT)
	:"r7","r8","r10","r11","r12","r13"
	);
#endif
}

/***********************************************
This will rotate a point around all 3 axis.
It performs checks on the rotation values to make sure the
values are not zero before performing the rotation calculations.
rx,ry,and rz are degrees and are NOT fixed point
Make sure the rotation values are between -360 and 360
************************************************/
void  G3d_rotateAllAxis(s32 rx, s32 ry, s32 rz, vector3d* v, vector3d* o)
{
	vector3d t;

	if(rx==0 && ry==0 && rz==0)
	{
		G3d_copyVector3d(v,o);
		return;
	}

	if(rx<0) rx=359+rx;
	if(ry<0) ry=359+ry;
	if(rz<0) rz=359+rz;

	G3d_copyVector3d(v,&t);
	if(ry != 0)
	{
		G3d_rotateYAxis(ry,&t,o);
		G3d_copyVector3d(o,&t);
	}
	if(rx != 0)
	{
		G3d_rotateXAxis(rx,&t,o);
		G3d_copyVector3d(o,&t);
	}
	if(rz != 0)
	{
		G3d_rotateZAxis(rz,&t,o);
		G3d_copyVector3d(o,&t);
	}
	G3d_copyVector3d(&t,o);
}

/*******************************
This translates or moves a point
********************************/
void  G3d_translate(s32 x, s32 y, s32 z, vector3d* v, vector3d* o)
{
#ifndef __ASM_CODE
	o->x = v->x + x;
	o->y = v->y + y;
	o->z = v->z + z;
#else
	asm(
	"ld.w 0[%[v]], r5\n"
	"add %[x],     r5\n"
	"st.w r5,      0[%[o]]\n"
	"ld.w 4[%[v]], r5\n"
	"add %[y],     r5\n"
	"st.w r5,      4[%[o]]\n"
	"ld.w 8[%[v]], r5\n"
	"add %[z],     r5\n"
	"st.w r5,      8[%[o]]\n"
	:
	:[x] "r" (x), [y] "r" (y), [z] "r" (z), [v] "r" ((vector3d*)v), [o] "r" ((vector3d*)o)
	:"r5"
	);
#endif
}

/********************************
This performs the rotations for the camera.
It just rotates a point in the opposite direction
of the cameras rotation angles.
*********************************/
void  G3d_cameraRotateAllAxis(s32 rx, s32 ry, s32 rz, vector3d* v, vector3d* o)
{
#ifndef __ASM_CODE
	rx = (~rx)+1;
	ry = (~ry)+1;
	rz = (~rz)+1;
#else
	asm(
	"ld.w %[rx],           r5\n"
	"not r5,               r5\n"
	"addi 0x01,            r5,      r5\n"
	"st.w r5,              %[rx]\n"
	"ld.w %[ry],           r5\n"
	"not r5,               r5\n"
	"addi 0x01,            r5,      r5\n"
	"st.w r5,              %[ry]\n"
	"ld.w %[rz],           r5\n"
	"not r5,               r5\n"
	"addi 0x01,            r5,      r5\n"
	"st.w r5,              %[rz]\n"
	:
	:[rx] "m" (rx), [ry] "m" (ry), [rz] "m" (rz)
	:"r5"
	);
#endif
	G3d_rotateAllAxis(rx,ry,rz,v,o);
}

/**********************************
This performs the camera translate or move by
calculating the difference between the camera's position
and the points position.
***********************************/
void  G3d_cameraTranslate(s32 x, s32 y, s32 z, vector3d* v, vector3d* o)
{
#ifndef __ASM_CODE
	o->x = v->x - x;
	o->y = v->y - y;
	o->z = v->z - z;
#else
	asm(
	"ld.w 0[%[v]], r5\n"
	"sub %[x],     r5\n"
	"st.w r5,      0[%[o]]\n"
	"ld.w 4[%[v]], r5\n"
	"sub %[y],     r5\n"
	"st.w r5,      4[%[o]]\n"
	"ld.w 8[%[v]], r5\n"
	"sub %[z],     r5\n"
	"st.w r5,      8[%[o]]\n"
	:
	:[x] "r" (x), [y] "r" (y), [z] "r" (z), [v] "r" ((vector3d*)v), [o] "r" ((vector3d*)o)
	:"r5"
	);
#endif
}

/**********************************
This performs all affine transformation
functions against a vector3d object
***********************************/
void G3d_renderVector3d(object* obj, vector3d* v, vector3d* o, u8 initHitCube)
{
	vector3d t;
	//Transformations
	G3d_scale(&obj->worldScale,v,&t);
	G3d_copyVector3d(&t,o);

	G3d_rotateAllAxis(obj->worldRotation.x,obj->worldRotation.y,obj->worldRotation.z,o,&t);
	G3d_copyVector3d(&t,o);

	G3d_translate(obj->worldPosition.x,obj->worldPosition.y,obj->worldPosition.z,o,&t);
	G3d_copyVector3d(&t,o);

	if(cam.worldRotation.x != 0 || cam.worldRotation.y != 0 || cam.worldRotation.z != 0)
	{
		G3d_cameraRotateAllAxis(cam.worldRotation.x,cam.worldRotation.y,cam.worldRotation.z,o,&t);
		G3d_copyVector3d(&t,o);
	}

	G3d_cameraTranslate(cam.worldPosition.x,cam.worldPosition.y,cam.worldPosition.z,o,&t);
	G3d_copyVector3d(&t,o);
}

void G3d_calculateProjection(vector3d* o)
{
#ifndef __ASM_CODE
	o->sx = F_NUM_DN(F_ADD(F_DIV(F_MUL(o->x,cam.d),F_ADD(cam.d,o->z)),F_NUM_UP(SCREEN_WIDTH>>1)));
	o->sy = F_NUM_DN(F_ADD(F_DIV(F_MUL(o->y,cam.d),F_ADD(cam.d,o->z)),F_NUM_UP(SCREEN_HEIGHT>>1)));
	o->sy = SCREEN_HEIGHT - o->sy;//flip y axis
#else
	asm volatile(
	"ld.w %[ox],              r6\n"
	"ld.w %[oy],              r7\n"
	"ld.w %[oz],              r8\n"
	"ld.w %[camd],            r9\n"
	"movea %[scrHalfW],       r0,      r10\n"
	"movea %[scrHalfH],       r0,      r11\n"
	"movea %[scrH],           r0,      r12\n"
	//F_NUM_UP(SCREEN_WIDTH>>1)
	"shl %[fixShift],         r10\n"
	//F_NUM_UP(SCREEN_HEIGHT>>1)
	"shl %[fixShift],         r11\n"
	//F_MUL(o->x,cam.d)
	"shl 8,                   r6\n"                   //specific to cam.d of 256 and fixShift of 3 << 11 >> 3
	                                                  //This is a multiply of cam.d or shift left 11 and a F_NUM_DN of the fix point shift value of 3
	//F_MUL(o->y,cam.d)
	"shl 8,                   r7\n"                   //specific to cam.d of 256 and fixShift of 3 << 11 >> 3
	//F_ADD(cam.d, o->z)
	"add r9,                  r8\n"
	//F_DIV : sx
	"shl %[fixShift],         r6\n"
	"div r8,                  r6\n"                   //Need to find a way to eliminate this divide
	                                                  //Perhaps lookup table of some sort?
	//F_DIV : sy
	"shl %[fixShift],         r7\n"
	"div r8,                  r7\n"                   //Need to find a way to eliminate this divide
	//F_ADD : sx
	"add r10,                 r6\n"
	//F_ADD : sy
	"add r11,                 r7\n"
	//F_NUM_DN : sx
	"sar %[fixShift],         r6\n"
	"st.w r6,                 0x0[%[sx]]\n"
	//F_NUM_DN : sy
	"sar %[fixShift],         r7\n"
	"sub r7,                  r12\n"
	"st.w r12,                0x0[%[sy]]\n"
	://output
	: [ox]         "m" (o->x)             , [oy]       "m" (o->y)            , [oz]   "m" (o->z)
	 ,[scrHalfW]   "i" (SCREEN_WIDTH >> 1), [scrHalfH] "i" (SCREEN_HEIGHT>>1), [scrH] "i" (SCREEN_HEIGHT)
	 ,[camd]       "m" (cam.d)            , [sx]       "r" (&o->sx)          , [sy]   "r" (&o->sy)
	 ,[fixShiftm1] "i" (FIXED_SHIFT-1)    , [fixShift] "i" (FIXED_SHIFT)
	:"r6","r7","r8","r9","r10","r11","r12","r13"
	);
#endif
}

/**************************
This procedure actually draws an
object
**************************/
void G3d_drawObject(object* o)
{
	s32 vertices,lines,v,verts,i;
	vector3d v1;
	vector3d v2;
	vector3d* v1p;
	vector3d* v2p;
	vector3d* vtp;
	v1p = &v1;
	v2p = &v2;

	//Clip objects if needed
	G3d_clipObject(o);

	if(o->properties.visible == 0 || o->properties.clip == 1) return;

	vertices=o->objData->vertexSize;//total elements in array
	lines=o->objData->lineSize;//Total line endpoints
	verts=o->objData->faceSize;//total vertices per section

	v=0;
	i=0;

	//Put object through the render pipeline
	G3d_renderObject(o);
	/*vertices=o->objData->vertexSize;//total elements in array
	lines=o->objData->lineSize;//Total line endpoints
	verts=o->objData->faceSize;//total vertices per section

	v=0;
	i=0;
	//Load and render all distinct vertices into the vertex buffer;
	//This will render all object vertices based on the objects position,rotation etc..
	_CacheEnable
	while(v < vertices)
	{
		v1.x = o->objData->data[v];
		v1.y = o->objData->data[v+1];
		v1.z = o->objData->data[v+2];

		G3d_renderVector3d(o, &v1, &v2, ((v==0)?(1):(0)));

		vertexBuffer[i] = v2;
		i++;
		v+=3;
	};
	_CacheDisable*/

	//This reads the "faces" section of the data and draws lines between points.
	//We'll use the vertex buffer's already rendered vertices
	v = vertices;
	while(v < (lines+vertices))
	{
		v1p = &vertexBuffer[o->objData->data[v]];

		for(i=1; i<verts; i++)
		{
			v++;
			v2p = &vertexBuffer[o->objData->data[v]];

			if((v1p->z > cam.d) || (v2p->z > cam.d))
			{
				G3d_clipZAxis(v1p, v2p);
				G3d_calculateProjection(v1p);
				G3d_calculateProjection(v2p);
				G3d_drawLine(v1p,v2p,o->properties.lineColor);
			}
			vtp = v2p;
			v1p = v2p;
			v2p = vtp;

		}
		v++;
	}
}

void G3d_renderObject(object* o)
{
	u8 resetCube;
#ifndef __ASM_CODE
	s32 vertices, lines, verts, v, i;
	vector3d v1;
	vector3d v2;

	vertices=o->objData->vertexSize;//total elements in array

	v=0;
	i=0;
	//Load and render all distinct vertices into the vertex buffer;
	//This will render all object vertices based on the objects position,rotation etc..
	_CacheEnable
	while(v < vertices)
	{
		v1.x = o->objData->data[v];
		v1.y = o->objData->data[v+1];
		v1.z = o->objData->data[v+2];

		G3d_renderVector3d(o, &v1, &v2, ((v==0)?(1):(0)));

		vertexBuffer[i] = v2;
		i++;
		v+=3;
	};
	_CacheDisable
#else
	/*

	Max and Min values for sine and cosine are based on a signed byte so -128 -> 127.
	We'll store one value per byte within the registers
	r6 = rotation indicator flag
	r7 = cos[x],sine[x],cos[y],sine[y]
	r8 = cos[z],sine[z],cos[camx],sine[camx]
	r9 = cos[camy],sine[camy],cos[camz],sine[camz]

	r10 = worldposx
	r11 = worldposy
	r12 = worldposz
	r13 = camposx
	r14 = camposy
	r15 = camposz
	r16 = vertex count

	r17, r18 are scratch
	r19,r20,r12 are x,y,z vertex values
	*/
	asm volatile(
	//Now we'll store the cosine and sine values for the axis rotation angles
	//Well multiply the degree number by 4 (shl 2) to get the number of bytes for the offset in the sine and cosine arrays
	/*********************
	Cosine and sine of x axis
	**********************/
	"mov r0, r6\n"
	"objCosineSineX:\n"
	"ld.w 40[%[obj]], r17\n"                             //X axis rotation
	"or r17, r6\n"
	"cmp r0, r17\n"
	"bge _x_ge_0\n"
	"addi 359, r17, r17\n"                               //if x < 0 x = 359 + x
	"_x_ge_0:\n"
	"shl 2, r17\n"
	"mov r17, r18\n"
	"add %[cosine], r18\n"                               //Adds start address of cosine plus number of bytes offset for degrees
	"ld.w 0x00[r18], r18\n"                              //Get the cosine value
	"andi 0xFF, r18, r18\n"
	"shl 24, r18\n"                                      //Move cosine x to 1st byte of r18
	"mov r18, r7\n"                                      //Store cosine x in 1st byte of r7

	"mov r17, r18\n"                                     //Begin to get the sine value
	"add %[sine], r18\n"
	"ld.w 0x00[r18], r18\n"                              //Get the sine value
	"andi 0xFF, r18, r18\n"
	"shl 16, r18\n"                                      //Move sine x to 2nd byte of r18
	"or r18, r7\n"                                       //Store sine x in 2nd byte of r7
	/*****************************
	Cosine and sine of y axis
	******************************/
	"objCosineSineY:\n"
	"ld.w 44[%[obj]], r17\n"                             //Y axis rotation
	"or r17, r6\n"
	"cmp r0, r17\n"
	"bge _y_ge_0\n"
	"addi 359, r17, r17\n"                               //if y < 0 y = 359 + y
	"_y_ge_0:\n"
	"shl 2, r17\n"
	"mov r17, r18\n"
	"add %[cosine], r18\n"
	"ld.w 0x00[r18], r18\n"
	"andi 0xFF, r18, r18\n"
	"shl 8, r18\n"                                       //Move cosine of y to 3rd byte of r18
	"or r18, r7\n"                                       //Store cosine of y to 3rd byte of r7

	"mov r17, r18\n"
	"add %[sine], r18\n"
	"ld.w 0x00[r18], r18\n"
	"andi 0xFF, r18, r18\n"
	"or r18, r7\n"                                       //Store sine of y to 4th byte of r7
	/*****************************
	Cosine and sine of z axis
	******************************/
	"objCosineSineZ:\n"
	"ld.w 48[%[obj]], r17\n"                             //Z axis rotation
	"or r17, r6\n"
	"cmp r0, r17\n"
	"bge _z_ge_0\n"
	"addi 359, r17, r17\n"                               //if z < 0 z = 359 + z
	"_z_ge_0:\n"
	"shl 2, r17\n"
	"mov r17, r18\n"
	"add %[cosine], r18\n"
	"ld.w 0x00[r18], r18\n"
	"shl 24, r18\n"                                      //Move cosine z in 1st byte of r18
	"mov r18, r8\n"                                      //Store cosine z in 1st byte of r8

	"mov r17, r18\n"
	"add %[sine], r18\n"
	"ld.w 0x00[r18], r18\n"
	"andi 0xFF, r18, r18\n"
	"shl 16, r18\n"                                      //Move sine z in 2nd byte of r18
	"or r18, r8\n"                                       //Store sine z in 2nd byte of r8
	/********************************
	Camera cosine and sine of x axis
	*********************************/
	"camCosineSineX:\n"
	"ld.w 100[%[cam]], r17\n"
	"or r17, r6\n"
	"not r17, r17\n"
	"addi 0x01, r17, r17\n"                              //Twos complement
	"cmp r0, r17\n"
	"bge _cam_x_ge_0\n"
	"addi 359, r17, r17\n"                               //if x < 0 x = 359 + x
	"_cam_x_ge_0:\n"
	"shl 2, r17\n"
	"mov r17, r18\n"
	"add %[cosine], r18\n"
	"ld.w 0x00[r18], r18\n"
	"andi 0xFF, r18, r18\n"
	"shl 8, r18\n"                                       //Move cosine x to 3rd byte of r18
	"or r18, r8\n"                                       //Store cosine x to 3rd byte of r8

	"mov r17, r18\n"
	"add %[sine], r18\n"
	"ld.w 0x00[r18], r18\n"
	"andi 0xFF, r18, r18\n"
	"or r18, r8\n"                                       //Store sine x to 4th byte of r8
	/*****************************
	Camera cosine and sine of y axis
	******************************/
	"camCosineSineY:\n"
	"ld.w 104[%[cam]], r17\n"
	"or r17, r6\n"
	"not r17, r17\n"
	"addi 0x01, r17, r17\n"                              //Twos complement
	"cmp r0, r17\n"
	"bge _cam_y_ge_0\n"
	"addi 359, r17, r17\n"
	"_cam_y_ge_0:\n"
	"shl 2, r17\n"
	"mov r17, r18\n"
	"add %[cosine], r18\n"
	"ld.w 0x00[r18], r18\n"
	"andi 0xFF, r18, r18\n"
	"shl 24, r18\n"                                      //Move cosine y to 1st byte of r18
	"mov r18, r9\n"                                      //Store cosine y to 1st byte of r9

	"mov r17, r18\n"
	"add %[sine], r18\n"
	"ld.w 0x00[r18], r18\n"
	"andi 0xFF, r18, r18\n"
	"shl 16, r18\n"                                      //Move sine y to 2nd byte of r18
	"or r18, r9\n"                                       //Store sine y to 2nd byte of r9
	/******************************
	Camera cosine and sine of z axis
	*******************************/
	"camCosineSineZ:\n"
	"ld.w 108[%[cam]], r17\n"
	"or r17, r6\n"
	"not r17, r17\n"
	"addi 0x01, r17, r17\n"                              //Twos complement
	"cmp r0, r17\n"
	"bge _cam_z_ge_0\n"
	"addi 359, r17, r17\n"                               //if z < 0 z = 359 + z
	"_cam_z_ge_0:\n"
	"shl 2, r17\n"
	"mov r17, r18\n"
	"add %[cosine], r18\n"
	"ld.w 0x00[r18], r18\n"
	"andi 0xFF, r18, r18\n"
	"shl 8, r18\n"                                       //Move cosine z to 3rd byte of r18
	"or r18, r9\n"                                       //Store cosine z to 3rd byte of r9

	"mov r17, r18\n"
	"add %[sine], r18\n"
	"ld.w 0x00[r18], r18\n"
	"andi 0xFF, r18, r18\n"
	"or r18, r9\n"                                       //Store sine z to 4th byte of r9
	/*******************************
	Load the objects world position
	********************************/
	"objLoadPosition:\n"
	"ld.w 0[%[obj]], r10\n"                              //X coordinate
	"ld.w 4[%[obj]], r11\n"                              //Y coordinate
	"ld.w 8[%[obj]], r12\n"                              //Z coordinate
	/*******************************
	Load the x, y, and z camera position values
	and make them their twos complement
	********************************/
	"camLoadPosition:\n"
	"ld.w 0x00[%[cam]], r13\n"                           //X coordinate
	"not r13, r13\n"
	"addi 0x01, r13, r13\n"                              //Twos complement
	"ld.w 0x04[%[cam]], r14\n"                           //Y coordinate
	"not r14, r14\n"
	"addi 0x01, r14, r14\n"
	"ld.w 0x08[%[cam]], r15\n"                           //Z coordinate
	"not r15, r15\n"
	"addi 0x01, r15, r15\n"
	/**********************************
	Loop through all the vertices and perform
	all the operations
	***********************************/
	"mov 2, r17\n"
	"ldsr r17, sr24\n"                                   //Enable Caching
	"_renderObjectTop:\n"                                //Top of loop
	"cmp r0, %[vertices]\n"
	"ble _verts_le_0\n"                                  //if vertices <= 0 quit
	/**********************************
	Get vertex
	***********************************/
	"objLoadVertex:\n"
	"ld.w 0x00[%[objData]], r19\n"                       //X value
	"ld.w 0x04[%[objData]], r20\n"                       //Y value
	"ld.w 0x08[%[objData]], r21\n"                       //Z value

	"addi -3, %[vertices], %[vertices]\n"                //Subtract 3 from vertices for loop
	/*******************************
	Perform scaling
	********************************/
	"objScaling:\n"
	"ld.w 120[%[obj]], r17\n"                            //X scale factor
	"cmp %[fixNumUp1], r17\n"
	"be _skip_scale_x\n"
	"mul r17, r19\n"                                     //Scale x by factor
	"sar %[fixShift], r19\n"
	"_skip_scale_x:\n"

	"ld.w 124[%[obj]], r17\n"                            //Y scale factor
	"cmp %[fixNumUp1], r17\n"
	"be _skip_scale_y\n"
	"mul r17, r20\n"                                     //Scale y by factor
	"sar %[fixShift], r20\n"
	"_skip_scale_y:\n"

	"ld.w 128[%[obj]], r17\n"                            //Z scale factor
	"cmp %[fixNumUp1], r17\n"
	"be _skip_scale_z\n"
	"mul r17, r21\n"                                     //Scale z by factor
	"sar %[fixShift], r21\n"
	"_skip_scale_z:\n"
	/******************************
	Check rotation flag and skip code
	if not needed
	*******************************/
	"cmp r0, r6\n"
	"be _skip_obj_rot_z\n"
	/**********************************
	Perform rotation for x axis
	***********************************/
	"objXAxisRot:\n"
	"mov r7, r17\n"
	"sar 24, r17\n"                                      //Get cosine x axis
	"mul r21, r17\n"                                     //z * cosine[degrees]
	"sar %[fixShift], r17\n"                             //F_NUM_DN

	"mov r7, r18\n"
	"shl 8, r18\n"
	"sar 24, r18\n"                                      //Get sine x axis
	"mul r20, r18\n"                                     //y * sine[degrees]
	"sar %[fixShift], r18\n"                             //F_NUM_DN
	"add r18, r17\n"                                     //r17 now contains new z value

	"mov r7, r18\n"
	"shl 8, r18\n"
	"sar 24, r18\n"                                      //Get sine x
	"not r18, r18\n"
	"addi 0x01, r18, r18\n"                              //-sine[degrees]
	"mul r21, r18\n"                                     //z * -sine[degrees]
	"sar %[fixShift], r18\n"                             //F_NUM_DN

	"mov r7, r22\n"
	"sar 24, r22\n"                                      //Get cosine x
	"mul r20, r22\n"                                     //y * cosine[degrees]
	"sar %[fixShift], r22\n"                             //F_NUM_DN
	"add r22, r18\n"                                     //r18 now contains new y value

	"mov r17, r21\n"                                     //Update z value
	"mov r18, r20\n"                                     //Update y value
	"_skip_obj_rot_x:\n"
	/****************************
	Perform rotation for y axis
	*****************************/
	"objYAxisRot:\n"
	"mov r7, r17\n"
	"shl 16, r17\n"
	"sar 24, r17\n"                                      //Get cosine of y axis rotation
	"mul r19, r17\n"                                     //x * cosine[degrees]
	"sar %[fixShift], r17\n"                             //F_NUM_DN

	"mov r7, r18\n"
	"shl 24, r18\n"
	"sar 24, r18\n"
	"mul r21, r18\n"
	"sar %[fixShift], r18\n"
	"add r18, r17\n"                                     //r17 now contains new value for x

	"mov r7, r18\n"                                      //Get the sine of degrees
	"shl 24, r18\n"
	"sar 24, r18\n"                                      //Sign extend sine
	"not r18, r18\n"
	"addi 0x01, r18, r18\n"                              //Two's complement of sine value
	"mul r19, r18\n"                                     //x * -sine[degrees]
	"sar %[fixShift], r18\n"                             //F_NUM_DN

	"mov r7, r22\n"
	"shl 16, r22\n"
	"sar 24, r22\n"                                      //Get cosine of y axis
	"mul r21, r22\n"                                     //z * cosine[degrees]
	"sar %[fixShift], r22\n"                             //F_NUM_DN
	"add r22, r18\n"                                     //r18 now contains new z value

	"mov r17, r19\n"                                     //Update new x value
	"mov r18, r21\n"                                     //Update new z value
	"_skip_obj_rot_y:\n"
	/***************************
	Perform rotation for z axis
	****************************/
	"objZAxisRot:\n"
	"mov r8, r17\n"
	"sar 24, r17\n"                                      //Get cosine z axis
	"mul r19, r17\n"                                     //x * cosine[degrees]
	"sar %[fixShift], r17\n"                             //F_NUM_DN

	"mov r8, r18\n"
	"shl 8, r18\n"
	"sar 24, r18\n"                                      //Get sine z axis
	"mul r20, r18\n"                                     //y * sine[degrees]
	"sar %[fixShift], r18\n"                             //F_NUM_DN
	"add r18, r17\n"                                     //r17 now contains new x value

	"mov r8, r18\n"
	"shl 8, r18\n"
	"sar 24, r18\n"                                      //Get sine of z
	"not r18, r18\n"
	"addi 0x01, r18, r18\n"                              //Two's complement
	"mul r19, r18\n"                                     //x * -sine[degrees]
	"sar %[fixShift], r18\n"                             //F_NUM_DN

	"mov r8, r22\n"
	"sar 24, r22\n"                                      //Get cosine z
	"mul r20, r22\n"                                     //y * cosine[degrees]
	"sar %[fixShift], r22\n"                             //F_NUM_DN
	"add r22, r18\n"                                     //r18 now contains new y value

	"mov r17, r19\n"                                     //Update x value
	"mov r18, r20\n"                                     //Update y value
	"_skip_obj_rot_z:\n"
	/***********************
	Translate object
	************************/
	"objTranslate:\n"
	"add r10, r19\n"                                     //Add x
	"add r11, r20\n"                                     //Add y
	"add r12, r21\n"                                     //Add z

	"cmp r0, r6\n"
	"be _skip_cam_rot_z\n"
	/***************************
	Camera rotation x axis
	****************************/
	"camRotXAxis:\n"
	"mov r8, r17\n"
	"shl 16, r17\n"
	"sar 24, r17\n"                                      //Get cosine x axis
	"mul r21, r17\n"                                     //z * cosine[degrees]
	"sar %[fixShift], r17\n"                             //F_NUM_DN

	"mov r8, r18\n"
	"shl 24, r18\n"
	"sar 24, r18\n"                                      //Get sine x axis
	"mul r20, r18\n"                                     //y * sine[degrees]
	"sar %[fixShift], r18\n"                             //F_NUM_DN
	"add r18, r17\n"                                     //r17 now contains new z value

	"mov r8, r18\n"
	"shl 24, r18\n"
	"sar 24, r18\n"                                      //Get sine x
	"not r18, r18\n"
	"addi 0x01, r18, r18\n"                              //-sine[degrees]
	"mul r21, r18\n"                                     //z * -sine[degrees]
	"sar %[fixShift], r18\n"                             //F_NUM_DN

	"mov r8, r22\n"
	"shl 16, r22\n"
	"sar 24, r22\n"                                      //Get cosine x
	"mul r20, r22\n"                                     //y * cosine[degrees]
	"sar %[fixShift], r22\n"                             //F_NUM_DN
	"add r22, r18\n"                                     //r18 now contains new y value

	"mov r17, r21\n"                                     //Update z value
	"mov r18, r20\n"                                     //Update y value
	"_skip_cam_rot_x:\n"
	/***************************
	Camera rotation y axis
	****************************/
	"camRotYAxis:\n"
	"mov r9, r17\n"
	"sar 24, r17\n"                                      //Get cosine of y axis rotation
	"mul r19, r17\n"                                     //x * cosine[degrees]
	"sar %[fixShift], r17\n"                             //F_NUM_DN

	"mov r9, r18\n"
	"shl 8, r18\n"
	"sar 24, r18\n"
	"mul r21, r18\n"
	"sar %[fixShift], r18\n"
	"add r18, r17\n"                                     //r17 now contains new value for x

	"mov r9, r18\n"                                      //Get the sine of degrees
	"shl 8, r18\n"
	"sar 24, r18\n"                                      //Sign extend sine
	"not r18, r18\n"
	"addi 0x01, r18, r18\n"                              //Two's complement of sine value
	"mul r19, r18\n"                                     //x * -sine[degrees]
	"sar %[fixShift], r18\n"                             //F_NUM_DN

	"mov r9, r22\n"
	"sar 24, r22\n"                                      //Get cosine of y axis
	"mul r21, r22\n"                                     //z * cosine[degrees]
	"sar %[fixShift], r22\n"                             //F_NUM_DN
	"add r22, r18\n"                                     //r18 now contains new z value

	"mov r17, r19\n"                                     //Update new x value
	"mov r18, r21\n"                                     //Update new z value
	"_skip_cam_rot_y:\n"
	/***************************
	Camera rotation z axis
	****************************/
	"camRotZAxis:\n"
	"mov r9, r17\n"
	"shl 16, r17\n"
	"sar 24, r17\n"                                      //Get cosine z axis
	"mul r19, r17\n"                                     //x * cosine[degrees]
	"sar %[fixShift], r17\n"                             //F_NUM_DN

	"mov r9, r18\n"
	"shl 24, r18\n"
	"sar 24, r18\n"                                      //Get sine z axis
	"mul r20, r18\n"                                     //y * sine[degrees]
	"sar %[fixShift], r18\n"                             //F_NUM_DN
	"add r18, r17\n"                                     //r17 now contains new x value

	"mov r9, r18\n"
	"shl 24, r18\n"
	"sar 24, r18\n"                                      //Get sine of z
	"not r18, r18\n"
	"addi 0x01, r18, r18\n"                              //Two's complement
	"mul r19, r18\n"                                     //x * -sine[degrees]
	"sar %[fixShift], r18\n"                             //F_NUM_DN

	"mov r9, r22\n"
	"shl 16, r22\n"
	"sar 24, r22\n"                                      //Get cosine z
	"mul r20, r22\n"                                     //y * cosine[degrees]
	"sar %[fixShift], r22\n"                             //F_NUM_DN
	"add r22, r18\n"                                     //r18 now contains new y value

	"mov r17, r19\n"                                     //Update x value
	"mov r18, r20\n"                                     //Update y value
	"_skip_cam_rot_z:\n"
	/***************************
	Camera translation
	****************************/
	"camTranslate:\n"
	"add r13, r19\n"
	"add r14, r20\n"
	"add r15, r21\n"
	/***************************
	Update vertex values in vertex buffer
	****************************/
	"objUpdateVertex:\n"
	"st.w r19, 0x00%[vertexBuff]\n"
	"st.w r20, 0x04%[vertexBuff]\n"
	"st.w r21, 0x08%[vertexBuff]\n"

	"addi 12, %[objData], %[objData]\n"                  //Add 12 to objData to point to next vertex
	"addi 20, %[vertexBuff], %[vertexBuff]\n"            //Add 20 to vertexBuffer to point to next open vector3d slot
	/***************************
	End of Loop
	****************************/
	"jr _renderObjectTop\n"                              //Continue looping
	"_verts_le_0:\n"                                     //End of loop
	"ldsr r0, sr24\n"                                    //Disable Caching
	:
	:[obj]         "r" (o),
	 [cam]         "r" (&cam),
	 [cosine]      "r" ((s32*)cosine),
	 [sine]        "r" ((s32*)sine),
	 [vertices]    "r" (o->objData->vertexSize),
	 [objData]     "r" (&o->objData->data),
	 [idx]         "r" (0),
	 [vertexBuff]  "r" (&vertexBuffer),
	 [fixShift]    "i" (FIXED_SHIFT),
	 [fixShiftM1]  "i" (FIXED_SHIFT-1),
	 [fixNumUp1]   "i" (F_NUM_UP(1)),
	 [collCube]    "r" (&o->properties.hitCube),
	 [resetCube]   "m" (resetCube)
	:"r6","r7","r8","r9","r10","r11","r12","r13","r14","r15","r16","r17","r18","r19","r20","r21","r22","r23"
	);
#endif
}

/**********************
Quick and dirty cube based
collision detection.
***********************/
void  G3d_detectCollision(vector3d* position1, collisionCube* c1, vector3d* position2, collisionCube* c2, u32* flag)
{
	s32 c1minX, c1maxX,
	    c1minY, c1maxY,
		c1minZ, c1maxZ,
		c2minX, c2maxX,
		c2minY, c2maxY,
		c2minZ, c2maxZ;

	c1minX = position1->x - (c1->width >> 1);
	c1maxX = position1->x + (c1->width >> 1);
	c1minY = position1->y - (c1->height >> 1);
	c1maxY = position1->y + (c1->height >> 1);
	c1minZ = position1->z - (c1->depth >> 1);
	c1maxZ = position1->z + (c1->depth >> 1);

	c2minX = position2->x - (c2->width >> 1);
	c2maxX = position2->x + (c2->width >> 1);
	c2minY = position2->y - (c2->height >> 1);
	c2maxY = position2->y + (c2->height >> 1);
	c2minZ = position2->z - (c2->depth >> 1);
	c2maxZ = position2->z + (c2->depth >> 1);

	if(c1minX > c2maxX ||
	   c1minY > c2maxY ||
	   c1minZ > c2maxZ ||
	   c1maxX < c2minX ||
	   c1maxY < c2minY ||
	   c1maxZ < c2maxZ
	) return;

	//compare c1 against c2
	if(
		(c1minX >= c2minX && c1minX <= c2maxX) ||
		(c1maxX >= c2minX && c1maxX <= c2maxX))
		{
			asm("compareC1toC2:\n");
			if(
				(c1minY >= c2minY && c1minY <= c2maxY) ||
				(c1maxY >= c2minY && c1maxY <= c2maxY))
	{
					if(
						(c1minZ >= c2minZ && c1minZ <= c2maxZ) ||
						(c1maxZ >= c2minZ && c1maxZ <= c2maxZ))
						{
							*flag = 1;
					}
			}
	}
	//compare c2 against c1
	if(
		(c2minX >= c1minX && c2minX <= c1maxX) ||
		(c2maxX >= c1minX && c2maxX <= c1maxX))
		{
			if(
				(c2minY >= c1minY && c2minY <= c1maxY) ||
				(c2maxY >= c1minY && c2maxY <= c1maxY))
	        {
					if(
						(c2minZ >= c1minZ && c2minZ <= c1maxZ) ||
						(c2maxZ >= c1minZ && c2maxZ <= c1maxZ))
						{
							*flag = 1;
					}
			}
	}
}

/************************************
Cheap little object clipping function.
If the center of the object is off the
screen we'll clip it.
*************************************/
void  G3d_clipObject(object* o)
{
	vector3d worldTemp, worldOrig;

	worldOrig.x = o->worldPosition.x;
	worldOrig.y = o->worldPosition.y;
	worldOrig.z = o->worldPosition.z;

	G3d_cameraTranslate(cam.worldPosition.x,cam.worldPosition.y,cam.worldPosition.z,&worldOrig,&worldTemp);
	G3d_copyVector3d(&worldTemp,&worldOrig);

	if(cam.worldRotation.x != 0 || cam.worldRotation.y != 0 || cam.worldRotation.z != 0)
	{
		G3d_cameraRotateAllAxis(cam.worldRotation.x,cam.worldRotation.y,cam.worldRotation.z,&worldOrig,&worldTemp);
		G3d_copyVector3d(&worldTemp,&worldOrig);
	}

	G3d_calculateProjection(&worldOrig);
	o->properties.clip = 0;
	if(worldOrig.sx < 0 || worldOrig.sx > SCREEN_WIDTH) o->properties.clip = 1;
	if(worldOrig.sy < -SCREEN_HEIGHT || worldOrig.sy > (SCREEN_HEIGHT<<1)) o->properties.clip = 1;
	if(worldOrig.z < (cam.d>>1) || worldOrig.z > FAR_Z) o->properties.clip = 1;
}

/*************************************
Clips the z axis of the vectors if needed
We are going to break up the line into equal
pieces. We'll determine which piece the
camera is closest to and clip the line
to the closest piece. Not an exact science
but should be ok for general purposes.
This function needs performed before the
projection calculation occurs.
**************************************/
void  G3d_clipZAxis(vector3d* v1, vector3d* v2)
{
	s32 fracz,fracx,fracy,diff,mult;
	vector3d* minV;
	vector3d* maxV;

	if(v1->z > cam.d && v2->z > cam.d) return;
	if(v1->z < cam.d && v2->z < cam.d) return;

	/****************
	Calculate 8 positions between the two z points and
	determine which position the camera is closest to
	*****************/
	if(v1->z < cam.d)
	{
		minV = v1;
		maxV = v2;
	}else {
		minV = v2;
		maxV = v1;
	}
	fracz = (maxV->z - minV->z) >> 4;
	//Get the fractional x portion
	fracx = (maxV->x - minV->x) >> 4;
	//Do same thing with y as we did with x
	fracy = (maxV->y - minV->y) >> 4;
	//Get the difference or length of the z axis line
	//From the minimum z value to the camara depth position
	diff = cam.d - minV->z;
	//Determine how many fractional portions there are
	//and set our multiplier. Division is 36 clock cycles
	//so it should be less expensive to just loop and subtract under most circumstances
	if(fracz > 0)
	{
		while(diff > 0)
		{
			diff -= fracz;
			mult++;
		}
		if(mult > 0)mult--;//The while loop always makes the mulitiplier one more than needed
	}

	minV->z = cam.d;
	//Set new x value
	minV->x += (fracx * mult);
	//Set new y value
	minV->y += (fracy * mult);
}

/*******************************
Draws a pixel onto the screen
*******************************/
void  drawPoint(s32 x, s32 y, u8 color, s32 p)
{
	if(y<0 || y>SCREEN_HEIGHT) return;
	if(x<0 || x>SCREEN_WIDTH) return;

	s32 loffset,roffset;
	u8 yleft;

	//Put a cap on parallax
	if(p>PARALLAX_MAX) p=PARALLAX_MAX;

	loffset = (((x-p)<<4) + (y>>4));
	roffset = (loffset + (p<<5));

	if(loffset>0x1800 || loffset<0) return;
	if(roffset>0x1800 || roffset<0) return;

	color &= 0x03;

	yleft = (y&0x0F)<<1;

	currentFrameBuffer[loffset] |= (color<<yleft);
	((u32*)(currentFrameBuffer+0x4000))[roffset] |= (color<<yleft);
}

/*******************************
My Line Algorithm (Brezenham based)
*******************************/
void /*__attribute__((section(".data")))*/ G3d_drawLine(vector3d* v1, vector3d* v2, u8 color)
{
	s32 vx,vy,vz,vx2,vy2;
	s32 dx, dy, dz;
	s32 sx,sy,sz,pixels,err;
	#ifdef __ASM_CODE
	s32 loffset,roffset;
	u8 yleft;
	#else
	s32 p;
	#endif

	vx = v1->sx;
	vy = v1->sy;
	vx2 = v2->sx;
	vy2 = v2->sy;

	dx=(~(vx - vx2)+1);
	dy=(~(vy - vy2)+1);
	dz=(~(F_NUM_DN(F_SUB(v1->z,v2->z))+1));
	//dz=(~(F_NUM_DN(F_SUB(vz,vz2))+1));

	sx=(dx<0)?(-1):(1);
	sy=(dy<0)?(-1):(1);
	sz=(dz<0)?(-1):(1);

	if(dx<0) dx=(~dx)+1;
	if(dy<0) dy=(~dy)+1;
	if(dz<0) dz=(~dz)+1;

	pixels=((dx>dy)?(dx):(dy))+1;
	vz = v1->z;
	_CacheEnable
	#ifndef __ASM_CODE
	if(dy<dx)
	{
		err=(dx>>1);
		sz=(sz)*(F_NUM_UP(dz)/((dx==0)?(1):(dx)));
		for(p=0;p<pixels;p++)
		{
			drawPoint(vx,vy,color,(F_NUM_DN(vz)>>PARALLAX_SHIFT));
			err+=dy;
			if(err>dx)
			{
				vy+=sy;
				err-=dx;
			}
			vz+=sz;
			vx+=sx;
		}
	}
	else
	{
		err=(dy>>1);
		sz=(sz)*(F_NUM_UP(dz)/((dy==0)?(1):(dy)));
		for(p=0;p<pixels;p++)
		{
			drawPoint(vx,vy,color,(F_NUM_DN(vz)>>PARALLAX_SHIFT));
			err+=dx;
			if(err>dy)
			{
				vx+=sx;
				err-=dy;
			}
			vz+=sz;
			vy+=sy;
		}
	}
	#else
	//The VB only runs at 20Mhz but it has 30 registers.
	//Lets use em all and squeeze out as much processing speed as possible.
	asm volatile(
	//Setup all the registers with initial values
	//r28,r29 are scratch for our purposes
	"ld.w %[dy],                r6\n"
	"ld.w %[dx],                r7\n"
	"ld.w %[err],               r8\n"
	"ld.w %[sz],                r9\n"
	"ld.w %[dz],                r10\n"
	"ld.w %[pixels],            r11\n"
	"ld.w %[vy],                r12\n"
	"ld.w %[sy],                r13\n"
	"ld.w %[vz],                r14\n"
	"ld.w %[vx],                r15\n"
	"ld.w %[sx],                r16\n"
	"ld.w %[loffset],           r17\n"
	"ld.w %[roffset],           r18\n"
	"ld.b %[yleft],             r19\n"
	"movea %[screenH],          r0,         r21\n"
	"movea %[screenW],          r0,         r22\n"
	"movea %[parallaxM],        r0,         r23\n"
	"ld.w %[fbLeft],            r24\n"
	"movea %[fbRightOff],       r0,         r25\n"
	"shl 0x02,                  r25\n"
	//if(dy<dx){
	"cmp r6,                    r7\n"
	"ble _dxltdy\n"
	//err=(dx>>1);
	"mov r7,                    r8\n"
	"sar 0x01,                  r8\n"
	//sz=(sz)*(F_NUM_UP(dz)/((dx==0)?(1):(dx)));
	"mov r7,                    r28\n"
	"cmp r0,                    r7\n"
	"bne _nextLine1\n"
	"mov 0x01,                  r28\n"
	"_nextLine1:\n"
	"mov r10,                   r29\n"
	"shl %[fixedShift],         r29\n"
	"div r28,                   r29\n"
	"mul r29,                   r9\n"
	//for(p=0;p<pixels;p++){
	"_lineLoop1Top:\n"
	"cmp r0,                    r11\n"
	"ble _lineLoop1End\n"
	"addi -1,                   r11,        r11\n"
	//drawPoint(vx,vy,color,(F_NUM_DN(vz)>>PARALLAX_SHIFT));
	//******************************************************
	"ld.b %[color],             r27\n"
	"mov r14,                   r26\n"
	"sar %[fixedShift],         r26\n"
	"sar %[parallaxShift],      r26\n"
	//if(y<0 || y>SCREEN_HEIGHT) return;
	"cmp r0,                    r12\n"
	"blt _endDrawPoint1\n"
	"cmp r12,                   r21\n"
	"blt _endDrawPoint1\n"
	//if(x<0 || x>SCREEN_WIDTH) return;
	"cmp r0,                    r15\n"
	"blt _endDrawPoint1\n"
	"cmp r15,                   r22\n"
	"blt _endDrawPoint1\n"
	//if(p>PARALLAX_MAX) p=PARALLAX_MAX;
		//"and r23,r26\n"
	"cmp r26,                   r23\n"
	"bgt _nextPoint1\n"
	"mov r23,                   r26\n"
	"_nextPoint1:\n"
	//loffset = (((x-p)<<4) + (y>>4));
	"mov r12,                   r17\n"
	"sar 0x04,                  r17\n"
	"mov r15,                   r28\n"
	"sub r26,                   r28\n"
	"shl 0x04,                  r28\n"
	"add r28,                   r17\n"
	//roffset = (loffset + (p<<5));
	"mov r26,                   r18\n"
	"shl 0x05,                  r18\n"
	"add r17,                   r18\n"
	//if(loffset>0x1800 || loffset<0) return;
	"movea 0x17FF,              r0,        r28\n"
	"cmp r28,                   r17\n"
	"bge _endDrawPoint1\n"
	"cmp r0,                    r17\n"
	"blt _endDrawPoint1\n"
	//if(roffset>0x1800 || roffset<0) return;
		//"movea 0x17FF,r0,r28\n"
	"cmp r28,                   r18\n"
	"bge _endDrawPoint1\n"
	"cmp r0,                    r18\n"
	"blt _endDrawPoint1\n"
	//color &= 0x03;
	"andi 0x03,                 r27,       r27\n"
	//yleft = (y&0x0F)<<1;
	"andi 0x0F,                 r12,       r19\n"
	"shl 0x01,                  r19\n"
	//currentFrameBuffer[loffset] |= (color<<yleft);
	"mov r17,                   r28\n"
	"shl 0x02,                  r28\n"
	"add r24,                   r28\n"
	"ld.w 0x0[r28],             r29\n"
	"shl r19,                   r27\n"
	"or r27,                    r29\n"
	"st.w r29,                  0x0[r28]\n"
	//((u32*)(currentFrameBuffer+0x4000))[roffset] |= (color<<yleft);
		//"movhi 0x01,r28,r28\n"
		//"add r18,r28\n"
		//"st.w r29,0x0[r28]\n"

	"mov r18,                   r28\n"
	"shl 0x02,                  r28\n"
	"add r25,                   r28\n"
	"add r24,                   r28\n"
	"ld.w 0x0[r28],             r29\n"
	"or r27,                    r29\n"
	"st.w r29,                  0x0[r28]\n"

	"_endDrawPoint1:\n"
	//******************************************************
	//err+=dy;
	"add r6,                    r8\n"
	//if(err>dx){
	"cmp r8,                    r7\n"
	"bgt _nextLine2\n"
	//vy+=sy;
	"add r13,                   r12\n"
	//err-=dx;
	"sub r7,                    r8\n"
	//}
	"_nextLine2:\n"
	//vz+=sz;
	"add r9,                    r14\n"
	//vx+=sx;
	"add r16,                   r15\n"
	//}
	"jr _lineLoop1Top\n"
	"_lineLoop1End:\n"
	//else{
	"_dxltdy:\n"
	//err=(dy>>1);
	"mov r6,                    r8\n"
	"sar 0x01,                  r8\n"
	//sz=(sz)*(F_NUM_UP(dz)/((dy==0)?(1):(dy)));
	"mov r6,                    r28\n"
	"cmp r0,                    r6\n"
	"bne _nextLine3\n"
	"mov 0x01,                  r28\n"
	"_nextLine3:\n"
	"mov r10,                   r29\n"
	"shl %[fixedShift],         r29\n"
	"div r28,                   r29\n"
	"mul r29,                   r9\n"
	//for(p=0;p<pixels;p++){
	"_lineLoop2Top:\n"
	"cmp r0,                    r11\n"
	"ble _lineLoop2End\n"
	"addi -1,                   r11,      r11\n"
	//drawPoint(vx,vy,color,(F_NUM_DN(vz)>>PARALLAX_SHIFT));
	//******************************************************
	"ld.b %[color],             r27\n"
	"mov r14,                   r26\n"
	"sar %[fixedShift],         r26\n"
	"sar %[parallaxShift],      r26\n"
	//if(y<0 || y>SCREEN_HEIGHT) return;
	"cmp r0,                    r12\n"
	"blt _endDrawPoint2\n"
	"cmp r12,                   r21\n"
	"blt _endDrawPoint2\n"
	//if(x<0 || x>SCREEN_WIDTH) return;
	"cmp r0,                    r15\n"
	"blt _endDrawPoint2\n"
	"cmp r15,                   r22\n"
	"blt _endDrawPoint2\n"
	//if(p>PARALLAX_MAX) p=PARALLAX_MAX;
		//"and r23,r26\n"
	"cmp r26,                   r23\n"
	"bgt _nextPoint2\n"
	"mov r23,                   r26\n"
	"_nextPoint2:\n"
	//loffset = (((x-p)<<4) + (y>>4));
	"mov r12,                   r17\n"
	"sar 0x04,                  r17\n"
	"mov r15,                   r28\n"
	"sub r26,                   r28\n"
	"shl 0x04,                  r28\n"
	"add r28,                   r17\n"
	//roffset = (loffset + (p<<5));
	"mov r26,                   r18\n"
	"shl 0x05,                  r18\n"
	"add r17,                   r18\n"
	//if(loffset>0x1800 || loffset<0) return;
	"movea 0x1800,              r0,       r28\n"
	"cmp r28,                   r17\n"
	"bge _endDrawPoint2\n"
	"cmp r0,                    r17\n"
	"blt _endDrawPoint2\n"
	//if(roffset>0x1800 || roffset<0) return;
	"movea 0x1800,              r0,       r28\n"
	"cmp r28,                   r18\n"
	"bge _endDrawPoint2\n"
	"cmp r0,                    r18\n"
	"blt _endDrawPoint2\n"
	//color &= 0x03;
	"andi 0x03,                 r27,      r27\n"
	//yleft = (y&0x0F)<<1;
	"andi 0x0F,                 r12,      r19\n"
	"shl 0x01,                  r19\n"
	//currentFrameBuffer[loffset] |= (color<<yleft);
	"mov r17,                   r28\n"
	"shl 0x02,                  r28\n"
	"add r24,                   r28\n"
	"ld.w 0x0[r28],             r29\n"
	"shl r19,                   r27\n"
	"or r27,                    r29\n"
	"st.w r29,                  0x0[r28]\n"
	//((u32*)(currentFrameBuffer+0x4000))[roffset] |= (color<<yleft);
		//"movhi 0x01,r28,r28\n"
		//"add r18,r28\n"
		//"st.w r29,0x0[r28]\n"

	"mov r18,                   r28\n"
	"shl 0x02,                  r28\n"
	"add r25,                   r28\n"
	"add r24,                   r28\n"
	"ld.w 0x0[r28],             r29\n"
	"or r27,                    r29\n"
	"st.w r29,                  0x0[r28]\n"

	"_endDrawPoint2:\n"
	//******************************************************
	//err+=dx;
	"add r7,                    r8\n"
	//if(err>dy){
	"cmp r8,                    r6\n"
	"bgt _nextLine4\n"
	//vx+=sx;
	"add r16,                   r15\n"
	//err-=dy;
	"sub r6,                    r8\n"
	//}
	"_nextLine4:\n"
	//vz+=sz;
	"add r9,                    r14\n"
	//vy+=sy;
	"add r13,                   r12\n"
	//}
	"jr _lineLoop2Top\n"
	"_lineLoop2End:\n"
	"_end:\n"
	:/*no output*/
	:[dy]            "m" (dy)                , [dx]         "m" (dx)          , [err]        "m" (err),
	 [sz]            "m" (sz)                , [dz]         "m" (dz)          , [pixels]     "m" (pixels),
	 [vy]            "m" (vy)                , [sy]         "m" (sy)          , [vz]         "m" (vz),
	 [vx]            "m" (vx)                , [sx]         "m" (sx)          , [fixedShift] "i" (FIXED_SHIFT),
	 [loffset]       "m" (loffset)           , [roffset]    "m" (roffset)     , [yleft]      "m" (yleft),
	 [screenH]       "i" (SCREEN_HEIGHT)     , [screenW]    "i" (SCREEN_WIDTH), [parallaxM]  "i" (PARALLAX_MAX),
	 [fbLeft]        "m" (currentFrameBuffer), [fbRightOff] "i" (0x4000)      , [color]      "m" (color),
	 [parallaxShift] "i" (PARALLAX_SHIFT)
	 :"r6","r7","r8","r9","r10","r11","r12","r13","r14","r15","r16","r17","r18","r19","r21","r22","r23","r24","r25","r26","r27","r28","r29"
	);
	#endif
	_CacheDisable
}


/*********************************
This initializes an object type
*********************************/
void  G3d_initObject(object* o, objectData* objData)
{
	o->worldPosition.x = 0;
	o->worldPosition.y = 0;
	o->worldPosition.z = 0;
	o->moveTo.x        = 0;
	o->moveTo.y        = 0;
	o->moveTo.z        = 0;
	o->worldRotation.x = 0;
	o->worldRotation.y = 0;
	o->worldRotation.z = 0;
	o->rotation.x      = 0;
	o->rotation.y      = 0;
	o->rotation.z      = 0;
	o->worldSpeed.x    = 0;
	o->worldSpeed.y    = 0;
	o->worldSpeed.z    = 0;
	o->speed.x         = 0;
	o->speed.y         = 0;
	o->speed.z         = 0;
	o->worldScale.x    = F_NUM_UP(1);
	o->worldScale.y    = F_NUM_UP(1);
	o->worldScale.z    = F_NUM_UP(1);
	o->scale.x         = 0;
	o->scale.y         = 0;
	o->scale.z         = 0;
	o->parent          = (object*)0x00;
	o->objData         = (objectData*)objData;

	o->properties.visible         = 1;
	o->properties.detectCollision = 0;
	o->properties.lineColor       = 3;
	o->properties.state           = 0;
}

void  G3d_moveObject(object* o)
{
	//Increment attributes
	if(o->rotation.x != 0)           o->worldRotation.x += o->rotation.x;
	if(o->rotation.y != 0)           o->worldRotation.y += o->rotation.y;
	if(o->rotation.z != 0)           o->worldRotation.z += o->rotation.z;

	if(o->speed.x != 0)              o->worldSpeed.x += o->speed.x;
	if(o->speed.y != 0)              o->worldSpeed.y += o->speed.y;
	if(o->speed.z != 0)              o->worldSpeed.z += o->speed.z;

	if(o->scale.x != 0)              o->worldScale.x += o->scale.x;
	if(o->scale.y != 0)              o->worldScale.y += o->scale.y;
	if(o->scale.z != 0)              o->worldScale.z += o->scale.z;

	//Check rotation angles
	while(o->worldRotation.x > 359)  o->worldRotation.x -= 360;
	while(o->worldRotation.y > 359)  o->worldRotation.y -= 360;
	while(o->worldRotation.z > 359)  o->worldRotation.z -= 360;
	while(o->worldRotation.x < -359) o->worldRotation.x += 360;
	while(o->worldRotation.y < -359) o->worldRotation.y += 360;
	while(o->worldRotation.z < -359) o->worldRotation.z += 360;

	//Move the object based on moveto and speed
	if(o->moveTo.x != o->worldPosition.x)
	{
		if(o->worldPosition.x < o->moveTo.x)
		{
			o->worldPosition.x += o->worldSpeed.x;
			if(o->worldPosition.x > o->moveTo.x) o->worldPosition.x = o->moveTo.x;
		}
		else
		{
			o->worldPosition.x -= o->worldSpeed.x;
			if(o->worldPosition.x < o->moveTo.x) o->worldPosition.x = o->moveTo.x;
		}
	}
	if(o->moveTo.y != o->worldPosition.y)
	{
		if(o->worldPosition.y < o->moveTo.y)
		{
			o->worldPosition.y += o->worldSpeed.y;
			if(o->worldPosition.y > o->moveTo.y) o->worldPosition.y = o->moveTo.y;
		}
		else
		{
			o->worldPosition.y -= o->worldSpeed.y;
			if(o->worldPosition.y < o->moveTo.y) o->worldPosition.y = o->moveTo.y;
		}
	}
	if(o->moveTo.z != o->worldPosition.z)
	{
		if(o->worldPosition.z < o->moveTo.z)
		{
			o->worldPosition.z += o->worldSpeed.z;
			if(o->worldPosition.z > o->moveTo.z) o->worldPosition.z = o->moveTo.z;
		}
		else
		{
			o->worldPosition.z -= o->worldSpeed.z;
			if(o->worldPosition.z < o->moveTo.z) o->worldPosition.z = o->moveTo.z;
		}
	}
}

void  G3d_moveCamera(camera* c)
{
	//Do increments
	if(c->rotation.x != 0)   c->worldRotation.x += c->rotation.x;
	if(c->rotation.y != 0)   c->worldRotation.y += c->rotation.y;
	if(c->rotation.z != 0)   c->worldRotation.z += c->rotation.z;

	if(c->speed.x != 0)      c->worldSpeed.x += c->speed.x;
	if(c->speed.y != 0)      c->worldSpeed.y += c->speed.y;
	if(c->speed.z != 0)      c->worldSpeed.z += c->speed.z;

	//Check rotation angles
	while(c->worldRotation.x > 359)  c->worldRotation.x -= 360;
	while(c->worldRotation.y > 359)  c->worldRotation.y -= 360;
	while(c->worldRotation.z > 359)  c->worldRotation.z -= 360;
	while(c->worldRotation.x < -359) c->worldRotation.x += 360;
	while(c->worldRotation.y < -359) c->worldRotation.y += 360;
	while(c->worldRotation.z < -359) c->worldRotation.z += 360;
	//Move camera based on moveto and speed
	if(c->moveTo.x != c->worldPosition.x)
	{
		if(c->worldPosition.x < c->moveTo.x)
		{
			c->worldPosition.x += c->worldSpeed.x;
			if(c->worldPosition.x > c->moveTo.x) c->worldPosition.x = c->moveTo.x;
		}
		else
		{
			c->worldPosition.x -= c->worldSpeed.x;
			if(c->worldPosition.x < c->moveTo.x) c->worldPosition.x = c->moveTo.x;
		}
	}
	if(c->moveTo.y != c->worldPosition.y)
	{
		if(c->worldPosition.y < c->moveTo.y)
		{
			c->worldPosition.y += c->worldSpeed.y;
			if(c->worldPosition.y > c->moveTo.y) c->worldPosition.y = c->moveTo.y;
		}
		else
		{
			c->worldPosition.y -= c->worldSpeed.y;
			if(c->worldPosition.y < c->moveTo.y) c->worldPosition.y = c->moveTo.y;
		}
	}
	if(c->moveTo.z != c->worldPosition.z)
	{
		if(c->worldPosition.z < c->moveTo.z)
		{
			c->worldPosition.z += c->worldSpeed.z;
			if(c->worldPosition.z > c->moveTo.z) c->worldPosition.z = c->moveTo.z;
		}
		else
		{
			c->worldPosition.z -= c->worldSpeed.z;
			if(c->worldPosition.z < c->moveTo.z) c->worldPosition.z = c->moveTo.z;
		}
	}
}

#endif
