/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <DirectDraw.h>
#include <DebugConfig.h>
#include <PixelVector.h>
#include <VirtualList.h>
#include <VirtualNode.h>
#include <WireframeManager.h>

#include "Mesh.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATIONS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

friend class VirtualNode;
friend class VirtualList;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static RightBox Mesh::getRightBoxFromSpec(MeshSpec* meshSpec)
{
	RightBox rightBox = {0, 0, 0, 0, 0, 0};

	bool isEndSegment = false;
	uint16 i = 0;

	do
	{
		Vector3D startVector = Vector3D::getFromPixelVector(meshSpec->segments[i][0]);
		Vector3D endVector = Vector3D::getFromPixelVector(meshSpec->segments[i][1]);

		isEndSegment = Vector3D::areEqual(Vector3D::zero(), startVector) && Vector3D::areEqual(Vector3D::zero(), endVector);

		if(!isEndSegment)
		{
			if(startVector.x < endVector.x)
			{
				if(startVector.x < rightBox.x0)
				{
					rightBox.x0 = startVector.x;
				}
			}
			else
			{
				if(endVector.x < rightBox.x0)
				{
					rightBox.x0 = endVector.x;
				}
			}

			if(startVector.x > endVector.x)
			{
				if(startVector.x > rightBox.x1)
				{
					rightBox.x1 = startVector.x;
				}
			}
			else
			{
				if(endVector.x > rightBox.x1)
				{
					rightBox.x1 = endVector.x;
				}
			}

			if(startVector.y < endVector.y)
			{
				if(startVector.y < rightBox.y0)
				{
					rightBox.y0 = startVector.y;
				}
			}
			else
			{
				if(endVector.y < rightBox.y0)
				{
					rightBox.y0 = endVector.y;
				}
			}

			if(startVector.y > endVector.y)
			{
				if(startVector.y > rightBox.y1)
				{
					rightBox.y1 = startVector.y;
				}
			}
			else
			{
				if(endVector.y > rightBox.y1)
				{
					rightBox.y1 = endVector.y;
				}
			}

			if(startVector.z < endVector.z)
			{
				if(startVector.z < rightBox.z0)
				{
					rightBox.z0 = startVector.z;
				}
			}
			else
			{
				if(endVector.z < rightBox.z0)
				{
					rightBox.z0 = endVector.z;
				}
			}

			if(startVector.z > endVector.z)
			{
				if(startVector.z > rightBox.z1)
				{
					rightBox.z1 = startVector.z;
				}
			}
			else
			{
				if(endVector.z > rightBox.z1)
				{
					rightBox.z1 = endVector.z;
				}
			}
		}

		i++;
	}
	while(!isEndSegment);

	return rightBox;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Mesh::constructor(Entity owner, const MeshSpec* meshSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, &meshSpec->wireframeSpec);

	this->segments = new VirtualList();
	this->vertices = new VirtualList();

	if(NULL != this->componentSpec)
	{
		Mesh::addSegments(this, ((MeshSpec*)this->componentSpec)->segments, Vector3D::zero());
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Mesh::destructor()
{
	Mesh::hide(this);

	Mesh::deleteLists(this);

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

RightBox Mesh::getRightBox()
{
	RightBox rightBox = {-1, -1, -1, 1, 1, 1};

	for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
	{
		Vertex* vertex = (Vertex*)node->data;

		Vector3D vector3D = Vector3D::sum(vertex->vector, this->displacement);

		if(vector3D.x < rightBox.x0)
		{
			rightBox.x0 = vector3D.x;
		}

		if(vector3D.x > rightBox.x1)
		{
			rightBox.x1 = vector3D.x;
		}

		if(vector3D.y < rightBox.y0)
		{
			rightBox.y0 = vector3D.y;
		}

		if(vector3D.y > rightBox.y1)
		{
			rightBox.y1 = vector3D.y;
		}

		if(vector3D.z < rightBox.z0)
		{
			rightBox.z0 = vector3D.z;
		}

		if(vector3D.z > rightBox.z1)
		{
			rightBox.z1 = vector3D.z;
		}
	}

	return rightBox;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

VirtualList Mesh::getVertices()
{
	return this->vertices;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Mesh::render(Vector3D relativePosition)
{
	NM_ASSERT(NULL != this->transformation, "Mesh::render: NULL transformation");

	bool scale = 
		(__1I_FIX7_9 != this->transformation->scale.x) + (__1I_FIX7_9 != this->transformation->scale.y) + 
		(__1I_FIX7_9 != this->transformation->scale.z);
	
	bool rotate = 
		(0 != this->transformation->rotation.x) + (0 != this->transformation->rotation.y) + (0 != this->transformation->rotation.z);

	if(!scale && !rotate)
	{
		CACHE_RESET;

		for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
		{
			Vertex* vertex = (Vertex*)node->data;

			Vector3D vector = Vector3D::rotate(Vector3D::sum(relativePosition, vertex->vector), _previousCameraInvertedRotation);

			vertex->pixelVector = PixelVector::projectVector3D(vector, Optics::calculateParallax(vector.z));
		}
	}
	else if(scale && rotate)
	{
		CACHE_RESET;

		Rotation rotation = this->transformation->rotation;
		Scale scale = this->transformation->scale;

		for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
		{
			Vertex* vertex = (Vertex*)node->data;

			Vector3D vector = 
				Vector3D::rotate
				(
					Vector3D::sum(relativePosition, Vector3D::rotate(Vector3D::scale(vertex->vector, scale), rotation)), 
					_previousCameraInvertedRotation
				);

			vertex->pixelVector = PixelVector::projectVector3D(vector, Optics::calculateParallax(vector.z));
		}
	}
	else if(rotate)
	{
		CACHE_RESET;

		Rotation rotation = this->transformation->rotation;

		for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
		{
			Vertex* vertex = (Vertex*)node->data;

			Vector3D vector = 
				Vector3D::rotate
				(
					Vector3D::sum(relativePosition, Vector3D::rotate(vertex->vector, rotation)), _previousCameraInvertedRotation
				);

			vertex->pixelVector = PixelVector::projectVector3D(vector, Optics::calculateParallax(vector.z));
		}
	}
	else if(scale)
	{
		CACHE_RESET;

		Scale scale = this->transformation->scale;

		for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
		{
			Vertex* vertex = (Vertex*)node->data;

			Vector3D vector = 
				Vector3D::rotate
				(
					Vector3D::sum(relativePosition, Vector3D::scale(vertex->vector, scale)), _previousCameraInvertedRotation
				);

			vertex->pixelVector = PixelVector::projectVector3D(vector, Optics::calculateParallax(vector.z));
		}
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Mesh::draw()
{
	NM_ASSERT(NULL != this->transformation, "Mesh::draw: NULL transformation");

	bool drawn = false;

	for(VirtualNode node = this->segments->head; NULL != node; node = node->next)
	{
		MeshSegment* meshSegment = (MeshSegment*)node->data;

		// Draw the line in both buffers
		drawn |= 
			DirectDraw::drawLine
			(
				meshSegment->fromVertex->pixelVector, meshSegment->toVertex->pixelVector, this->color, 
				this->bufferIndex, this->interlaced
			);
	}

	this->bufferIndex = !this->bufferIndex;

	return drawn;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Mesh::addSegments(PixelVector (*segments)[2], Vector3D displacement)
{
	if(NULL == segments)
	{
		return;
	}

	bool isEndSegment = false;
	uint16 i = 0;

	// Prevent rendering when modifying the mesh because if segments are added after the initial render
	// There can be graphical glitches if XPEND kicks in in the mist of adding new vertexes
	this->rendered = false;

	do
	{
		Vector3D startVector = Vector3D::getFromPixelVector(segments[i][0]);
		Vector3D endVector = Vector3D::getFromPixelVector(segments[i][1]);

		isEndSegment = Vector3D::areEqual(Vector3D::zero(), startVector) && Vector3D::areEqual(Vector3D::zero(), endVector);

		if(!isEndSegment)
		{
			Mesh::addSegment(this, Vector3D::sum(startVector, displacement), Vector3D::sum(endVector, displacement));
		}

		i++;
	}
	while(!isEndSegment);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Mesh::addSegment(Vector3D startVector, Vector3D endVector)
{
	MeshSegment* newMeshSegment = new MeshSegment;
	newMeshSegment->fromVertex = NULL;
	newMeshSegment->toVertex = NULL;

	for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
	{
		Vertex* vertex = (Vertex*)node->data;

		if(NULL == newMeshSegment->fromVertex && Vector3D::areEqual(vertex->vector, startVector))
		{
			newMeshSegment->fromVertex = vertex;
		}
		else if(NULL == newMeshSegment->toVertex && Vector3D::areEqual(vertex->vector, endVector))
		{
			newMeshSegment->toVertex = vertex;
		}

		if(NULL != newMeshSegment->fromVertex && NULL != newMeshSegment->toVertex)
		{
			break;
		}
	}

	if(NULL == newMeshSegment->fromVertex)
	{
		newMeshSegment->fromVertex = new Vertex;
		newMeshSegment->fromVertex->vector = startVector;
		newMeshSegment->fromVertex->pixelVector = (PixelVector){0, 0, 0, 0};

		VirtualList::pushBack(this->vertices, newMeshSegment->fromVertex);
	}

	if(NULL == newMeshSegment->toVertex)
	{
		newMeshSegment->toVertex = new Vertex;
		newMeshSegment->toVertex->vector = endVector;
		newMeshSegment->toVertex->pixelVector = (PixelVector){0, 0, 0, 0};

		VirtualList::pushBack(this->vertices, newMeshSegment->toVertex);
	}

	VirtualList::pushBack(this->segments, newMeshSegment);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Mesh::drawInterlaced()
{
	bool drawn = false;

	for(VirtualNode node = this->segments->head; NULL != node; node = node->next)
	{
		MeshSegment* meshSegment = (MeshSegment*)node->data;

		// Draw the line in both buffers
		drawn |= 
			DirectDraw::drawLine
			(
				meshSegment->fromVertex->pixelVector, meshSegment->toVertex->pixelVector, this->color, this->bufferIndex, true
			) 
			|| 
			this->drawn;
	}

	this->bufferIndex = !this->bufferIndex;

	return drawn;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Mesh::deleteLists()
{
	if(!isDeleted(this->vertices))
	{
		VirtualList::deleteData(this->vertices);
		delete this->vertices;
		this->vertices = NULL;
	}
	
	if(!isDeleted(this->segments))
	{
		VirtualList::deleteData(this->segments);
		delete this->segments;
		this->segments = NULL;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
