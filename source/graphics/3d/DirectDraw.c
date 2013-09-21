/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
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

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <DirectDraw.h>
#include <Rect.h>
#include <Polygon.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// DirectDraw.c

#define DirectDraw_ATTRIBUTES					\
												\
	/* super's attributes */					\
	Object_ATTRIBUTES;							\
												\
	/* actuar frames per second */				\
	int fps;


// define the DirectDraw
__CLASS_DEFINITION(DirectDraw);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// class's constructor
static void DirectDraw_constructor(DirectDraw this);

// draw a pixel on the screen (DirectDraw)
static void DirectDraw_putPixel(DirectDraw this, u32 buffer, int x, int y, int pallet);

/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
__SINGLETON(DirectDraw);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void DirectDraw_constructor(DirectDraw this){
	
	__CONSTRUCT_BASE(Object);
	
	this->fps = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void DirectDraw_destructor(DirectDraw this){
	
	// allow a new construct
	__SINGLETON_DESTROY(Object);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// draw a pixel on the screen (DirectDraw)
static void DirectDraw_putPixel(DirectDraw this, u32 buffer, int x, int y, int pallet){
	
	// a pointer to the buffer
	//int* pointer = (int*)buffer;
	BYTE* pointer = (BYTE*)buffer;
	
	// calculate pixel position each column has 16 words, 
	// so 16 * 4 bytes = 64, 8 bytes are 4 pixels
	// pointer += x * 64 + y / 4;
	pointer += ((x << 6) + (y >> 2));
	
	// calculate the pixel to be draw
	*pointer |= (pallet << ((y & 3) << 1));
		
	//VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] & ~XPEN;	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// line draw algorithm from ....
void DirectDraw_lineFast(DirectDraw this, VBVec2D fromPoint, VBVec2D toPoint, int pallet){
	
	int dy = toPoint.y - fromPoint.y;
	int dx = toPoint.x - fromPoint.x;
	int stepx = 0, stepy = 0;
	fix19_13 parallax = ITOFIX19_13(fromPoint.parallax);
	 
	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	
	dy <<= 1;
	dx <<= 1;
	
	if(((unsigned)(fromPoint.x) < __SCREENWIDTH) && ((unsigned)(fromPoint.y) < __SCREENHEIGHT)){
	
		DirectDraw_putPixel(this, __LEFTBUFFER1, fromPoint.x - FIX19_13TOI(parallax), fromPoint.y, pallet);
		DirectDraw_putPixel(this, __RIGHTBUFFER1, fromPoint.x + FIX19_13TOI(parallax), fromPoint.y, pallet);
	}

	if (dx > dy) {
		
		int fraction = dy - (dx >> 1);
		
		fix19_13 parallaxStep = (dx >> 1)? FIX19_13_DIV(ITOFIX19_13(toPoint.parallax - fromPoint.parallax), ITOFIX19_13(abs(dx >> 1))): 0;
		
		while (fromPoint.x != toPoint.x) {
			
			if (fraction >= 0){
				
				fromPoint.y += stepy;
				fraction -= dx;	
			}
			
			fromPoint.x += stepx;
			fraction += dy;
			
			parallax += parallaxStep;
			
			if(((unsigned)(fromPoint.x) < __SCREENWIDTH) && ((unsigned)(fromPoint.y) < __SCREENHEIGHT)){                
				
				DirectDraw_putPixel(this, __LEFTBUFFER1, fromPoint.x - FIX19_13TOI(parallax), fromPoint.y, pallet);
				DirectDraw_putPixel(this, __RIGHTBUFFER1, fromPoint.x + FIX19_13TOI(parallax), fromPoint.y, pallet);
			}
		}
		
	}
	else{
	
		int fraction = dx - (dy >> 1);
		
		fix19_13 parallaxStep = (dy >> 1)? FIX19_13_DIV(ITOFIX19_13(toPoint.parallax - fromPoint.parallax), ITOFIX19_13(abs(dy >> 1))): 0;
		
		while (fromPoint.y != toPoint.y) {
		
			if (fraction >= 0) {
			
				fromPoint.x += stepx;
				fraction -= dy;
			}
			
			fromPoint.y += stepy;
			fraction += dx;
			
			parallax += parallaxStep;
			
			if(((unsigned)(fromPoint.x) < __SCREENWIDTH)&&((unsigned)(fromPoint.y) < __SCREENHEIGHT)){
				
				DirectDraw_putPixel(this, __LEFTBUFFER1, fromPoint.x - FIX19_13TOI(parallax), fromPoint.y, pallet);
				DirectDraw_putPixel(this, __RIGHTBUFFER1, fromPoint.x + FIX19_13TOI(parallax), fromPoint.y, pallet);
			}
		}
	}
}

