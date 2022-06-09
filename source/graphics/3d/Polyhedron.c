/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
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

friend class VirtualNode;
friend class VirtualList;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 */
void Polyhedron::constructor(uint8 color)
{
	// construct base object
	Base::constructor(color);

	// don't create the list yet
	this->vertices = NULL;
}

/**
 * Class destructor
 */
void Polyhedron::destructor()
{
	Wireframe::hide(this);

	// delete the vertices list
	if(this->vertices)
	{
		VirtualNode node = this->vertices->head;

		// delete each vertex
		for(; node ; node = node->next)
		{
			delete node->data;
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
 * @param x		Vertex' x coordinate
 * @param y		Vertex' y coordinate
 * @param z		Vertex' x coordinate
 */
void Polyhedron::addVertex(fix10_6 x, fix10_6 y, fix10_6 z)
{
	// create the vertex
	Vector3D* vertex = new Vector3D;
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
 * @param calculateParallax	True to compute the parallax displacement for each pixel
 */
void Polyhedron::draw(bool calculateParallax)
{
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
			fromVertex2D = PixelVector::project(fromVertex3D, 0);
			toVertex2D = PixelVector::project(toVertex3D, 0);

			// calculate parallax
			if(calculateParallax)
			{
				fromVertex2D.parallax = Optics::calculateParallax(fromVertex3D.z);
				toVertex2D.parallax = Optics::calculateParallax(toVertex3D.z);
			}

			// draw the line in both buffers
			DirectDraw::drawColorLine(fromVertex2D, toVertex2D, this->color, 0, false);
		}
	}
}
