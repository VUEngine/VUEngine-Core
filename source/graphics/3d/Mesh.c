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

//---------------------------------------------------------------------------------------------------------
//												CLASS'S DECLARATIONS
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
void Mesh::constructor(MeshSpec* meshSpec)
{
	// construct base object
	Base::constructor(meshSpec->color);

	this->segments = new VirtualList();
	this->meshSpec = meshSpec;
	this->position = NULL;
	this->rotation = NULL;

	Mesh::addSegments(this);
}

/**
 * Class destructor
 */
void Mesh::destructor()
{
	Mesh::hide(this);

	Mesh::deleteSegments(this);

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void Mesh::deleteSegments()
{
	for(VirtualNode node = this->segments->head; node; node = node->next)
	{
		MeshSegment* meshSegment = (MeshSegment*)node->data;

		if(!isDeleted(meshSegment->startPoint))
		{
			delete meshSegment->startPoint;
		}

		if(!isDeleted(meshSegment->endPoint))
		{
			delete meshSegment->endPoint;
		}

		delete meshSegment;
	}

	delete this->segments;
	this->segments = NULL;
}

void Mesh::addSegments()
{
	if(NULL != this->segments->head)
	{
		Mesh::deleteSegments(this);

		this->segments = new VirtualList();
	}

	bool isEndSegment = false;
	uint16 i = 0;

	do
	{
		Vector3D startVector = this->meshSpec->segments[i][0];
		Vector3D endVector = this->meshSpec->segments[i][1];

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
	newMeshSegment->startPoint = NULL;
	newMeshSegment->endPoint = NULL;

	for(VirtualNode node = this->segments->head; node; node = node->next)
	{
		MeshSegment* meshSegment = (MeshSegment*)node->data;

		if(Vector3D::areEqual(meshSegment->startPoint->vector, startVector))
		{
			newMeshSegment->startPoint = meshSegment->startPoint;
		}
		else if(Vector3D::areEqual(meshSegment->startPoint->vector, endVector))
		{
			newMeshSegment->endPoint = meshSegment->startPoint;
		}
		else if(Vector3D::areEqual(meshSegment->endPoint->vector, startVector))
		{
			newMeshSegment->startPoint = meshSegment->endPoint;
		}
		else if(Vector3D::areEqual(meshSegment->endPoint->vector, endVector))
		{
			newMeshSegment->endPoint = meshSegment->endPoint;
		}

		if(NULL != newMeshSegment->startPoint && NULL != newMeshSegment->endPoint)
		{
			break;
		}
	}

	if(NULL == newMeshSegment->startPoint)
	{
		newMeshSegment->startPoint = new MeshPoint;
		newMeshSegment->startPoint->vector = startVector;
		newMeshSegment->startPoint->pixelVector = (PixelVector){0, 0, 0, 0};
		newMeshSegment->startPoint->projected = false;
	}

	if(NULL == newMeshSegment->endPoint)
	{
		newMeshSegment->endPoint = new MeshPoint;
		newMeshSegment->endPoint->vector = endVector;
		newMeshSegment->endPoint->pixelVector = (PixelVector){0, 0, 0, 0};
		newMeshSegment->endPoint->projected = false;
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
	
	for(VirtualNode node = this->segments->head; node; node = node->next)
	{
		MeshSegment* meshSegment = (MeshSegment*)node->data;

		// project to 2d coordinates
		if(!meshSegment->startPoint->projected)
		{
			Vector3D realStartPoint = Vector3D::sum(position, Vector3D::rotate(meshSegment->startPoint->vector, rotation));

			meshSegment->startPoint->pixelVector = Vector3D::projectToPixelVector(Vector3D::getRelativeToCamera(realStartPoint), 
			Optics::calculateParallax(realStartPoint.x, realStartPoint.z));
			meshSegment->startPoint->projected = true;
		}

		if(!meshSegment->endPoint->projected)
		{
			Vector3D realEndPoint = Vector3D::sum(position, Vector3D::rotate(meshSegment->endPoint->vector, rotation));

			meshSegment->endPoint->pixelVector = Vector3D::projectToPixelVector(Vector3D::getRelativeToCamera(realEndPoint), 
			Optics::calculateParallax(realEndPoint.x, realEndPoint.z));
			meshSegment->endPoint->projected = true;
		}
	}
}

void Mesh::draw(bool calculateParallax __attribute__((unused)))
{
	for(VirtualNode node = this->segments->head; node; node = node->next)
	{
		MeshSegment* meshSegment = (MeshSegment*)node->data;
		meshSegment->startPoint->projected = false;
		meshSegment->endPoint->projected = false;

		// draw the line in both buffers
		DirectDraw::drawColorLine(DirectDraw::getInstance(), meshSegment->startPoint->pixelVector, meshSegment->endPoint->pixelVector, this->meshSpec->color);
	}
}

void Mesh::setup(const Vector3D* position __attribute__((unused)), const Rotation* rotation __attribute__((unused)), const Scale* scale __attribute__((unused)))
{
	this->position = position;
	this->rotation = rotation;
}
