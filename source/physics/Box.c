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

#include <Box.h>
#include <InverseBox.h>
#include <Ball.h>
#include <CollisionHelper.h>
#include <Vector3D.h>
#include <Optics.h>
#include <Polyhedron.h>
#include <HardwareManager.h>
#include <VirtualList.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Box
 * @extends Shape
 * @ingroup physics
 */
__CLASS_DEFINITION(Box, Shape);
__CLASS_FRIEND_DEFINITION(InverseBox);


//---------------------------------------------------------------------------------------------------------
//												DEFINES
//---------------------------------------------------------------------------------------------------------

#define __MAX_NUMBER_OF_PASSES			10
#define __FLOAT_0_5_F					0x00001000


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void Box_configureWireframe(Box this, int renew);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(Box, SpatialObject owner)
__CLASS_NEW_END(Box, owner);


// class's constructor
void Box_constructor(Box this, SpatialObject owner)
{
	ASSERT(this, "Box::constructor: null this");

	__CONSTRUCT_BASE(Shape, owner);

	this->polyhedron = NULL;

	this->rotationVertexDisplacement = (Vector3D){0, 0, 0};

	this->normals = NULL;

	int normalIndex = 0;
	for(; normalIndex < __SHAPE_NORMALS; normalIndex++)
	{
		this->vertexProjections[normalIndex].min = 0;
		this->vertexProjections[normalIndex].max = 0;
	}
}

