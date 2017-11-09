/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
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

#include <CollisionHelper.h>
#include <Box.h>
#include <InverseBox.h>
#include <Ball.h>
#include <Vector.h>
#include <VirtualList.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

#define CollisionHelper_ATTRIBUTES																		\
		/* super's attributes */																		\
		Object_ATTRIBUTES																				\

/**
 * @class	CollisionHelper
 * @extends Object
 * @ingroup graphics-3d
 */
__CLASS_DEFINITION(CollisionHelper, Object);
__CLASS_FRIEND_DEFINITION(Box);
__CLASS_FRIEND_DEFINITION(InverseBox);
__CLASS_FRIEND_DEFINITION(Ball);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

static void CollisionHelper_constructor(CollisionHelper this);
static CollisionInformation CollisionHelper_checkIfBoxOverlapsBox(CollisionHelper this __attribute__ ((unused)), Box boxA, Box boxB);
static CollisionInformation CollisionHelper_checkIfBoxOverlapsInverseBox(CollisionHelper this __attribute__ ((unused)), Box boxA, InverseBox inverseBoxB);
static CollisionInformation CollisionHelper_checkIfBoxOverlapsBall(CollisionHelper this __attribute__ ((unused)), Box boxA, Ball ballB);
static CollisionInformation CollisionHelper_checkIfInverseBoxOverlapsInverseBox(CollisionHelper this __attribute__ ((unused)), InverseBox inverseBoxA, InverseBox inverseBoxB);
static CollisionInformation CollisionHelper_checkIfInverseBoxOverlapsBall(CollisionHelper this __attribute__ ((unused)), InverseBox inverseBoxA, Ball ballB);
static CollisionInformation CollisionHelper_checkIfBallOverlapsBall(CollisionHelper this __attribute__ ((unused)), Ball ballA, Ball ballB);
static CollisionSolution CollisionHelper_getCollisionSolutionBetweenBoxAndBox(CollisionHelper this __attribute__ ((unused)), Box boxA, Box boxB);
static CollisionSolution CollisionHelper_getCollisionSolutionBetweenBoxAndInverseBox(CollisionHelper this __attribute__ ((unused)), Box boxA, InverseBox inverseBoxB);
static CollisionSolution CollisionHelper_getCollisionSolutionBetweenBoxAndBall(CollisionHelper this __attribute__ ((unused)), Box boxA, Ball ballB);
static CollisionSolution CollisionHelper_getCollisionSolutionBetweenInverseBoxAndInverseBox(CollisionHelper this __attribute__ ((unused)), InverseBox inverseBoxA, InverseBox inverseBoxB);
static CollisionSolution CollisionHelper_getCollisionSolutionBetweenInverseBoxAndBall(CollisionHelper this __attribute__ ((unused)), InverseBox inverseBoxA, Ball ballB);
static CollisionSolution CollisionHelper_getCollisionSolutionBetweenBallAndBall(CollisionHelper this __attribute__ ((unused)), Ball ballA, Ball ballB);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			CollisionHelper_getInstance()
 * @memberof	CollisionHelper
 * @public
 *
 * @return		CollisionHelper instance
 */
__SINGLETON(CollisionHelper);

/**
 * Class constructor
 *
 * @memberof	CollisionHelper
 * @private
 *
 * @param this	Function scope
 */
static void CollisionHelper_constructor(CollisionHelper this)
{
	ASSERT(this, "CollisionHelper::constructor: null this");

	__CONSTRUCT_BASE(Object);
}

/**
 * Class destructor
 *
 * @memberof	CollisionHelper
 * @public
 *
 * @param this	Function scope
 */
void CollisionHelper_destructor(CollisionHelper this)
{
	ASSERT(this, "CollisionHelper::destructor: null this");

	// allow a new construct
	__SINGLETON_DESTROY;
}

/**
 * Check if two shapes overlap
 *
 * @memberof			CollisionHelper
 * @public
 *
 * @param this			Function scope
 * @param shapeA		Shape
 * @param shapeB		Shape
 */
