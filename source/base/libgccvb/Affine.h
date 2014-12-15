#ifndef AFFINE_H_
#define AFFINE_H_


//---------------------------------------------------------------------------------------------------------
// 											INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Types.h>
#include <Math.h>


//---------------------------------------------------------------------------------------------------------
// 											DECLARATIONS
//---------------------------------------------------------------------------------------------------------

void Affine_rotateScale(u8 world,s16 alpha, float zoom,s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y);
void Affine_rotateZ(u16 param, fix7_9 zoomX, fix7_9 zoomY, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y, int angle);
void Affine_rotateY(u16 param,s16 alpha, float zoomX, float zoomY, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y);
void Affine_scale(u16 param, fix7_9 zoomX, fix7_9 zoomY, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y);
void Affine_scaleProject(u16 param,s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y, float zoomX, float zoomY, float inc);


#endif