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
	Base::constructor(meshSpec->color);

	this->segments = new VirtualList();
	this->vertices = new VirtualList();
	
	this->meshSpec = meshSpec;

	Mesh::addSegments(this);

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

void Mesh::addSegments()
{
	if(NULL != this->segments->head)
	{
		Mesh::deleteLists(this);

		this->segments = new VirtualList();
	}

	bool isEndSegment = false;
	uint16 i = 0;

	do
	{
		Vector3D startVector = Vector3D::getFromPixelVector(this->meshSpec->segments[i][0]);
		Vector3D endVector = Vector3D::getFromPixelVector(this->meshSpec->segments[i][1]);

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
	newMeshSegment->bufferIndex = 0;

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
 * Class draw
 */
void Mesh::render()
{
	Vector3D position = *this->position;
	Rotation rotation = *this->rotation;

	Vector3D relativePosition = Vector3D::getRelativeToCamera(position);
	Vector3D rotatedPosition = Vector3D::rotate(relativePosition, *_cameraInvertedRotation);

	fix10_6 cosAngle = Vector3D::dotProduct(Vector3D::normalize(rotatedPosition), Vector3D::unit(__Z_AXIS));

	// cull off at 45 degrees
	if(__FIX7_9_TO_FIX10_6(__COS(64)) > cosAngle)
	{
		this->culled = true;
		return;
	}

	this->culled = false;

	PixelVector displacement = PixelVector::getProjectionDisplacementHighPrecision(rotatedPosition, 0);

	for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
	{
		Vertex* vertex = (Vertex*)node->data;

		Vector3D vector = Vector3D::sum(relativePosition, Vector3D::rotate(vertex->vector, rotation));
		vector = Vector3D::rotate(vector, *_cameraInvertedRotation);

		vertex->pixelVector = Vector3D::projectToPixelVector(vector, Optics::calculateParallax(vector.x, vector.z));
		vertex->pixelVector = PixelVector::sub(vertex->pixelVector, displacement);
/*
		// Pre clamp to prevent weird glitches due to overflows and speed up drawing
		if(-__FIX10_6_MAXIMUM_VALUE_TO_I > vertex->pixelVector.x)
		{
			vertex->pixelVector.x = -__FIX10_6_MAXIMUM_VALUE_TO_I;
		}
		else if(__FIX10_6_MAXIMUM_VALUE_TO_I < vertex->pixelVector.x)
		{
			vertex->pixelVector.x = __FIX10_6_MAXIMUM_VALUE_TO_I;
		}

		if(-__FIX10_6_MAXIMUM_VALUE_TO_I > vertex->pixelVector.y)
		{
			vertex->pixelVector.y = -__FIX10_6_MAXIMUM_VALUE_TO_I;
		}
		else if(__FIX10_6_MAXIMUM_VALUE_TO_I < vertex->pixelVector.y)
		{
			vertex->pixelVector.y = __FIX10_6_MAXIMUM_VALUE_TO_I;
		}
		*/
	}
}

void Mesh::draw(bool calculateParallax __attribute__((unused)))
{
	for(VirtualNode node = this->segments->head; NULL != node; node = node->next)
	{
		MeshSegment* meshSegment = (MeshSegment*)node->data;

		// draw the line in both buffers
		DirectDraw::drawColorLine(meshSegment->fromVertex->pixelVector, meshSegment->toVertex->pixelVector, this->meshSpec->color, meshSegment->bufferIndex, false);
	}
}

void Mesh::drawInterlaced(bool calculateParallax __attribute__((unused)))
{
	for(VirtualNode node = this->segments->head; NULL != node; node = node->next)
	{
		MeshSegment* meshSegment = (MeshSegment*)node->data;
		meshSegment->bufferIndex = !meshSegment->bufferIndex;

		// draw the line in both buffers
		DirectDraw::drawColorLine(meshSegment->fromVertex->pixelVector, meshSegment->toVertex->pixelVector, this->meshSpec->color, meshSegment->bufferIndex, true);

	//	PixelVector::print(meshSegment->fromVertex->pixelVector, 1, 10);
	//	PixelVector::print(meshSegment->toVertex->pixelVector, 11, 10);
	}
}

VirtualList Mesh::getVertices()
{
	return this->vertices;
}