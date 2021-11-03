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

#include <CollisionHelper.h>
#include <SpatialObject.h>
#include <Box.h>
#include <InverseBox.h>
#include <Ball.h>
#include <LineField.h>
#include <VirtualList.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

friend class Box;
friend class InverseBox;
friend class Ball;
friend class LineField;


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

/**
 * Get instance
 *
 * @fn			CollisionHelper::getInstance()
 * @memberof	CollisionHelper
 * @public
 * @return		CollisionHelper instance
 */


/**
 * Class constructor
 *
 * @private
 */
void CollisionHelper::constructor()
{
	Base::constructor();
}

/**
 * Class destructor
 */
void CollisionHelper::destructor()
{
	// allow a new construct
	Base::destructor();
}

/**
 * Check if two shapes overlap
 *
 * @param shapeA	Shape
 * @param shapeB	Shape
 */
CollisionInformation CollisionHelper::checkIfOverlap(Shape shapeA, Shape shapeB)
{
	ASSERT(shapeA, "CollisionHelper::checkIfOverlap: null shapeA");
	ASSERT(shapeB, "CollisionHelper::checkIfOverlap: null shapeA");

	CollisionInformation collisionInformation = (CollisionInformation){NULL, NULL, {{0, 0, 0}, 0}};

	if(__IS_INSTANCE_OF(Ball, shapeA))
	{
		if(__IS_INSTANCE_OF(Box, shapeB))
    	{
			collisionInformation = CollisionHelper::checkIfBoxOverlapsBall(this, Box::safeCast(shapeB), Ball::safeCast(shapeA));
		}
		else if(__IS_INSTANCE_OF(InverseBox, shapeB))
		{
			collisionInformation = CollisionHelper::checkIfInverseBoxOverlapsBall(this, InverseBox::safeCast(shapeB), Ball::safeCast(shapeA));
		}
		else if(__IS_INSTANCE_OF(Ball, shapeB))
		{
			collisionInformation = CollisionHelper::checkIfBallOverlapsBall(this, Ball::safeCast(shapeA), Ball::safeCast(shapeB));
		}
		else if(__IS_INSTANCE_OF(LineField, shapeB))
		{
			collisionInformation = CollisionHelper::checkIfBallOverlapsLineField(this, Ball::safeCast(shapeA), LineField::safeCast(shapeB));
		}
	}
	else if(__IS_INSTANCE_OF(Box, shapeA))
	{
		if(__IS_INSTANCE_OF(Box, shapeB))
    	{
			collisionInformation = CollisionHelper::checkIfBoxOverlapsBox(this, Box::safeCast(shapeA), Box::safeCast(shapeB));
		}
		else if(__IS_INSTANCE_OF(InverseBox, shapeB))
		{
			collisionInformation = CollisionHelper::checkIfBoxOverlapsInverseBox(this, Box::safeCast(shapeA), InverseBox::safeCast(shapeB));
		}
		else if(__IS_INSTANCE_OF(Ball, shapeB))
		{
			collisionInformation = CollisionHelper::checkIfBoxOverlapsBall(this, Box::safeCast(shapeA), Ball::safeCast(shapeB));
		}
	}
	else if(__IS_INSTANCE_OF(InverseBox, shapeA))
	{
		if(__IS_INSTANCE_OF(Box, shapeB))
    	{
			collisionInformation = CollisionHelper::checkIfBoxOverlapsInverseBox(this, Box::safeCast(shapeB), InverseBox::safeCast(shapeA));
		}
		else if(__IS_INSTANCE_OF(InverseBox, shapeB))
		{
			collisionInformation = CollisionHelper::checkIfInverseBoxOverlapsInverseBox(this, InverseBox::safeCast(shapeA), InverseBox::safeCast(shapeB));
		}
		else if(__IS_INSTANCE_OF(Ball, shapeB))
		{
			collisionInformation = CollisionHelper::checkIfInverseBoxOverlapsBall(this, InverseBox::safeCast(shapeA), Ball::safeCast(shapeB));
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

CollisionInformation CollisionHelper::checkIfBoxOverlapsBox(Box boxA, Box boxB)
{
	Vector3D intervalDistance =
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

		SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};
		fix10_6 minimumIntervalDistance = Math::fix10_6Infinity();

		// if axis aligned, then SAT check is not needed
		// and we can calculate the minimum displacement vector
		// to resolve the collision right now
		if(isSATCheckPending)
		{
			solutionVector = CollisionHelper::getSolutionVectorBetweenBoxAndBox(this, boxA, boxB);
		}
		else
		{
			Vector3D boxACenter =
			{
				(boxA->rightBox.x0 + boxA->rightBox.x1) >> 1,
				(boxA->rightBox.y0 + boxA->rightBox.y1) >> 1,
				(boxA->rightBox.z0 + boxA->rightBox.z1) >> 1,
			};

			Vector3D boxBCenter =
			{
				(boxB->rightBox.x0 + boxB->rightBox.x1) >> 1,
				(boxB->rightBox.y0 + boxB->rightBox.y1) >> 1,
				(boxB->rightBox.z0 + boxB->rightBox.z1) >> 1,
			};

			Vector3D distanceVector = Vector3D::get(boxBCenter, boxACenter);

			Vector3D normals[__SHAPE_NORMALS] =
			{
				{__I_TO_FIX10_6(1), 0, 0},
				{0, __I_TO_FIX10_6(1), 0},
				{0, 0, __I_TO_FIX10_6(1)},
			};

			int32 i = 0;
			fix10_6* component = &intervalDistance.x;

			for(i = 0; i < __SHAPE_NORMALS; i++)
			{
				fix10_6 intervalDistance = __ABS(component[i]);

				if(intervalDistance < minimumIntervalDistance)
				{
					solutionVector.magnitude = minimumIntervalDistance = intervalDistance;
					solutionVector.direction = normals[i];

					if(Vector3D::dotProduct(distanceVector, solutionVector.direction) < 0)
					{
						solutionVector.direction = Vector3D::scalarProduct(solutionVector.direction, __I_TO_FIX10_6(-1));
					}
				}
			}
		}

		return (CollisionInformation){Shape::safeCast(boxA), Shape::safeCast(boxB), solutionVector};
	}

	return (CollisionInformation){NULL, NULL, {{0, 0, 0}, 0}};
}

CollisionInformation CollisionHelper::checkIfBoxOverlapsInverseBox(Box boxA, InverseBox inverseBoxB)
{
	// test for collision
	if((boxA->rightBox.x0 < inverseBoxB->rightBox.x0) | (boxA->rightBox.x1 > inverseBoxB->rightBox.x1) |
		(boxA->rightBox.y0 < inverseBoxB->rightBox.y0) | (boxA->rightBox.y1 > inverseBoxB->rightBox.y1) |
		(boxA->rightBox.z0 < inverseBoxB->rightBox.z0) | (boxA->rightBox.z1 > inverseBoxB->rightBox.z1)
	)
	{
		return (CollisionInformation){Shape::safeCast(boxA), Shape::safeCast(inverseBoxB), {{0, 0, 0}, 0}};
	}

	return (CollisionInformation){NULL, NULL, {{0, 0, 0}, 0}};
}

CollisionInformation CollisionHelper::checkIfBoxOverlapsBall(Box boxA, Ball ballB)
{
	Vector3D boxACenter =
	{
		(boxA->rightBox.x0 + boxA->rightBox.x1) >> 1,
		(boxA->rightBox.y0 + boxA->rightBox.y1) >> 1,
		(boxA->rightBox.z0 + boxA->rightBox.z1) >> 1,
	};

	Vector3D intervalDistance =
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

		SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};
		fix10_6 minimumIntervalDistance = Math::fix10_6Infinity();

		// if axis aligned, then SAT check is not needed
		// and we can calculate the minimum displacement vector
		// to resolve the collision right now
		if(isSATCheckPending)
		{
			solutionVector = CollisionHelper::getSolutionVectorBetweenBoxAndBall(this, boxA, ballB);
		}
		else
		{
			Vector3D distanceVector = Vector3D::get(boxACenter, ballB->center);

			Vector3D normals[__SHAPE_NORMALS] =
			{
				{__I_TO_FIX10_6(1), 0, 0},
				{0, __I_TO_FIX10_6(1), 0},
				{0, 0, __I_TO_FIX10_6(1)},
			};

			int32 i = 0;
			fix10_6* component = &intervalDistance.x;

			for(i = 0; i < __SHAPE_NORMALS; i++)
			{
				fix10_6 intervalDistance = __ABS(component[i]);

				if(intervalDistance < minimumIntervalDistance)
				{
					solutionVector.magnitude = minimumIntervalDistance = intervalDistance;
					solutionVector.direction = normals[i];
				}
			}

			if(Vector3D::dotProduct(distanceVector, solutionVector.direction) < 0)
			{
				solutionVector.direction = Vector3D::scalarProduct(solutionVector.direction, __I_TO_FIX10_6(-1));
			}
		}

		return (CollisionInformation){Shape::safeCast(boxA), Shape::safeCast(ballB), solutionVector};
	}

	return (CollisionInformation){NULL, NULL, {{0, 0, 0}, 0}};
}

