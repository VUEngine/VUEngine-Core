/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Polyhedron.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <VirtualList.h>
#include <WireframeManager.h>
#include <VIPManager.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Polyhedron
 * @extends Object
 * @ingroup graphics-3d
 */

friend class VirtualNode;
friend class VirtualList;



//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------




/**
 * Class constructor
 *
 * @memberof	Polyhedron
 * @private
 *
 * @param this	Function scope
 */
void Polyhedron::constructor()
{
	// construct base object
	Base::constructor();

	// don't create the list yet
	this->vertices = NULL;
}

/**
 * Class destructor
 *
 * @memberof	Polyhedron
 * @public
 *
 * @param this	Function scope
 */
void Polyhedron::destructor()
{
	Wireframe::hide(__SAFE_CAST(Wireframe, this));

	// delete the vertices list
	if(this->vertices)
	{
		VirtualNode node = this->vertices->head;

		// delete each vertex
		for(; node ; node = node->next)
		{
			__DELETE_BASIC(node->data);
		}

		// delete the list
		delete this->vertices;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

/**
 * Add a vertex
 *
 * @memberof	Polyhedron
 * @public
 *
 * @param this	Function scope
 * @param x		Vertex' x coordinate
 * @param y		Vertex' y coordinate
 * @param z		Vertex' x coordinate
 */
void Polyhedron::addVertex(fix10_6 x, fix10_6 y, fix10_6 z)
{
	// create the vertex
	Vector3D* vertex = __NEW_BASIC(Vector3D);
	vertex->x = x;
	vertex->y = y;
	vertex->z = z;

	// if not vertices list yet created
	// delete the vertices list
	if(!this->vertices)
	{
		this->vertices = new VirtualList();
	}

	// add vertex to the end of the list
	VirtualList::pushBack(this->vertices, vertex);
}

/**
 * Write to the frame buffers
 *
 * @memberof				Polyhedron
 * @public
 *
 * @param this				Function scope
 * @param this				Function scope
 * @param calculateParallax	True to compute the parallax displacement for each pixel
 */
void Polyhedron::draw(bool calculateParallax)
{
	int color = __COLOR_BRIGHT_RED;

	// if I have some vertex, draw them
	if(this->vertices && 2 < VirtualList::getSize(this->vertices))
	{
		// the nodes which hold the vertices
		VirtualNode fromNode = this->vertices->head;
		VirtualNode toNode = fromNode->next;

		// 3d vertices
		Vector3D fromVertex3D = {0, 0, 0};
		Vector3D toVertex3D = {0, 0, 0};

		// 2d vertices
		PixelVector fromVertex2D = {0, 0, 0, 0};
		PixelVector toVertex2D = {0, 0, 0, 0};

		// draw the lines
		for(; toNode ; fromNode = fromNode->next, toNode = toNode->next)
		{
			// normalize vertex to screen coordinates
			fromVertex3D = Vector3D::getRelativeToCamera(*((Vector3D*)fromNode->data));
			toVertex3D = Vector3D::getRelativeToCamera(*((Vector3D*)toNode->data));

			// project to 2d coordinates
			fromVertex2D = Vector3D::projectToPixelVector(fromVertex3D, 0);
			toVertex2D = Vector3D::projectToPixelVector(toVertex3D, 0);

			// calculate parallax
			if(calculateParallax)
			{
				fromVertex2D.parallax = Optics::calculateParallax(fromVertex3D.x, fromVertex3D.z);
				toVertex2D.parallax = Optics::calculateParallax(toVertex3D.x, toVertex3D.z);
			}

			// draw the line in both buffers
			DirectDraw::drawLine(DirectDraw::getInstance(), fromVertex2D, toVertex2D, color);
		}
	}
}
