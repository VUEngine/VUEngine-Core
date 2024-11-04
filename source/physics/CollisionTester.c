/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <Ball.h>
#include <Box.h>
#include <InverseBox.h>
#include <LineField.h>
#include <Collider.h>

#include "CollisionTester.h"



//=========================================================================================================
// CLASS' DECLARATIONS
//=========================================================================================================

friend class Collider;
friend class Box;
friend class InverseBox;
friend class Ball;
friend class LineField;


//=========================================================================================================
// CLASS' STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testOverlaping(Collider requesterCollider, Collider otherCollider, CollisionInformation* collisionInformation, fixed_t sizeDelta)
{
	NM_ASSERT(!isDeleted(requesterCollider), "CollisionTester::testOverlaping: deleted requesterCollider");
	NM_ASSERT(!isDeleted(otherCollider), "CollisionTester::testOverlaping: deleted otherCollider");

	NM_ASSERT(4 > (unsigned)requesterCollider->classIndex, "CollisionTester::testOverlaping: wrong requesterCollider's class index");
	NM_ASSERT(4 > (unsigned)otherCollider->classIndex, "CollisionTester::testOverlaping: wrong otherCollider's class index");

	if(isDeleted(requesterCollider) || isDeleted(otherCollider) || NULL == collisionInformation)
	{
		return;
	}

	if(0 != sizeDelta)
	{
		Collider::resize(requesterCollider, sizeDelta);
	}

	typedef void (*CollisionTesterMethod)(Collider, Collider, CollisionInformation*);

	// Will need to update this as more colliders are added
	// {Ball, Box, InverseBox, LineField} x {Ball, Box, InverseBox, LineField}
	static CollisionTesterMethod collisionTesterMethods[][4] =
	{
		// Ball against others
		{CollisionTester::testIfBallOverlapsBall, CollisionTester::testIfBallOverlapsBox, CollisionTester::testIfBallOverlapsInverseBox, CollisionTester::testIfBallOverlapsLineField},
		// Box against others
		{CollisionTester::testIfBoxOverlapsBall, CollisionTester::testIfBoxOverlapsBox, CollisionTester::testIfBoxOverlapsInverseBox, CollisionTester::testIfBoxOverlapsLineField},
		// InverseBox against others
		{CollisionTester::testIfInverseBoxOverlapsBall, CollisionTester::testIfInverseBoxOverlapsBox, CollisionTester::testIfInverseBoxOverlapsInverseBox, CollisionTester::testIfInverseBoxOverlapsLineField},
		// LineField against others
		{CollisionTester::testIfLineFieldOverlapsBall, CollisionTester::testIfLineFieldOverlapsBox, CollisionTester::checkLineFieldIfOverlapsInverseBox, CollisionTester::testIfLineFieldOverlapsLineField},
	};

	CollisionTesterMethod collisionTesterMethod = collisionTesterMethods[requesterCollider->classIndex][otherCollider->classIndex];

	collisionInformation->collider = NULL;

	collisionTesterMethod(requesterCollider, otherCollider, collisionInformation);

	// We could have swapped the arguments to the checking methods to avoid code repetition
	if(NULL != collisionInformation->collider)
	{
		collisionInformation->collider = requesterCollider;
		collisionInformation->otherCollider = otherCollider;
	}

	if(0 != sizeDelta)
	{
		Collider::resize(requesterCollider, -sizeDelta);
	}
}
//---------------------------------------------------------------------------------------------------------

//=========================================================================================================
// CLASS' PRIVATE STATIC METHODS
//=========================================================================================================

