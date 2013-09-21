#ifndef AFFINE_H_
#define AFFINE_H_
/*---------------------------------INCLUDES--------------------------------*/
#include "types.h"
#include "video.h"
#include "mem.h"
#include "math.h"

/*---------------------------------FUNCTIONS-------------------------------*/

typedef struct AFFINE_ST {
    fix13_3	pb_y;		// *y+Dx /= 8.0
    s16		paralax;
    fix13_3	pd_y;		// *y+Dy /= 8.0
    fix7_9	pa;			// /=512.0
    fix7_9	pc;			// /=512.0
    u16 spacer[3];		//unknown
} AFFINE_ST ;

typedef struct PDx_ST {
	fix7_9 pa;
	fix13_3 pb;
	fix7_9 pc;
	fix13_3 pd;
	fix13_3 dx;
	fix13_3 dy;
	s16   paralax;
} PDx_ST ;

void affineSetAll(u16 param, PDx_ST * pdx,s16 max);
void affineRotateScale(u8 world,s16 alpha, float zoom,s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y);
void affineRotateZ(u16 param, fix7_9 zoomX, fix7_9 zoomY, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y, int angle);
//void affineRotateZ(u16 param,s16 alpha,float zoomX,float zoomY, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y);
void affineRotateY(u16 param,s16 alpha,float zoomX,float zoomY, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y);
void affineScale(u16 param, fix7_9 zoomX, fix7_9 zoomY, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y);

void affineSetAll0(u16 param, PDx_ST * pdx,s16 max,int  i);
void affineScaleProject(u16 param,s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y,float zoomX,float zoomY,float inc);

//#define affine_clear_all(w) affine_set_all(w, 1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0)


#endif // DT_AFFINE_H
