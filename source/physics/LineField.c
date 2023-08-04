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

#include <LineField.h>
#include <Line.h>
#include <CollisionHelper.h>
#include <Optics.h>
#include <VirtualList.h>
#include <Sprite.h>
#include <SpatialObject.h>
#include <Printing.h>
#include <debugConfig.h>


//---------------------------------------------------------------------------------------------------------
//												CLASS'S MACROS
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void LineField::constructor(SpatialObject owner, const ShapeSpec* shapeSpec)
{
	Base::constructor(owner, shapeSpec);

	this->lineSpec = NULL;
	this->a = Vector3D::zero();
	this->b = Vector3D::zero();
	this->normal = Vector3D::zero();
	this->normalLength = __I_TO_FIX7_9(1);
}

// class's destructor
void LineField::destructor()
{
	if(NULL != this->lineSpec)
	{
		delete this->lineSpec;
	}

	this->lineSpec = NULL;

	// destroy the super object
	// must always be called at the end of the destructor
	Base::destructor();
}

void LineField::transform(const Vector3D* position, const Rotation* rotation, const Scale* scale, const Size* size)
{
	if(NULL == position || NULL == rotation || NULL == size || NULL == scale)
	{
		return;
	}

	Base::transform(this, position, rotation, scale, size);

	fix7_9 normalScale = __I_TO_FIX7_9(1);
	if(scale->x > normalScale)
	{
		normalScale = scale->x;
	}

	if(scale->y > normalScale)
	{
		normalScale = scale->y;
	}

	if(scale->z > normalScale)
	{
		normalScale = scale->z;
	}
	

	this->normalLength = __FIXED_MULT(__PIXELS_TO_METERS(8), __FIX7_9_TO_FIXED(normalScale));	
	
	if(0 != size->x)
	{
		this->a.x = size->x >> 1;
		this->a.y = 0;
		this->a.y = 0;

		if(rotation->y)
		{
			this->a.x = __FIXED_MULT((size->x >> 1), __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation->y))));
			this->a.z = __FIXED_MULT((size->x >> 1), __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(rotation->y))));
			this->a.y = 0;
		}
		else if(rotation->z)
		{
			this->a.x = __FIXED_MULT((size->x >> 1), __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation->z))));
			this->a.y = __FIXED_MULT((size->x >> 1), __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(rotation->z))));
			this->a.z = 0;
		}
	}
	else if(0 != size->y)
	{
		this->a.x = 0;
		this->a.y = size->y >> 1;
		this->a.y = 0;

		if(rotation->x)
		{
			this->a.x = __FIXED_MULT((size->y >> 1), __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation->x))));
			this->a.z = __FIXED_MULT((size->y >> 1), __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(rotation->x))));
			this->a.y = 0;
		}
		else if(rotation->z)
		{
			this->a.x = __FIXED_MULT((size->y >> 1), __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation->z))));
			this->a.y = __FIXED_MULT((size->y >> 1), __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(rotation->z))));
			this->a.z = 0;
		}
	}
	else if(0 != size->z)
	{
		this->a.x = 0;
		this->a.y = 0;
		this->a.z = size->z >> 1;

		if(rotation->x)
		{
			this->a.x = __FIXED_MULT((size->z >> 1), __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation->y))));
			this->a.y = __FIXED_MULT((size->z >> 1), __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(rotation->y))));
			this->a.z = 0;
		}
		else if(rotation->y)
		{
			this->a.x = __FIXED_MULT((size->z >> 1), __FIX7_9_TO_FIXED(__COS(__FIXED_TO_I(rotation->x))));
			this->a.y = 0;
			this->a.z = __FIXED_MULT((size->z >> 1), __FIX7_9_TO_FIXED(__SIN(__FIXED_TO_I(rotation->x))));
		}
	}

	this->b = Vector3D::scalarProduct(this->a, __I_TO_FIXED(-1));

	this->a = Vector3D::sum(this->a, *position);
	this->b = Vector3D::sum(this->b, *position);

	fixed_t dx = this->b.x - this->a.x;
	fixed_t dy = this->b.y - this->a.y;
	fixed_t dz = this->b.z - this->a.z;

	this->normal = Vector3D::normalize((Vector3D){dy, -dx, dz});
}

void LineField::addDisplacement(fixed_t displacement)
{
	this->a = Vector3D::sum(this->a, Vector3D::scalarProduct(this->normal, displacement));
	this->b = Vector3D::sum(this->b, Vector3D::scalarProduct(this->normal, displacement));
}

static void LineField::project(Vector3D center, fixed_t radius, Vector3D vector, fixed_t* min, fixed_t* max)
{
	// project this onto the current normal
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

CollisionInformation LineField::testForCollision(Shape shape __attribute__((unused)), Vector3D displacement __attribute__((unused)), fixed_t sizeIncrement __attribute__((unused)))
{
	// TODO
	CollisionInformation collisionInformation = CollisionHelper::checkIfOverlap(Shape::safeCast(this), shape);

	return collisionInformation;
}

// configure Polyhedron
void LineField::configureWireframe()
{
	if(!isDeleted(this->wireframe))
	{
		return;
	}

	this->lineSpec = new LineSpec;
	*this->lineSpec = (LineSpec)
	{
		{
			__TYPE(Line),

			/// displacement
			{0, 0, 0},

			/// color
			__COLOR_BRIGHT_RED,

			/// transparent
			__TRANSPARENCY_NONE,
		
			/// interlaced
			false
		},

//		Vector3D::intermediate(this->a, this->b),
//		Vector3D::sum(Vector3D::intermediate(this->a, this->b), this->normal)

		this->a,
		this->b,
	};

	// create a wireframe
	this->wireframe = Wireframe::safeCast(new Line(this->lineSpec));

	Wireframe::setup(this->wireframe, SpatialObject::getPosition(this->owner), NULL, NULL, false);
}

void LineField::getVertexes(Vector3D vertexes[__LINE_FIELD_VERTEXES])
{
	vertexes[0] = this->a;
	vertexes[1] = this->b;
}

Vector3D LineField::getNormal()
{
	return this->normal;
}

// print debug data
void LineField::print(int32 x, int32 y)
{
	Printing::text(Printing::getInstance(), "L:             " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(Vector3D::length(Vector3D::get(this->a, this->b))), x + 2, y++, NULL);
	Printing::text(Printing::getInstance(), "C:         " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(Vector3D::intermediate(this->a, this->b).x), x + 2, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(Vector3D::intermediate(this->a, this->b).y), x + 8, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(Vector3D::intermediate(this->a, this->b).z), x + 14, y++, NULL);

	Printing::text(Printing::getInstance(), "X:              " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->a.x), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "," , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->b.x), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Y:               " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->a.y), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "," , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->b.y), x + 8, y++, NULL);

	Printing::text(Printing::getInstance(), "Z:               " , x, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->a.z), x + 2, y, NULL);
	Printing::text(Printing::getInstance(), "," , x + 6, y, NULL);
	Printing::int32(Printing::getInstance(), __METERS_TO_PIXELS(this->a.z), x + 8, y++, NULL);
}
