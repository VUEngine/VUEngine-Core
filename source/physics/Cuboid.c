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
static void Cuboid_constructor(Cuboid this, InGameEntity owner, int deep);

// check if overlaps with other rect
static int Cuboid_overlapsCuboid(Cuboid this, Cuboid other);


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
__CLASS_NEW_DEFINITION(Cuboid, __PARAMETERS(InGameEntity owner, int deep))
__CLASS_NEW_END(Cuboid, __ARGUMENTS(owner, deep));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Cuboid_constructor(Cuboid this, InGameEntity owner, int deep){

	__CONSTRUCT_BASE(Shape, __ARGUMENTS(owner, deep));
	
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

	return Cuboid_overlapsCuboid(this, (Cuboid)shape);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if overlaps with other rect
int Cuboid_overlapsCuboid(Cuboid this, Cuboid other){
	
	// must positione the rects in the owner's positions
	Rightcuboid otherRightcuboid = other->positionedRightcuboid;
	Rightcuboid myRightcuboid = this->positionedRightcuboid;
	
	// test for collision
	return !(myRightcuboid.x0 > otherRightcuboid.x1 || myRightcuboid.x1 < otherRightcuboid.x0 || 
			myRightcuboid.y0 > otherRightcuboid.y1 || myRightcuboid.y1 < otherRightcuboid.y0 ||
			myRightcuboid.z0 > otherRightcuboid.z1 || myRightcuboid.z1 < otherRightcuboid.z0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Cuboid_setup(Cuboid this){

	// cuboid's center if placed on P(0, 0, 0)
	this->rightCuboid.x1 = ITOFIX19_13(__VIRTUAL_CALL(int, Entity, getWidth, (Entity)this->owner) >> 1);
	this->rightCuboid.y1 = ITOFIX19_13(__VIRTUAL_CALL(int, Entity, getHeight, (Entity)this->owner) >> 1);
	this->rightCuboid.z1 = ITOFIX19_13(__VIRTUAL_CALL(int, Entity, getDeep, (Entity)this->owner) >> 1);
	
	this->rightCuboid.x0 = -this->rightCuboid.x1;
	this->rightCuboid.y0 = -this->rightCuboid.y1;
	this->rightCuboid.z0 = -this->rightCuboid.z1;

	// if owner does not move
	if(!this->moves){
		
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
	
	fix19_13 z = Container_getGlobalPosition((Container)this->owner).z;
	
	// add vertices
	Polygon_addVertice(polygon, this->positionedRightcuboid.x0, this->positionedRightcuboid.y0, z);
	Polygon_addVertice(polygon, this->positionedRightcuboid.x1, this->positionedRightcuboid.y0, z);
	Polygon_addVertice(polygon, this->positionedRightcuboid.x1, this->positionedRightcuboid.y1, z);
	Polygon_addVertice(polygon, this->positionedRightcuboid.x0, this->positionedRightcuboid.y1, z);
	
	// draw the polygon
	Polygon_draw(polygon, false);
	
	__DELETE(polygon);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// draw rect
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