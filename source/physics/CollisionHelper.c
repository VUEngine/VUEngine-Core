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
#include <Box.h>
#include <InverseBox.h>
#include <Ball.h>
#include <LineField.h>
#include <Shape.h>
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

static void CollisionHelper::getSolutionVectorBetweenBoxAndBox(Box boxA, Box boxB, SolutionVector* solutionVector)
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

 	fixed_t minimumIntervalDistance = Math::fixedInfinity();

	int32 boxIndex = 0;

	// has to project all points on all the normals of both boxes
	for(; boxIndex < 2; boxIndex++)
	{
		int32 normalIndex = 0;

		// test all 3 normals of each box
		for(; normalIndex < __SHAPE_NORMALS; normalIndex++)
		{
			Vector3D currentNormal = normals[boxIndex][normalIndex];

			fixed_t boxAMin = boxA->vertexProjections[normalIndex].min;
			fixed_t boxAMax = boxA->vertexProjections[normalIndex].max;
			fixed_t boxBMin = boxB->vertexProjections[normalIndex].min;
			fixed_t boxBMax = boxB->vertexProjections[normalIndex].max;

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

			fixed_t intervalDistance = 0;

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
				*solutionVector = (SolutionVector) {{0, 0, 0}, 0};
				return;
			}

			intervalDistance = __ABS(intervalDistance);

			if(intervalDistance < minimumIntervalDistance)
			{
				solutionVector->magnitude = minimumIntervalDistance = intervalDistance;
				solutionVector->direction = currentNormal;
			}
		}

		if(Vector3D::dotProduct(distanceVector, solutionVector->direction) < 0)
		{
			solutionVector->direction = Vector3D::scalarProduct(solutionVector->direction, __I_TO_FIXED(-1));
		}
	}
}

static void CollisionHelper::getSolutionVectorBetweenBoxAndInverseBox(Box boxA __attribute__ ((unused)), InverseBox inverseBoxB __attribute__ ((unused)), SolutionVector* solutionVector __attribute__ ((unused)))
{
}

static void CollisionHelper::getSolutionVectorBetweenInverseBoxAndInverseBox(InverseBox inverseBoxA __attribute__ ((unused)), InverseBox inverseBoxB __attribute__ ((unused)), SolutionVector* solutionVector __attribute__ ((unused)))
{
}

static void CollisionHelper::getSolutionVectorBetweenInverseBoxAndBall(InverseBox inverseBoxA __attribute__ ((unused)), Ball ballB __attribute__ ((unused)), SolutionVector* solutionVector __attribute__ ((unused)))
{
}

static void CollisionHelper::getSolutionVectorBetweenBallAndBall(Ball ballA, Ball ballB, SolutionVector* solutionVector)
{
	// Compute the distance vector backwards to avoid the need to multiply by -1 the direction
	Vector3D distanceVector = Vector3D::get(ballB->position, ballA->position);
	fixed_ext_t distanceVectorSquareLength = Vector3D::squareLength(distanceVector);
	fixed_t radiusesLength = ballA->radius + ballB->radius;

	if(distanceVectorSquareLength < __FIXED_SQUARE(radiusesLength))
	{
		fixed_t distanceVectorLength = Math::squareRootFixed(distanceVectorSquareLength);

		// add padding to prevent rounding problems
		solutionVector->magnitude = radiusesLength - distanceVectorLength;
		solutionVector->magnitude += 0 == solutionVector->magnitude ? __PIXELS_TO_METERS(1) : 0;
		solutionVector->direction = Vector3D::scalarDivision(distanceVector, distanceVectorLength);

		if(__I_TO_FIXED(1) < solutionVector->direction.x)
		{
			solutionVector->direction.x = __I_TO_FIXED(1);
		}
		else if(-__I_TO_FIXED(1) > solutionVector->direction.x)
		{
			solutionVector->direction.x = -__I_TO_FIXED(1);
		}

		if(__I_TO_FIXED(1) < solutionVector->direction.y)
		{
			solutionVector->direction.y = __I_TO_FIXED(1);
		}
		else if(-__I_TO_FIXED(1) > solutionVector->direction.y)
		{
			solutionVector->direction.y = -__I_TO_FIXED(1);
		}

		if(__I_TO_FIXED(1) < solutionVector->direction.z)
		{
			solutionVector->direction.z = __I_TO_FIXED(1);
		}
		else if(-__I_TO_FIXED(1) > solutionVector->direction.z)
		{
			solutionVector->direction.z = -__I_TO_FIXED(1);
		}
	}
}

