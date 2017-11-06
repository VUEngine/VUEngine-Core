/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal 'me engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#include <Cuboid.h>
#include <Math.h>
#include <Vector.h>
#include <InverseCuboid.h>
#include <Optics.h>
#include <Polyhedron.h>
#include <Sphere.h>
#include <Math.h>
#include <HardwareManager.h>
#include <VirtualList.h>
#include <Printing.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Cuboid
 * @extends Shape
 * @ingroup physics
 */
__CLASS_DEFINITION(Cuboid, Shape);
__CLASS_FRIEND_DEFINITION(InverseCuboid);


//---------------------------------------------------------------------------------------------------------
//												DEFINES
//---------------------------------------------------------------------------------------------------------

#define __MAX_NUMBER_OF_PASSES			10
#define __FLOAT_0_5_F					0x00001000


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static u16 Cuboid_testIfCollisionWithCuboid(Cuboid this, Cuboid cuboid, VBVec3D displacement);
static void Cuboid_configurePolyhedron(Cuboid this, int renew);
static CollisionInformation Cuboid_overlapsCuboid(Cuboid this, Cuboid cuboid);
static CollisionInformation Cuboid_overlapsInverseCuboid(Cuboid this, InverseCuboid other);
static VBVec3D Cuboid_intervalDistance(RightCuboid rightCuboidA, RightCuboid rightCuboidB);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Cuboid, SpatialObject owner)
__CLASS_NEW_END(Cuboid, owner);


// class's constructor
void Cuboid_constructor(Cuboid this, SpatialObject owner)
{
	ASSERT(this, "Cuboid::constructor: null this");

	__CONSTRUCT_BASE(Shape, owner);

	this->polyhedron = NULL;

	this->rotationVertexDisplacement = (VBVec3D){0, 0, 0};

	this->normals = NULL;
}

