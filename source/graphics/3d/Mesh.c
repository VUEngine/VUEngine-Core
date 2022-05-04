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

	Mesh::setup(this, this->meshSpec);
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

void Mesh::setup(MeshSpec* meshSpec)
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
		Vector3D startVector = meshSpec->segments[i][0];
		Vector3D endVector = meshSpec->segments[i][1];

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
static inline Vector3D Vector3D_rotateXAxis(Vector3D vector, int16 degrees)
{
	return (Vector3D) 
		{
			vector.x,
			__FIX10_6_MULT(vector.z, -__FIX7_9_TO_FIX10_6(__SIN(degrees))) + __FIX10_6_MULT(vector.y, __FIX7_9_TO_FIX10_6(__COS(degrees))),
			__FIX10_6_MULT(vector.z, __FIX7_9_TO_FIX10_6(__COS(degrees))) + __FIX10_6_MULT(vector.y, __FIX7_9_TO_FIX10_6(__SIN(degrees)))
		};
/*
	o->x = v->x;
	o->y = (F_MUL(v->z, -sine[degrees])) + (F_MUL(cosine[degrees], v->y));
	o->z = (F_MUL(v->z, cosine[degrees])) + (F_MUL(sine[degrees], v->y));
*/
}

static inline Vector3D Vector3D_rotateYAxis(Vector3D vector, int16 degrees)
{
	return (Vector3D) 
		{
			__FIX10_6_MULT(vector.x, __FIX7_9_TO_FIX10_6(__COS(degrees))) + __FIX10_6_MULT(vector.z, __FIX7_9_TO_FIX10_6(__SIN(degrees))),
			vector.y,
			__FIX10_6_MULT(vector.x, -__FIX7_9_TO_FIX10_6(__SIN(degrees))) + __FIX10_6_MULT(vector.z, __FIX7_9_TO_FIX10_6(__COS(degrees)))
		};
/*
	o->y = v->y;
	o->x = (F_MUL(v->x, cosine[degrees])) + (F_MUL(sine[degrees], v->z));
	o->z = (F_MUL(v->x, -sine[degrees])) + (F_MUL(cosine[degrees], v->z));
*/
}


static inline Vector3D Vector3D_rotateZAxis(Vector3D vector, int16 degrees)
{
	return (Vector3D) 
		{
			__FIX10_6_MULT(vector.x, __FIX7_9_TO_FIX10_6(__COS(degrees))) + __FIX10_6_MULT(vector.y, __FIX7_9_TO_FIX10_6(__SIN(degrees))),
			__FIX10_6_MULT(vector.x, -__FIX7_9_TO_FIX10_6(__SIN(degrees))) + __FIX10_6_MULT(vector.y, __FIX7_9_TO_FIX10_6(__COS(degrees))),
			vector.z,
		};
/*
	o->x = (F_MUL(v->x, cosine[degrees])) + (F_MUL(sine[degrees], v->y));
	o->y = (F_MUL(v->x, -sine[degrees])) + (F_MUL(cosine[degrees], v->y));
	o->z = v->z;
*/
}

void Mesh::render()
{
	for(VirtualNode node = this->segments->head; node; node = node->next)
	{
		MeshSegment* meshSegment = (MeshSegment*)node->data;

		// project to 2d coordinates
		if(!meshSegment->startPoint->projected)
		{
			Vector3D realStartPoint = meshSegment->startPoint->vector;
			
			if(0 != this->rotation->x)
			{
				realStartPoint = Vector3D_rotateXAxis(realStartPoint, this->rotation->x);
			}

			if(0 != this->rotation->y)
			{
				realStartPoint = Vector3D_rotateYAxis(realStartPoint, this->rotation->y);
			}

			if(0 != this->rotation->z)
			{
				realStartPoint = Vector3D_rotateZAxis(realStartPoint, this->rotation->z);
			}

			realStartPoint = Vector3D::sum(*this->position, realStartPoint);

			meshSegment->startPoint->pixelVector = Vector3D::projectToPixelVector(Vector3D::getRelativeToCamera(realStartPoint), 
			Optics::calculateParallax(realStartPoint.x, realStartPoint.z));
			meshSegment->startPoint->projected = true;
		}

		if(!meshSegment->endPoint->projected)
		{
			Vector3D realEndPoint = meshSegment->endPoint->vector;

			if(0 != this->rotation->x)
			{
				realEndPoint = Vector3D_rotateXAxis(realEndPoint, this->rotation->x);
			}

			if(0 != this->rotation->y)
			{
				realEndPoint = Vector3D_rotateYAxis(realEndPoint, this->rotation->y);
			}

			if(0 != this->rotation->z)
			{
				realEndPoint = Vector3D_rotateZAxis(realEndPoint, this->rotation->z);
			}

			realEndPoint = Vector3D::sum(*this->position, realEndPoint);

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