CollisionInformation CollisionHelper::checkIfInverseBoxOverlapsInverseBox(InverseBox inverseBoxA __attribute__ ((unused)), InverseBox inverseBoxB __attribute__ ((unused)))
{
	return (CollisionInformation){NULL, NULL, {{0, 0, 0}, 0}};
}

CollisionInformation CollisionHelper::checkIfInverseBoxOverlapsBall(InverseBox inverseBoxA __attribute__ ((unused)), Ball ballB __attribute__ ((unused)))
{
	Vector3D inverseBoxACenter =
	{
		(inverseBoxA->rightBox.x0 + inverseBoxA->rightBox.x1) >> 1,
		(inverseBoxA->rightBox.y0 + inverseBoxA->rightBox.y1) >> 1,
		(inverseBoxA->rightBox.z0 + inverseBoxA->rightBox.z1) >> 1,
	};

	Vector3D intervalDistance =
	{
		inverseBoxACenter.x > ballB->center.x ? ((ballB->center.x - ballB->radius) - inverseBoxA->rightBox.x0) : (inverseBoxA->rightBox.x1 - (ballB->center.x + ballB->radius)),
		inverseBoxACenter.y > ballB->center.y ? ((ballB->center.y - ballB->radius) - inverseBoxA->rightBox.y0) : (inverseBoxA->rightBox.y1 - (ballB->center.y + ballB->radius)),
		inverseBoxACenter.z > ballB->center.z ? ((ballB->center.z - ballB->radius) - inverseBoxA->rightBox.z0) : (inverseBoxA->rightBox.z1 - (ballB->center.z + ballB->radius)),
	};

	// test for collision
	if(0 > intervalDistance.x || 0 > intervalDistance.y || 0 > intervalDistance.z)
	{
		// check if both boxes are axis aligned
		SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};
		fix10_6 minimumIntervalDistance = Math::fix10_6Infinity();

		// no SAT when checking inverse boxes
		// if axis aligned, then SAT check is not needed
		// and we can calculate the minimum displacement vector
		// to resolve the collision right now
		Vector3D distanceVector = Vector3D::get(inverseBoxACenter, ballB->center);

		Vector3D normals[__SHAPE_NORMALS] =
		{
			{__I_TO_FIX10_6(1), 0, 0},
			{0, __I_TO_FIX10_6(1), 0},
			{0, 0, __I_TO_FIX10_6(1)},
		};

		int32 i = 0;
		fix10_6* component = &intervalDistance.x;

		for(i = 0; i < __SHAPE_NORMALS; i++)
		{
			fix10_6 intervalDistance = __ABS(component[i]);

			if(intervalDistance < minimumIntervalDistance)
			{
				solutionVector.magnitude = minimumIntervalDistance = intervalDistance;
				solutionVector.direction = normals[i];
			}
		}

		if(Vector3D::dotProduct(distanceVector, solutionVector.direction) < 0)
		{
			solutionVector.direction = Vector3D::scalarProduct(solutionVector.direction, __I_TO_FIX10_6(-1));
		}

		return (CollisionInformation){Shape::safeCast(inverseBoxA), Shape::safeCast(ballB), solutionVector};
	}

	return (CollisionInformation){NULL, NULL, {{0, 0, 0}, 0}};
}