// class's destructor
void Cuboid_destructor(Cuboid this)
{
	ASSERT(this, "Cuboid::destructor: null this");

	Cuboid_hide(this);

	if(this->normals)
	{
		__DELETE_BASIC(this->normals);
		this->normals = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// check if two rectangles overlap
CollisionInformation Cuboid_overlaps(Cuboid this, Shape shape)
{
	ASSERT(this, "Cuboid::overlaps: null this");

	if(__IS_INSTANCE_OF(Cuboid, shape))
	{
		return Cuboid_overlapsCuboid(this, __SAFE_CAST(Cuboid, shape));
	}
	else if(__IS_INSTANCE_OF(InverseCuboid, shape))
	{
		return Cuboid_overlapsInverseCuboid(this, __SAFE_CAST(InverseCuboid, shape));
	}

	return (CollisionInformation){NULL, NULL, {0, 0, 0}, __NO_AXIS};
}

void Cuboid_getVertexes(Cuboid this, VBVec3D vertexes[8])
{
	VBVec3D leftTopNear 	= {this->rightCuboid.x0, this->rightCuboid.y0, this->rightCuboid.z0};
	VBVec3D rightTopNear 	= {this->rightCuboid.x1, this->rightCuboid.y0, this->rightCuboid.z0};
	VBVec3D leftBottomNear 	= {this->rightCuboid.x0, this->rightCuboid.y1, this->rightCuboid.z0};
	VBVec3D rightBottomNear = {this->rightCuboid.x1, this->rightCuboid.y1, this->rightCuboid.z0};
	VBVec3D leftTopFar 		= {this->rightCuboid.x0, this->rightCuboid.y0, this->rightCuboid.z1};
	VBVec3D rightTopFar 	= {this->rightCuboid.x1, this->rightCuboid.y0, this->rightCuboid.z1};
	VBVec3D leftBottomFar 	= {this->rightCuboid.x0, this->rightCuboid.y1, this->rightCuboid.z1};
	VBVec3D rightBottomFar 	= {this->rightCuboid.x1, this->rightCuboid.y1, this->rightCuboid.z1};

	if(!this->rotationVertexDisplacement.z)
	{
		leftTopNear.y 		+= this->rotationVertexDisplacement.y;
		rightTopNear.x 		-= this->rotationVertexDisplacement.x;
		leftBottomNear.x 	+= this->rotationVertexDisplacement.x;
		rightBottomNear.y 	-= this->rotationVertexDisplacement.y;

		leftTopFar.y 		+= this->rotationVertexDisplacement.y;
		rightTopFar.x 		-= this->rotationVertexDisplacement.x;
		leftBottomFar.x 	+= this->rotationVertexDisplacement.x;
		rightBottomFar.y 	-= this->rotationVertexDisplacement.y;
	}
	else if(!this->rotationVertexDisplacement.y)
	{
		leftTopNear.x 		+= this->rotationVertexDisplacement.x;
		rightTopNear.z 		+= this->rotationVertexDisplacement.z;
		leftBottomNear.x 	+= this->rotationVertexDisplacement.x;
		rightBottomNear.z 	+= this->rotationVertexDisplacement.z;

		leftTopFar.z 		-= this->rotationVertexDisplacement.z;
		rightTopFar.x 		-= this->rotationVertexDisplacement.x;
		leftBottomFar.z 	-= this->rotationVertexDisplacement.z;
		rightBottomFar.x 	-= this->rotationVertexDisplacement.x;
	}
	else if(!this->rotationVertexDisplacement.x)
	{
		leftTopNear.z 		+= this->rotationVertexDisplacement.z;
		rightTopNear.z 		+= this->rotationVertexDisplacement.z;
		leftBottomNear.y 	-= this->rotationVertexDisplacement.y;
		rightBottomNear.y 	-= this->rotationVertexDisplacement.y;

		leftTopFar.y 		+= this->rotationVertexDisplacement.y;
		rightTopFar.y 		+= this->rotationVertexDisplacement.y;
		leftBottomFar.z 	-= this->rotationVertexDisplacement.z;
		rightBottomFar.z 	-= this->rotationVertexDisplacement.z;
	}

	vertexes[0] = leftTopNear;
	vertexes[1] = rightTopNear;
	vertexes[2] = leftBottomNear;
	vertexes[3] = rightBottomNear;

	vertexes[4] = leftTopFar;
	vertexes[5] = rightTopFar;
	vertexes[6] = leftBottomFar;
	vertexes[7] = rightBottomFar;
}

VBVec3D Vector_getPlaneNormal(VBVec3D vectorA, VBVec3D vectorB, VBVec3D vectorC);

void Cuboid_computeNormals(Cuboid this, VBVec3D vertexes[8])
{
/*
	// generic way
	normals[0] = Vector_getPlaneNormal(vertexes[6], vertexes[4], vertexes[0]);
	normals[1] = Vector_getPlaneNormal(vertexes[0], vertexes[4], vertexes[5]);
	normals[2] = Vector_getPlaneNormal(vertexes[0], vertexes[1], vertexes[3]);
*/

	// fast way given that the cubes are regular
	this->normals->vectors[0] = (VBVec3D)
	{
		vertexes[1].x - vertexes[0].x,
		vertexes[1].y - vertexes[0].y,
		vertexes[1].z - vertexes[0].z,
	};

	this->normals->vectors[1] = (VBVec3D)
	{
		vertexes[2].x - vertexes[0].x,
		vertexes[2].y - vertexes[0].y,
		vertexes[2].z - vertexes[0].z,
	};

	this->normals->vectors[2] = (VBVec3D)
	{
		vertexes[4].x - vertexes[0].x,
		vertexes[4].y - vertexes[0].y,
		vertexes[4].z - vertexes[0].z,
	};

	this->normals->vectors[0] = Vector_normalize(this->normals->vectors[0]);
	this->normals->vectors[1] = Vector_normalize(this->normals->vectors[1]);
	this->normals->vectors[2] = Vector_normalize(this->normals->vectors[2]);
}

void Cuboid_project(VBVec3D vertexes[8], VBVec3D vector, fix19_13* min, fix19_13* max)
{
	int vertexIndex = 0;

	// project this onto the current normal
	fix19_13 dotProduct = Vector_dotProduct(vector, vertexes[vertexIndex]);

	*min = dotProduct;
	*max = dotProduct;

	// project this onto the current normal
	for(; vertexIndex < __CUBOID_VERTEXES; vertexIndex++)
	{
		dotProduct = Vector_dotProduct(vector, vertexes[vertexIndex]);

		if(dotProduct < *min)
		{
			*min = dotProduct;
		}
		else if(dotProduct > *max)
		{
			*max = dotProduct;
		}
	}
}

VBVec3D Cuboid_checkCollision(Cuboid this, Cuboid cuboid)
{
	// get the vertexes of each cuboid
	VBVec3D vertexes[2][__CUBOID_VERTEXES];
	Cuboid_getVertexes(this, vertexes[0]);
	Cuboid_getVertexes(cuboid, vertexes[1]);

	// if the normals have not been computed yet do so now
	if(!this->normals)
	{
		this->normals = __NEW_BASIC(Normals);
		Cuboid_computeNormals(this, vertexes[0]);
	}

	if(!cuboid->normals)
	{
		cuboid->normals = __NEW_BASIC(Normals);
		Cuboid_computeNormals(cuboid, vertexes[1]);
	}

	VBVec3D* normals[2] =
	{
		this->normals->vectors,
		cuboid->normals->vectors,
	};

	// will need
	VBVec3D centers[2] =
	{
		{
			(this->rightCuboid.x0 + this->rightCuboid.x1) >> 1,
			(this->rightCuboid.y0 + this->rightCuboid.y1) >> 1,
			(this->rightCuboid.z0 + this->rightCuboid.z1) >> 1,
		},
		{
			(cuboid->rightCuboid.x0 + cuboid->rightCuboid.x1) >> 1,
			(cuboid->rightCuboid.y0 + cuboid->rightCuboid.y1) >> 1,
			(cuboid->rightCuboid.z0 + cuboid->rightCuboid.z1) >> 1,
		}
	};

	VBVec3D distanceVector = Vector_get(centers[1], centers[0]);

	VBVec3D minimumTranslationVector = {0, 0, 0};
	fix19_13 minimumIntervalDistance = Math_fix19_13Infinity();

	int cuboidIndex = 0;

	// has to project all points on all the normals of both cuboids
	for(; cuboidIndex < 2; cuboidIndex++)
	{
		int normalIndex = 0;

		// test all 3 normals of each cuboid
		for(; normalIndex < __CUBOID_NORMALS; normalIndex++)
		{
			VBVec3D currentNormal = normals[cuboidIndex][normalIndex];

			fix19_13 min[2] = {0, 0};
			fix19_13 max[2] = {0, 0};

			Cuboid_project(vertexes[0], currentNormal, &min[0], &max[0]);
			Cuboid_project(vertexes[1], currentNormal, &min[1], &max[1]);

			fix19_13 intervalDistance = 0;

			if (min[0] < min[1])
			{
				intervalDistance = min[1] - max[0];
			}
			else
			{
				intervalDistance = min[0] - max[1];
			}

			if(0 < intervalDistance)
			{
				return (VBVec3D){0, 0, 0};
			}

			intervalDistance = __ABS(intervalDistance);
			if(intervalDistance < minimumIntervalDistance)
			{
				minimumIntervalDistance = intervalDistance;
				minimumTranslationVector = currentNormal;

				if(Vector_dotProduct(distanceVector, minimumTranslationVector) < 0)
				{
					minimumTranslationVector = Vector_scalarProduct(minimumTranslationVector, __I_TO_FIX19_13(-1));
				}
			}
		}
	}

	minimumTranslationVector = Vector_scalarProduct(minimumTranslationVector, minimumIntervalDistance);

	return minimumTranslationVector;
}

VBVec3D Cuboid_computeMinimumTranslationVector(Cuboid this, Shape shape)
{
	ASSERT(this, "Cuboid::computeMinimumTranslationVector: null this");

	if(__IS_INSTANCE_OF(Cuboid, shape))
	{
		return Cuboid_checkCollision(this, __SAFE_CAST(Cuboid, shape));
	}
	else if(__IS_INSTANCE_OF(InverseCuboid, shape))
	{
		//return Cuboid_overlapsInverseCuboid(this, __SAFE_CAST(InverseCuboid, shape));
	}

	return (VBVec3D) {0, 0, 0};
}

static VBVec3D Cuboid_intervalDistance(RightCuboid rightCuboidA, RightCuboid rightCuboidB)
{
	return (VBVec3D)
	{
		(rightCuboidA.x0 + rightCuboidA.x1) >> 1 < (rightCuboidB.x0 + rightCuboidB.x1) >> 1 ? (rightCuboidB.x0 - rightCuboidA.x1) : (rightCuboidA.x0 - rightCuboidB.x1),
		(rightCuboidA.y0 + rightCuboidA.y1) >> 1 < (rightCuboidB.y0 + rightCuboidB.y1) >> 1 ? (rightCuboidB.y0 - rightCuboidA.y1) : (rightCuboidA.y0 - rightCuboidB.y1),
		(rightCuboidA.z0 + rightCuboidA.z1) >> 1 < (rightCuboidB.z0 + rightCuboidB.z1) >> 1 ? (rightCuboidB.z0 - rightCuboidA.z1) : (rightCuboidA.z0 - rightCuboidB.z1),
	};
}

// check if overlaps with other rect
CollisionInformation Cuboid_overlapsCuboid(Cuboid this, Cuboid cuboid)
{
	ASSERT(this, "Cuboid::overlapsCuboid: null this");

	VBVec3D intervalDistance = Cuboid_intervalDistance(this->rightCuboid, cuboid->rightCuboid);

	// test for collision
	if(0 > intervalDistance.x && 0 > intervalDistance.y && 0 > intervalDistance.z)
	{
		// check if both cuboids are axis aligned
		bool isThisRotated = this->rotationVertexDisplacement.x | this->rotationVertexDisplacement.y | this->rotationVertexDisplacement.z ? true : false;
		bool isCuboidRotated = cuboid->rotationVertexDisplacement.x | cuboid->rotationVertexDisplacement.y | cuboid->rotationVertexDisplacement.z ? true : false;
		bool pendingSATCheck = isThisRotated || isCuboidRotated;

		VBVec3D minimumTranslationVector = {0, 0, 0};
		fix19_13 minimumIntervalDistance = Math_fix19_13Infinity();

		// if axis aligned, then SAT check is not needed
		// and we can calculate the minimum displacement vector
		// to resolve the collision right now
		if(!pendingSATCheck)
		{
			VBVec3D thisCenter =
			{
				(this->rightCuboid.x0 + this->rightCuboid.x1) >> 1,
				(this->rightCuboid.y0 + this->rightCuboid.y1) >> 1,
				(this->rightCuboid.z0 + this->rightCuboid.z1) >> 1,
			};

			VBVec3D cuboidCenter =
			{
				(cuboid->rightCuboid.x0 + cuboid->rightCuboid.x1) >> 1,
				(cuboid->rightCuboid.y0 + cuboid->rightCuboid.y1) >> 1,
				(cuboid->rightCuboid.z0 + cuboid->rightCuboid.z1) >> 1,
			};

			VBVec3D distanceVector = Vector_get(cuboidCenter, thisCenter);

			VBVec3D normals[__CUBOID_NORMALS] =
			{
				{__I_TO_FIX19_13(1), 0, 0},
				{0, __I_TO_FIX19_13(1), 0},
				{0, 0, __I_TO_FIX19_13(1)},
			};

			int i = 0;
			fix19_13* component = &intervalDistance.x;

			for(i = 0; i < __CUBOID_NORMALS; i++)
			{
				fix19_13 intervalDistance = __ABS(component[i]);

				if(intervalDistance < minimumIntervalDistance)
				{
					minimumIntervalDistance = intervalDistance;
					minimumTranslationVector = normals[i];

					if(Vector_dotProduct(distanceVector, minimumTranslationVector) < 0)
					{
						minimumTranslationVector = Vector_scalarProduct(minimumTranslationVector, __I_TO_FIX19_13(-1));
					}
				}
			}

			minimumTranslationVector = Vector_scalarProduct(minimumTranslationVector, minimumIntervalDistance);
		}

		return (CollisionInformation){__SAFE_CAST(Shape, this), __SAFE_CAST(Shape, cuboid), minimumTranslationVector, pendingSATCheck};
	}

	return (CollisionInformation){NULL, NULL, {0, 0, 0}, __NO_AXIS};
}

CollisionInformation Cuboid_overlapsInverseCuboid(Cuboid this, InverseCuboid inverseCuboid)
{
	ASSERT(this, "Cuboid::overlapsInverseCuboid: null this");

	// test for collision
	if((this->rightCuboid.x0 < inverseCuboid->rightCuboid.x0) | (this->rightCuboid.x1 > inverseCuboid->rightCuboid.x1) |
	 (this->rightCuboid.y0 < inverseCuboid->rightCuboid.y0) | (this->rightCuboid.y1 > inverseCuboid->rightCuboid.y1) |
	 (this->rightCuboid.z0 < inverseCuboid->rightCuboid.z0) | (this->rightCuboid.z1 > inverseCuboid->rightCuboid.z1)
	)
	{
		u8 pendingSATCheck = true;
		return (CollisionInformation){__SAFE_CAST(Shape, this), __SAFE_CAST(Shape, inverseCuboid), {0, 0, 0}, pendingSATCheck};
	}

	return (CollisionInformation){NULL, NULL, {0, 0, 0}, __NO_AXIS};
}

void Cuboid_setup(Cuboid this, const VBVec3D* position, const Rotation* rotation, const Scale* scale, const Size* size)
{
	ASSERT(this, "Cuboid::setup: null this");

	this->rotationVertexDisplacement.x = 0;
	this->rotationVertexDisplacement.y = 0;
	this->rotationVertexDisplacement.z = 0;

	Size surroundingCuboidSize = *size;

	// angle | theta | psi
	if(rotation->z | rotation->y | rotation->x)
	{
		fix19_13 width = __I_TO_FIX19_13(surroundingCuboidSize.x) >> 1;
		fix19_13 height = __I_TO_FIX19_13(surroundingCuboidSize.y) >> 1;
		fix19_13 depth = __I_TO_FIX19_13(surroundingCuboidSize.z) >> 1;

		// allow only one rotation
		if(rotation->z)
		{
			// clamp value around 256 degrees (180) to avoid conditionals later when calculating rotationVertexDisplacement
			s16 angle = rotation->z - ((rotation->z / 256) << 8);
			angle = angle < 0 ? 256 + angle : angle;

			// calculate position of cuboid's right-bottom corner
			fix19_13 sinAngle = __FIX7_9_TO_FIX19_13(__SIN(angle));
			fix19_13 cosAngle = __FIX7_9_TO_FIX19_13(__COS(angle));

			// use vectors (x1, y0, z1) and (x1, y1, z1)
			VBVec3D topRight =
			{
				__FIX19_13_MULT(width, cosAngle) - __FIX19_13_MULT(-height, sinAngle),
				__FIX19_13_MULT(width, sinAngle) + __FIX19_13_MULT(-height, cosAngle),
				depth
			};

			VBVec3D bottomRight =
			{
				__FIX19_13_MULT(width, cosAngle) - __FIX19_13_MULT(height, sinAngle),
				__FIX19_13_MULT(width, sinAngle) + __FIX19_13_MULT(height, cosAngle),
				depth
			};

			VBVec3D topRightHelper =
			{
				__ABS(topRight.x),
				__ABS(topRight.y),
				__ABS(topRight.z),
			};

			VBVec3D bottomRightHelper =
			{
				__ABS(bottomRight.x),
				__ABS(bottomRight.y),
				__ABS(bottomRight.z),
			};

			surroundingCuboidSize.x = __FIX19_13_TO_I(bottomRightHelper.x > topRightHelper.x ? bottomRightHelper.x : topRightHelper.x) << 1;
			surroundingCuboidSize.y = __FIX19_13_TO_I(bottomRightHelper.y > topRightHelper.y ? bottomRightHelper.y : topRightHelper.y) << 1;

			// find the displacement over each axis for the rotated cuboid
			this->rotationVertexDisplacement.x = bottomRightHelper.x < topRightHelper.x ? bottomRight.x : topRight.x;
			this->rotationVertexDisplacement.y = bottomRightHelper.y < topRightHelper.y ? bottomRight.y : topRight.y;
			this->rotationVertexDisplacement.y = angle >= 128 ? -this->rotationVertexDisplacement.y : this->rotationVertexDisplacement.y;

			this->rotationVertexDisplacement.x = (__I_TO_FIX19_13(surroundingCuboidSize.x) >> 1) + this->rotationVertexDisplacement.x;
			this->rotationVertexDisplacement.y = (__I_TO_FIX19_13(surroundingCuboidSize.y) >> 1) - this->rotationVertexDisplacement.y;

			if(!(__MODULO(angle, 128)))
			{
				this->rotationVertexDisplacement.x = 0;
				this->rotationVertexDisplacement.y = 0;
			}
		}
		else if(rotation->y)
		{
			// clamp value around 256 degrees (180) to avoid conditionals later when calculating rotationVertexDisplacement
			s16 angle = rotation->y - ((rotation->y / 256) << 8);
			angle = angle < 0 ? 256 + angle : angle;

			// calculate position of cuboid's right-bottom corner
			fix19_13 sinAngle = __FIX7_9_TO_FIX19_13(__SIN(angle));
			fix19_13 cosAngle = __FIX7_9_TO_FIX19_13(__COS(0));

			// use vectors (x0, y1, z0) and (x1, y1, z0)
			VBVec3D bottomLeft =
			{
				__FIX19_13_MULT(-width, cosAngle) + __FIX19_13_MULT(-depth, sinAngle),
				height,
				-__FIX19_13_MULT(-width, sinAngle) + __FIX19_13_MULT(-depth, cosAngle),
			};

			VBVec3D bottomRight =
			{
				__FIX19_13_MULT(width, cosAngle) + __FIX19_13_MULT(-depth, sinAngle),
				height,
				-__FIX19_13_MULT(width, sinAngle) + __FIX19_13_MULT(-depth, cosAngle),
			};

			VBVec3D bottomLeftHelper =
			{
				__ABS(bottomLeft.x),
				__ABS(bottomLeft.y),
				__ABS(bottomLeft.z),
			};

			VBVec3D bottomRightHelper =
			{
				__ABS(bottomRight.x),
				__ABS(bottomRight.y),
				__ABS(bottomRight.z),
			};

			surroundingCuboidSize.x = __FIX19_13_TO_I(bottomLeftHelper.x > bottomRightHelper.x ? bottomLeftHelper.x : bottomRightHelper.x) << 1;
			surroundingCuboidSize.z = __FIX19_13_TO_I(bottomLeftHelper.z > bottomRightHelper.z ? bottomLeftHelper.z : bottomRightHelper.z) << 1;

			// find the displacement over each axis for the rotated cuboid
			this->rotationVertexDisplacement.x = bottomLeftHelper.x < bottomRightHelper.x ? bottomLeft.x : bottomRight.x;
			this->rotationVertexDisplacement.x = angle >= 128 ? -this->rotationVertexDisplacement.x : this->rotationVertexDisplacement.x;
			this->rotationVertexDisplacement.z = bottomLeftHelper.z < bottomRightHelper.z ? bottomLeft.z : bottomRight.z;

			this->rotationVertexDisplacement.x = (__I_TO_FIX19_13(surroundingCuboidSize.x) >> 1) - this->rotationVertexDisplacement.x;
			this->rotationVertexDisplacement.z = (__I_TO_FIX19_13(surroundingCuboidSize.z) >> 1) + this->rotationVertexDisplacement.z;

			if(!(__MODULO(angle, 128)))
			{
				this->rotationVertexDisplacement.x = 0;
				this->rotationVertexDisplacement.z = 0;
			}
		}
		else if(rotation->x)
		{
			// clamp value around 256 degrees (180) to avoid conditionals later when calculating rotationVertexDisplacement
			s16 angle = rotation->x - ((rotation->x / 256) << 8);
			angle = angle < 0 ? 256 + angle : angle;

			// calculate position of cuboid's right-bottom corner
			fix19_13 sinAngle = __FIX7_9_TO_FIX19_13(__SIN(angle));
			fix19_13 cosAngle = __FIX7_9_TO_FIX19_13(__COS(angle));

			// use vectors (x1, y1, z0) and (x1, y1, z1)
			VBVec3D bottomNear =
			{
				width,
				__FIX19_13_MULT(height, cosAngle) - __FIX19_13_MULT(-depth, sinAngle),
				__FIX19_13_MULT(height, sinAngle) + __FIX19_13_MULT(-depth, cosAngle),
			};

			VBVec3D bottomFar =
			{
				width,
				__FIX19_13_MULT(height, cosAngle) - __FIX19_13_MULT(depth, sinAngle),
				__FIX19_13_MULT(height, sinAngle) + __FIX19_13_MULT(depth, cosAngle),
			};

			VBVec3D bottomNearHelper =
			{
				__ABS(bottomNear.x),
				__ABS(bottomNear.y),
				__ABS(bottomNear.z),
			};

			VBVec3D bottomFarHelper =
			{
				__ABS(bottomFar.x),
				__ABS(bottomFar.y),
				__ABS(bottomFar.z),
			};

			surroundingCuboidSize.y = __FIX19_13_TO_I(bottomFarHelper.y > bottomNearHelper.y ? bottomFarHelper.y : bottomNearHelper.y) << 1;
			surroundingCuboidSize.z = __FIX19_13_TO_I(bottomFarHelper.z > bottomNearHelper.z ? bottomFarHelper.z : bottomNearHelper.z) << 1;

			// find the displacement over each axis for the rotated cuboid
			this->rotationVertexDisplacement.y = bottomFarHelper.y < bottomNearHelper.y ? bottomFar.y : bottomNear.y;
			this->rotationVertexDisplacement.z = bottomFarHelper.z < bottomNearHelper.z ? bottomFar.z : bottomNear.z;
			this->rotationVertexDisplacement.z = angle >= 128 ? -this->rotationVertexDisplacement.z : this->rotationVertexDisplacement.z;

			this->rotationVertexDisplacement.y = (__I_TO_FIX19_13(surroundingCuboidSize.y) >> 1) - this->rotationVertexDisplacement.y;
			this->rotationVertexDisplacement.z = (__I_TO_FIX19_13(surroundingCuboidSize.z) >> 1) + this->rotationVertexDisplacement.z;

			if(!(__MODULO(angle, 128)))
			{
				this->rotationVertexDisplacement.y = 0;
				this->rotationVertexDisplacement.z = 0;
			}
		}

		if(this->normals)
		{
			VBVec3D vertexes[__CUBOID_VERTEXES];

			Cuboid_getVertexes(this, vertexes);
			Cuboid_computeNormals(this, vertexes);
		}
	}

	// cuboid's center if placed on P(0, 0, 0)
	this->rightCuboid.x1 = __I_TO_FIX19_13(surroundingCuboidSize.x >> 1);
	this->rightCuboid.y1 = __I_TO_FIX19_13(surroundingCuboidSize.y >> 1);
	this->rightCuboid.z1 = __I_TO_FIX19_13(surroundingCuboidSize.z >> 1);

	this->rightCuboid.x0 = -this->rightCuboid.x1;
	this->rightCuboid.y0 = -this->rightCuboid.y1;
	this->rightCuboid.z0 = -this->rightCuboid.z1;

	// position the shape to avoid in real time calculation
	this->rightCuboid.x0 += position->x;
	this->rightCuboid.x1 += position->x;
	this->rightCuboid.y0 += position->y;
	this->rightCuboid.y1 += position->y;
	this->rightCuboid.z0 += position->z;
	this->rightCuboid.z1 += position->z;

	// no more setup needed
	this->ready = true;
}

// retrieve rightCuboid
RightCuboid Cuboid_getRightCuboid(Cuboid this)
{
	ASSERT(this, "Cuboid::getRightCuboid: null this");

	return this->rightCuboid;
}

// retrieve rightCuboid
RightCuboid Cuboid_getPositionedRightCuboid(Cuboid this)
{
	ASSERT(this, "Cuboid::getPositionedRightCuboid: null this");

	return this->rightCuboid;
}

// test if collision with the entity give the displacement
bool Cuboid_testIfCollision(Cuboid this, Shape collidingShape, VBVec3D displacement)
{
	ASSERT(this, "Cuboid::testIfCollision: null this");

	if(__IS_INSTANCE_OF(Cuboid, collidingShape))
	{
		return Cuboid_testIfCollisionWithCuboid(this, __SAFE_CAST(Cuboid, collidingShape), displacement);
	}
	// TODO: implement
//	else if(__IS_INSTANCE_OF(InverseCuboid, shape))

	return false;
}

VBVec3D Cuboid_getPosition(Cuboid this)
{
	ASSERT(this, "Cuboid::getPosition: null this");

	VBVec3D position =
	{
		this->rightCuboid.x0 + ((this->rightCuboid.x1 - this->rightCuboid.x0) >> 1),
		this->rightCuboid.y0 + ((this->rightCuboid.y1 - this->rightCuboid.y0) >> 1),
		this->rightCuboid.z0 + ((this->rightCuboid.z1 - this->rightCuboid.z0) >> 1),
	};

	return position;
}

RightCuboid Cuboid_getSurroundingRightCuboid(Cuboid this)
{
	ASSERT(this, "Cuboid::getSurroundingRightCuboid: null this");

	return this->rightCuboid;
}

RightCuboid Cuboid_getPositionedSurroundingRightCuboid(Cuboid this)
{
	ASSERT(this, "Cuboid::getPositionedSurroundingRightCuboid: null this");

	return this->rightCuboid;
}

// test if collision with the entity give the displacement
static u16 Cuboid_testIfCollisionWithCuboid(Cuboid this, Cuboid cuboid, VBVec3D displacement)
{
	ASSERT(this, "Cuboid::testIfCollisionWithCuboid: null this");

	// setup a cuboid representing the previous position
	RightCuboid displacedRightCuboid = this->rightCuboid;

	displacedRightCuboid.x0 += displacement.x;
	displacedRightCuboid.x1 += displacement.x;

	displacedRightCuboid.y0 += displacement.y;
	displacedRightCuboid.y1 += displacement.y;

	displacedRightCuboid.z0 += displacement.z;
	displacedRightCuboid.z1 += displacement.z;


	return false;
}

// configure Polyhedron
static void Cuboid_configurePolyhedron(Cuboid this, int renew)
{
	ASSERT(this, "Cuboid::draw: null this");

	if(renew)
	{
		Cuboid_hide(this);
	}
	else if(this->polyhedron)
	{
		return;
	}

	// create a Polyhedron
	this->polyhedron = __NEW(Polyhedron);

	// add vertices
/*
	Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y0, this->rightCuboid.z0);
	Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y0, this->rightCuboid.z0);
	Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y1, this->rightCuboid.z0);
	Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y1, this->rightCuboid.z0);
	Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y0, this->rightCuboid.z0);
/*	Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y0, this->rightCuboid.z1);
	Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y0, this->rightCuboid.z1);
	Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y1, this->rightCuboid.z1);
	Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y1, this->rightCuboid.z1);
	Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y0, this->rightCuboid.z1);
*/

	if(this->rotationVertexDisplacement.x | this->rotationVertexDisplacement.y | this->rotationVertexDisplacement.z)
	{
		if(!this->rotationVertexDisplacement.z)
		{
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y0 + this->rotationVertexDisplacement.y, this->rightCuboid.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1 - this->rotationVertexDisplacement.x, this->rightCuboid.y0, this->rightCuboid.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y1 - this->rotationVertexDisplacement.y, this->rightCuboid.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0 + this->rotationVertexDisplacement.x, this->rightCuboid.y1, this->rightCuboid.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y0 + this->rotationVertexDisplacement.y, this->rightCuboid.z0);
/*
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y0 + this->rotationVertexDisplacement.y, this->rightCuboid.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1 - this->rotationVertexDisplacement.x, this->rightCuboid.y0, this->rightCuboid.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y1 - this->rotationVertexDisplacement.y, this->rightCuboid.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0 + this->rotationVertexDisplacement.x, this->rightCuboid.y1, this->rightCuboid.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y0 + this->rotationVertexDisplacement.y, this->rightCuboid.z1);
*/		}

		if(!this->rotationVertexDisplacement.y)
		{
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0 + this->rotationVertexDisplacement.x, this->rightCuboid.y0, this->rightCuboid.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y0, this->rightCuboid.z0 + this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1 - this->rotationVertexDisplacement.x, this->rightCuboid.y0, this->rightCuboid.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y0, this->rightCuboid.z1 - this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0 + this->rotationVertexDisplacement.x, this->rightCuboid.y0, this->rightCuboid.z0);

			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0 + this->rotationVertexDisplacement.x, this->rightCuboid.y1, this->rightCuboid.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y1, this->rightCuboid.z0 + this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1 - this->rotationVertexDisplacement.x, this->rightCuboid.y1, this->rightCuboid.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y1, this->rightCuboid.z1 - this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0 + this->rotationVertexDisplacement.x, this->rightCuboid.y1, this->rightCuboid.z0);
		}

		if(!this->rotationVertexDisplacement.x)
		{
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y0, this->rightCuboid.z0 + this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y0 + this->rotationVertexDisplacement.y, this->rightCuboid.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y1, this->rightCuboid.z1 - this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y1 - this->rotationVertexDisplacement.y, this->rightCuboid.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x0, this->rightCuboid.y0, this->rightCuboid.z0 + this->rotationVertexDisplacement.z);

			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y0, this->rightCuboid.z0 + this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y0 + this->rotationVertexDisplacement.y, this->rightCuboid.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y1, this->rightCuboid.z1 - this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y1 - this->rotationVertexDisplacement.y, this->rightCuboid.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightCuboid.x1, this->rightCuboid.y0, this->rightCuboid.z0 + this->rotationVertexDisplacement.z);
		}
	}
}