//---------------------------------------------------------------------------------------------------------
static void CollisionTester::getSolutionVectorBetweenBoxAndBox(Box boxA, Box boxB, SolutionVector* solutionVector)
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

	Vector3D distanceVector = Vector3D::get(boxB->position, boxA->position);

 	fixed_t minimumIntervalDistance = Math::fixedInfinity();

	int32 boxIndex = 0;

	// has to project all points on all the normals of both boxes
	for(; boxIndex < 2; boxIndex++)
	{
		int32 normalIndex = 0;

		// test all 3 normals of each box
		for(; normalIndex < __COLLIDER_NORMALS; normalIndex++)
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
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::getSolutionVectorBetweenBoxAndInverseBox(Box boxA __attribute__ ((unused)), InverseBox inverseBoxB __attribute__ ((unused)), SolutionVector* solutionVector __attribute__ ((unused)))
{
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::getSolutionVectorBetweenInverseBoxAndInverseBox(InverseBox inverseBoxA __attribute__ ((unused)), InverseBox inverseBoxB __attribute__ ((unused)), SolutionVector* solutionVector __attribute__ ((unused)))
{
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::getSolutionVectorBetweenInverseBoxAndBall(InverseBox inverseBoxA __attribute__ ((unused)), Ball ballB __attribute__ ((unused)), SolutionVector* solutionVector __attribute__ ((unused)))
{
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::getSolutionVectorBetweenBallAndBall(Ball ballA, Ball ballB, SolutionVector* solutionVector)
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
/*
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
		*/
	}
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::getSolutionVectorBetweenBallAndLineField(Ball ball, LineField lineField, SolutionVector* solutionVector)
{
	// TODO: this misses some cases when the ball's radius is bigger than the line field's length
	// A first check should compare them and use the bigger's collider axis as the line onto which
	// project the other collider's points

	Vector3D ballSideToCheck = Vector3D::sum(ball->position, Vector3D::scalarProduct(lineField->normal, ball->radius));

	Vector3D lineFieldA = Vector3D::sum(lineField->a, lineField->position);
	Vector3D lineFieldB = Vector3D::sum(lineField->b, lineField->position);

	// Test against the bounding box first to avoid the projection if possible
	if(
		(
			lineFieldA.x != lineFieldB.x
			&&
			(
				(ballSideToCheck.x + (ball->radius << 1) < lineFieldA.x && ballSideToCheck.x < lineFieldB.x)
				||
				(ballSideToCheck.x - (ball->radius << 1) > lineFieldA.x && ballSideToCheck.x > lineFieldB.x)
			)
		)
		||
		(
			lineFieldA.y != lineFieldB.y
			&&
			(
				(ballSideToCheck.y + (ball->radius << 1) < lineFieldA.y && ballSideToCheck.y < lineFieldB.y)
				||
				(ballSideToCheck.y - (ball->radius << 1) > lineFieldA.y && ballSideToCheck.y > lineFieldB.y)
			)
		)
	)
	{
		return;
	}

	fixed_t position = __FIXED_MULT((lineFieldB.x - lineFieldA.x), (ballSideToCheck.y - lineFieldA.y)) - __FIXED_MULT((lineFieldB.y - lineFieldA.y), (ballSideToCheck.x - lineFieldA.x));

	if(0 > position)
	{
		Vector3D projection = Vector3D::projectOntoHighPrecision(ballSideToCheck, lineFieldA, lineFieldB);

		bool collision = Vector3D::isVectorInsideLine(projection, lineFieldA, lineFieldB);

		if(!collision)
		{
			Vector3D ballRadiusVector = Vector3D::scalarProduct(lineField->normal, ball->radius);

			// Check both sides of the ball
			// This is a rough approximation since it identifies a collision even if the ball and the line field
			// are not really overlapping
			for(bool left = true; !collision && left; left = false)
			{
				Vector3D projectionPlusRadio = Vector3D::sum(projection, Vector3D::perpendicularZPlane(ballRadiusVector, left));
				
				collision = Vector3D::isVectorInsideLine(projectionPlusRadio, lineFieldA, lineFieldB);
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
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::getSolutionVectorBetweenBoxAndBall(Box boxA, Ball ballB, SolutionVector* solutionVector)
{
	// if the normals have not been computed yet do so now
	if(NULL == boxA->normals)
	{
		Box::projectOntoItself(boxA);
	}

	Vector3D* normals = boxA->normals->vectors;

	Vector3D distanceVector = Vector3D::get(boxA->position, ballB->position);

	fixed_t minimumIntervalDistance = Math::fixedInfinity();

	// has to project all points on all the normals of the tilted box
	int32 normalIndex = 0;

	// test all 3 normals of each box
	for(; normalIndex < __COLLIDER_NORMALS; normalIndex++)
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
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfBallOverlapsBall(Collider colliderA, Collider colliderB, CollisionInformation* collisionInformation)
{
	Ball ballA = Ball::safeCast(colliderA);
	Ball ballB = Ball::safeCast(colliderB);

	SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};

	CollisionTester::getSolutionVectorBetweenBallAndBall(ballA, ballB, &solutionVector);

	if(0 != solutionVector.magnitude)
	{
		collisionInformation->collider = Collider::safeCast(ballA);
		collisionInformation->otherCollider = Collider::safeCast(ballB);
		collisionInformation->solutionVector = solutionVector;
	}
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfBallOverlapsBox(Collider colliderA, Collider colliderB, CollisionInformation* collisionInformation)
{
	CollisionTester::testIfBoxOverlapsBall(colliderB, colliderA, collisionInformation);
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfBallOverlapsInverseBox(Collider colliderA, Collider colliderB, CollisionInformation* collisionInformation)
{
	CollisionTester::testIfInverseBoxOverlapsBall(colliderB, colliderA, collisionInformation);
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfBallOverlapsLineField(Collider colliderA, Collider colliderB, CollisionInformation* collisionInformation)
{
	Ball ball = Ball::safeCast(colliderA);
	LineField lineField = LineField::safeCast(colliderB);

	SolutionVector solutionVector = (SolutionVector) {{0, 0, 0}, 0};

	CollisionTester::getSolutionVectorBetweenBallAndLineField(ball, lineField, &solutionVector);

	if(0 != solutionVector.magnitude)
	{
		collisionInformation->collider = Collider::safeCast(ball);
		collisionInformation->otherCollider = Collider::safeCast(lineField);
		collisionInformation->solutionVector = solutionVector;
	}
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfBoxOverlapsBall(Collider colliderA, Collider colliderB, CollisionInformation* collisionInformation)
{
	Box boxA = Box::safeCast(colliderA); 
	Ball ballB = Ball::safeCast(colliderB);

	Vector3D intervalDistance =
	{
		boxA->position.x < ballB->position.x ? ((ballB->position.x - ballB->radius) - (boxA->position.x + boxA->rightBox.x1)) : ((boxA->position.x + boxA->rightBox.x0) - (ballB->position.x + ballB->radius)),
		boxA->position.y < ballB->position.y ? ((ballB->position.y - ballB->radius) - (boxA->position.y + boxA->rightBox.y1)) : ((boxA->position.y + boxA->rightBox.y0) - (ballB->position.y + ballB->radius)),
		boxA->position.z < ballB->position.z ? ((ballB->position.z - ballB->radius) - (boxA->position.z + boxA->rightBox.z1)) : ((boxA->position.z + boxA->rightBox.z0) - (ballB->position.z + ballB->radius)),
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
			CollisionTester::getSolutionVectorBetweenBoxAndBall(boxA, ballB, &solutionVector);
		}
		else
		{
			Vector3D distanceVector = Vector3D::get(boxA->position, ballB->position);

			Vector3D normals[__COLLIDER_NORMALS] =
			{
				{__I_TO_FIXED(1), 0, 0},
				{0, __I_TO_FIXED(1), 0},
				{0, 0, __I_TO_FIXED(1)},
			};

			int32 i = 0;
			fixed_t* component = &intervalDistance.x;

			for(i = 0; i < __COLLIDER_NORMALS; i++)
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

		collisionInformation->collider = Collider::safeCast(boxA);
		collisionInformation->otherCollider = Collider::safeCast(ballB);
		collisionInformation->solutionVector = solutionVector;
		return;
	}
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfBoxOverlapsBox(Collider colliderA, Collider colliderB, CollisionInformation* collisionInformation)
{
	Box boxA = Box::safeCast(colliderA); 
	Box boxB = Box::safeCast(colliderB); 

	Vector3D intervalDistance =
	{
		boxA->position.x < boxB->position.x ? ((boxB->position.x + boxB->rightBox.x0) - (boxA->position.x + boxA->rightBox.x1)) : ((boxA->position.x + boxA->rightBox.x0) - (boxB->position.x + boxB->rightBox.x1)),
		boxA->position.y < boxB->position.y ? ((boxB->position.y + boxB->rightBox.y0) - (boxA->position.y + boxA->rightBox.y1)) : ((boxA->position.y + boxA->rightBox.y0) - (boxB->position.y + boxB->rightBox.y1)),
		boxA->position.z < boxB->position.z ? ((boxB->position.z + boxB->rightBox.z0) - (boxA->position.z + boxA->rightBox.z1)) : ((boxA->position.z + boxA->rightBox.z0) - (boxB->position.z + boxB->rightBox.z1)),
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
			CollisionTester::getSolutionVectorBetweenBoxAndBox(boxA, boxB, &solutionVector);
		}
		else
		{
			Vector3D distanceVector = Vector3D::get(boxB->position, boxA->position);

			Vector3D normals[__COLLIDER_NORMALS] =
			{
				{__I_TO_FIXED(1), 0, 0},
				{0, __I_TO_FIXED(1), 0},
				{0, 0, __I_TO_FIXED(1)},
			};

			int32 i = 0;
			fixed_t* component = &intervalDistance.x;

			for(i = 0; i < __COLLIDER_NORMALS; i++)
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

		collisionInformation->collider = Collider::safeCast(boxA);
		collisionInformation->otherCollider = Collider::safeCast(boxB);
		collisionInformation->solutionVector = solutionVector;

		return;
	}
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfBoxOverlapsInverseBox(Collider colliderA, Collider colliderB, CollisionInformation* collisionInformation)
{
	Box boxA = Box::safeCast(colliderA); 
	InverseBox inverseBoxB = InverseBox::safeCast(colliderB);

	// test for collision
	if
	(
		(boxA->rightBox.x0 < inverseBoxB->rightBox.x0) | (boxA->rightBox.x1 > inverseBoxB->rightBox.x1) |
		(boxA->rightBox.y0 < inverseBoxB->rightBox.y0) | (boxA->rightBox.y1 > inverseBoxB->rightBox.y1) |
		(boxA->rightBox.z0 < inverseBoxB->rightBox.z0) | (boxA->rightBox.z1 > inverseBoxB->rightBox.z1)
	)
	{
		collisionInformation->collider = Collider::safeCast(boxA);
		collisionInformation->otherCollider = Collider::safeCast(inverseBoxB);
		collisionInformation->solutionVector = (SolutionVector){{0, 0, 0}, 0};
	}
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfBoxOverlapsLineField(Collider colliderA __attribute__((unused)), Collider colliderB __attribute__((unused)), CollisionInformation* collisionInformation __attribute__((unused)))
{
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfInverseBoxOverlapsBall(Collider colliderA, Collider colliderB, CollisionInformation* collisionInformation)
{
	InverseBox inverseBoxA = InverseBox::safeCast(colliderA); 
	Ball ballB = Ball::safeCast(colliderB);

	Vector3D intervalDistance =
	{
		inverseBoxA->position.x > ballB->position.x ? ((ballB->position.x - ballB->radius) - (inverseBoxA->position.x + inverseBoxA->rightBox.x0)) : ((inverseBoxA->position.x + inverseBoxA->rightBox.x1) - (ballB->position.x + ballB->radius)),
		inverseBoxA->position.y > ballB->position.y ? ((ballB->position.y - ballB->radius) - (inverseBoxA->position.y + inverseBoxA->rightBox.y0)) : ((inverseBoxA->position.y + inverseBoxA->rightBox.y1) - (ballB->position.y + ballB->radius)),
		inverseBoxA->position.z > ballB->position.z ? ((ballB->position.z - ballB->radius) - (inverseBoxA->position.z + inverseBoxA->rightBox.z0)) : ((inverseBoxA->position.z + inverseBoxA->rightBox.z1) - (ballB->position.z + ballB->radius)),
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
		Vector3D distanceVector = Vector3D::get(inverseBoxA->position, ballB->position);

		Vector3D normals[__COLLIDER_NORMALS] =
		{
			{__I_TO_FIXED(1), 0, 0},
			{0, __I_TO_FIXED(1), 0},
			{0, 0, __I_TO_FIXED(1)},
		};

		int32 i = 0;
		fixed_t* component = &intervalDistance.x;

		for(i = 0; i < __COLLIDER_NORMALS; i++)
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

		collisionInformation->collider = Collider::safeCast(inverseBoxA);
		collisionInformation->otherCollider = Collider::safeCast(ballB);
		collisionInformation->solutionVector = solutionVector;

		return;
	}
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfInverseBoxOverlapsBox(Collider colliderA, Collider colliderB, CollisionInformation* collisionInformation)
{
	CollisionTester::testIfBoxOverlapsInverseBox(colliderB, colliderA, collisionInformation);
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfInverseBoxOverlapsInverseBox(Collider colliderA __attribute__ ((unused)), Collider colliderB __attribute__ ((unused)), CollisionInformation* collisionInformation __attribute__ ((unused)))
{
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfInverseBoxOverlapsLineField(Collider colliderA __attribute__ ((unused)), Collider colliderB __attribute__ ((unused)), CollisionInformation* collisionInformation __attribute__ ((unused)))
{
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfLineFieldOverlapsBall(Collider colliderA __attribute__((unused)), Collider colliderB __attribute__((unused)), CollisionInformation* collisionInformation __attribute__((unused)))
{
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfLineFieldOverlapsBox(Collider colliderA __attribute__((unused)), Collider colliderB __attribute__((unused)), CollisionInformation* collisionInformation __attribute__((unused)))
{
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::checkLineFieldIfOverlapsInverseBox(Collider colliderA __attribute__((unused)), Collider colliderB __attribute__((unused)), CollisionInformation* collisionInformation __attribute__((unused)))
{
}
//---------------------------------------------------------------------------------------------------------
static void CollisionTester::testIfLineFieldOverlapsLineField(Collider colliderA __attribute__((unused)), Collider colliderB __attribute__((unused)), CollisionInformation* collisionInformation __attribute__((unused)))
{
}
//---------------------------------------------------------------------------------------------------------