CollisionInformation CollisionHelper::checkIfBallOverlapsBall(Ball ballA, Ball ballB)
{
	SolutionVector solutionVector = CollisionHelper::getSolutionVectorBetweenBallAndBall(this, ballA, ballB);

	if(solutionVector.magnitude)
	{
		return (CollisionInformation){Shape::safeCast(ballA), Shape::safeCast(ballB), solutionVector};
	}

	return (CollisionInformation){NULL, NULL, {{0, 0, 0}, 0}};
}

CollisionInformation CollisionHelper::checkIfBallOverlapsLineField(Ball ball, LineField lineField)
{
	SolutionVector solutionVector = CollisionHelper::getSolutionVectorBetweenBallAndLineField(this, ball, lineField);

	if(solutionVector.magnitude)
	{
		return (CollisionInformation){Shape::safeCast(ball), Shape::safeCast(lineField), solutionVector};
	}

	return (CollisionInformation){NULL, NULL, {{0, 0, 0}, 0}};
}


SolutionVector CollisionHelper::getSolutionVectorBetweenBoxAndBox(Box boxA, Box boxB)
{
	// get the vertexes of each box
	Vector3D boxAVertexes[__BOX_VERTEXES];
	Vector3D boxBVertexes[__BOX_VERTEXES];
	Box::getVertexes(boxA, boxAVertexes);
	Box::getVertexes(boxB, boxBVertexes);

	// if the normals have not been computed yet do so now
	if(!boxA->normals)
	{
		Box::projectOntoItself(boxA);
	}

	if(!boxB->normals)
	{
		Box::projectOntoItself(boxA);
	}

	Vector3D* normals[2] =
	{
		boxA->normals->vectors,
		boxB->normals->vectors,
	};

	// will need
	Vector3D centers[2] =
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

	Vector3D distanceVector = Vector3D::get(centers[1], centers[0]);

	SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};
 	fix10_6 minimumIntervalDistance = Math::fix10_6Infinity();

	int32 boxIndex = 0;

	// has to project all points on all the normals of both boxes
	for(; boxIndex < 2; boxIndex++)
	{
		int32 normalIndex = 0;

		// test all 3 normals of each box
		for(; normalIndex < __SHAPE_NORMALS; normalIndex++)
		{
			Vector3D currentNormal = normals[boxIndex][normalIndex];

			fix10_6 boxAMin = boxA->vertexProjections[normalIndex].min;
			fix10_6 boxAMax = boxA->vertexProjections[normalIndex].max;
			fix10_6 boxBMin = boxB->vertexProjections[normalIndex].min;
			fix10_6 boxBMax = boxB->vertexProjections[normalIndex].max;

			if(normals[boxIndex] == boxA->normals->vectors)
			{
				boxBMin = boxBMax = 0;
				Box::project(boxBVertexes, currentNormal, &boxBMin, &boxBMax);
			}
			else if(normals[boxIndex] == boxB->normals->vectors)
			{
				boxAMin = boxAMax = 0;
				Box::project(boxAVertexes, currentNormal, &boxAMin, &boxAMax);
			}

			fix10_6 intervalDistance = 0;

			if (boxAMin < boxBMin)
			{
				intervalDistance = boxBMin - boxAMax;
			}
			else
			{
				intervalDistance = boxAMin - boxBMax;
			}

			if(0 < intervalDistance)
			{
				solutionVector = (SolutionVector) {{0, 0, 0}, 0};
				return solutionVector;
			}

			intervalDistance = __ABS(intervalDistance);

			if(intervalDistance < minimumIntervalDistance)
			{
				solutionVector.magnitude = minimumIntervalDistance = intervalDistance;
				solutionVector.direction = currentNormal;
			}
		}

		if(Vector3D::dotProduct(distanceVector, solutionVector.direction) < 0)
		{
			solutionVector.direction = Vector3D::scalarProduct(solutionVector.direction, __I_TO_FIX10_6(-1));
		}
	}

	return solutionVector;
}

