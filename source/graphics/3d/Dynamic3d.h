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

#ifndef DYNAMIC3D_H_
#define DYNAMIC3D_H_


/*---------------------------------INCLUDES--------------------------------*/
/*--------------------------------STRUCTURES-------------------------------*/
/*---------------------------------FUNCTIONS-------------------------------*/
//draw a pixel on the screen (DirectDraw)
/*
inline putPixel1(u32 buffer,int x,int y, int pallet){
	       
	buffer=LEFTBUFFER1;
	asm("mov 	%0,r6"	

		: // Output //
		: "r" (buffer) // Input //
		: "r6"// Clobber //
		);
	//load x position
	asm("mov 	%0,r7"
		: // Output //
		: "r" (x) // Input //
		: "r7" // Clobber //
		);
	//load y position
	asm("mov 	%0,r8"
		: // Output //
		: "r" (y) // Input //
		: "r8" // Clobber //
		);

	//load pallet
	asm("mov 	%0,r9"
		: // Output //
		: "r" (pallet) // Input //
		: "r9" // Clobber //
		);

	asm("mov	r8,r12");
	asm("shl	6,r7");
	asm("shr	2,r12");
	asm("add	r12,r7");
	asm("add	r7,r6");
	asm("mov	0x3,r11");
	asm("and	r11,r8");
	asm("shl	1,r8");
	asm("shl	r8,r9");
	asm("ld.h	0x0[r6],r10");
	asm("or		r10,r9");
	asm("st.b	r9,0x0[r6]");

}
*/
//draw a pixel on the screen (DirectDraw)
inline putPixel(u32 buffer,int x,int y, int pallet){
	int* pointer=(int *)buffer;
	//calculate pixel position
	//each column has 16 words, so 16*4 bytes=64
	//8 bytes are 4 pixels
	//pointer+=x*64 + y/4
	pointer+=(x<<6)+(y>>2); 	
	//calculate the pixel to be draw
	*pointer|=pallet<<((y&3)<<1);
//	VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] & ~XPEN; 	
}

//draw a line
inline unsigned int abs1(int number){
	//return number&0x7FFF;
	return number - ((number+number) & (number>>31));
}
/*
void lineFast1(int buffer,int x0, int y0, int x1, int y1, int pallet){
//	float m;
	int y;
	int i;
//	int x1,x2;
	int x, m, count;
	if(y0>y1){
		y=y0;
		y0=y1;
		y1=y;
		}
	m = (x1 - x0) << 16;
	m /= (y1 - y0);
	x = x1 << 16;                   
	for (count = y0; count < y1; count++){
		putPixel(buffer,(x>>16), count, pallet);
		//edge[count] = x >> 16;     
          x += m;
     }
	
	if(p1->x<p2->x){
		x1=p1->x;
	}
	else{
		x1=p2->x;
	}
     m = (p2->y - p1->y)/(p2->x - p1->x);
     for (i = x1; i<x2-x1; i++){
		edge1[i] = y;
		y += m;
	}		
*/
}

