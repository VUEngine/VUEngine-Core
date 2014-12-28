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


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Affine.h>


//---------------------------------------------------------------------------------------------------------
// 											DEFINES
//---------------------------------------------------------------------------------------------------------

typedef struct AFFINE_ST
{
    fix13_3	pb_y;		// *y+Dx /= 8.0
    s16		paralax;
    fix13_3	pd_y;		// *y+Dy /= 8.0
    fix7_9	pa;			// /=512.0
    fix7_9	pc;			// /=512.0
    u16 spacer[3];		//unknown
} AFFINE_ST ;

typedef struct PDx_ST
{
	fix7_9 pa;
	fix13_3 pb;
	fix7_9 pc;
	fix13_3 pd;
	fix13_3 dx;
	fix13_3 dy;
	s16   paralax;
} PDx_ST ;


//---------------------------------------------------------------------------------------------------------
// 											FUNCTIONS
//---------------------------------------------------------------------------------------------------------

void Affine_setAll(u16 param, PDx_ST * pdx, s16 max)
{
	s16 i;
	AFFINE_ST *affine;
	affine = (AFFINE_ST*)PARAM(param);
	CACHE_ENABLE;
	for (i = 0; i < max; i++)
	{
		fix13_3 iFix = ITOFIX13_3(i);
		affine[i].pb_y    = FIX13_3_MULT(iFix, pdx->pb) + pdx->dx;
		affine[i].paralax = pdx->paralax;
		affine[i].pd_y    = FIX13_3_MULT(iFix , pdx->pd) + pdx->dy;
		affine[i].pa      = pdx->pa;
		affine[i].pc      = pdx->pc;
	}
	CACHE_DISABLE;
}

void Affine_scale(u16 param, fix7_9 zoomX, fix7_9 zoomY, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y)
{
	PDx_ST pdx;

	if (zoomX < 0)
	{
		fg_x *= (-1);
	}

	pdx.pa  = FIX7_9_DIV(ITOFIX7_9(1), zoomX);
	pdx.pb  = ITOFIX13_3(0);
	pdx.pc  = ITOFIX7_9(0);
	//pdx.pd  = FIX7_9TOFIX13_3(FIX7_9_DIV(ITOFIX7_9(1), zoomY));
	pdx.pd  = FIX19_13_DIV(ITOFIX19_13(1), FIX7_9TOFIX19_13(zoomY));

	pdx.dx = ITOFIX13_3(bg_x - (/*abs(FIX7_9TOI(pdx.pa)) **/ fg_x/* + FIX13_3TOI(pdx.pb) * fg_y*/));
	pdx.dy = ITOFIX13_3(bg_y-(/*FIX7_9TOI(pdx.pc) * fg_x + FIX13_3TOI(pdx.pd) * */fg_y));

	pdx.paralax = 0x0000;
	
	AFFINE_ST *affine = (AFFINE_ST*)PARAM(param);

	AFFINE_ST source = 
	{
			pdx.dx,
			pdx.paralax,
			pdx.dy,
			pdx.pa,
			pdx.pc
	};

	int i = FIX7_9TOI(FIX7_9_MULT(ITOFIX7_9(fg_y << 1), zoomY)) + 2;
	//int i = FIX7_9TOF(zoomY) * (fg_y << 1) + 2;
	
	if (0 > i)
	{
		i *= -1;
	}

	CACHE_ENABLE;
	for (; i--; ) 
	{
		// not sure why don't need following line
		//source.pb_y = FIX13_3_MULT(iFix, pdx.pb) + pdx.dx;
		
		//source.pd_y = FIX13_3_MULT(iFix, pdx.pd) + pdx.dy;
		source.pd_y = FIX19_13TOFIX13_3(FIX19_13_MULT(ITOFIX19_13(i), pdx.pd)) + pdx.dy;
		
		affine[i] = source;
	}
	CACHE_DISABLE;
}

//***FixMe, modify theas to update GX,GY to fit the image
// This means we need an image structure to contain the
// dimentions of the image, also add a copy_map fn to
// move a map into the map table and relocate the char pointer
// as needed.