CollisionInformation CollisionHelper_checkIfOverlap(CollisionHelper this __attribute__ ((unused)), Shape shapeA, Shape shapeB)
{
	ASSERT(this, "CollisionHelper::checkIfOverlap: null this");
	ASSERT(shapeA, "CollisionHelper::checkIfOverlap: null shapeA");
	ASSERT(shapeB, "CollisionHelper::checkIfOverlap: null shapeA");

	CollisionInformation collisionInformation = (CollisionInformation){NULL, NULL, false, {{0, 0, 0}, {0, 0, 0}, 0}};

	if(__IS_INSTANCE_OF(Box, shapeA))
	{
		if(__IS_INSTANCE_OF(Box, shapeB))
    	{
			collisionInformation = CollisionHelper_checkIfBoxOverlapsBox(this, __SAFE_CAST(Box, shapeA), __SAFE_CAST(Box, shapeB));
		}
		else if(__IS_INSTANCE_OF(InverseBox, shapeB))
		{
			collisionInformation = CollisionHelper_checkIfBoxOverlapsInverseBox(this, __SAFE_CAST(Box, shapeA), __SAFE_CAST(InverseBox, shapeB));
		}
		else if(__IS_INSTANCE_OF(Ball, shapeB))
		{
			collisionInformation = CollisionHelper_checkIfBoxOverlapsBall(this, __SAFE_CAST(Box, shapeA), __SAFE_CAST(Ball, shapeB));
		}
	}
	else if(__IS_INSTANCE_OF(InverseBox, shapeA))
	{
		if(__IS_INSTANCE_OF(Box, shapeB))
    	{
			collisionInformation = CollisionHelper_checkIfBoxOverlapsInverseBox(this, __SAFE_CAST(Box, shapeB), __SAFE_CAST(InverseBox, shapeA));
		}
		else if(__IS_INSTANCE_OF(InverseBox, shapeB))
		{
			collisionInformation = CollisionHelper_checkIfInverseBoxOverlapsInverseBox(this, __SAFE_CAST(InverseBox, shapeA), __SAFE_CAST(InverseBox, shapeB));
		}
		else if(__IS_INSTANCE_OF(Ball, shapeB))
		{
			collisionInformation = CollisionHelper_checkIfInverseBoxOverlapsBall(this, __SAFE_CAST(InverseBox, shapeA), __SAFE_CAST(Ball, shapeB));
		}
	}
	else if(__IS_INSTANCE_OF(Ball, shapeA))
	{
		if(__IS_INSTANCE_OF(Box, shapeB))
    	{
			collisionInformation = CollisionHelper_checkIfBoxOverlapsBall(this, __SAFE_CAST(Box, shapeB), __SAFE_CAST(Ball, shapeA));
		}
		else if(__IS_INSTANCE_OF(InverseBox, shapeB))
		{
			collisionInformation = CollisionHelper_checkIfInverseBoxOverlapsBall(this, __SAFE_CAST(InverseBox, shapeB), __SAFE_CAST(Ball, shapeA));
		}
		else if(__IS_INSTANCE_OF(Ball, shapeB))
		{
			collisionInformation = CollisionHelper_checkIfBallOverlapsBall(this, __SAFE_CAST(Ball, shapeA), __SAFE_CAST(Ball, shapeB));
		}
	}

	// check if must swap shapes in the collision information struct since
	// we swaped the calls to the checking methods to avoid code repetition
	if(collisionInformation.shape)
	{
		collisionInformation.shape = shapeA;
		collisionInformation.collidingShape = shapeB;
	}

	return collisionInformation;
}