// class's destructor
void Box_destructor(Box this)
{
	ASSERT(this, "Box::destructor: null this");

	Box_hide(this);

	if(this->normals)
	{
		__DELETE_BASIC(this->normals);
		this->normals = NULL;
	}

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

void Box_position(Box this, const Vector3D* position, const Rotation* rotation, const Scale* scale __attribute__ ((unused)), const Size* size)
{
	ASSERT(this, "Box::position: null this");

	this->rotationVertexDisplacement.x = 0;
	this->rotationVertexDisplacement.y = 0;
	this->rotationVertexDisplacement.z = 0;

	Size surroundingBoxSize = *size;

	// angle | theta | psi
	if(rotation->z | rotation->y | rotation->x)
	{
		fix19_13 width = __I_TO_FIX19_13(surroundingBoxSize.x) >> 1;
		fix19_13 height = __I_TO_FIX19_13(surroundingBoxSize.y) >> 1;
		fix19_13 depth = __I_TO_FIX19_13(surroundingBoxSize.z) >> 1;

		// allow only one rotation
		if(rotation->z)
		{
			// clamp value around 256 degrees (180) to avoid conditionals later when calculating rotationVertexDisplacement
			s16 angle = rotation->z - ((rotation->z / 256) << 8);
			angle = angle < 0 ? 256 + angle : angle;

			// calculate position of box's right-bottom corner
			fix19_13 sinAngle = __FIX7_9_TO_FIX19_13(__SIN(angle));
			fix19_13 cosAngle = __FIX7_9_TO_FIX19_13(__COS(angle));

			// use vectors (x1, y0, z1) and (x1, y1, z1)
			Vector3D topRight =
			{
				__FIX19_13_MULT(width, cosAngle) - __FIX19_13_MULT(-height, sinAngle),
				__FIX19_13_MULT(width, sinAngle) + __FIX19_13_MULT(-height, cosAngle),
				depth
			};

			Vector3D bottomRight =
			{
				__FIX19_13_MULT(width, cosAngle) - __FIX19_13_MULT(height, sinAngle),
				__FIX19_13_MULT(width, sinAngle) + __FIX19_13_MULT(height, cosAngle),
				depth
			};

			Vector3D topRightHelper =
			{
				__ABS(topRight.x),
				__ABS(topRight.y),
				__ABS(topRight.z),
			};

			Vector3D bottomRightHelper =
			{
				__ABS(bottomRight.x),
				__ABS(bottomRight.y),
				__ABS(bottomRight.z),
			};

			surroundingBoxSize.x = __FIX19_13_TO_I(bottomRightHelper.x > topRightHelper.x ? bottomRightHelper.x : topRightHelper.x) << 1;
			surroundingBoxSize.y = __FIX19_13_TO_I(bottomRightHelper.y > topRightHelper.y ? bottomRightHelper.y : topRightHelper.y) << 1;

			// find the displacement over each axis for the rotated box
			this->rotationVertexDisplacement.x = bottomRightHelper.x < topRightHelper.x ? bottomRight.x : topRight.x;
			this->rotationVertexDisplacement.y = bottomRightHelper.y < topRightHelper.y ? bottomRight.y : topRight.y;
			this->rotationVertexDisplacement.y = angle >= 128 ? -this->rotationVertexDisplacement.y : this->rotationVertexDisplacement.y;

			this->rotationVertexDisplacement.x = (__I_TO_FIX19_13(surroundingBoxSize.x) >> 1) + this->rotationVertexDisplacement.x;
			this->rotationVertexDisplacement.y = (__I_TO_FIX19_13(surroundingBoxSize.y) >> 1) - this->rotationVertexDisplacement.y;

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

			// calculate position of box's right-bottom corner
			fix19_13 sinAngle = __FIX7_9_TO_FIX19_13(__SIN(angle));
			fix19_13 cosAngle = __FIX7_9_TO_FIX19_13(__COS(0));

			// use vectors (x0, y1, z0) and (x1, y1, z0)
			Vector3D bottomLeft =
			{
				__FIX19_13_MULT(-width, cosAngle) + __FIX19_13_MULT(-depth, sinAngle),
				height,
				-__FIX19_13_MULT(-width, sinAngle) + __FIX19_13_MULT(-depth, cosAngle),
			};

			Vector3D bottomRight =
			{
				__FIX19_13_MULT(width, cosAngle) + __FIX19_13_MULT(-depth, sinAngle),
				height,
				-__FIX19_13_MULT(width, sinAngle) + __FIX19_13_MULT(-depth, cosAngle),
			};

			Vector3D bottomLeftHelper =
			{
				__ABS(bottomLeft.x),
				__ABS(bottomLeft.y),
				__ABS(bottomLeft.z),
			};

			Vector3D bottomRightHelper =
			{
				__ABS(bottomRight.x),
				__ABS(bottomRight.y),
				__ABS(bottomRight.z),
			};

			surroundingBoxSize.x = __FIX19_13_TO_I(bottomLeftHelper.x > bottomRightHelper.x ? bottomLeftHelper.x : bottomRightHelper.x) << 1;
			surroundingBoxSize.z = __FIX19_13_TO_I(bottomLeftHelper.z > bottomRightHelper.z ? bottomLeftHelper.z : bottomRightHelper.z) << 1;

			// find the displacement over each axis for the rotated box
			this->rotationVertexDisplacement.x = bottomLeftHelper.x < bottomRightHelper.x ? bottomLeft.x : bottomRight.x;
			this->rotationVertexDisplacement.x = angle >= 128 ? -this->rotationVertexDisplacement.x : this->rotationVertexDisplacement.x;
			this->rotationVertexDisplacement.z = bottomLeftHelper.z < bottomRightHelper.z ? bottomLeft.z : bottomRight.z;

			this->rotationVertexDisplacement.x = (__I_TO_FIX19_13(surroundingBoxSize.x) >> 1) - this->rotationVertexDisplacement.x;
			this->rotationVertexDisplacement.z = (__I_TO_FIX19_13(surroundingBoxSize.z) >> 1) + this->rotationVertexDisplacement.z;

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

			// calculate position of box's right-bottom corner
			fix19_13 sinAngle = __FIX7_9_TO_FIX19_13(__SIN(angle));
			fix19_13 cosAngle = __FIX7_9_TO_FIX19_13(__COS(angle));

			// use vectors (x1, y1, z0) and (x1, y1, z1)
			Vector3D bottomNear =
			{
				width,
				__FIX19_13_MULT(height, cosAngle) - __FIX19_13_MULT(-depth, sinAngle),
				__FIX19_13_MULT(height, sinAngle) + __FIX19_13_MULT(-depth, cosAngle),
			};

			Vector3D bottomFar =
			{
				width,
				__FIX19_13_MULT(height, cosAngle) - __FIX19_13_MULT(depth, sinAngle),
				__FIX19_13_MULT(height, sinAngle) + __FIX19_13_MULT(depth, cosAngle),
			};

			Vector3D bottomNearHelper =
			{
				__ABS(bottomNear.x),
				__ABS(bottomNear.y),
				__ABS(bottomNear.z),
			};

			Vector3D bottomFarHelper =
			{
				__ABS(bottomFar.x),
				__ABS(bottomFar.y),
				__ABS(bottomFar.z),
			};

			surroundingBoxSize.y = __FIX19_13_TO_I(bottomFarHelper.y > bottomNearHelper.y ? bottomFarHelper.y : bottomNearHelper.y) << 1;
			surroundingBoxSize.z = __FIX19_13_TO_I(bottomFarHelper.z > bottomNearHelper.z ? bottomFarHelper.z : bottomNearHelper.z) << 1;

			// find the displacement over each axis for the rotated box
			this->rotationVertexDisplacement.y = bottomFarHelper.y < bottomNearHelper.y ? bottomFar.y : bottomNear.y;
			this->rotationVertexDisplacement.z = bottomFarHelper.z < bottomNearHelper.z ? bottomFar.z : bottomNear.z;
			this->rotationVertexDisplacement.z = angle >= 128 ? -this->rotationVertexDisplacement.z : this->rotationVertexDisplacement.z;

			this->rotationVertexDisplacement.y = (__I_TO_FIX19_13(surroundingBoxSize.y) >> 1) - this->rotationVertexDisplacement.y;
			this->rotationVertexDisplacement.z = (__I_TO_FIX19_13(surroundingBoxSize.z) >> 1) + this->rotationVertexDisplacement.z;

			if(!(__MODULO(angle, 128)))
			{
				this->rotationVertexDisplacement.y = 0;
				this->rotationVertexDisplacement.z = 0;
			}
		}
	}

	// box's center if placed on P(0, 0, 0)
	this->rightBox.x1 = __I_TO_FIX19_13(surroundingBoxSize.x >> 1);
	this->rightBox.y1 = __I_TO_FIX19_13(surroundingBoxSize.y >> 1);
	this->rightBox.z1 = __I_TO_FIX19_13(surroundingBoxSize.z >> 1);

	this->rightBox.x0 = -this->rightBox.x1;
	this->rightBox.y0 = -this->rightBox.y1;
	this->rightBox.z0 = -this->rightBox.z1;

	// position the shape to avoid in real time calculation
	this->rightBox.x0 += position->x;
	this->rightBox.x1 += position->x;
	this->rightBox.y0 += position->y;
	this->rightBox.y1 += position->y;
	this->rightBox.z0 += position->z;
	this->rightBox.z1 += position->z;

	if(this->normals)
	{
		Box_projectOntoItself(this);
	}

	__CALL_BASE_METHOD(Shape, position, this, position, rotation, scale, size);
}

void Box_getVertexes(Box this, Vector3D vertexes[__BOX_VERTEXES])
{
	Vector3D leftTopNear 		= {this->rightBox.x0, this->rightBox.y0, this->rightBox.z0};
	Vector3D rightTopNear 		= {this->rightBox.x1, this->rightBox.y0, this->rightBox.z0};
	Vector3D leftBottomNear 	= {this->rightBox.x0, this->rightBox.y1, this->rightBox.z0};
	Vector3D rightBottomNear	= {this->rightBox.x1, this->rightBox.y1, this->rightBox.z0};
	Vector3D leftTopFar 		= {this->rightBox.x0, this->rightBox.y0, this->rightBox.z1};
	Vector3D rightTopFar 		= {this->rightBox.x1, this->rightBox.y0, this->rightBox.z1};
	Vector3D leftBottomFar 		= {this->rightBox.x0, this->rightBox.y1, this->rightBox.z1};
	Vector3D rightBottomFar 	= {this->rightBox.x1, this->rightBox.y1, this->rightBox.z1};

	if(this->rotationVertexDisplacement.x | this->rotationVertexDisplacement.y | this->rotationVertexDisplacement.z)
	{
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

void Box_computeNormals(Box this, Vector3D vertexes[__BOX_VERTEXES])
{
/*
	// generic way
	normals[0] = Vector3D_getPlaneNormal(vertexes[6], vertexes[4], vertexes[0]);
	normals[1] = Vector3D_getPlaneNormal(vertexes[0], vertexes[4], vertexes[5]);
	normals[2] = Vector3D_getPlaneNormal(vertexes[0], vertexes[1], vertexes[3]);
*/

	if(!this->normals)
	{
		this->normals = __NEW_BASIC(Normals);
	}

	// fast way given that the cubes are regular
	this->normals->vectors[0] = (Vector3D)
	{
		vertexes[1].x - vertexes[0].x,
		vertexes[1].y - vertexes[0].y,
		vertexes[1].z - vertexes[0].z,
	};

	this->normals->vectors[1] = (Vector3D)
	{
		vertexes[2].x - vertexes[0].x,
		vertexes[2].y - vertexes[0].y,
		vertexes[2].z - vertexes[0].z,
	};

	this->normals->vectors[2] = (Vector3D)
	{
		vertexes[4].x - vertexes[0].x,
		vertexes[4].y - vertexes[0].y,
		vertexes[4].z - vertexes[0].z,
	};

	this->normals->vectors[0] = Vector3D_normalize(this->normals->vectors[0]);
	this->normals->vectors[1] = Vector3D_normalize(this->normals->vectors[1]);
	this->normals->vectors[2] = Vector3D_normalize(this->normals->vectors[2]);
}

void Box_project(Vector3D vertexes[__BOX_VERTEXES], Vector3D vector, fix19_13* min, fix19_13* max)
{
	int vertexIndex = 0;

	// project this onto the current normal
	fix19_13 dotProduct = Vector3D_dotProduct(vector, vertexes[vertexIndex]);

	fix19_13 finalMin = dotProduct;
	fix19_13 finalMax = dotProduct;

	// project this onto the current normal
	for(; vertexIndex < __BOX_VERTEXES; vertexIndex++)
	{
		dotProduct = Vector3D_dotProduct(vector, vertexes[vertexIndex]);

		if(dotProduct < finalMin)
		{
			finalMin = dotProduct;
		}
		else if(dotProduct > finalMax)
		{
			finalMax = dotProduct;
		}
	}

	*min = finalMin;
	*max = finalMax;
}

void Box_projectOntoItself(Box this)
{
	ASSERT(this, "Box::projectOntoItself: null this");

	Vector3D vertexes[__BOX_VERTEXES];
	Box_getVertexes(this, vertexes);

	// compute normals
	Box_computeNormals(this, vertexes);

	// has to project all points on all the normals of the tilted box
	int normalIndex = 0;

	// initialize vertex projections
	for(; normalIndex < __SHAPE_NORMALS; normalIndex++)
	{
		Box_project(vertexes, this->normals->vectors[normalIndex], &this->vertexProjections[normalIndex].min, &this->vertexProjections[normalIndex].max);
	}
}

// test if collision with the entity give the displacement
CollisionInformation Box_testForCollision(Box this, Shape shape, Vector3D displacement, fix19_13 sizeIncrement)
{
	ASSERT(this, "Box::testForCollision: null this");

	// save position
	RightBox rightBox = this->rightBox;

	// add displacement
	this->rightBox.x0 += displacement.x - sizeIncrement;
	this->rightBox.x1 += displacement.x + sizeIncrement;

	this->rightBox.y0 += displacement.y - sizeIncrement;
	this->rightBox.y1 += displacement.y + sizeIncrement;

	this->rightBox.z0 += displacement.z - sizeIncrement;
	this->rightBox.z1 += displacement.z + sizeIncrement;

	Box_projectOntoItself(this);

	// test for collision on displaced center
	CollisionInformation collisionInformation = CollisionHelper_checkIfOverlap(CollisionHelper_getInstance(), __SAFE_CAST(Shape, this), shape);

	// put back myself
	this->rightBox = rightBox;

	Box_projectOntoItself(this);

	return collisionInformation;
}

Vector3D Box_getPosition(Box this)
{
	ASSERT(this, "Box::getPosition: null this");

	Vector3D position =
	{
		this->rightBox.x0 + ((this->rightBox.x1 - this->rightBox.x0) >> 1),
		this->rightBox.y0 + ((this->rightBox.y1 - this->rightBox.y0) >> 1),
		this->rightBox.z0 + ((this->rightBox.z1 - this->rightBox.z0) >> 1),
	};

	return position;
}

RightBox Box_getSurroundingRightBox(Box this)
{
	ASSERT(this, "Box::getSurroundingRightBox: null this");

	return this->rightBox;
}

// configure Polyhedron
static void Box_configureWireframe(Box this, int renew)
{
	ASSERT(this, "Box::draw: null this");

	if(renew)
	{
		Box_hide(this);
	}
	else if(this->polyhedron)
	{
		return;
	}

	// create a wireframe
	this->polyhedron = __NEW(Polyhedron);

	if(this->rotationVertexDisplacement.x | this->rotationVertexDisplacement.y | this->rotationVertexDisplacement.z)
	{
		if(!this->rotationVertexDisplacement.z)
		{
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y0 + this->rotationVertexDisplacement.y, this->rightBox.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1 - this->rotationVertexDisplacement.x, this->rightBox.y0, this->rightBox.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y1 - this->rotationVertexDisplacement.y, this->rightBox.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0 + this->rotationVertexDisplacement.x, this->rightBox.y1, this->rightBox.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y0 + this->rotationVertexDisplacement.y, this->rightBox.z0);

/*
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y0 + this->rotationVertexDisplacement.y, this->rightBox.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1 - this->rotationVertexDisplacement.x, this->rightBox.y0, this->rightBox.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y1 - this->rotationVertexDisplacement.y, this->rightBox.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0 + this->rotationVertexDisplacement.x, this->rightBox.y1, this->rightBox.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y0 + this->rotationVertexDisplacement.y, this->rightBox.z1);
*/		}

		if(!this->rotationVertexDisplacement.y)
		{
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0 + this->rotationVertexDisplacement.x, this->rightBox.y0, this->rightBox.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y0, this->rightBox.z0 + this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1 - this->rotationVertexDisplacement.x, this->rightBox.y0, this->rightBox.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y0, this->rightBox.z1 - this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0 + this->rotationVertexDisplacement.x, this->rightBox.y0, this->rightBox.z0);

			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0 + this->rotationVertexDisplacement.x, this->rightBox.y1, this->rightBox.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y1, this->rightBox.z0 + this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1 - this->rotationVertexDisplacement.x, this->rightBox.y1, this->rightBox.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y1, this->rightBox.z1 - this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0 + this->rotationVertexDisplacement.x, this->rightBox.y1, this->rightBox.z0);
		}

		if(!this->rotationVertexDisplacement.x)
		{
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y0, this->rightBox.z0 + this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y0 + this->rotationVertexDisplacement.y, this->rightBox.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y1, this->rightBox.z1 - this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y1 - this->rotationVertexDisplacement.y, this->rightBox.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y0, this->rightBox.z0 + this->rotationVertexDisplacement.z);

			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y0, this->rightBox.z0 + this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y0 + this->rotationVertexDisplacement.y, this->rightBox.z1);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y1, this->rightBox.z1 - this->rotationVertexDisplacement.z);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y1 - this->rotationVertexDisplacement.y, this->rightBox.z0);
			Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y0, this->rightBox.z0 + this->rotationVertexDisplacement.z);
		}
	}
	else
	{

		Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y0, this->rightBox.z0);
		Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y0, this->rightBox.z0);
		Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y1, this->rightBox.z0);
		Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y1, this->rightBox.z0);
		Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y0, this->rightBox.z0);
	/*	Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y0, this->rightBox.z1);
		Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y0, this->rightBox.z1);
		Polyhedron_addVertex(this->polyhedron, this->rightBox.x1, this->rightBox.y1, this->rightBox.z1);
		Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y1, this->rightBox.z1);
		Polyhedron_addVertex(this->polyhedron, this->rightBox.x0, this->rightBox.y0, this->rightBox.z1);
	*/
	}
}

// show me
void Box_show(Box this)
{
	ASSERT(this, "Box::draw: null this");

	Box_configureWireframe(this, true);

	// draw the Polyhedron
	Wireframe_show(__SAFE_CAST(Wireframe, this->polyhedron));
}

// hide polyhedron
void Box_hide(Box this)
{
	if(this->polyhedron)
	{
		// draw the Polyhedron
		__DELETE(this->polyhedron);

		this->polyhedron = NULL;
	}
}

// print debug data
void Box_print(Box this, int x, int y)
{
	ASSERT(this, "Box::print: null this");

	RightBox rightBox = this->rightBox;

	Printing_text(Printing_getInstance(), "X:" , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(rightBox.x0), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(rightBox.x1), x + 7, y++, NULL);

	Printing_text(Printing_getInstance(), "Y:" , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(rightBox.y0), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(rightBox.y1), x + 7, y++, NULL);

	Printing_text(Printing_getInstance(), "Z:" , x, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(rightBox.z0), x + 2, y, NULL);
	Printing_text(Printing_getInstance(), "-" , x + 5, y, NULL);
	Printing_int(Printing_getInstance(), __FIX19_13_TO_I(rightBox.z1), x + 7, y++, NULL);
}
