/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Polygon.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Polygon
__CLASS_DEFINITION(Polygon, Object);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// globals
extern const VBVec3D* _screenPosition;
extern const Optical* _optical;

// class's constructor
static void Polygon_constructor(Polygon this);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

__CLASS_NEW_DEFINITION(Polygon)
__CLASS_NEW_END(Polygon);

// class's constructor
static void Polygon_constructor(Polygon this)
{
	// construct base object
	__CONSTRUCT_BASE();

	// don't create the list yet
	this->vertices = NULL;
}

// class's destructor
void Polygon_destructor(Polygon this)
{
	// delete the vertices list
	if(this->vertices)
	{
		VirtualNode node = VirtualList_begin(this->vertices);

		// delete each vertex
		for(; node ; node = VirtualNode_getNext(node))
		{
			__DELETE_BASIC(VirtualNode_getData(node));
		}

		// delete the list
		__DELETE(this->vertices);
	}

	// destroy the super object
	__DESTROY_BASE;
}

// add a vertice
void Polygon_addVertice(Polygon this, fix19_13 x, fix19_13 y, fix19_13 z)
{
	// create the vertice
	VBVec3D* vertice = __NEW_BASIC(VBVec3D);
	vertice->x = x;
	vertice->y = y;
	vertice->z = z;

	// if not vertices list yet created
	// delete the vertices list
	if(!this->vertices)
	{
		this->vertices = __NEW(VirtualList);
	}

	// add vertice to the end of the list
	VirtualList_pushBack(this->vertices, vertice);
}


// draw polygon to screen
void Polygon_draw(Polygon this, int calculateParallax)
{
	int palette = 0x03;

	// if I have some vertex, draw them
	if(this->vertices && 2 < VirtualList_getSize(this->vertices))
	{
		// the node's which hold the vertices
		VirtualNode fromNode = VirtualList_begin(this->vertices);
		VirtualNode toNode = VirtualNode_getNext(fromNode);

		// 3d vertices
		VBVec3D fromVertice3D = {0, 0, 0};
		VBVec3D toVertice3D = {0, 0, 0};

		// 2d vertices
		VBVec2D fromVertice2D = {0, 0, 0};
		VBVec2D toVertice2D = {0, 0, 0};

		// draw the lines
		for(; toNode ; fromNode = VirtualNode_getNext(fromNode), toNode = VirtualNode_getNext(toNode))
		{
			// normalize vertice to screen coordinates
			fromVertice3D = *((VBVec3D*)VirtualNode_getData(fromNode));
			toVertice3D = *((VBVec3D*)VirtualNode_getData(toNode));
			__OPTICS_NORMALIZE(fromVertice3D);
			__OPTICS_NORMALIZE(toVertice3D);

			// project to 2d coordinates
			__OPTICS_PROJECT_TO_2D(fromVertice3D, fromVertice2D);
			__OPTICS_PROJECT_TO_2D(toVertice3D, toVertice2D);

			// calculate parallax
			if(calculateParallax)
			{
				fromVertice2D.parallax = Optics_calculateParallax(fromVertice3D.x, fromVertice3D.z);
				toVertice2D.parallax = Optics_calculateParallax(toVertice3D.x, toVertice3D.z);
			}


			// draw the line in both buffers
			DirectDraw_lineFast(DirectDraw_getInstance(), fromVertice2D, toVertice2D, palette);
		}

		if(fromNode && toNode && 2 < VirtualList_getSize(this->vertices))
		{
			fromVertice3D = *((VBVec3D*)VirtualNode_getData(fromNode));
			toVertice3D = *((VBVec3D*)VirtualNode_getData(toNode));
			__OPTICS_NORMALIZE(fromVertice3D);
			__OPTICS_NORMALIZE(toVertice3D);

			// project to 2d coordinates
			__OPTICS_PROJECT_TO_2D(fromVertice3D, fromVertice2D);
			__OPTICS_PROJECT_TO_2D(toVertice3D, toVertice2D);

			// calculate parallax
			if(calculateParallax)
			{
				fromVertice2D.parallax = Optics_calculateParallax(fromVertice3D.x, fromVertice3D.z);
				toVertice2D.parallax = Optics_calculateParallax(toVertice3D.x, toVertice3D.z);
			}

			// draw the line in both buffers
			DirectDraw_lineFast(DirectDraw_getInstance(), fromVertice2D, toVertice2D, palette);
		}
	}
	//VIP_REGS[XPCTRL] = VIP_REGS[XPSTTS] & ~XPEN;
}