static CollisionInformation CollisionHelper_checkIfBoxOverlapsBox(CollisionHelper this __attribute__ ((unused)), Box boxA, Box boxB)
{
	ASSERT(this, "CollisionHelper::checkIfBoxOverlapsBox: null this");

	VBVec3D intervalDistance =
	{
		(boxA->rightBox.x0 + boxA->rightBox.x1) >> 1 < (boxB->rightBox.x0 + boxB->rightBox.x1) >> 1 ? (boxB->rightBox.x0 - boxA->rightBox.x1) : (boxA->rightBox.x0 - boxB->rightBox.x1),
		(boxA->rightBox.y0 + boxA->rightBox.y1) >> 1 < (boxB->rightBox.y0 + boxB->rightBox.y1) >> 1 ? (boxB->rightBox.y0 - boxA->rightBox.y1) : (boxA->rightBox.y0 - boxB->rightBox.y1),
		(boxA->rightBox.z0 + boxA->rightBox.z1) >> 1 < (boxB->rightBox.z0 + boxB->rightBox.z1) >> 1 ? (boxB->rightBox.z0 - boxA->rightBox.z1) : (boxA->rightBox.z0 - boxB->rightBox.z1),
	};

	// test for collision
	if(0 > intervalDistance.x && 0 > intervalDistance.y && 0 > intervalDistance.z)
	{
		// check if both boxes are axis aligned
		bool isBoxARotated = boxA->rotationVertexDisplacement.x | boxA->rotationVertexDisplacement.y | boxA->rotationVertexDisplacement.z ? true : false;
		bool isBoxBRotated = boxB->rotationVertexDisplacement.x | boxB->rotationVertexDisplacement.y | boxB->rotationVertexDisplacement.z ? true : false;
		bool isSATCheckPending = isBoxARotated || isBoxBRotated;

		CollisionSolution collisionSolution = (CollisionSolution) {{0, 0, 0}, {0, 0, 0}, 0};
		fix19_13 minimumIntervalDistance = Math_fix19_13Infinity();

		// if axis aligned, then SAT check is not needed
		// and we can calculate the minimum displacement vector
		// to resolve the collision right now
		if(!isSATCheckPending)
		{
			VBVec3D boxACenter =
			{
				(boxA->rightBox.x0 + boxA->rightBox.x1) >> 1,
				(boxA->rightBox.y0 + boxA->rightBox.y1) >> 1,
				(boxA->rightBox.z0 + boxA->rightBox.z1) >> 1,
			};

			VBVec3D boxBCenter =
			{
				(boxB->rightBox.x0 + boxB->rightBox.x1) >> 1,
				(boxB->rightBox.y0 + boxB->rightBox.y1) >> 1,
				(boxB->rightBox.z0 + boxB->rightBox.z1) >> 1,
			};

			VBVec3D distanceVector = Vector_get(boxBCenter, boxACenter);

			VBVec3D normals[__SHAPE_NORMALS] =
			{
				{__I_TO_FIX19_13(1), 0, 0},
				{0, __I_TO_FIX19_13(1), 0},
				{0, 0, __I_TO_FIX19_13(1)},
			};

			int i = 0;
			fix19_13* component = &intervalDistance.x;

			for(i = 0; i < __SHAPE_NORMALS; i++)
			{
				fix19_13 intervalDistance = __ABS(component[i]);

				if(intervalDistance < minimumIntervalDistance)
				{
					collisionSolution.translationVectorLength = minimumIntervalDistance = intervalDistance;
					collisionSolution.collisionPlaneNormal = normals[i];

					if(Vector_dotProduct(distanceVector, collisionSolution.collisionPlaneNormal) < 0)
					{
						collisionSolution.collisionPlaneNormal = Vector_scalarProduct(collisionSolution.collisionPlaneNormal, __I_TO_FIX19_13(-1));
					}
				}
			}

			collisionSolution.translationVector = Vector_scalarProduct(collisionSolution.collisionPlaneNormal, collisionSolution.translationVectorLength);
		}

		return (CollisionInformation){__SAFE_CAST(Shape, boxA), __SAFE_CAST(Shape, boxB), !isSATCheckPending, collisionSolution};
	}

	return (CollisionInformation){NULL, NULL, false, {{0, 0, 0}, {0, 0, 0}, 0}};
}

static CollisionInformation CollisionHelper_checkIfBoxOverlapsInverseBox(CollisionHelper this __attribute__ ((unused)), Box boxA, InverseBox inverseBoxB)
{
	ASSERT(this, "CollisionHelper::checkIfBoxOverlapsInverseBox: null this");

	// test for collision
	if((boxA->rightBox.x0 < inverseBoxB->rightBox.x0) | (boxA->rightBox.x1 > inverseBoxB->rightBox.x1) |
	 (boxA->rightBox.y0 < inverseBoxB->rightBox.y0) | (boxA->rightBox.y1 > inverseBoxB->rightBox.y1) |
	 (boxA->rightBox.z0 < inverseBoxB->rightBox.z0) | (boxA->rightBox.z1 > inverseBoxB->rightBox.z1)
	)
	{
		bool isCollisionSolutionValid = true;

		return (CollisionInformation){__SAFE_CAST(Shape, boxA), __SAFE_CAST(Shape, inverseBoxB), isCollisionSolutionValid, {{0, 0, 0}, {0, 0, 0}, 0}};
	}

	return (CollisionInformation){NULL, NULL, false, {{0, 0, 0}, {0, 0, 0}, 0}};
}

