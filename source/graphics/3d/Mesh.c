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
	newMeshSegment->bufferIndex = 0;

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

static PixelVector Mesh::projectVector(Vector3D vector, Vector3D position, Rotation rotation)
{
	extern Vector3D _cameraRealPosition;
	extern Rotation _cameraRealRotation;

	vector = Vector3D::sum(position, Vector3D::rotate(vector, _cameraRealRotation));

	vector = Vector3D::sub(vector, _cameraRealPosition);
//	vector = Vector3D::rotate(vector, _cameraRealRotation);
	vector = Vector3D::sum(vector, _cameraRealPosition);

	vector = Vector3D::getRelativeToCamera(vector);

	PixelVector pixelVector = Vector3D::projectToPixelVector(vector, Optics::calculateParallax(vector.x, vector.z));
/*
	// Pre clamp to prevent weird glitches due to overflows and speed up drawing
	if(-__FIX10_6_MAXIMUM_VALUE_TO_I > pixelVector.x)
	{
		pixelVector.x = -__FIX10_6_MAXIMUM_VALUE_TO_I;
	}
	else if(__FIX10_6_MAXIMUM_VALUE_TO_I < pixelVector.x)
	{
		pixelVector.x = __FIX10_6_MAXIMUM_VALUE_TO_I;
	}

	if(-__FIX10_6_MAXIMUM_VALUE_TO_I > pixelVector.y)
	{
		pixelVector.y = -__FIX10_6_MAXIMUM_VALUE_TO_I;
	}
	else if(__FIX10_6_MAXIMUM_VALUE_TO_I < pixelVector.y)
	{
		pixelVector.y = __FIX10_6_MAXIMUM_VALUE_TO_I;
	}
*/
	return pixelVector;
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
			meshSegment->startPoint->pixelVector = Mesh::projectVector(meshSegment->startPoint->vector, position, rotation);
			meshSegment->startPoint->projected = true;
		}

		if(!meshSegment->endPoint->projected)
		{
			meshSegment->endPoint->pixelVector = Mesh::projectVector(meshSegment->endPoint->vector, position, rotation);
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

		meshSegment->bufferIndex = !meshSegment->bufferIndex;

		// draw the line in both buffers
		DirectDraw::drawColorLine(meshSegment->startPoint->pixelVector, meshSegment->endPoint->pixelVector, this->meshSpec->color, __FIX10_6_MAXIMUM_VALUE_TO_I, meshSegment->bufferIndex);
	}
}

void Mesh::setup(const Vector3D* position __attribute__((unused)), const Rotation* rotation __attribute__((unused)), const Scale* scale __attribute__((unused)))
{
	this->position = position;
	this->rotation = rotation;
}