static void CollisionHelper::getSolutionVectorBetweenBallAndLineField(Ball ball, LineField lineField, SolutionVector* solutionVector)
{
	// TODO: this misses some cases when the ball's radius is bigger than the line field's length
	// A first check should compare them and use the bigger's shape axis as the line onto which
	// project the other shape's points

	Vector3D ballSideToCheck = Vector3D::sum(ball->position, Vector3D::scalarProduct(lineField->normal, ball->radius));

	fixed_t position = __FIXED_MULT((lineField->b.x - lineField->a.x), (ballSideToCheck.y - lineField->a.y)) - __FIXED_MULT((lineField->b.y - lineField->a.y), (ballSideToCheck.x - lineField->a.x));

	if(0 > position)
	{
		Vector3D projection = Vector3D::projectOnto(ballSideToCheck, lineField->a, lineField->b);

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
			fixed_t distanceToLine = Vector3D::length(Vector3D::get(projection, ballSideToCheck));

			if(distanceToLine < lineField->normalLength + (ball->radius << 1))
			{
				solutionVector->magnitude = distanceToLine + __PIXELS_TO_METERS(1);
				solutionVector->direction = Vector3D::scalarProduct(lineField->normal, __I_TO_FIXED(-1));
			}
		}
	}
}

static void CollisionHelper::getSolutionVectorBetweenBoxAndBall(Box boxA, Ball ballB, SolutionVector* solutionVector)
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

	Vector3D distanceVector = Vector3D::get(boxACenter, ballB->position);

	fixed_t minimumIntervalDistance = Math::fixedInfinity();

	// has to project all points on all the normals of the tilted box
	int32 normalIndex = 0;

	// test all 3 normals of each box
	for(; normalIndex < __SHAPE_NORMALS; normalIndex++)
	{
		Vector3D currentNormal = normals[normalIndex];

		fixed_t ballBMin = 0;
		fixed_t ballBMax = 0;

		Ball::project(ballB->position, ballB->radius, currentNormal, &ballBMin, &ballBMax);

		fixed_t intervalDistance = 0;

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
			*solutionVector = (SolutionVector) {{0, 0, 0}, 0};

			return;
		}

		intervalDistance = __ABS(intervalDistance);

		if(intervalDistance < minimumIntervalDistance)
		{
			solutionVector->magnitude = minimumIntervalDistance = intervalDistance;
			solutionVector->direction = currentNormal;
		}
	}

	if(Vector3D::dotProduct(distanceVector, solutionVector->direction) < 0)
	{
		solutionVector->direction = Vector3D::scalarProduct(solutionVector->direction, __I_TO_FIXED(-1));
	}
}

static void CollisionHelper::checkIfBoxOverlapsBox(Box boxA, Box boxB, CollisionInformation* collisionInformation)
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
		fixed_t minimumIntervalDistance = Math::fixedInfinity();

		// if axis aligned, then SAT check is not needed
		// and we can calculate the minimum displacement vector
		// to resolve the collision right now
		if(isSATCheckPending)
		{
			CollisionHelper::getSolutionVectorBetweenBoxAndBox(boxA, boxB, &solutionVector);
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
				{__I_TO_FIXED(1), 0, 0},
				{0, __I_TO_FIXED(1), 0},
				{0, 0, __I_TO_FIXED(1)},
			};

			int32 i = 0;
			fixed_t* component = &intervalDistance.x;

			for(i = 0; i < __SHAPE_NORMALS; i++)
			{
				fixed_t intervalDistance = __ABS(component[i]);

				if(intervalDistance < minimumIntervalDistance)
				{
					solutionVector.magnitude = minimumIntervalDistance = intervalDistance;
					solutionVector.direction = normals[i];

					if(Vector3D::dotProduct(distanceVector, solutionVector.direction) < 0)
					{
						solutionVector.direction = Vector3D::scalarProduct(solutionVector.direction, __I_TO_FIXED(-1));
					}
				}
			}
		}

		collisionInformation->shape = Shape::safeCast(boxA);
		collisionInformation->collidingShape = Shape::safeCast(boxB);
		collisionInformation->solutionVector = solutionVector;

		return;
	}
}