static SolutionVector CollisionHelper::getSolutionVectorBetweenBoxAndInverseBox(Box boxA __attribute__ ((unused)), InverseBox inverseBoxB __attribute__ ((unused)))
{
	return (SolutionVector) {{0, 0, 0}, 0};
}

SolutionVector CollisionHelper::getSolutionVectorBetweenBoxAndBall(Box boxA __attribute__ ((unused)), Ball ballB __attribute__ ((unused)))
{
	// if the normals have not been computed yet do so now
	if(!boxA->normals)
	{
		Box::projectOntoItself(boxA);
	}

	Vector3D* normals = boxA->normals->vectors;

	// will need
	Vector3D boxACenter =
	{
		(boxA->rightBox.x0 + boxA->rightBox.x1) >> 1,
		(boxA->rightBox.y0 + boxA->rightBox.y1) >> 1,
		(boxA->rightBox.z0 + boxA->rightBox.z1) >> 1,
	};

	Vector3D distanceVector = Vector3D::get(boxACenter, ballB->center);

	SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};
	fix10_6 minimumIntervalDistance = Math::fix10_6Infinity();

	// has to project all points on all the normals of the tilted box
	int32 normalIndex = 0;

	// test all 3 normals of each box
	for(; normalIndex < __SHAPE_NORMALS; normalIndex++)
	{
		Vector3D currentNormal = normals[normalIndex];

		fix10_6 ballBMin = 0;
		fix10_6 ballBMax = 0;

		Ball::project(ballB->center, ballB->radius, currentNormal, &ballBMin, &ballBMax);

		fix10_6 intervalDistance = 0;

		if (boxA->vertexProjections[normalIndex].min < ballBMin)
		{
			intervalDistance = ballBMin - boxA->vertexProjections[normalIndex].max;
		}
		else
		{
			intervalDistance = boxA->vertexProjections[normalIndex].min - ballBMax;
		}

		if(0 < intervalDistance)
		{
			solutionVector = (SolutionVector) {{0, 0, 0}, 0};

			return solutionVector;
		}

		intervalDistance = __ABS(intervalDistance);

		if(intervalDistance < minimumIntervalDistance)
		{
			solutionVector.magnitude = minimumIntervalDistance = intervalDistance;
			solutionVector.direction = currentNormal;
		}
	}

	if(Vector3D::dotProduct(distanceVector, solutionVector.direction) < 0)
	{
		solutionVector.direction = Vector3D::scalarProduct(solutionVector.direction, __I_TO_FIX10_6(-1));
	}

	return solutionVector;
}