// show me
void Cuboid_show(Cuboid this)
{
	ASSERT(this, "Cuboid::draw: null this");

//	Cuboid_configurePolyhedron(this, __VIRTUAL_CALL(SpatialObject, moves, this->owner) || !this->ready);
	Cuboid_configurePolyhedron(this, true);

	// draw the Polyhedron
	Wireframe_show(__SAFE_CAST(Wireframe, this->polyhedron));
}

// hide polyhedron
void Cuboid_hide(Cuboid this)
{
	if(this->polyhedron)
	{
		// draw the Polyhedron
		__DELETE(this->polyhedron);

		this->polyhedron = NULL;
	}
}

// print debug data
void Cuboid_print(Cuboid this, int x, int y)
{
	ASSERT(this, "Cuboid::print: null this");

	RightCuboid rightCuboid = this->rightCuboid;

	Printing_text(Printing_getInstance(), "X:" , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(rightCuboid.x0), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(rightCuboid.x1), x + 7, y++, NULL);

	Printing_text(Printing_getInstance(), "Y:" , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(rightCuboid.y0), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(rightCuboid.y1), x + 7, y++, NULL);

	Printing_text(Printing_getInstance(), "Z:" , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(rightCuboid.z0), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(rightCuboid.z1), x + 7, y++, NULL);
}
