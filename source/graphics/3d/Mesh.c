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

/**
 * Get pixel right box
 */
PixelRightBox Mesh::getPixelRightBox()
{
	PixelRightBox pixelRightBox = {0, 0, 0, 0, 0, 0};

	for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
	{
		Vertex* vertex = (Vertex*)node->data;

		PixelVector pixelVector = PixelVector::getFromVector3D(vertex->vector, 0);

		if(pixelVector.x < pixelRightBox.x0)
		{
			pixelRightBox.x0 = pixelVector.x;
		}

		if(pixelVector.x > pixelRightBox.x1)
		{
			pixelRightBox.x1 = pixelVector.x;
		}

		if(pixelVector.y < pixelRightBox.y0)
		{
			pixelRightBox.y0 = pixelVector.y;
		}

		if(pixelVector.y > pixelRightBox.y1)
		{
			pixelRightBox.y1 = pixelVector.y;
		}

		if(pixelVector.z < pixelRightBox.z0)
		{
			pixelRightBox.z0 = pixelVector.z;
		}

		if(pixelVector.z > pixelRightBox.z1)
		{
			pixelRightBox.z1 = pixelVector.z;
		}
	}

	return pixelRightBox;
}

static PixelRightBox Mesh::getPixelRightBoxFromSpec(MeshSpec* meshSpec)
{
	PixelRightBox pixelRightBox = {0, 0, 0, 0, 0, 0};

	bool isEndSegment = false;
	uint16 i = 0;

	do
	{
		Vector3D startVector = Vector3D::getFromPixelVector(meshSpec->segments[i][0]);
		Vector3D endVector = Vector3D::getFromPixelVector(meshSpec->segments[i][1]);

		isEndSegment = Vector3D::areEqual(Vector3D::zero(), startVector) && Vector3D::areEqual(Vector3D::zero(), endVector);

		if(!isEndSegment)
		{
			PixelVector startPixelVector = PixelVector::getFromVector3D(startVector, 0);
			PixelVector endPixelVector = PixelVector::getFromVector3D(endVector, 0);

			if(startPixelVector.x < endPixelVector.x)
			{
				if(startPixelVector.x < pixelRightBox.x0)
				{
					pixelRightBox.x0 = startPixelVector.x;
				}
			}
			else
			{
				if(endPixelVector.x < pixelRightBox.x0)
				{
					pixelRightBox.x0 = endPixelVector.x;
				}
			}

			if(startPixelVector.x > endPixelVector.x)
			{
				if(startPixelVector.x > pixelRightBox.x1)
				{
					pixelRightBox.x1 = startPixelVector.x;
				}
			}
			else
			{
				if(endPixelVector.x > pixelRightBox.x1)
				{
					pixelRightBox.x1 = endPixelVector.x;
				}
			}

			if(startPixelVector.y < endPixelVector.y)
			{
				if(startPixelVector.y < pixelRightBox.y0)
				{
					pixelRightBox.y0 = startPixelVector.y;
				}
			}
			else
			{
				if(endPixelVector.y < pixelRightBox.y0)
				{
					pixelRightBox.y0 = endPixelVector.y;
				}
			}

			if(startPixelVector.y > endPixelVector.y)
			{
				if(startPixelVector.y > pixelRightBox.y1)
				{
					pixelRightBox.y1 = startPixelVector.y;
				}
			}
			else
			{
				if(endPixelVector.y > pixelRightBox.y1)
				{
					pixelRightBox.y1 = endPixelVector.y;
				}
			}
						
			if(startPixelVector.z < endPixelVector.z)
			{
				if(startPixelVector.z < pixelRightBox.z0)
				{
					pixelRightBox.z0 = startPixelVector.z;
				}
			}
			else
			{
				if(endPixelVector.z < pixelRightBox.z0)
				{
					pixelRightBox.z0 = endPixelVector.z;
				}
			}

			if(startPixelVector.z > endPixelVector.z)
			{
				if(startPixelVector.z > pixelRightBox.z1)
				{
					pixelRightBox.z1 = startPixelVector.z;
				}
			}
			else
			{
				if(endPixelVector.z > pixelRightBox.z1)
				{
					pixelRightBox.z1 = endPixelVector.z;
				}
			}
		}

		i++;
	}
	while(!isEndSegment);

	return pixelRightBox;
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
	NM_ASSERT(NULL != this->position, "Mesh::render: NULL position");
	NM_ASSERT(NULL != this->rotation, "Mesh::render: NULL rotation");
	NM_ASSERT(NULL != this->scale, "Mesh::render: NULL scale");

	extern Vector3D _previousCameraPosition;
	extern Rotation _previousCameraInvertedRotation;

	Vector3D relativePosition = Vector3D::sub(*this->position, _previousCameraPosition);
	Mesh::setupRenderingMode(this, &relativePosition);

	if(__COLOR_BLACK == this->color)
	{
		return;
	}

	bool scale = (__1I_FIX7_9 != this->scale->x) + (__1I_FIX7_9 != this->scale->y) + (__1I_FIX7_9 != this->scale->z);
	bool rotate = (0 != this->rotation->x) + (0 != this->rotation->y) + (0 != this->rotation->z);

	if(!scale && !rotate)
	{
		for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
		{
			Vertex* vertex = (Vertex*)node->data;

			Vector3D vector = Vector3D::rotate(Vector3D::sum(relativePosition, vertex->vector), _previousCameraInvertedRotation);

			vertex->pixelVector = Vector3D::projectToPixelVector(vector, Optics::calculateParallax(vector.z));
		}
	}
	else if(scale && rotate)
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
	else if(rotate)
	{
		Rotation rotation = *this->rotation;

		for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
		{
			Vertex* vertex = (Vertex*)node->data;

			Vector3D vector = Vector3D::rotate(Vector3D::sum(relativePosition, Vector3D::rotate(vertex->vector, rotation)), _previousCameraInvertedRotation);

			vertex->pixelVector = Vector3D::projectToPixelVector(vector, Optics::calculateParallax(vector.z));
		}
	}
	else if(scale)
	{
		Scale scale = *this->scale;

		for(VirtualNode node = this->vertices->head; NULL != node; node = node->next)
		{
			Vertex* vertex = (Vertex*)node->data;

			Vector3D vector = Vector3D::rotate(Vector3D::sum(relativePosition, Vector3D::scale(vertex->vector, scale)), _previousCameraInvertedRotation);

			vertex->pixelVector = Vector3D::projectToPixelVector(vector, Optics::calculateParallax(vector.z));
		}
	}
}

/**
 * Draw
 */
void Mesh::draw()
{
	NM_ASSERT(NULL != this->position, "Mesh::draw: NULL position");
	NM_ASSERT(NULL != this->rotation, "Mesh::draw: NULL position");
	NM_ASSERT(NULL != this->scale, "Mesh::draw: NULL position");

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