static void CollisionHelper::checkIfBoxOverlapsInverseBox(Box boxA, InverseBox inverseBoxB, CollisionInformation* collisionInformation)
{
	// test for collision
	if((boxA->rightBox.x0 < inverseBoxB->rightBox.x0) | (boxA->rightBox.x1 > inverseBoxB->rightBox.x1) |
		(boxA->rightBox.y0 < inverseBoxB->rightBox.y0) | (boxA->rightBox.y1 > inverseBoxB->rightBox.y1) |
		(boxA->rightBox.z0 < inverseBoxB->rightBox.z0) | (boxA->rightBox.z1 > inverseBoxB->rightBox.z1)
	)
	{
		collisionInformation->shape = Shape::safeCast(boxA);
		collisionInformation->collidingShape = Shape::safeCast(inverseBoxB);
		collisionInformation->solutionVector = (SolutionVector){{0, 0, 0}, 0};
	}
}

static void CollisionHelper::checkIfBoxOverlapsBall(Box boxA, Ball ballB, CollisionInformation* collisionInformation)
{
	Vector3D boxACenter =
	{
		(boxA->rightBox.x0 + boxA->rightBox.x1) >> 1,
		(boxA->rightBox.y0 + boxA->rightBox.y1) >> 1,
		(boxA->rightBox.z0 + boxA->rightBox.z1) >> 1,
	};

	Vector3D intervalDistance =
	{
		boxACenter.x < ballB->position.x ? ((ballB->position.x - ballB->radius) - boxA->rightBox.x1) : (boxA->rightBox.x0 - (ballB->position.x + ballB->radius)),
		boxACenter.y < ballB->position.y ? ((ballB->position.y - ballB->radius) - boxA->rightBox.y1) : (boxA->rightBox.y0 - (ballB->position.y + ballB->radius)),
		boxACenter.z < ballB->position.z ? ((ballB->position.z - ballB->radius) - boxA->rightBox.z1) : (boxA->rightBox.z0 - (ballB->position.z + ballB->radius)),
	};

	// test for collision
	if(0 > intervalDistance.x && 0 > intervalDistance.y && 0 > intervalDistance.z)
	{
		// check if both boxes are axis aligned
		bool isSATCheckPending = boxA->rotationVertexDisplacement.x | boxA->rotationVertexDisplacement.y | boxA->rotationVertexDisplacement.z ? true : false;

		SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};
		fixed_t minimumIntervalDistance = Math::fixedInfinity();

		// if axis aligned, then SAT check is not needed
		// and we can calculate the minimum displacement vector
		// to resolve the collision right now
		if(isSATCheckPending)
		{
			CollisionHelper::getSolutionVectorBetweenBoxAndBall(boxA, ballB, &solutionVector);
		}
		else
		{
			Vector3D distanceVector = Vector3D::get(boxACenter, ballB->position);

			Vector3D normals[__SHAPE_NORMALS] =
			{
				{__I_TO_FIXED(1), 0, 0},
				{0, __I_TO_FIXED(1), 0},
				{0, 0, __I_TO_FIXED(1)},
			};

			int32 i = 0;
			fixed_t* component = &intervalDistance.x;

			for(i = 0; i < __SHAPE_NORMALS; i++)
			{
				fixed_t intervalDistance = __ABS(component[i]);

				if(intervalDistance < minimumIntervalDistance)
				{
					solutionVector.magnitude = minimumIntervalDistance = intervalDistance;
					solutionVector.direction = normals[i];
				}
			}

			if(Vector3D::dotProduct(distanceVector, solutionVector.direction) < 0)
			{
				solutionVector.direction = Vector3D::scalarProduct(solutionVector.direction, __I_TO_FIXED(-1));
			}
		}

		collisionInformation->shape = Shape::safeCast(boxA);
		collisionInformation->collidingShape = Shape::safeCast(ballB);
		collisionInformation->solutionVector = solutionVector;
		return;
	}
}

