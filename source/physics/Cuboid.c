/* VbJaEngine: bitmap graphics engine for the Nintendo Virtual Boy 
 * 
 * Copyright (C) 2007 Jorge Eremiev
 * jorgech3@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */
 
/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												INCLUDES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

#include <Cuboid.h>
#include <Polygon.h>
#include <Math.h>
#include <InGameEntity.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// define the Cuboid
__CLASS_DEFINITION(Cuboid);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


// class's constructor
static void Cuboid_constructor(Cuboid this, InGameEntity owner);

// check if overlaps with other rect
static int Cuboid_overlapsCuboid(Cuboid this, Cuboid other);

// determine axis of collision
static int Cuboid_getAxisOfCollisionWithCuboid(Cuboid this, Cuboid cuboid, VBVec3D displacement);

// test if collision with the entity give the displacement
static int Cuboid_testIfCollisionWithCuboid(Cuboid this, Cuboid cuboid, Gap gap, VBVec3D displacement);

// retrieve shape
Shape InGameEntity_getShape(InGameEntity this);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												CLASS'S METHODS
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// always call these to macros next to each other
__CLASS_NEW_DEFINITION(Cuboid, __PARAMETERS(InGameEntity owner))
__CLASS_NEW_END(Cuboid, __ARGUMENTS(owner));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Cuboid_constructor(Cuboid this, InGameEntity owner){

	__CONSTRUCT_BASE(Shape, __ARGUMENTS(owner));
	
	//Rightcuboid r;
	/*Rightcuboid rect = {0, 0, 0, 0};
	this->positionedRightcuboid = this->rightCuboid = rect;
	*/ 
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Cuboid_destructor(Cuboid this){

	// destroy the super object
	__DESTROY_BASE(Shape);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if two rects overlap
int Cuboid_overlaps(Cuboid this, Shape shape){

	if(__GET_CAST(Cuboid, shape)){
		
		return Cuboid_overlapsCuboid(this, (Cuboid)shape);
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if overlaps with other rect
int Cuboid_overlapsWithRightcuboids(Rightcuboid* first, Rightcuboid* second){
	
	// test for collision
	return !(first->x0 > second->x1 || first->x1 < second->x0 || 
			first->y0 > second->y1 || first->y1 < second->y0 ||
			first->z0 > second->z1 || first->z1 < second->z0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if overlaps with other rect
int Cuboid_overlapsCuboid(Cuboid this, Cuboid other){
	
	return Cuboid_overlapsWithRightcuboids(&this->positionedRightcuboid, &other->positionedRightcuboid);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Cuboid_setup(Cuboid this){

	// cuboid's center if placed on P(0, 0, 0)
	this->rightCuboid.x1 = ITOFIX19_13(__VIRTUAL_CALL(int, Entity, getWidth, (Entity)this->owner) >> 1);
	this->rightCuboid.y1 = ITOFIX19_13(__VIRTUAL_CALL(int, Entity, getHeight, (Entity)this->owner) >> 1);
	this->rightCuboid.z1 = ITOFIX19_13(__VIRTUAL_CALL(int, Entity, getDeep, (Entity)this->owner) >> 0);
	
	this->rightCuboid.x0 = -this->rightCuboid.x1;
	this->rightCuboid.y0 = -this->rightCuboid.y1;
	this->rightCuboid.z0 = 0;//-this->rightCuboid.z1;

	// if owner does not move
	if(!this->moves){
		
		ASSERT(Entity_getSprites((Entity)this->owner), "Cuboid::testIfCollision: null sprites");

		// position the shape to avoid in real time calculation
		VBVec3D ownerPosition = Entity_getPosition((Entity)this->owner);
		Gap ownerGap = InGameEntity_getGap(this->owner);
		
		// calculate gap on each side of the rightCuboid
		this->rightCuboid.x0 += ownerPosition.x + ITOFIX19_13(ownerGap.left);
		this->rightCuboid.x1 += ownerPosition.x - ITOFIX19_13(ownerGap.right);
		this->rightCuboid.y0 += ownerPosition.y + ITOFIX19_13(ownerGap.up);
		this->rightCuboid.y1 += ownerPosition.y - ITOFIX19_13(ownerGap.down);
		this->rightCuboid.z0 += ownerPosition.z;
		this->rightCuboid.z1 += ownerPosition.z;
	}
	
	this->positionedRightcuboid = this->rightCuboid;
	
	// no more setup needed
	this->ready = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prepare the shape to be checked
void Cuboid_positione(Cuboid this){
	
	ASSERT(Entity_getSprites((Entity)this->owner), "Cuboid::positione: null sprites");

	Gap gap = InGameEntity_getGap(this->owner);

	// get owner's position
	VBVec3D myOwnerPosition = __VIRTUAL_CALL_UNSAFE(VBVec3D, Entity, getPosition, (Entity)this->owner);

	// calculate positioned rightCuboid	
	this->positionedRightcuboid = this->rightCuboid;
	
	this->positionedRightcuboid.x0 += myOwnerPosition.x + ITOFIX19_13(gap.left);
	this->positionedRightcuboid.x1 += myOwnerPosition.x - ITOFIX19_13(gap.right);
	this->positionedRightcuboid.y0 += myOwnerPosition.y + ITOFIX19_13(gap.up);
	this->positionedRightcuboid.y1 += myOwnerPosition.y - ITOFIX19_13(gap.down);
	this->positionedRightcuboid.z0 += myOwnerPosition.z;
	this->positionedRightcuboid.z1 += myOwnerPosition.z;

	// not checked yet
	this->checked = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve rightCuboid
Rightcuboid Cuboid_getRightcuboid(Cuboid this){
	
	return this->rightCuboid;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve rightCuboid
Rightcuboid Cuboid_getPositionedRightcuboid(Cuboid this){
	
	return this->positionedRightcuboid;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// draw rect
void Cuboid_draw(Cuboid this){
	
	// create a polygon
	Polygon polygon = __NEW(Polygon);
	
	// add vertices
	Polygon_addVertice(polygon, this->positionedRightcuboid.x0, this->positionedRightcuboid.y0, this->positionedRightcuboid.z0);
	Polygon_addVertice(polygon, this->positionedRightcuboid.x1, this->positionedRightcuboid.y0, this->positionedRightcuboid.z0);
	Polygon_addVertice(polygon, this->positionedRightcuboid.x1, this->positionedRightcuboid.y1, this->positionedRightcuboid.z0);
	Polygon_addVertice(polygon, this->positionedRightcuboid.x0, this->positionedRightcuboid.y1, this->positionedRightcuboid.z0);

	Polygon_addVertice(polygon, this->positionedRightcuboid.x0, this->positionedRightcuboid.y0, this->positionedRightcuboid.z1);
	Polygon_addVertice(polygon, this->positionedRightcuboid.x1, this->positionedRightcuboid.y0, this->positionedRightcuboid.z1);
	Polygon_addVertice(polygon, this->positionedRightcuboid.x1, this->positionedRightcuboid.y1, this->positionedRightcuboid.z1);
	Polygon_addVertice(polygon, this->positionedRightcuboid.x0, this->positionedRightcuboid.y1, this->positionedRightcuboid.z1);

	// draw the polygon
	Polygon_draw(polygon, false);
	
	__DELETE(polygon);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// draw rect
/*
void Cuboid_print(Cuboid this, int x, int y){

	Rightcuboid rightCuboid = this->positionedRightcuboid;

	Printing_text("X:" , x, y);
	Printing_int(FIX19_13TOI(rightCuboid.x0), x + 2, y);
	Printing_text("-" , x + 5, y);
	Printing_int(FIX19_13TOI(rightCuboid.x1), x + 7, y++);

	Printing_text("Y:" , x, y);
	Printing_int(FIX19_13TOI(rightCuboid.y0), x + 2, y);
	Printing_text("-" , x + 5, y);
	Printing_int(FIX19_13TOI(rightCuboid.y1), x + 7, y++);

	Printing_text("Z:" , x, y);
	Printing_int(FIX19_13TOI(rightCuboid.z0), x + 2, y);
	Printing_text("-" , x + 5, y);
	Printing_int(FIX19_13TOI(rightCuboid.z1), x + 7, y++);
}
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// determine axis of collision
int Cuboid_getAxisOfCollision(Cuboid this, InGameEntity collidingEntity, VBVec3D displacement){
	
	Shape shape = InGameEntity_getShape(collidingEntity);
	
	if(__GET_CAST(Cuboid, shape)){
		
		return Cuboid_getAxisOfCollisionWithCuboid(this, (Cuboid)shape, displacement);
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// determine axis of collision
static int Cuboid_getAxisOfCollisionWithCuboid(Cuboid this, Cuboid cuboid, VBVec3D displacement){
	
	ASSERT(Entity_getSprites((Entity)this->owner), "Cuboid::getAxisOfCollisionWithCuboid: null sprites");

	Gap gap = InGameEntity_getGap(this->owner);

	VBVec3D displacementIncrement = {
			FIX19_13_MULT(displacement.x, FTOFIX19_13(0.1f)),
			FIX19_13_MULT(displacement.y, FTOFIX19_13(0.1f)),
			FIX19_13_MULT(displacement.z, FTOFIX19_13(0.1f))
	};
	
	// setup a cuboid representing the previous position
	Rightcuboid positionedRightCuboid = this->rightCuboid;
	
	// get colliding entity's rightcuboid
	Rightcuboid otherRightcuboid = cuboid->positionedRightcuboid;

	VBVec3D previousPosition = __VIRTUAL_CALL_UNSAFE(VBVec3D, InGameEntity, getPreviousPosition, (InGameEntity)this->owner);
	positionedRightCuboid.x0 += previousPosition.x + ITOFIX19_13(gap.left);
	positionedRightCuboid.x1 += previousPosition.x - ITOFIX19_13(gap.right);
	positionedRightCuboid.y0 += previousPosition.y + ITOFIX19_13(gap.up);
	positionedRightCuboid.y1 += previousPosition.y - ITOFIX19_13(gap.down);
	positionedRightCuboid.z0 += previousPosition.z - displacement.z;
	positionedRightCuboid.z1 += previousPosition.z - displacement.z;

	int numberOfAxis = 0;
	int axisOfCollision = 0;

#ifdef __DEBUG		
		int counter = 0;
#endif

	do{
#ifdef __DEBUG		
		ASSERT(counter++ < 100, "Cuboid::getAxisOfCollisionWithCuboid: cannot resolve collision");
#endif
		numberOfAxis = 0;
		axisOfCollision = 0;

		if(displacement.x) {

			positionedRightCuboid.x0 += displacement.x;
			positionedRightCuboid.x1 += displacement.x;
	
			if(Cuboid_overlapsWithRightcuboids(&positionedRightCuboid, &otherRightcuboid)){
				
				axisOfCollision |= __XAXIS;
				numberOfAxis++;
			}
			
			positionedRightCuboid.x0 -= displacement.x;
			positionedRightCuboid.x1 -= displacement.x;
		}

		if(displacement.y) {

			positionedRightCuboid.y0 += displacement.y;
			positionedRightCuboid.y1 += displacement.y;
		
			// test for collision
			if(Cuboid_overlapsWithRightcuboids(&positionedRightCuboid, &otherRightcuboid)){
				
				axisOfCollision |= __YAXIS;
				numberOfAxis++;
			}
			
			positionedRightCuboid.y0 -= displacement.y;
			positionedRightCuboid.y1 -= displacement.y;
		}
		
		if(displacement.z) {

			positionedRightCuboid.z0 += displacement.z;
			positionedRightCuboid.z1 += displacement.z;
	
			// test for collision
			if(Cuboid_overlapsWithRightcuboids(&positionedRightCuboid, &otherRightcuboid)){
				
				axisOfCollision |= __ZAXIS;
				numberOfAxis++;
			}
			
			positionedRightCuboid.z0 -= displacement.z;
			positionedRightCuboid.z1 -= displacement.z;
		}
		
		if(0 == numberOfAxis) {
			
			displacement.x += displacementIncrement.x;
			displacement.y += displacementIncrement.y;
			displacement.z += displacementIncrement.z;
			
			positionedRightCuboid = this->rightCuboid;
			
			positionedRightCuboid.x0 += previousPosition.x + ITOFIX19_13(gap.left);
			positionedRightCuboid.x1 += previousPosition.x - ITOFIX19_13(gap.right);
			positionedRightCuboid.y0 += previousPosition.y + ITOFIX19_13(gap.up);
			positionedRightCuboid.y1 += previousPosition.y - ITOFIX19_13(gap.down);
			positionedRightCuboid.z0 += previousPosition.z;
			positionedRightCuboid.z1 += previousPosition.z;		
		}
#ifdef __DEBUG		
	}while(0 == numberOfAxis && counter < 10);
#else
	}while(0 == numberOfAxis);
#endif
/*
#ifdef __DEBUG		
	if(((__XAXIS & axisOfCollision) && (__YAXIS & axisOfCollision)) || 
			((__XAXIS & axisOfCollision) && (__ZAXIS & axisOfCollision)) ||
			((__YAXIS & axisOfCollision) && (__ZAXIS & axisOfCollision))
	){

		Printing_text("Cuboid: more than one axis for collision", 1, 5);
		Printing_hex(axisOfCollision, 1, 6);
	}
#endif
*/
	return axisOfCollision;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// test if collision with the entity give the displacement
int Cuboid_testIfCollision(Cuboid this, InGameEntity collidingEntity, VBVec3D displacement){
	
	ASSERT(Entity_getSprites((Entity)this->owner), "Cuboid::testIfCollision: null sprites");

	Shape shape = InGameEntity_getShape(collidingEntity);
	
	if(__GET_CAST(Cuboid, shape)){
		
		return Cuboid_testIfCollisionWithCuboid(this, (Cuboid)shape, InGameEntity_getGap(collidingEntity), displacement);
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// test if collision with the entity give the displacement
static int Cuboid_testIfCollisionWithCuboid(Cuboid this, Cuboid cuboid, Gap gap, VBVec3D displacement){

	// setup a cuboid representing the previous position
	Rightcuboid positionedRightCuboid = this->positionedRightcuboid;

	// get colliding entity's rightcuboid
	Rightcuboid otherRightcuboid = cuboid->positionedRightcuboid;

	int axisOfPossibleCollision = 0;
	
	if(displacement.x){
		
		positionedRightCuboid.x0 += displacement.x;
		positionedRightCuboid.x1 += displacement.x;

		// test for collision
		if(Cuboid_overlapsWithRightcuboids(&positionedRightCuboid, &otherRightcuboid)){
			
			axisOfPossibleCollision |= __XAXIS;
		}
	}
	
	if(displacement.y){
		
		positionedRightCuboid.y0 += displacement.y;
		positionedRightCuboid.y1 += displacement.y;

		// test for collision
		if(Cuboid_overlapsWithRightcuboids(&positionedRightCuboid, &otherRightcuboid)){
			
			axisOfPossibleCollision |= __YAXIS;
		}
	}

	if(displacement.z){
		
		positionedRightCuboid.z0 += displacement.z;
		positionedRightCuboid.z1 += displacement.z;

		// test for collision
		if(Cuboid_overlapsWithRightcuboids(&positionedRightCuboid, &otherRightcuboid)){
			
			axisOfPossibleCollision |= __ZAXIS;
		}
	}
	
	return axisOfPossibleCollision;
}


// print debug data
void Cuboid_print(Cuboid this){

}
