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

#include <Mesh.h>
#include <DirectDraw.h>
#include <Optics.h>
#include <VIPManager.h>
#include <WireframeManager.h>
#include <PixelVector.h>
#include <Math.h>
#include <Camera.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S DECLARATIONS
//---------------------------------------------------------------------------------------------------------

friend class VirtualNode;
friend class VirtualList;


VIPManager _vipManager = NULL;

//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Class constructor
 *
 * @private
 */
void Mesh::constructor(MeshSpec* meshSpec)
{
	// construct base object
	Base::constructor(&meshSpec->wireframeSpec);

	this->segments = new VirtualList();
	this->vertices = new VirtualList();

	if(NULL != this->wireframeSpec)
	{
		Mesh::addSegments(this, ((MeshSpec*)this->wireframeSpec)->segments);
	}

	if(NULL == _vipManager)
	{
		_vipManager = VIPManager::getInstance();
	}
}

/**
 * Class destructor
 */
void Mesh::destructor()
{
	Mesh::hide(this);

	Mesh::deleteLists(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void Mesh::deleteLists()
{
	for(VirtualNode node = this->segments->head; NULL != node; node = node->next)
	{
		MeshSegment* meshSegment = (MeshSegment*)node->data;

		if(!isDeleted(meshSegment->fromVertex))
		{
			delete meshSegment->fromVertex;
		}

		if(!isDeleted(meshSegment->toVertex))
		{
			delete meshSegment->toVertex;
		}

		delete meshSegment;
	}

	delete this->segments;
	this->segments = NULL;

	delete this->vertices;
	this->vertices = NULL;
}

void Mesh::addSegments(PixelVector (*segments)[2])
{
	if(NULL == segments)
	{
		return;
	}

	if(NULL != this->segments->head)
	{
		Mesh::deleteLists(this);

		this->segments = new VirtualList();
	}

	bool isEndSegment = false;
	uint16 i = 0;

	do
	{
		Vector3D startVector = Vector3D::getFromPixelVector(segments[i][0]);
		Vector3D endVector = Vector3D::getFromPixelVector(segments[i][1]);

		isEndSegment = Vector3D::areEqual(Vector3D::zero(), startVector) && Vector3D::areEqual(Vector3D::zero(), endVector);

		if(!isEndSegment)
		{
			Mesh::addSegment(this, startVector, endVector);
		}

		i++;
	}
	while(!isEndSegment);
}

void Mesh::addSegment(Vector3D startVector, Vector3D endVector)
{
	if(Vector3D::areEqual(startVector, endVector))
	{
		return;
	}

	MeshSegment* newMeshSegment = new MeshSegment;
	newMeshSegment->fromVertex = NULL;
	newMeshSegment->toVertex = NULL;

	for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
	{
		Vertex* vertex = (Vertex*)node->data;

		if(Vector3D::areEqual(vertex->vector, startVector))
		{
			newMeshSegment->fromVertex = vertex;
		}
		else if(Vector3D::areEqual(vertex->vector, endVector))
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

/**
 * Render
 */
void Mesh::render()
{
	if(NULL == this->position)
	{
		return;
	}

	extern Vector3D _previousCameraPosition;
	extern Rotation _previousCameraInvertedRotation;
	Vector3D position = *this->position;

	Vector3D relativePosition = Vector3D::sub(position, _previousCameraPosition);
	Mesh::setupRenderingMode(this, Vector3D::squareLength(relativePosition));

	if(NULL == this->rotation && NULL == this->scale)
	{
		for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
		{
			Vertex* vertex = (Vertex*)node->data;

			Vector3D vector = Vector3D::rotate(Vector3D::sum(relativePosition, vertex->vector), _previousCameraInvertedRotation);

			vertex->pixelVector = Vector3D::projectToPixelVector(vector, Optics::calculateParallax(vector.z));
		}
	}
	else if(NULL == this->rotation)
	{
		Scale scale = *this->scale;

		for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
		{
			Vertex* vertex = (Vertex*)node->data;

			Vector3D vector = Vector3D::rotate(Vector3D::sum(relativePosition, Vector3D::scale(vertex->vector, scale)), _previousCameraInvertedRotation);

			vertex->pixelVector = Vector3D::projectToPixelVector(vector, Optics::calculateParallax(vector.z));
		}
	}
	else if(NULL == this->scale)
	{
		Rotation rotation = *this->rotation;

		for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
		{
			Vertex* vertex = (Vertex*)node->data;

			Vector3D vector = Vector3D::rotate(Vector3D::sum(relativePosition, Vector3D::rotate(vertex->vector, rotation)), _previousCameraInvertedRotation);

			vertex->pixelVector = Vector3D::projectToPixelVector(vector, Optics::calculateParallax(vector.z));
		}
	}	
	else
	{
		Rotation rotation = *this->rotation;
		Scale scale = *this->scale;

		for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
		{
			Vertex* vertex = (Vertex*)node->data;

			Vector3D vector = Vector3D::rotate(Vector3D::sum(relativePosition, Vector3D::rotate(Vector3D::scale(vertex->vector, scale), rotation)), _previousCameraInvertedRotation);

			vertex->pixelVector = Vector3D::projectToPixelVector(vector, Optics::calculateParallax(vector.z));
		}
	}
}

/**
 * Draw
 */
void Mesh::draw()
{
	if(NULL == this->position)
	{
		return;
	}

	for(VirtualNode node = this->segments->head; NULL != node; node = node->next)
	{
		MeshSegment* meshSegment = (MeshSegment*)node->data;

		// draw the line in both buffers
		DirectDraw::drawColorLine(meshSegment->fromVertex->pixelVector, meshSegment->toVertex->pixelVector, this->color, this->bufferIndex, this->interlaced);
	}

	this->bufferIndex = !this->bufferIndex;
}

/**
 * Draw interlaced
 */
void Mesh::drawInterlaced()
{
	for(VirtualNode node = this->segments->head; NULL != node; node = node->next)
	{
		MeshSegment* meshSegment = (MeshSegment*)node->data;

		// draw the line in both buffers
		DirectDraw::drawColorLine(meshSegment->fromVertex->pixelVector, meshSegment->toVertex->pixelVector, this->color, this->bufferIndex, true);
	}

	this->bufferIndex = !this->bufferIndex;
}

VirtualList Mesh::getVertices()
{
	return this->vertices;
}