static void CollisionHelper::checkIfInverseBoxOverlapsInverseBox(InverseBox inverseBoxA __attribute__ ((unused)), InverseBox inverseBoxB __attribute__ ((unused)), CollisionInformation* collisionInformation __attribute__ ((unused)))
{
}

static void CollisionHelper::checkIfInverseBoxOverlapsBall(InverseBox inverseBoxA, Ball ballB, CollisionInformation* collisionInformation)
{
	Vector3D inverseBoxACenter =
	{
		(inverseBoxA->rightBox.x0 + inverseBoxA->rightBox.x1) >> 1,
		(inverseBoxA->rightBox.y0 + inverseBoxA->rightBox.y1) >> 1,
		(inverseBoxA->rightBox.z0 + inverseBoxA->rightBox.z1) >> 1,
	};

	Vector3D intervalDistance =
	{
		inverseBoxACenter.x > ballB->position.x ? ((ballB->position.x - ballB->radius) - inverseBoxA->rightBox.x0) : (inverseBoxA->rightBox.x1 - (ballB->position.x + ballB->radius)),
		inverseBoxACenter.y > ballB->position.y ? ((ballB->position.y - ballB->radius) - inverseBoxA->rightBox.y0) : (inverseBoxA->rightBox.y1 - (ballB->position.y + ballB->radius)),
		inverseBoxACenter.z > ballB->position.z ? ((ballB->position.z - ballB->radius) - inverseBoxA->rightBox.z0) : (inverseBoxA->rightBox.z1 - (ballB->position.z + ballB->radius)),
	};

	// test for collision
	if(0 > intervalDistance.x || 0 > intervalDistance.y || 0 > intervalDistance.z)
	{
		// check if both boxes are axis aligned
		SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};
		fixed_t minimumIntervalDistance = Math::fixedInfinity();

		// no SAT when checking inverse boxes
		// if axis aligned, then SAT check is not needed
		// and we can calculate the minimum displacement vector
		// to resolve the collision right now
		Vector3D distanceVector = Vector3D::get(inverseBoxACenter, ballB->position);

		Vector3D normals[__SHAPE_NORMALS] =
		{
			{__I_TO_FIXED(1), 0, 0},
			{0, __I_TO_FIXED(1), 0},
			{0, 0, __I_TO_FIXED(1)},
		};

		int32 i = 0;
		fixed_t* component = &intervalDistance.x;

		for(i = 0; i < __SHAPE_NORMALS; i++)
		{
			fixed_t intervalDistance = __ABS(component[i]);

			if(intervalDistance < minimumIntervalDistance)
			{
				solutionVector.magnitude = minimumIntervalDistance = intervalDistance;
				solutionVector.direction = normals[i];
			}
		}

		if(Vector3D::dotProduct(distanceVector, solutionVector.direction) < 0)
		{
			solutionVector.direction = Vector3D::scalarProduct(solutionVector.direction, __I_TO_FIXED(-1));
		}

		collisionInformation->shape = Shape::safeCast(inverseBoxA);
		collisionInformation->collidingShape = Shape::safeCast(ballB);
		collisionInformation->solutionVector = solutionVector;

		return;
	}
}

static void CollisionHelper::checkIfBallOverlapsBall(Ball ballA, Ball ballB, CollisionInformation* collisionInformation)
{
	SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};

	CollisionHelper::getSolutionVectorBetweenBallAndBall(ballA, ballB, &solutionVector);

	if(0 != solutionVector.magnitude)
	{
		collisionInformation->shape = Shape::safeCast(ballA);
		collisionInformation->collidingShape = Shape::safeCast(ballB);
		collisionInformation->solutionVector = solutionVector;
	}
}