static CollisionInformation CollisionHelper_checkIfBoxOverlapsBall(CollisionHelper this __attribute__ ((unused)), Box boxA, Ball ballB)
{
	ASSERT(this, "CollisionHelper::checkIfBoxOverlapsBall: null this");

	VBVec3D boxACenter =
	{
		(boxA->rightBox.x0 + boxA->rightBox.x1) >> 1,
		(boxA->rightBox.y0 + boxA->rightBox.y1) >> 1,
		(boxA->rightBox.z0 + boxA->rightBox.z1) >> 1,
	};

	VBVec3D intervalDistance =
	{
		boxACenter.x < ballB->center.x ? ((ballB->center.x - ballB->radius) - boxA->rightBox.x1) : (boxA->rightBox.x0 - (ballB->center.x + ballB->radius)),
		boxACenter.y < ballB->center.y ? ((ballB->center.y - ballB->radius) - boxA->rightBox.y1) : (boxA->rightBox.y0 - (ballB->center.y + ballB->radius)),
		boxACenter.z < ballB->center.z ? ((ballB->center.z - ballB->radius) - boxA->rightBox.z1) : (boxA->rightBox.z0 - (ballB->center.z + ballB->radius)),
	};

	// test for collision
	if(0 > intervalDistance.x && 0 > intervalDistance.y && 0 > intervalDistance.z)
	{
		// check if both boxes are axis aligned
		bool isSATCheckPending = boxA->rotationVertexDisplacement.x | boxA->rotationVertexDisplacement.y | boxA->rotationVertexDisplacement.z ? true : false;

		CollisionSolution collisionSolution = (CollisionSolution) {{0, 0, 0}, {0, 0, 0}, 0};
		fix19_13 minimumIntervalDistance = Math_fix19_13Infinity();

		// if axis aligned, then SAT check is not needed
		// and we can calculate the minimum displacement vector
		// to resolve the collision right now
		if(!isSATCheckPending)
		{
			VBVec3D distanceVector = Vector_get(boxACenter, ballB->center);

			VBVec3D normals[__SHAPE_NORMALS] =
			{
				{__I_TO_FIX19_13(1), 0, 0},
				{0, __I_TO_FIX19_13(1), 0},
				{0, 0, __I_TO_FIX19_13(1)},
			};

			int i = 0;
			fix19_13* component = &intervalDistance.x;

			for(i = 0; i < __SHAPE_NORMALS; i++)
			{
				fix19_13 intervalDistance = __ABS(component[i]);

				if(intervalDistance < minimumIntervalDistance)
				{
					collisionSolution.translationVectorLength = minimumIntervalDistance = intervalDistance;
					collisionSolution.collisionPlaneNormal = normals[i];

					if(Vector_dotProduct(distanceVector, collisionSolution.collisionPlaneNormal) < 0)
					{
						collisionSolution.collisionPlaneNormal = Vector_scalarProduct(collisionSolution.collisionPlaneNormal, __I_TO_FIX19_13(-1));
					}
				}
			}

			collisionSolution.translationVector = Vector_scalarProduct(collisionSolution.collisionPlaneNormal, collisionSolution.translationVectorLength);
		}

		return (CollisionInformation){__SAFE_CAST(Shape, boxA), __SAFE_CAST(Shape, ballB), !isSATCheckPending, collisionSolution};
	}

	return (CollisionInformation){NULL, NULL, false, {{0, 0, 0}, {0, 0, 0}, 0}};
}

static CollisionInformation CollisionHelper_checkIfInverseBoxOverlapsInverseBox(CollisionHelper this __attribute__ ((unused)), InverseBox inverseBoxA __attribute__ ((unused)), InverseBox inverseBoxB __attribute__ ((unused)))
{
	ASSERT(this, "CollisionHelper::checkIfInverseBoxOverlapsInverseBox: null this");

	return (CollisionInformation){NULL, NULL, false, {{0, 0, 0}, {0, 0, 0}, 0}};
}