static SolutionVector CollisionHelper::getSolutionVectorBetweenInverseBoxAndInverseBox(InverseBox inverseBoxA __attribute__ ((unused)), InverseBox inverseBoxB __attribute__ ((unused)))
{
	return (SolutionVector) {{0, 0, 0}, 0};
}

static SolutionVector CollisionHelper::getSolutionVectorBetweenInverseBoxAndBall(InverseBox inverseBoxA __attribute__ ((unused)), Ball ballB __attribute__ ((unused)))
{
	return (SolutionVector) {{0, 0, 0}, 0};
}

SolutionVector CollisionHelper::getSolutionVectorBetweenBallAndBall(Ball ballA, Ball ballB)
{
	// Compute the distance vector backwards to avoid the need to multiply by -1 the direction
	Vector3D distanceVector = Vector3D::get(ballB->center, ballA->center);
	fix10_6_ext distanceVectorSquareLength = Vector3D::squareLength(distanceVector);
	fix10_6 radiusesLength = ballA->radius + ballB->radius;

	SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};

	if(distanceVectorSquareLength < __FIX10_6_EXT_MULT(radiusesLength, radiusesLength))
	{
		fix10_6 distanceVectorLength = __F_TO_FIX10_6(Math::squareRoot(__FIX10_6_EXT_TO_F(distanceVectorSquareLength)));

		// add padding to prevent rounding problems
		solutionVector.magnitude = radiusesLength - distanceVectorLength;
		solutionVector.magnitude += 0 == solutionVector.magnitude ? __PIXELS_TO_METERS(1) : 0;
		solutionVector.direction = Vector3D::scalarDivision(distanceVector, distanceVectorLength);

		if(__I_TO_FIX10_6(1) < solutionVector.direction.x)
		{
			solutionVector.direction.x = __I_TO_FIX10_6(1);
		}
		else if(-__I_TO_FIX10_6(1) > solutionVector.direction.x)
		{
			solutionVector.direction.x = -__I_TO_FIX10_6(1);
		}

		if(__I_TO_FIX10_6(1) < solutionVector.direction.y)
		{
			solutionVector.direction.y = __I_TO_FIX10_6(1);
		}
		else if(-__I_TO_FIX10_6(1) > solutionVector.direction.y)
		{
			solutionVector.direction.y = -__I_TO_FIX10_6(1);
		}

		if(__I_TO_FIX10_6(1) < solutionVector.direction.z)
		{
			solutionVector.direction.z = __I_TO_FIX10_6(1);
		}
		else if(-__I_TO_FIX10_6(1) > solutionVector.direction.z)
		{
			solutionVector.direction.z = -__I_TO_FIX10_6(1);
		}
	}

	return solutionVector;
}