void Affine_rotateZ(u16 param, fix7_9 zoomX, fix7_9 zoomY, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y, int alpha)
{
	PDx_ST pdx;

	CACHE_ENABLE;

	if (zoomX < 0)
	{
		fg_x *= (-1);
	}

	pdx.pa  = FIX7_9_DIV(COS(alpha), zoomX);
	pdx.pb  = FIX7_9TOFIX13_3(FIX7_9_DIV(SIN(alpha), zoomY));
	pdx.pc  = FIX7_9_DIV(SIN(alpha), zoomY);
	pdx.pd  = abs(FIX7_9TOFIX13_3(pdx.pa));

	pdx.dx = ITOFIX13_3(bg_x - (FIX7_9TOI(pdx.pa) * fg_x + FIX13_3TOI(pdx.pb) * fg_y));
	pdx.dy = ITOFIX13_3(bg_y - (FIX7_9TOI(pdx.pc) * fg_x + FIX13_3TOI(pdx.pd) * fg_y));

	pdx.paralax = 0x0000;

	{
		AFFINE_ST *affine = (AFFINE_ST*)PARAM(param);

		AFFINE_ST source =
	{
				pdx.dx,
				pdx.paralax,
				pdx.dy,
				pdx.pa,
				pdx.pc

		};

		int i = FIX7_9TOI(FIX7_9_MULT(ITOFIX7_9(fg_y << 1), zoomY)) + 2;

		affine[0] = source;

		for (; --i; )
		{
			fix13_3 iFix = ITOFIX13_3(i);
			source.pb_y = FIX13_3_MULT(iFix, pdx.pb) + pdx.dx;
			source.pd_y = FIX13_3_MULT(iFix, pdx.pd) + pdx.dy;
			affine[i] = source;
		}
	}
	CACHE_DISABLE;
}

void Affine_rotateScale(u8 world,s16 alpha, float zoom, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y)
{
	PDx_ST pdx;
	pdx.pb  = 0.0f;

	pdx.pa  = COSF(alpha)*(1.0f/zoom);
	pdx.pb -= SINF(alpha)*(1.0f/zoom);
	pdx.pc  = SINF(alpha)*(1.0f/zoom);
	pdx.pd  = pdx.pa;

	pdx.dx = bg_x-(pdx.pa*fg_x + pdx.pb*fg_y);
	pdx.dy = bg_y-(pdx.pc*fg_x + pdx.pd*fg_y);

	pdx.paralax = 0x00FF;

	Affine_setAll(world,&pdx,fg_y);
}

//rotate over the y axis
void affineRotateZ1(u16 param,s16 alpha, float zoomX, float zoomY, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y)
{
	PDx_ST pdx;
	pdx.pb  = 0.0f;

	pdx.pa  = COSF(alpha)/(zoomX);
	pdx.pb -= SINF(alpha);
	pdx.pc  = SINF(alpha);
	pdx.pd  = pdx.pa/(zoomY);

	pdx.dx = bg_x-(pdx.pa*fg_x + pdx.pb*fg_y);
	pdx.dy = bg_y-(pdx.pc*fg_x + pdx.pd*fg_y);

	pdx.paralax = 0;

	Affine_setAll(param,&pdx,fg_y<<1);
}

//rotate over the z axis
void Affine_rotateY(u16 param,s16 alpha, float zoomX, float zoomY, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y)
{
	PDx_ST pdx;
	int direction=1;
	pdx.pa  = direction*COSF(alpha)*(fg_x/zoomX);
	pdx.pb  = 0.0f;
	pdx.pc  = 0.0f;
	pdx.pd  = 1.0f;

	pdx.dx = bg_x-(direction*pdx.pa*fg_x + 0);
	pdx.dy = bg_y-(0           + fg_y);
	pdx.paralax = 0;

	//delay(10);
	Affine_setAll(param,&pdx,(fg_y<<1)*zoomY);
}

void affineSetAll0(u16 param, PDx_ST * pdx,s16 max,int  i)
{
	AFFINE_ST *affine;
	affine = (AFFINE_ST*)PARAM(param);
	affine[i].pb_y    = FTOFIX13_3(i*pdx->pb+pdx->dx);
	affine[i].paralax = pdx->paralax;
	affine[i].pd_y    = FTOFIX13_3(i*pdx->pd+pdx->dy);
	affine[i].pa      = FTOFIX7_9(pdx->pa);
	affine[i].pc      = FTOFIX7_9(pdx->pc);
}

void Affine_scaleProject(u16 param, s16 bg_x, s16 bg_y, s16 fg_x, s16 fg_y, float zoomX, float zoomY, float inc)
{
	PDx_ST pdx;
	u16 i;
	//ent->map.zoom=1-((float)0.5/DES)*ent->gz;
	float paralax=0;
	for (i=0;i<fg_y*2;i++)
	{
		pdx.pa  = 1.0f/zoomX;
		pdx.pb  = 0.0f;
		pdx.pc  = 0.0f;
		pdx.pd  = 1.0f/zoomY;
		pdx.dx = bg_x-(fg_x + 0);
		pdx.dy = bg_y-(0           + fg_y);
		//zoomX-=0.0113f;
		paralax+=inc;
		pdx.paralax = paralax;
		affineSetAll0(param,&pdx,1,i);
	}
}