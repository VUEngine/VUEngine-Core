/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <DebugConfig.h>
#include <Mesh.h>
#include <Printing.h>

#include "Box.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void Box::project(Vector3D vertexes[__BOX_VERTEXES], Vector3D vector, fixed_t* min, fixed_t* max)
{
	int32 vertexIndex = 0;

	// project this onto the current normal
	fixed_t dotProduct = Vector3D::dotProduct(vector, vertexes[vertexIndex]);

	fixed_t finalMin = dotProduct;
	fixed_t finalMax = dotProduct;

	// project this onto the current normal
	for(; vertexIndex < __BOX_VERTEXES; vertexIndex++)
	{
		dotProduct = Vector3D::dotProduct(vector, vertexes[vertexIndex]);

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

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Box::constructor(Entity owner, const ColliderSpec* colliderSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, colliderSpec);

	this->classIndex = kColliderBoxIndex;

	this->rotationVertexDisplacement = Vector3D::zero();

	this->normals = NULL;

	int32 normalIndex = 0;
	for(; normalIndex < __COLLIDER_NORMALS; normalIndex++)
	{
		this->vertexProjections[normalIndex].min = 0;
		this->vertexProjections[normalIndex].max = 0;
	}

	Box::computeRightBox(this);
	Box::projectOntoItself(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Box::destructor()
{
	if(this->normals)
	{
		delete this->normals;
		this->normals = NULL;
	}

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Box::resize(fixed_t sizeDelta __attribute__((unused)))
{
	// add displacement
	this->rightBox.x0 -= sizeDelta;
	this->rightBox.x1 += sizeDelta;

	this->rightBox.y0 -= sizeDelta;
	this->rightBox.y1 += sizeDelta;

	this->rightBox.z0 -= sizeDelta;
	this->rightBox.z1 += sizeDelta;

	Box::projectOntoItself(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Box::configureWireframe()
{
	if(!isDeleted(this->wireframe))
	{
		return;
	}

	const PixelVector MeshesSegments[][2]=
	{
		{
			PixelVector::getFromVector3D((Vector3D){this->rightBox.x0, this->rightBox.y0, 0}, Optics::calculateParallax(this->rightBox.z0)),
			PixelVector::getFromVector3D((Vector3D){this->rightBox.x1, this->rightBox.y0, 0}, Optics::calculateParallax(this->rightBox.z0)),
		},
		{
			PixelVector::getFromVector3D((Vector3D){this->rightBox.x0, this->rightBox.y1, 0}, Optics::calculateParallax(this->rightBox.z0)),
			PixelVector::getFromVector3D((Vector3D){this->rightBox.x1, this->rightBox.y1, 0}, Optics::calculateParallax(this->rightBox.z0)),
		},
		{
			PixelVector::getFromVector3D((Vector3D){this->rightBox.x0, this->rightBox.y0, 0}, Optics::calculateParallax(this->rightBox.z0)),
			PixelVector::getFromVector3D((Vector3D){this->rightBox.x0, this->rightBox.y1, 0}, Optics::calculateParallax(this->rightBox.z0)),
		},
		{
			PixelVector::getFromVector3D((Vector3D){this->rightBox.x1, this->rightBox.y0, 0}, Optics::calculateParallax(this->rightBox.z0)),
			PixelVector::getFromVector3D((Vector3D){this->rightBox.x1, this->rightBox.y1, 0}, Optics::calculateParallax(this->rightBox.z0)),
		},
		
		// limiter
		{
			{0, 0, 0, 0}, 
			{0, 0, 0, 0}
		},
	};

	MeshSpec meshSpec =
	{
		{
			// Component
			{
				// Allocator
				__TYPE(Mesh),

				// Component type
				kWireframeComponent
			},

			/// Displacement
			{0, 0, 0},

			/// color
			__COLOR_BRIGHT_RED,

			/// Transparency mode (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
			__TRANSPARENCY_NONE,
		
			/// Flag to render the wireframe in interlaced mode
			false
		},

		// segments
		(PixelVector(*)[2])MeshesSegments
	};

	// create a wireframe
	this->wireframe = Wireframe::safeCast(new Mesh(this->owner, &meshSpec));

	Mesh::setDisplacement(this->wireframe, Vector3D::getFromPixelVector(((ColliderSpec*)this->componentSpec)->displacement));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void Box::print(int32 x, int32 y)
{
	Base::print(this, x, y);
	
	RightBox rightBox = this->rightBox;

	Printing::text(Printing::getInstance(), "X:             " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(rightBox.x0 + this->transformation->position.x), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(rightBox.x1 + this->transformation->position.x), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Y:             " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(rightBox.y0 + this->transformation->position.y), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(rightBox.y1 + this->transformation->position.y), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Z:             " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(rightBox.z0 + this->transformation->position.z), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "-" , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(rightBox.z1 + this->transformation->position.z), x + 8, y++, NULL);
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Box::getVertexes(Vector3D vertexes[__BOX_VERTEXES])
{
	Vector3D leftTopNear 		= {this->rightBox.x0, this->rightBox.y0, this->rightBox.z0};
	Vector3D rightTopNear 		= {this->rightBox.x1, this->rightBox.y0, this->rightBox.z0};
	Vector3D leftBottomNear 	= {this->rightBox.x0, this->rightBox.y1, this->rightBox.z0};
	Vector3D rightBottomNear	= {this->rightBox.x1, this->rightBox.y1, this->rightBox.z0};
	Vector3D leftTopFar 		= {this->rightBox.x0, this->rightBox.y0, this->rightBox.z1};
	Vector3D rightTopFar 		= {this->rightBox.x1, this->rightBox.y0, this->rightBox.z1};
	Vector3D leftBottomFar 		= {this->rightBox.x0, this->rightBox.y1, this->rightBox.z1};
	Vector3D rightBottomFar 	= {this->rightBox.x1, this->rightBox.y1, this->rightBox.z1};

	if(0 != this->rotationVertexDisplacement.x || 0 != this->rotationVertexDisplacement.y || 0 != this->rotationVertexDisplacement.z)
	{
		if(0 == this->rotationVertexDisplacement.z)
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
		else if(0 == this->rotationVertexDisplacement.y)
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
		else if(0 == this->rotationVertexDisplacement.x)
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

	vertexes[0] = Vector3D::sum(leftTopNear, this->transformation->position);
	vertexes[1] = Vector3D::sum(rightTopNear, this->transformation->position);
	vertexes[2] = Vector3D::sum(leftBottomNear, this->transformation->position);
	vertexes[3] = Vector3D::sum(rightBottomNear, this->transformation->position);

	vertexes[4] = Vector3D::sum(leftTopFar, this->transformation->position);
	vertexes[5] = Vector3D::sum(rightTopFar, this->transformation->position);
	vertexes[6] = Vector3D::sum(leftBottomFar, this->transformation->position);
	vertexes[7] = Vector3D::sum(rightBottomFar, this->transformation->position);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Box::projectOntoItself()
{
	Vector3D vertexes[__BOX_VERTEXES];
	Box::getVertexes(this, vertexes);

	// Compute normals
	Box::computeNormals(this, vertexes);

	// Has to project all points on all the normals of the tilted box
	int32 normalIndex = 0;

	// Initialize vertex projections
	for(; normalIndex < __COLLIDER_NORMALS; normalIndex++)
	{
		Box::project
		(
			vertexes, this->normals->vectors[normalIndex], &this->vertexProjections[normalIndex].min, 
			&this->vertexProjections[normalIndex].max
		);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Box::computeRightBox()
{
	this->rotationVertexDisplacement.x = 0;
	this->rotationVertexDisplacement.y = 0;
	this->rotationVertexDisplacement.z = 0;

	Size surroundingBoxSize = Size::getFromPixelSize(((ColliderSpec*)this->componentSpec)->pixelSize);

	// angle | theta | psi
	if(0 != this->transformation->rotation.z || 0 != this->transformation->rotation.y || 0 != this->transformation->rotation.x)
	{
		fixed_t width = surroundingBoxSize.x >> 1;
		fixed_t height = surroundingBoxSize.y >> 1;
		fixed_t depth = surroundingBoxSize.z >> 1;

		// allow only one this->transformation->rotation
		if(0 != this->transformation->rotation.z && 256 != this->transformation->rotation.z)
		{
			// clamp value around 256 degrees (180) to avoid conditionals later when calculating rotationVertexDisplacement
			int16 angle = this->transformation->rotation.z - ((this->transformation->rotation.z / 256) << 8);
			angle = angle < 0 ? 256 + angle : angle;

			// calculate position of box's right-bottom corner
			fixed_t sinAngle = __FIX7_9_TO_FIXED(__SIN(angle));
			fixed_t cosAngle = __FIX7_9_TO_FIXED(__COS(angle));

			// use vectors (x1, y0, z1) and (x1, y1, z1)
			Vector3D topRight =
			{
				__FIXED_MULT(width, cosAngle) - __FIXED_MULT(-height, sinAngle),
				__FIXED_MULT(width, sinAngle) + __FIXED_MULT(-height, cosAngle),
				depth
			};

			Vector3D bottomRight =
			{
				__FIXED_MULT(width, cosAngle) - __FIXED_MULT(height, sinAngle),
				__FIXED_MULT(width, sinAngle) + __FIXED_MULT(height, cosAngle),
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

			surroundingBoxSize.x = (bottomRightHelper.x > topRightHelper.x ? bottomRightHelper.x : topRightHelper.x) << 1;
			surroundingBoxSize.y = (bottomRightHelper.y > topRightHelper.y ? bottomRightHelper.y : topRightHelper.y) << 1;

			// find the displacement over each axis for the rotated box
			this->rotationVertexDisplacement.x = bottomRightHelper.x < topRightHelper.x ? bottomRight.x : topRight.x;
			this->rotationVertexDisplacement.y = bottomRightHelper.y < topRightHelper.y ? bottomRight.y : topRight.y;
			this->rotationVertexDisplacement.y = angle >= 128 ? -this->rotationVertexDisplacement.y : this->rotationVertexDisplacement.y;

			this->rotationVertexDisplacement.x = (surroundingBoxSize.x >> 1) + this->rotationVertexDisplacement.x;
			this->rotationVertexDisplacement.y = (surroundingBoxSize.y >> 1) - this->rotationVertexDisplacement.y;

			if(!(__MODULO(angle, 128)))
			{
				this->rotationVertexDisplacement.x = 0;
				this->rotationVertexDisplacement.y = 0;
			}
		}
		else if(0 != this->transformation->rotation.y && 256 != this->transformation->rotation.y)
		{
			// clamp value around 256 degrees (180) to avoid conditionals later when calculating rotationVertexDisplacement
			int16 angle = this->transformation->rotation.y - ((this->transformation->rotation.y / 256) << 8);
			angle = angle < 0 ? 256 + angle : angle;

			// calculate position of box's right-bottom corner
			fixed_t sinAngle = __FIX7_9_TO_FIXED(__SIN(angle));
			fixed_t cosAngle = __FIX7_9_TO_FIXED(__COS(0));

			// use vectors (x0, y1, z0) and (x1, y1, z0)
			Vector3D bottomLeft =
			{
				__FIXED_MULT(-width, cosAngle) + __FIXED_MULT(-depth, sinAngle),
				height,
				-__FIXED_MULT(-width, sinAngle) + __FIXED_MULT(-depth, cosAngle),
			};

			Vector3D bottomRight =
			{
				__FIXED_MULT(width, cosAngle) + __FIXED_MULT(-depth, sinAngle),
				height,
				-__FIXED_MULT(width, sinAngle) + __FIXED_MULT(-depth, cosAngle),
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

			surroundingBoxSize.x = (bottomLeftHelper.x > bottomRightHelper.x ? bottomLeftHelper.x : bottomRightHelper.x) << 1;
			surroundingBoxSize.z = (bottomLeftHelper.z > bottomRightHelper.z ? bottomLeftHelper.z : bottomRightHelper.z) << 1;

			// find the displacement over each axis for the rotated box
			this->rotationVertexDisplacement.x = bottomLeftHelper.x < bottomRightHelper.x ? bottomLeft.x : bottomRight.x;
			this->rotationVertexDisplacement.x = angle >= 128 ? -this->rotationVertexDisplacement.x : this->rotationVertexDisplacement.x;
			this->rotationVertexDisplacement.z = bottomLeftHelper.z < bottomRightHelper.z ? bottomLeft.z : bottomRight.z;

			this->rotationVertexDisplacement.x = (surroundingBoxSize.x >> 1) - this->rotationVertexDisplacement.x;
			this->rotationVertexDisplacement.z = (surroundingBoxSize.z >> 1) + this->rotationVertexDisplacement.z;

			if(!(__MODULO(angle, 128)))
			{
				this->rotationVertexDisplacement.x = 0;
				this->rotationVertexDisplacement.z = 0;
			}
		}
		else if(0 != this->transformation->rotation.x && 256 != this->transformation->rotation.x)
		{
			// clamp value around 256 degrees (180) to avoid conditionals later when calculating rotationVertexDisplacement
			int16 angle = this->transformation->rotation.x - ((this->transformation->rotation.x / 256) << 8);
			angle = angle < 0 ? 256 + angle : angle;

			// calculate position of box's right-bottom corner
			fixed_t sinAngle = __FIX7_9_TO_FIXED(__SIN(angle));
			fixed_t cosAngle = __FIX7_9_TO_FIXED(__COS(angle));

			// use vectors (x1, y1, z0) and (x1, y1, z1)
			Vector3D bottomNear =
			{
				width,
				__FIXED_MULT(height, cosAngle) - __FIXED_MULT(-depth, sinAngle),
				__FIXED_MULT(height, sinAngle) + __FIXED_MULT(-depth, cosAngle),
			};

			Vector3D bottomFar =
			{
				width,
				__FIXED_MULT(height, cosAngle) - __FIXED_MULT(depth, sinAngle),
				__FIXED_MULT(height, sinAngle) + __FIXED_MULT(depth, cosAngle),
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

			surroundingBoxSize.y = (bottomFarHelper.y > bottomNearHelper.y ? bottomFarHelper.y : bottomNearHelper.y) << 1;
			surroundingBoxSize.z = (bottomFarHelper.z > bottomNearHelper.z ? bottomFarHelper.z : bottomNearHelper.z) << 1;

			// find the displacement over each axis for the rotated box
			this->rotationVertexDisplacement.y = bottomFarHelper.y < bottomNearHelper.y ? bottomFar.y : bottomNear.y;
			this->rotationVertexDisplacement.z = bottomFarHelper.z < bottomNearHelper.z ? bottomFar.z : bottomNear.z;
			this->rotationVertexDisplacement.z = angle >= 128 ? -this->rotationVertexDisplacement.z : this->rotationVertexDisplacement.z;

			this->rotationVertexDisplacement.y = (surroundingBoxSize.y >> 1) - this->rotationVertexDisplacement.y;
			this->rotationVertexDisplacement.z = (surroundingBoxSize.z >> 1) + this->rotationVertexDisplacement.z;

			if(!(__MODULO(angle, 128)))
			{
				this->rotationVertexDisplacement.y = 0;
				this->rotationVertexDisplacement.z = 0;
			}
		}
	}

	Vector3D displacement = Vector3D::getFromPixelVector(((ColliderSpec*)this->componentSpec)->displacement);

	// box's center if placed on P(0, 0, 0)
	this->rightBox.x1 = surroundingBoxSize.x >> 1;
	this->rightBox.y1 = surroundingBoxSize.y >> 1;
	this->rightBox.z1 = surroundingBoxSize.z >> 1;

	this->rightBox.x0 = -this->rightBox.x1;
	this->rightBox.y0 = -this->rightBox.y1;
	this->rightBox.z0 = -this->rightBox.z1;

	this->rightBox.x0 += displacement.x;
	this->rightBox.y0 += displacement.y;
	this->rightBox.z0 += displacement.z;

	this->rightBox.x1 += displacement.x;
	this->rightBox.y1 += displacement.y;
	this->rightBox.z1 += displacement.z;
	
	if(NULL != this->normals)
	{
		Box::projectOntoItself(this);
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void Box::computeNormals(Vector3D vertexes[__BOX_VERTEXES])
{
/*
	// generic way
	normals[0] = Vector3D::getPlaneNormal(vertexes[6], vertexes[4], vertexes[0]);
	normals[1] = Vector3D::getPlaneNormal(vertexes[0], vertexes[4], vertexes[5]);
	normals[2] = Vector3D::getPlaneNormal(vertexes[0], vertexes[1], vertexes[3]);
*/

	if(NULL == this->normals)
	{
		this->normals = new Normals;
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

	this->normals->vectors[0] = Vector3D::normalize(this->normals->vectors[0]);
	this->normals->vectors[1] = Vector3D::normalize(this->normals->vectors[1]);
	this->normals->vectors[2] = Vector3D::normalize(this->normals->vectors[2]);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