static CollisionInformation CollisionHelper_checkIfInverseBoxOverlapsBall(CollisionHelper this __attribute__ ((unused)), InverseBox inverseBoxA __attribute__ ((unused)), Ball ballB __attribute__ ((unused)))
{
	ASSERT(this, "CollisionHelper::checkIfInverseBoxOverlapsBall: null this");

	return (CollisionInformation){NULL, NULL, false, {{0, 0, 0}, {0, 0, 0}, 0}};
}

static CollisionInformation CollisionHelper_checkIfBallOverlapsBall(CollisionHelper this __attribute__ ((unused)), Ball ballA __attribute__ ((unused)), Ball ballB __attribute__ ((unused)))
{
	ASSERT(this, "CollisionHelper::checkIfBallOverlapsBall: null this");

	return (CollisionInformation){NULL, NULL, false, {{0, 0, 0}, {0, 0, 0}, 0}};
}

/**
 * Check if two shapes overlap
 *
 * @memberof			CollisionHelper
 * @public
 *
 * @param this			Function scope
 * @param shapeA		Shape
 * @param shapeB		Shape
 */
CollisionSolution CollisionHelper_getCollisionSolution(CollisionHelper this __attribute__ ((unused)), Shape shapeA, Shape shapeB)
{
	ASSERT(this, "CollisionHelper::getCollisionSolution: null this");
	ASSERT(shapeA, "CollisionHelper::getCollisionSolution: null shapeA");
	ASSERT(shapeB, "CollisionHelper::getCollisionSolution: null shapeA");

	if(__IS_INSTANCE_OF(Box, shapeA))
	{
		if(__IS_INSTANCE_OF(Box, shapeB))
    	{
			return CollisionHelper_getCollisionSolutionBetweenBoxAndBox(this, __SAFE_CAST(Box, shapeA), __SAFE_CAST(Box, shapeB));
		}
		else if(__IS_INSTANCE_OF(InverseBox, shapeB))
		{
			return CollisionHelper_getCollisionSolutionBetweenBoxAndInverseBox(this, __SAFE_CAST(Box, shapeA), __SAFE_CAST(InverseBox, shapeB));
		}
		else if(__IS_INSTANCE_OF(Ball, shapeB))
		{
			return CollisionHelper_getCollisionSolutionBetweenBoxAndBall(this, __SAFE_CAST(Box, shapeA), __SAFE_CAST(Ball, shapeB));
		}
	}
	else if(__IS_INSTANCE_OF(InverseBox, shapeA))
	{
		if(__IS_INSTANCE_OF(Box, shapeB))
    	{
			return CollisionHelper_getCollisionSolutionBetweenBoxAndInverseBox(this, __SAFE_CAST(Box, shapeB), __SAFE_CAST(InverseBox, shapeA));
		}
		else if(__IS_INSTANCE_OF(InverseBox, shapeB))
		{
			return CollisionHelper_getCollisionSolutionBetweenInverseBoxAndInverseBox(this, __SAFE_CAST(InverseBox, shapeA), __SAFE_CAST(InverseBox, shapeB));
		}
		else if(__IS_INSTANCE_OF(Ball, shapeB))
		{
			return CollisionHelper_getCollisionSolutionBetweenInverseBoxAndBall(this, __SAFE_CAST(InverseBox, shapeA), __SAFE_CAST(Ball, shapeB));
		}
	}
	else if(__IS_INSTANCE_OF(Ball, shapeA))
	{
		if(__IS_INSTANCE_OF(Box, shapeB))
    	{
			return CollisionHelper_getCollisionSolutionBetweenBoxAndBall(this, __SAFE_CAST(Box, shapeB), __SAFE_CAST(Ball, shapeA));
		}
		else if(__IS_INSTANCE_OF(InverseBox, shapeB))
		{
			return CollisionHelper_getCollisionSolutionBetweenInverseBoxAndBall(this, __SAFE_CAST(InverseBox, shapeB), __SAFE_CAST(Ball, shapeA));
		}
		else if(__IS_INSTANCE_OF(Ball, shapeB))
		{
			return CollisionHelper_getCollisionSolutionBetweenBallAndBall(this, __SAFE_CAST(Ball, shapeA), __SAFE_CAST(Ball, shapeB));
		}
	}

	return (CollisionSolution) {{0, 0, 0}, {0, 0, 0}, 0};
}