SolutionVector CollisionHelper::getSolutionVectorBetweenBallAndLineField(Ball ball, LineField lineField)
{
	// TODO: this misses some cases when the ball's radius is bigger than the line field's length
	// A first check should compare them and use the bigger's shape axis as the line onto which
	// project the other shape's points

	SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};

	Vector3D ballSideToCheck = Vector3D::sum(ball->center, Vector3D::scalarProduct(lineField->normal, ball->radius));

	fix10_6 position = __FIX10_6_MULT((lineField->b.x - lineField->a.x), (ballSideToCheck.y - lineField->a.y)) - __FIX10_6_MULT((lineField->b.y - lineField->a.y), (ballSideToCheck.x - lineField->a.x));

	if(0 > position)
	{
		Vector3D projection = Vector3D::projectOntoHighPrecision(ballSideToCheck, lineField->a, lineField->b);

		bool collision = Vector3D::isVectorInsideLine(projection, lineField->a, lineField->b);

		if(!collision)
		{
			Vector3D ballRadiusVector = Vector3D::scalarProduct(lineField->normal, ball->radius);

			// Check both sides of the ball
			// This is a rough approximation since it identifies a collision even if the ball and the line field
			// are not really overlapping
			for(bool left = true; !collision && left; left = false)
			{
				Vector3D projectionPlusRadio = Vector3D::sum(projection, Vector3D::perpedicular(ballRadiusVector, left));
				
				collision = Vector3D::isVectorInsideLine(projectionPlusRadio, lineField->a, lineField->b);
			}
		}

		if(collision)
		{
			fix10_6 distanceToLine = Vector3D::length(Vector3D::get(projection, ballSideToCheck));

			if(distanceToLine < lineField->normalLength + (ball->radius << 1))
			{
				solutionVector.magnitude = distanceToLine + __PIXELS_TO_METERS(1);
				solutionVector.direction = Vector3D::scalarProduct(lineField->normal, __I_TO_FIX10_6(-1));
			}
		}
	}

	return solutionVector;
}