//line draw algorithm from ....
void lineFast(u32 buffer,int x0, int y0, int x1, int y1, int pallet){
	int pix = pallet;
	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;
	int fraction ;
	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;
	y0 *= 1;
	y1 *= 1;
	if(((unsigned)(x0)<384)&&((unsigned)(y0)<224))
	putPixel(buffer,x0, y0, pallet);
	if (dx > dy) {
		fraction = dy - (dx >> 1);
		while (x0 != x1) {
			if (fraction >= 0){
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			if(((unsigned)(x0)<384)&&((unsigned)(y0)<224))                
				putPixel(buffer,x0, y0, pallet);
			}
        	}else{
			int fraction = dx - (dy >> 1);
			while (y0 != y1) {
				if (fraction >= 0) {
					x0 += stepx;
					fraction -= dy;
				}
				 y0 += stepy;
				 fraction += dx;
				 if(((unsigned)(x0)<384)&&((unsigned)(y0)<224))           
					putPixel(buffer,x0, y0, pallet);
			}
		}
}
/*SOLID DEFINITIONS    
	To define a solid, write each vertex in the form
		x0,	y0,	z0,
		x1,	y1,	z0,
		�	�	�
		�	�	�,
		xn,	yn,	zn,
	next define the solids center for rotation operations
		centerX,	 centerY,		CenterZ
	and last, define the polygons, 1 line per polygon with the vertex from above
		p0-v0,	p0-v1,	�	�,	p0-vn
		p1-v0,	p1-v1,	�	�,	p1-vn
		�		�		�	�	�
		�		�		�	�	�
		pn-v0,	pn-v1,	�	�,	pn-vn	

*/
//3d point structure	
struct Point3D{
	//x coordinate
	int x;
	//y coordinate
	int y;
	//z coordinate
	int  z;
};
//2d point structure 3d projection point for scren draw
struct Point2D{
	//x coordinate
	int x;
	//y coordinate
	int y;
	//paralax to create the stereoscopic effect
	int paralax;
};
//convert a 3d point into a 2d point to show on the screen
inline void set3DPoint(struct Point3D *p3d, struct Point2D *p2d){
	float gxp,gyp;
	float leftEyePoint,rightEyePoint;
	float leftEjeGx,rightEjeGx;
/*	int sign=1;
	if(p3d->x>=HVPC){
		sign=-1;
	}	
	sign=1;
	gxp=p3d->x+sign*abs2(HVPC-p3d->x)*(p3d->z)/MVD;
	sign=1;
	if(p3d->y>=VVPC){
		sign=-1;
	}
	sign=1;
	gyp=p3d->y+sign*abs2(VVPC-p3d->y)*(p3d->z)/MVD;	
*/	
//	gyp=p3d->y+(_verticalViewPointCenter-p3d->y)*(p3d->z+ZZERO)/_maximunViewDistance;	
	gxp=p3d->x+(_horizontalViewPointCenter-p3d->x)*(p3d->z+__ZZERO)/_maximunViewDistance;
	leftEyePoint=_horizontalViewPointCenter-(_baseDistance>>1);
	rightEyePoint=_horizontalViewPointCenter+(_baseDistance>>1);

	leftEjeGx=gxp-(gxp-leftEyePoint)*(p3d->z+__ZZERO)/(_distanceEyeScreen+p3d->z+__ZZERO);
	rightEjeGx=gxp+(rightEyePoint-gxp)*(p3d->z+__ZZERO)/(_distanceEyeScreen+p3d->z+__ZZERO);	
	
	p2d->paralax=(rightEjeGx-leftEjeGx)/2;
	p2d->x=gxp;
	p2d->y=gyp;
}
//polygon definition	
struct Polygon{	
	//vertices that the polygon uses
	struct Point3D *vertex[__MAXVERTEXS];	
	//polygons number of vertices 
	int numVertexs;					
	//pallet (00,01,10,11)
	int pallet;
};
//solid definition
struct Solid{
	//polygons that the solid is composed of
	struct Polygon poly[__MAXPOLYS];		
	//solids vertices 
	struct Point3D vertex[__MAXPOLYS];		
	//solid orginal vertexs
	struct Point3D orgVertex[__MAXPOLYS];		
	//spatial solids center
	struct Point3D center;				
	//number of polygons
	int numPolys;						
	//number of vertexs
	int numVertexs;					
	//state for screen refresh
	int state;							
	int directionX;
	int directionY;
	int directionZ;
	int direction;
	//keep record of drawed lines no dont redraw and reduce the time to draw the solid
	struct Point3D *drawLines[__MAXPOLYS*__MAXVERTEXS];			
	//index for the drawLines array
	int index;									
};
// struct that handles all solids created	
struct Solids{
	//pointer array to solids
	void *array[__MAXSOLIDS];
	//current number of defined solids
	unsigned int index;
}static Solids;
/*
typedef struct {
	NODE *next[8];	
	SOLID *data[8];
}NODE;
struct OCTREE{
	NODE root;
	
};
*/
//set solid date and load the vertexs and polygon definition
static void setSolidData(struct Solid *solid,int* definition,int numPolys, int numVertexs,int numVerPoly){
	register unsigned int i,j;
	register unsigned int aux;
	//if there is space for one more solid
	if(Solids.index<__MAXSOLIDS){		
		//update the solids pointer array
		Solids.array[Solids.index]=solid;	
		//increment the number of solids
		Solids.index++;
		//set the solids number of polygons
		solid->numPolys=numPolys;
		//set as active for screen refresh 
		solid->state=__ACTIVE;	
		//set solids number of vertexs	
		solid->numVertexs=numVertexs;	
		//initilize the index of drawned lines
		solid->index=0;		
		aux=(solid->numVertexs+1)*3;
		//read the values for each vertex from the solid definition
		for(i=0,j=0;i<solid->numVertexs*3;i+=3,j++){
			solid->vertex[j].x=solid->orgVertex[j].x=definition[i];			
			solid->vertex[j].y=solid->orgVertex[j].y=definition[i+1];
			solid->vertex[j].z=solid->orgVertex[j].z=definition[i+2];
		}
		//read the values for the solids center point 
		solid->center.x=definition[i];
		solid->center.y=definition[i+1];
		solid->center.z=definition[i+2];
		//set initial solids directions
		solid->directionY=__UP;	
		solid->directionX=__LEFT;	
		solid->directionZ=__NEAR;	
		//set each polygons vertex to point to the corresponding solids vertex
		for(i=0;i<solid->numPolys;i++){
			solid->poly[i].numVertexs=numVerPoly;
			solid->poly[i].pallet=0x03;			
			for(j=0;j<solid->poly[i].numVertexs;j++){			
				//set the polygons vertex pointing to the vertex defined
				solid->poly[i].vertex[j]=&solid->vertex[definition[i*solid->poly[i].numVertexs+j+aux]];
			}
		}
	}
}
//draw the polygon on the screen
void fillPoly(struct Point2D *p1,struct Point2D *p2,struct Point2D *p3);
//obsoleted by obtimized refresh function
static void drawPoly(struct Polygon *poly){
	struct Point2D p2d1,p2d2,p2d3;
	struct Point2D p2d,p2dNetx;
	set3DPoint(poly->vertex[0],&p2d1);
	set3DPoint(poly->vertex[1],&p2d2);
	set3DPoint(poly->vertex[2],&p2d3);
	fillPoly(&p2d1,&p2d2,&p2d3);

	
/*	int i=0,j=1;
	for(;i<poly->numVertexs;i++,j=i+1){
		if(i==poly->numVertexs-1) j=0;			
		set3DPoint(poly->vertex[i],&p2d);
		set3DPoint(poly->vertex[j],&p2dNetx);
		lineFast(0x00000000,p2d.x-p2d.paralax,p2d.y,p2dNetx.x-p2dNetx.paralax,p2dNetx.y,poly->pallet);
		lineFast(0x00010000,p2d.x+p2d.paralax,p2d.y,p2dNetx.x+p2dNetx.paralax,p2dNetx.y,poly->pallet);
	}
*/	
}
//draw a solid
//obsoleted by obtimized refresh function
void drawSolid(struct Solid * solid){
	register unsigned int i=solid->numPolys;	
	for(;i--;){
		drawPoly(&solid->poly[i]);
	}
		VIP_REGS[XPCTRL] =0xFFFD ;
}
//draw a solid to black to make it invisible
//obsoleted by obtimized refresh function
void deleteSolid(struct Solid * solid){
	register unsigned int i=solid->numPolys;	
	for(;i--;){
		solid->poly[i].pallet=0x00;
		drawPoly(&solid->poly[i]);
		solid->poly[i].pallet=0x03;
	}
}
//change the state of each solid defined to active status for screen refresh
void activeSolids(){
	struct Solid *solid;
	register unsigned int i=Solids.index;	
	for(;i--;){
		solid=(struct Solid *)Solids.array[i];
		solid->state=__ACTIVE;
	}	
}
	
//draw all the 3d objects that have chaged their position or view point
int nextRefresh=1;
int nextCicle=0;
void refreshSolids(){
	unsigned int i=Solids.index,j,k,l,m;			
	struct Solid *solid;
	struct Point2D p2d,p2dNetx;
	nextRefresh*=-1;	

//	VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] | XPEN; // sets the XPEN bit to make sure Etch-A-Sketch board shows up
//	while (!(VIP_REGS[XPSTTS] & XPBSY1)); // makes sure to freeze screen on framebuffer 0
//	VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] & ~XPEN; // turn off screen refreshing so once a dot is drawn it stays there until refreshed again (start pressed)


	for(;i--;){
		solid=(struct Solid *)Solids.array[i];
		solid->state=__PASSIVE;
		solid->index=0;			
		j=solid->numPolys;	
		//if(j>1)j--;
		for(;j--;){					
			k=solid->poly[j].numVertexs-1;
			l=k;
			for(;l--;k--){					
				//determine if a line has been already drawn (need optimization)
				for(m=0;m<solid->index;m+=2){
					if(!(solid->drawLines[m]-solid->poly[j].vertex[l])){
						//if line have been drawn, break loop
						if(!(solid->drawLines[m+1]-solid->poly[j].vertex[k])) break;
					}
					else{
						//compare vertex in opposite direction
						if(!(solid->drawLines[m]-solid->poly[j].vertex[k])){
							//if line have been drawn, break loop
							if(!(solid->drawLines[m+1]-solid->poly[j].vertex[l]))	break;
						}
					}
				}
				//if the line has not been drawn, the draw it
				if(m>=solid->index){					
					//set vertex in drawn lines for skeep on next same line drawing
					solid->drawLines[solid->index++]=solid->poly[j].vertex[l];
					//set vertex in drawn lines for skeep on next same line drawing
					solid->drawLines[solid->index++]=solid->poly[j].vertex[k];
					//determine 2d screen position of 3d initial line point
					set3DPoint(solid->poly[j].vertex[l],&p2d);
					//determine 2d screen position of 3d ending line point
					set3DPoint(solid->poly[j].vertex[k],&p2dNetx);	
					//draw the line in left and right display buffer
					lineFast(__LEFTBUFFER1,p2d.x-p2d.paralax,p2d.y,p2dNetx.x-p2dNetx.paralax,p2dNetx.y,solid->poly[j].pallet);
					lineFast(__RIGHTBUFFER1,p2d.x+p2d.paralax,p2d.y,p2dNetx.x+p2dNetx.paralax,p2dNetx.y,solid->poly[j].pallet);
				}
			}
			//display the last line of the polygon to be drawn (outside for optimization)
			//set vertex in drawn lines for skeep on next same line drawing
			solid->drawLines[solid->index++]=solid->poly[j].vertex[solid->poly[j].numVertexs-1];
			//set vertex in drawn lines for skeep on next same line drawing
			solid->drawLines[solid->index++]=solid->poly[j].vertex[0];
			//determine 2d screen position of 3d initial line point
			set3DPoint(solid->poly[j].vertex[solid->poly[j].numVertexs-1],&p2dNetx);
			//determine 2d screen position of 3d ending line point
			set3DPoint(solid->poly[j].vertex[0],&p2d);					
			//draw the line in left and right display buffer
			lineFast(__LEFTBUFFER1,p2d.x-p2d.paralax,p2d.y,p2dNetx.x-p2dNetx.paralax,p2dNetx.y,solid->poly[j].pallet);
			lineFast(__RIGHTBUFFER1,p2d.x+p2d.paralax,p2d.y,p2dNetx.x+p2dNetx.paralax,p2dNetx.y,solid->poly[j].pallet);
		}
	}
}
/*
void refreshSolids1(){
	unsigned int i=_solids.index,j,k,l,m;			
	Solid *solid;
	P2D p2d,p2dNetx;
	nextRefresh*=-1;	
	VIP_REGS[XPCTRL]&=0xFFFD;	
//	if(!(nextCicle-2)){
//		VIP_REGS[XPCTRL] |=0x02 ;			
//		while (!(VIP_REGS[XPSTTS] & XPBSYR));
//		while (VIP_REGS[XPSTTS] & XPBSYR);
//		nextCicle=0;
//		}
		nextCicle++;
	
	//VIP_REGS[XPCTRL]=0x0000;	
//	VIP_REGS[FRMCYC]=0xFFFF;

		DispBuffer=0x00000000;
//		DispBuffer1=0x00010000;
		if(nextRefresh>0){
			DispBuffer=0x00010000;
//			DispBuffer=0x00008000;
						
//			DispBuffer1=0x00018000;
			}

	for(;i--;){
		solid=( Solid *)_solids.array[i];

//		if(solid->state==ACTIVE){			
			solid->state=PASIVE;
			solid->index=0;
			
			j=solid->numPolys;	
			//if(j>1)j--;
			for(;j--;){					
			//for(;j>0;j--){									
				k=solid->poly[j].numVertexs-1;
				l=k;
				for(;l--;k--){					
					//determine if a line has been already drawn (need optimization)
					for(m=0;m<solid->index;m+=2){
						if(!(solid->drawLines[m]-solid->poly[j].vertex[l])){
							//if line have been drawn, break loop
							if(!(solid->drawLines[m+1]-solid->poly[j].vertex[k])) break;
							}
						else{
							//compare vertex in opposite direction
							if(!(solid->drawLines[m]-solid->poly[j].vertex[k])){
								//if line have been drawn, break loop
								if(!(solid->drawLines[m+1]-solid->poly[j].vertex[l]))	break;
								}
							}
						}
					//if the line has not been drawn, the draw it
					if(m>=solid->index){					
						//set vertex in drawn lines for skeep on next same line drawing
						solid->drawLines[solid->index]=solid->poly[j].vertex[l];
						//set vertex in drawn lines for skeep on next same line drawing
						solid->drawLines[solid->index+1]=solid->poly[j].vertex[k];
						//increment the drawn lines in 2 (for pars)
						solid->index+=2;
						//determine 2d screen position of 3d initial line point
						set3DPoint(solid->poly[j].vertex[l],&p2d);
						//determine 2d screen position of 3d ending line point
						set3DPoint(solid->poly[j].vertex[k],&p2dNetx);	
						//change paralax according with the screen being drawned (left or right) for initial 2d point line	
						p2d.paralax*=nextRefresh;
						//change paralax according with the screen being drawned (left or right) for ending 2d point line	
						p2dNetx.paralax*=nextRefresh;
						//draw the line in left or right display buffer
						lineFast(DispBuffer,p2d.x+p2d.paralax,p2d.y,p2dNetx.x+p2dNetx.paralax,p2dNetx.y,solid->poly[j].pallet);
//						lineFast(DispBuffer1,p2d.x+p2d.paralax,p2d.y,p2dNetx.x+p2dNetx.paralax,p2dNetx.y,solid->poly[j].pallet);
						}
					}
					//display the last line of the polygon to be drawn (outside for optimization goals)
					//set vertex in drawn lines for skeep on next same line drawing
					solid->drawLines[solid->index]=solid->poly[j].vertex[solid->poly[j].numVertexs-1];
					//set vertex in drawn lines for skeep on next same line drawing
					solid->drawLines[solid->index+1]=solid->poly[j].vertex[0];
					//increment the drawn lines in 2 (for pars)
					solid->index+=2;
					//determine 2d screen position of 3d initial line point
					set3DPoint(solid->poly[j].vertex[solid->poly[j].numVertexs-1],&p2dNetx);
					//determine 2d screen position of 3d ending line point
					set3DPoint(solid->poly[j].vertex[0],&p2d);
					//change paralax according with the screen being drawned (left or right) for initial 2d point line	
					p2d.paralax*=nextRefresh;
					//change paralax according with the screen being drawned (left or right) for ending 2d point line	
					p2dNetx.paralax*=nextRefresh;
					
					//draw the line in left or right display buffer
					lineFast(DispBuffer,p2d.x+p2d.paralax,p2d.y,p2dNetx.x+p2dNetx.paralax,p2dNetx.y,solid->poly[j].pallet);
//					lineFast(DispBuffer1,p2d.x+p2d.paralax,p2d.y,p2dNetx.x+p2dNetx.paralax,p2dNetx.y,solid->poly[j].pallet);
					
				}
//			}	

	}
	//VIP_REGS[XPCTRL]&=0xFFFD;	
}

*/
/*
int edge1X[384];
int edge1Y[384];
int edge1Count=0;
int edge2X[384];
int edge2Y[384];
int edge2Count=0;
*/
void scaleLine(struct Point2D *p1,struct Point2D *p2,int *edgeX,int *edgeY,int *counter){
	float m,y;
	int mInt;
	int i;
	int x1,x2;
	int fraction;
	fix7_9 mFix;
	if(p1->x<p2->x){
		x1=p1->x;
		x2=p2->x;
		y=p1->y;
	}
	else{
		x1=p2->x;
		x2=p1->x;		
		y=p2->y;		
	}
     m = (float)(p2->y - p1->y)/(p2->x - p1->x);
     mFix=FTOFIX7_9(m);
     mInt=FIX7_9TOI(mFix);

     for (i = x1; i<x2; i++){
     	
     	edgeX[*counter] = i;
		edgeY[*counter] = y;
		*counter=*counter+1;
		y += m;
	}		
}
long calcDistance(struct Point2D *p1,struct Point2D *p2){
	return (p1->x-p2->x)*(p1->x-p2->x)+(p1->y-p2->y)*(p1->y-p2->y);
}
void fillPoly(struct Point2D *A,struct Point2D *B,struct Point2D *C){
	long d1,d2,d3,buffer=0x00000000;
	struct Point2D p1[2],p2[2],p3[2];
	int i;
	int j=0;
	p1[0].x=A->x-A->paralax;
	p1[0].y=A->y;
	p1[1].x=A->x+A->paralax;
	p1[1].y=A->y;

	p2[0].x=B->x-B->paralax;
	p2[0].y=B->y;
	p2[1].x=B->x+B->paralax;
	p2[1].y=B->y;

	p3[0].x=C->x-C->paralax;
	p3[0].y=C->y;
	p3[1].x=C->x+C->paralax;
	p3[1].y=C->y;

	for(;j<2;j++){
		d1=calcDistance(&p1[j],&p2[j]);
		d2=calcDistance(&p1[j],&p3[j]);
		d3=calcDistance(&p2[j],&p3[j]);
		edge1Count=0;
		edge2Count=0;
	
		if(d1>d2 && d1>d3){
			scaleLine(&p1[j],&p2[j],edge1X,edge1Y,&edge1Count);
			scaleLine(&p1[j],&p3[j],edge2X,edge2Y,&edge2Count);
			scaleLine(&p3[j],&p2[j],edge2X,edge2Y,&edge2Count);
		}
		else{
			if(d2>d3){
				scaleLine(&p1[j],&p3[j],edge1X,edge1Y,&edge1Count);
				scaleLine(&p1[j],&p2[j],edge2X,edge2Y,&edge2Count);
				scaleLine(&p2[j],&p3[j],edge2X,edge2Y,&edge2Count);
			}
			else{
				scaleLine(&p2[j],&p3[j],edge1X,edge1Y,&edge1Count);
				scaleLine(&p1[j],&p2[j],edge2X,edge2Y,&edge2Count);
				scaleLine(&p1[j],&p3[j],edge2X,edge2Y,&edge2Count);
			}
		}
		for(i=0;i<edge1Count;i++){
			if(!(i&4))
			lineFast(buffer,edge1X[i], edge1Y[i], edge2X[i], edge2Y[i], 0x03);
//			lineFast(0x00010000,edge1X[i], edge1Y[i], edge2X[i], edge2Y[i], 0x03);		
		}
		edge1Count=0;
		edge2Count=0;
		buffer=0x00010000;

	}

}
//draw the polygon on the screen

//obsolete by optimized refreshSolids function
void refreshSolidsObs(){
	struct Solid *solid;
	unsigned int i=Solids.index;	
//	VIP_REGS[XPCTRL] =0xFFFF;

	for(;i--;){
		solid=(struct Solid *)Solids.array[i];
//		if(solid->state){
//			deleteSolid(solid);			
//			fillSolid(solid);
			drawSolid(solid);
	//		}
		solid->state=__PASSIVE;
		}	
//	VIP_REGS[XPCTRL] =0xFFFD ;			

	}

//rotation funcions
//Z-Axis rotation
rotateZ(struct Solid * solid,int alpha){
	float x,y;
	unsigned int i=solid->numVertexs;	
	//set solid state to active for screen redraw
	solid->state=__ACTIVE;
	for (;i--;){		
		//for each vertex aply the trasfomation matrix
		x=(solid->orgVertex[i].x-solid->center.x)*COSF(alpha)-(solid->orgVertex[i].y-solid->center.y)*SINF(alpha);
		y=(solid->orgVertex[i].x-solid->center.x)*SINF(alpha)+(solid->orgVertex[i].y-solid->center.y)*COSF(alpha);
		solid->vertex[i].x=x+solid->center.x;
		solid->vertex[i].y=y+solid->center.y;		
	}
}
//X-Axis rotation
rotateX(struct Solid * solid,int alpha){
	float y,z;
     unsigned int i=solid->numVertexs;	
     //set solid state to active for screen redraw
	solid->state=__ACTIVE;
	for (;i--;){
		//for each vertex aply the trasfomation matrix
		y=(solid->orgVertex[i].y-solid->center.y)*COSF(alpha)-(solid->orgVertex[i].z-solid->center.z)*SINF(alpha);
		z=(solid->orgVertex[i].y-solid->center.y)*SINF(alpha)+(solid->orgVertex[i].z-solid->center.z)*COSF(alpha);
		solid->vertex[i].y=y+solid->center.y;
		solid->vertex[i].z=z+solid->center.z;		
	}
}

//Y-Axis rotation
rotateY(struct Solid * solid,int alpha){
	float x,z;
     unsigned int i=solid->numVertexs;	
     //set solid state to active for screen redraw
	solid->state=__ACTIVE;
	for (;i--;){
		//for each vertex aply the trasfomation matrix
		x=(solid->orgVertex[i].x-solid->center.x)*COSF(alpha)-(solid->orgVertex[i].z-solid->center.z)*SINF(alpha);
		z=(solid->orgVertex[i].x-solid->center.x)*SINF(alpha)+(solid->orgVertex[i].z-solid->center.z)*COSF(alpha);
		solid->vertex[i].x=x+solid->center.x;
		solid->vertex[i].z=z+solid->center.z;		
	}
}
	
//move a solid toward direction	
void moveSolid(struct Solid * solid,int direction,int displacement){
	  int i=solid->numVertexs;
	//activeSolids();	  
	///solid->state=ACTIVE;	
	switch(direction){
		case __LEFT:
//			if(solid->vertex[0].x>=displacement){
				for(;i--;){
					solid->vertex[i].x-=displacement;
					solid->orgVertex[i].x-=displacement;
				}
				solid->center.x-=displacement;				
				solid->directionX=__LEFT;
				solid->direction=__LEFT;
//			}
//			else solid->directionX=RIGHT;
			break;
		case __RIGHT:
//			if(solid->vertex[0].x<384){
				for(;i--;){
					solid->vertex[i].x+=displacement;
					solid->orgVertex[i].x+=displacement;
				}					
				solid->center.x+=displacement;								
				solid->directionX=__RIGHT;
				solid->direction=__RIGHT;
//			}
//			else solid->directionX=LEFT;
			break;
/*		case UP:
			if(solid->vertex[0].y>displacement){			
				for(;i--;){
					solid->vertex[i].y-=displacement;
					solid->orgVertex[i].y-=displacement;
				}					
				solid->center.y-=displacement;				
				solid->directionY=UP;
				solid->direction=UP;
			}
			else solid->directionY=DOWN;				
			break;
		case DOWN:
			if(solid->vertex[0].y<224){			
						
				for(;i--;){
					solid->vertex[i].y+=displacement;
					solid->orgVertex[i].y+=displacement;
				}					

				solid->center.y+=displacement;
				solid->directionY=DOWN;
			}
			else solid->directionY=UP;								
			break;
		case FAR:
			if(solid->vertex[0].z<DES){			

				for(;i--;){
					solid->vertex[i].z+=displacement;
					solid->orgVertex[i].z+=displacement;
				}					
					
				solid->center.z+=displacement;
				solid->directionZ=FAR;
			}
			else solid->directionZ=NEAR;
			break;
		case NEAR:
			if(solid->vertex[0].z>displacement){				
				for(;i--;){
					solid->vertex[i].z-=displacement;
					solid->orgVertex[i].z-=displacement;
				}					
					
				solid->center.z-=displacement;				
				solid->directionZ=NEAR;
			}				
			else solid->directionZ=FAR;
			break;
*/	}	
//	deleteSolid(solid);		
}
	
#endif