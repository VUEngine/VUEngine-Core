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

#include <ComponentManager.h>
#include <DebugConfig.h>
#include <Line.h>
#include <Printer.h>
#include <Entity.h>

#include "LineField.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC STATIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void LineField::project(Vector3D center, fixed_t radius, Vector3D vector, fixed_t* min, fixed_t* max)
{
	// Project this onto the current normal
	fixed_t dotProduct = Vector3D::dotProduct(vector, center);

	*min = dotProduct - radius;
	*max = dotProduct + radius;

	if(*min > *max)
	{
		fixed_t aux = *min;
		*min = *max;
		*max = aux;
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PUBLIC METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void LineField::constructor(Entity owner, const ColliderSpec* colliderSpec)
{
	// Always explicitly call the base's constructor 
	Base::constructor(owner, colliderSpec);

	this->classIndex = kColliderLineFieldIndex;

	this->meshSpec = NULL;
	this->a = Vector3D::zero();
	this->b = Vector3D::zero();
	this->normal = Vector3D::zero();
	this->normalLength = __I_TO_FIX7_9(1);

	LineField::computeSize(this);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void LineField::destructor()
{
	if(NULL != this->meshSpec)
	{
		delete this->meshSpec;
	}

	this->meshSpec = NULL;

	// Always explicitly call the base's destructor 
	Base::destructor();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Vector3D LineField::getNormal()
{
	return this->normal;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void LineField::configureWireframe()
{
	if(!isDeleted(this->wireframe))
	{
		return;
	}

	this->meshSpec = new MeshSpec;

	PixelVector MeshesSegments[][2]=
	{
		// Line
		{
			PixelVector::getFromVector3D(this->a, 0),
			PixelVector::getFromVector3D(this->b, 0),
		},

		// Normal
		{
			PixelVector::getFromVector3D(Vector3D::intermediate(this->a, this->b), 0),
			PixelVector::getFromVector3D
			(
				Vector3D::sum(Vector3D::intermediate(this->a, this->b), Vector3D::scalarProduct(this->normal, this->normalLength)), 0
			),
		},

		// Limiter
		{
			{0, 0, 0, 0}, 
			{0, 0, 0, 0}
		},
	};

	*this->meshSpec = (MeshSpec)
	{
		{
			// VisualComponent
			{
				// Component
				{
					// Allocator
					__TYPE(Mesh),

					// Component type
					kWireframeComponent
				},

				// Array of function animations
				(const AnimationFunction**)NULL
			},

			// Displacement
			{0, 0, 0},

			/// color
			__COLOR_BRIGHT_RED,

			/// Transparency mode (__TRANSPARENCY_NONE, __TRANSPARENCY_EVEN or __TRANSPARENCY_ODD)
			__TRANSPARENCY_NONE,
		
			/// Flag to render the wireframe in interlaced mode
			true
		},

		/// segments
		(PixelVector(*)[2])MeshesSegments
	};

	// Create a wireframe
	this->wireframe = Wireframe::safeCast(ComponentManager::createComponent(this->owner, (const ComponentSpec*)this->meshSpec));

	if(!isDeleted(this->wireframe))
	{
		Line::setDisplacement(this->wireframe, Vector3D::getFromPixelVector(((ColliderSpec*)this->componentSpec)->displacement));
	}
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#ifndef __SHIPPING
void LineField::print(int32 x, int32 y)
{
	Base::print(this, x, y);
	
	Printer::text("L:             " , x, y, NULL);
	Printer::int32(__METERS_TO_PIXELS(Vector3D::length(Vector3D::get(this->a, this->b))), x + 2, y++, NULL);
	Printer::text("C:         " , x, y, NULL);
	Printer::int32(__METERS_TO_PIXELS(Vector3D::intermediate(this->a, this->b).x), x + 2, y, NULL);
	Printer::int32(__METERS_TO_PIXELS(Vector3D::intermediate(this->a, this->b).y), x + 8, y, NULL);
	Printer::int32(__METERS_TO_PIXELS(Vector3D::intermediate(this->a, this->b).z), x + 14, y++, NULL);

	Printer::text("X:              " , x, y, NULL);
	Printer::int32(__METERS_TO_PIXELS(this->a.x), x + 2, y, NULL);
	Printer::text("," , x + 6, y, NULL);
	Printer::int32(__METERS_TO_PIXELS(this->b.x), x + 8, y++, NULL);

	Printer::text("Y:               " , x, y, NULL);
	Printer::int32(__METERS_TO_PIXELS(this->a.y), x + 2, y, NULL);
	Printer::text("," , x + 6, y, NULL);
	Printer::int32(__METERS_TO_PIXELS(this->b.y), x + 8, y++, NULL);

	Printer::text("Z:               " , x, y, NULL);
	Printer::int32(__METERS_TO_PIXELS(this->a.z), x + 2, y, NULL);
	Printer::text("," , x + 6, y, NULL);
	Printer::int32(__METERS_TO_PIXELS(this->a.z), x + 8, y++, NULL);
}
#else
void LineField::print(int32 x __attribute__((unused)), int32 y __attribute__((unused)))
{}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void LineField::displace(fixed_t displacement)
{
	this->a = Vector3D::sum(this->a, Vector3D::scalarProduct(this->normal, displacement));
	this->b = Vector3D::sum(this->b, Vector3D::scalarProduct(this->normal, displacement));
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Vector3D LineField::getCenter()
{
	return 
		Vector3D::sum
		(
			Vector3D::sum
			(
				this->transformation->position, 
				Vector3D::intermediate(this->a, this->b)), Vector3D::getFromPixelVector(((ColliderSpec*)this->componentSpec)->displacement
			)
		);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void LineField::getVertexes(Vector3D vertexes[__LINE_FIELD_VERTEXES])
{
	vertexes[0] = 
		Vector3D::sum
		(
			this->a, 
			Vector3D::sum(this->transformation->position, Vector3D::getFromPixelVector(((ColliderSpec*)this->componentSpec)->displacement))
		);
	
	vertexes[1] = 
		Vector3D::sum
		(
			this->b, 
			Vector3D::sum(this->transformation->position, Vector3D::getFromPixelVector(((ColliderSpec*)this->componentSpec)->displacement))
		);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void LineField::setNormalLength(fixed_t normalLength)
{
	this->normalLength = normalLength;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' PRIVATE METHODS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void LineField::computeSize()
{
	if(NULL == this->transformation)
	{
		return;
	}

	fix7_9 normalScale = __I_TO_FIX7_9(1);

	if(this->transformation->scale.x > normalScale)
	{
		normalScale = this->transformation->scale.x;
	}

	if(this->transformation->scale.y > normalScale)
	{
		normalScale = this->transformation->scale.y;
	}

	if(this->transformation->scale.z > normalScale)
	{
		normalScale = this->transformation->scale.z;
	}

	this->normalLength = __FIXED_MULT(this->normalLength, __FIX7_9_TO_FIXED(normalScale));	

	Rotation rotation = 
		Rotation::sum
		(
			Rotation::getFromPixelRotation(((ColliderSpec*)this->componentSpec)->pixelRotation), this->transformation->rotation
		);	
	
	Size size = Size::getFromPixelSize(((ColliderSpec*)this->componentSpec)->pixelSize);

	if(0 != size.x)
	{
		this->a.x = size.x >> 1;
		this->a.y = 0;
		this->a.z = 0;

		if(0 != rotation.y)
		{
			this->a.x = __FIXED_MULT((size.x >> 1), __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation.y))));
			this->a.y = 0;
			this->a.z = __FIXED_MULT((size.x >> 1), __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(rotation.y))));
		}
		else if(0 != rotation.z)
		{
			this->a.x = __FIXED_MULT((size.x >> 1), __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation.z))));
			this->a.y = __FIXED_MULT((size.x >> 1), __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(rotation.z))));
			this->a.z = 0;
		}
	}
	else if(0 != size.y)
	{
		this->a.x = 0;
		this->a.y = size.y >> 1;
		this->a.z = 0;

		if(0 != rotation.x)
		{
			this->a.x = 0;
			this->a.y = __FIXED_MULT((size.y >> 1), __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation.x))));
			this->a.z = __FIXED_MULT((size.y >> 1), __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(rotation.x))));
		}
		else if(0 != rotation.z)
		{
			this->a.x = __FIXED_MULT((size.y >> 1), __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(rotation.z))));
			this->a.y = __FIXED_MULT((size.y >> 1), __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation.z))));
			this->a.z = 0;
		}
	}
	else if(0 != size.z)
	{
		this->a.x = 0;
		this->a.y = 0;
		this->a.z = size.z >> 1;

		if(0 != rotation.x)
		{
			this->a.x = 0;
			this->a.y = __FIXED_MULT((size.z >> 1), __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(rotation.x))));
			this->a.z = __FIXED_MULT((size.z >> 1), __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation.x))));
		}
		else if(0 != rotation.y)
		{
			this->a.x = __FIXED_MULT((size.z >> 1), __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(rotation.y))));
			this->a.y = 0;
			this->a.z = __FIXED_MULT((size.z >> 1), __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation.y))));
		}
	}

	this->b = Vector3D::scalarProduct(this->a, __I_TO_FIXED(-1));

	fixed_t dx = this->b.x - this->a.x;
	fixed_t dy = this->b.y - this->a.y;
	fixed_t dz = this->b.z - this->a.z;

	this->normal = Vector3D::normalize((Vector3D){dy, -dx, dz});
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