static CollisionSolution CollisionHelper_getCollisionSolutionBetweenBoxAndBox(CollisionHelper this __attribute__ ((unused)), Box boxA, Box boxB)
{
	ASSERT(this, "CollisionHelper::getCollisionSolutionBetweenBoxAndBox: null this");

	// get the vertexes of each box
	VBVec3D vertexes[2][__BOX_VERTEXES];
	Box_getVertexes(boxA, vertexes[0]);
	Box_getVertexes(boxB, vertexes[1]);

	// if the normals have not been computed yet do so now
	if(!boxA->normals)
	{
		boxA->normals = __NEW_BASIC(Normals);
		Box_computeNormals(boxA, vertexes[0]);
	}

	if(!boxB->normals)
	{
		boxB->normals = __NEW_BASIC(Normals);
		Box_computeNormals(boxB, vertexes[1]);
	}

	VBVec3D* normals[2] =
	{
		boxA->normals->vectors,
		boxB->normals->vectors,
	};

	// will need
	VBVec3D centers[2] =
	{
		{
			(boxA->rightBox.x0 + boxA->rightBox.x1) >> 1,
			(boxA->rightBox.y0 + boxA->rightBox.y1) >> 1,
			(boxA->rightBox.z0 + boxA->rightBox.z1) >> 1,
		},
		{
			(boxB->rightBox.x0 + boxB->rightBox.x1) >> 1,
			(boxB->rightBox.y0 + boxB->rightBox.y1) >> 1,
			(boxB->rightBox.z0 + boxB->rightBox.z1) >> 1,
		}
	};

	VBVec3D distanceVector = Vector_get(centers[1], centers[0]);

	CollisionSolution collisionSolution = (CollisionSolution) {{0, 0, 0}, {0, 0, 0}, 0};
 	fix19_13 minimumIntervalDistance = Math_fix19_13Infinity();

	int boxIndex = 0;

	// has to project all points on all the normals of both boxes
	for(; boxIndex < 2; boxIndex++)
	{
		int normalIndex = 0;

		// test all 3 normals of each box
		for(; normalIndex < __SHAPE_NORMALS; normalIndex++)
		{
			VBVec3D currentNormal = normals[boxIndex][normalIndex];

			fix19_13 min[2] = {0, 0};
			fix19_13 max[2] = {0, 0};

			Box_project(vertexes[0], currentNormal, &min[0], &max[0]);
			Box_project(vertexes[1], currentNormal, &min[1], &max[1]);

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
				collisionSolution = (CollisionSolution) {{0, 0, 0}, {0, 0, 0}, 0};
				return collisionSolution;
			}

			intervalDistance = __ABS(intervalDistance);

			if(intervalDistance < minimumIntervalDistance)
			{
				collisionSolution.translationVectorLength = minimumIntervalDistance = intervalDistance;
				collisionSolution.collisionPlaneNormal = currentNormal;

				if(Vector_dotProduct(distanceVector, collisionSolution.collisionPlaneNormal) < 0)
				{
					collisionSolution.collisionPlaneNormal = Vector_scalarProduct(collisionSolution.collisionPlaneNormal, __I_TO_FIX19_13(-1));
				}
			}
		}
	}

	collisionSolution.translationVector = Vector_scalarProduct(collisionSolution.collisionPlaneNormal, collisionSolution.translationVectorLength);

	return collisionSolution;
}

static CollisionSolution CollisionHelper_getCollisionSolutionBetweenBoxAndInverseBox(CollisionHelper this __attribute__ ((unused)), Box boxA __attribute__ ((unused)), InverseBox inverseBoxB __attribute__ ((unused)))
{
	ASSERT(this, "CollisionHelper::getCollisionSolutionBetweenBoxAndInverseBox: null this");

	return (CollisionSolution) {{0, 0, 0}, {0, 0, 0}, 0};
}