static void CollisionHelper::checkIfBallOverlapsLineField(Ball ball, LineField lineField, CollisionInformation* collisionInformation)
{
	SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};

	CollisionHelper::getSolutionVectorBetweenBallAndLineField(ball, lineField, &solutionVector);

	if(0 != solutionVector.magnitude)
	{
		collisionInformation->shape = Shape::safeCast(ball);
		collisionInformation->collidingShape = Shape::safeCast(lineField);
		collisionInformation->solutionVector = solutionVector;
	}
}

/**
 * Check if two shapes overlap
 *
 * @param shapeA	Shape
 * @param shapeB	Shape
 */
static CollisionInformation CollisionHelper::checkIfOverlap(Shape shapeA, Shape shapeB)
{
	ASSERT(shapeA, "CollisionHelper::checkIfOverlap: null shapeA");
	ASSERT(shapeB, "CollisionHelper::checkIfOverlap: null shapeA");

	CollisionInformation collisionInformation = (CollisionInformation){NULL, NULL, {{0, 0, 0}, 0}};

	if(__IS_INSTANCE_OF(Ball, shapeA))
	{
		if(__IS_INSTANCE_OF(Box, shapeB))
    	{
			CollisionHelper::checkIfBoxOverlapsBall(Box::safeCast(shapeB), Ball::safeCast(shapeA), &collisionInformation);
		}
		else if(__IS_INSTANCE_OF(InverseBox, shapeB))
		{
			CollisionHelper::checkIfInverseBoxOverlapsBall(InverseBox::safeCast(shapeB), Ball::safeCast(shapeA), &collisionInformation);
		}
		else if(__IS_INSTANCE_OF(Ball, shapeB))
		{
			CollisionHelper::checkIfBallOverlapsBall(Ball::safeCast(shapeA), Ball::safeCast(shapeB), &collisionInformation);
		}
		else if(__IS_INSTANCE_OF(LineField, shapeB))
		{
			CollisionHelper::checkIfBallOverlapsLineField(Ball::safeCast(shapeA), LineField::safeCast(shapeB), &collisionInformation);
		}
	}
	else if(__IS_INSTANCE_OF(Box, shapeA))
	{
		if(__IS_INSTANCE_OF(Box, shapeB))
    	{
			CollisionHelper::checkIfBoxOverlapsBox(Box::safeCast(shapeA), Box::safeCast(shapeB), &collisionInformation);
		}
		else if(__IS_INSTANCE_OF(InverseBox, shapeB))
		{
			CollisionHelper::checkIfBoxOverlapsInverseBox(Box::safeCast(shapeA), InverseBox::safeCast(shapeB), &collisionInformation);
		}
		else if(__IS_INSTANCE_OF(Ball, shapeB))
		{
			CollisionHelper::checkIfBoxOverlapsBall(Box::safeCast(shapeA), Ball::safeCast(shapeB), &collisionInformation);
		}
	}
	else if(__IS_INSTANCE_OF(InverseBox, shapeA))
	{
		if(__IS_INSTANCE_OF(Box, shapeB))
    	{
			CollisionHelper::checkIfBoxOverlapsInverseBox(Box::safeCast(shapeB), InverseBox::safeCast(shapeA), &collisionInformation);
		}
		else if(__IS_INSTANCE_OF(InverseBox, shapeB))
		{
			CollisionHelper::checkIfInverseBoxOverlapsInverseBox(InverseBox::safeCast(shapeA), InverseBox::safeCast(shapeB), &collisionInformation);
		}
		else if(__IS_INSTANCE_OF(Ball, shapeB))
		{
			CollisionHelper::checkIfInverseBoxOverlapsBall(InverseBox::safeCast(shapeA), Ball::safeCast(shapeB), &collisionInformation);
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