static CollisionSolution CollisionHelper_getCollisionSolutionBetweenBoxAndBall(CollisionHelper this __attribute__ ((unused)), Box boxA __attribute__ ((unused)), Ball ballB __attribute__ ((unused)))
{
	ASSERT(this, "CollisionHelper::getCollisionSolutionBetweenBoxAndBall: null this");

	// get the vertexes of each box
	VBVec3D vertexes[__BOX_VERTEXES];
	Box_getVertexes(boxA, vertexes);

	// if the normals have not been computed yet do so now
	if(!boxA->normals)
	{
		boxA->normals = __NEW_BASIC(Normals);
		Box_computeNormals(boxA, vertexes);
	}

	VBVec3D* normals = boxA->normals->vectors;

	// will need
	VBVec3D boxACenter =
	{
		(boxA->rightBox.x0 + boxA->rightBox.x1) >> 1,
		(boxA->rightBox.y0 + boxA->rightBox.y1) >> 1,
		(boxA->rightBox.z0 + boxA->rightBox.z1) >> 1,
	};

	VBVec3D distanceVector = Vector_get(boxACenter, ballB->center);

	CollisionSolution collisionSolution = (CollisionSolution) {{0, 0, 0}, {0, 0, 0}, 0};
	fix19_13 minimumIntervalDistance = Math_fix19_13Infinity();

	// has to project all points on all the normals of the tilted box
	int normalIndex = 0;

	// test all 3 normals of each box
	for(; normalIndex < __SHAPE_NORMALS; normalIndex++)
	{
		VBVec3D currentNormal = normals[normalIndex];

		fix19_13 min[2] = {0, 0};
		fix19_13 max[2] = {0, 0};

		Box_project(vertexes, currentNormal, &min[0], &max[0]);
		Ball_project(ballB->center, ballB->radius, currentNormal, &min[1], &max[1]);

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
			collisionSolution = (CollisionSolution) {{0, 0, 0}, {0, 0, 0}, 0};
			return collisionSolution;
		}

		intervalDistance = __ABS(intervalDistance);

		if(intervalDistance < minimumIntervalDistance)
		{
			collisionSolution.translationVectorLength = minimumIntervalDistance = intervalDistance;
			collisionSolution.collisionPlaneNormal = currentNormal;

			if(Vector_dotProduct(distanceVector, collisionSolution.collisionPlaneNormal) < 0)
			{
				collisionSolution.collisionPlaneNormal = Vector_scalarProduct(collisionSolution.collisionPlaneNormal, __I_TO_FIX19_13(-1));
			}
		}
	}

	collisionSolution.translationVector = Vector_scalarProduct(collisionSolution.collisionPlaneNormal, collisionSolution.translationVectorLength);

	return collisionSolution;
}

static CollisionSolution CollisionHelper_getCollisionSolutionBetweenInverseBoxAndInverseBox(CollisionHelper this __attribute__ ((unused)), InverseBox inverseBoxA __attribute__ ((unused)), InverseBox inverseBoxB __attribute__ ((unused)))
{
	ASSERT(this, "CollisionHelper::getCollisionSolutionBetweenInverseBoxAndInverseBox: null this");

	return (CollisionSolution) {{0, 0, 0}, {0, 0, 0}, 0};
}

static CollisionSolution CollisionHelper_getCollisionSolutionBetweenInverseBoxAndBall(CollisionHelper this __attribute__ ((unused)), InverseBox inverseBoxA __attribute__ ((unused)), Ball ballB __attribute__ ((unused)))
{
	ASSERT(this, "CollisionHelper::getCollisionSolutionBetweenInverseBoxAndBall: null this");

	return (CollisionSolution) {{0, 0, 0}, {0, 0, 0}, 0};
}

static CollisionSolution CollisionHelper_getCollisionSolutionBetweenBallAndBall(CollisionHelper this __attribute__ ((unused)), Ball ballA __attribute__ ((unused)), Ball ballB __attribute__ ((unused)))
{
	ASSERT(this, "CollisionHelper::getCollisionSolutionBetweenBallAndBall: null this");

	return (CollisionSolution) {{0, 0, 0}, {0, 0, 0}, 0};
}