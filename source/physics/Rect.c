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

#include <Rect.h>
#include <Polygon.h>


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 											CLASS'S DEFINITION
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */

// define the Rect
__CLASS_DEFINITION(Rect);


/* ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * 												PROTOTYPES
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 * ---------------------------------------------------------------------------------------------------------
 */


// class's constructor
static void Rect_constructor(Rect this, InGameEntity owner, int deep);

// check if overlaps with other rect
static int Rect_overlapsRect(Rect this, Rect other, int axisMovement);


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
__CLASS_NEW_DEFINITION(Rect, __PARAMETERS(InGameEntity owner, int deep))
__CLASS_NEW_END(Rect, __ARGUMENTS(owner, deep));

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's constructor
static void Rect_constructor(Rect this, InGameEntity owner, int deep){

	__CONSTRUCT_BASE(Shape, __ARGUMENTS(owner, deep));
	
	//Rectangle r;
	/*Rectangle rect = {0, 0, 0, 0};
	this->positionedRectangle = this->rectangle = rect;
	*/ 
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class's destructor
void Rect_destructor(Rect this){

	// destroy the super object
	__DESTROY_BASE(Shape);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if two rects overlap
int Rect_overlaps(Rect this, Shape shape, int axisMovement){

	return Rect_overlapsRect(this, (Rect)shape, axisMovement);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// check if overlaps with other rect
int Rect_overlapsRect(Rect this, Rect other, int axisMovement){
	
	// must positione the rects in the owner's positions
	Rectangle otherRectangle = other->positionedRectangle;
	Rectangle myRectangle = this->positionedRectangle;
	
	// get owner's positions
	VBVec3D myOwnerPosition = Entity_getPosition((Entity)this->owner);
	VBVec3D otherOwnerPosition = Entity_getPosition((Entity)other->owner);

	// compare z positions
	if((myOwnerPosition.z  > (otherOwnerPosition.z + ITOFIX19_13(other->deep)) 
			|| 
		(myOwnerPosition.z + ITOFIX19_13(this->deep) < otherOwnerPosition.z))){
		
		return kNoCollision;
	}

	// if owner is moving
	if(axisMovement){

		// test for a collision
		if(!(myRectangle.x0 > otherRectangle.x1 || myRectangle.x1 < otherRectangle.x0 ||
				myRectangle.y0 > otherRectangle.y1 || myRectangle.y1 < otherRectangle.y0)){
			
			// this is to avoid the object being aligned in the Y axis
			// if already hit the side of the other shape
			fix19_13 halfWidth = (myRectangle.x1 - myRectangle.x0) >> 2;
			
			// now guess over which axis the collision is
			// giving preference to collisions over the x axis
			if((__XAXIS & axisMovement) &&
					(myOwnerPosition.x + halfWidth <= otherRectangle.x0 
							|| 
						myOwnerPosition.x - halfWidth >= otherRectangle.x1)){

				return kCollisionX;
			}
			
			if(__ZAXIS & axisMovement){
				
				return kCollisionZ;
			}
			
			if((__YAXIS & axisMovement) &&
			    (myOwnerPosition.y < otherRectangle.y0 
			    || 
			    myOwnerPosition.y > otherRectangle.y1)){
				
				return kCollisionY;
			}
					
			return kCollisionXY;
		}
	}
	
	// if not moving or a collision was not detected
	// check if the shape is below
	if(!(myRectangle.x0 > otherRectangle.x1 || myRectangle.x1 < otherRectangle.x0)){

		myRectangle.y1 += ITOFIX19_13(1);
		
		if(myRectangle.y1 >= otherRectangle.y0 && myRectangle.y0 < otherRectangle.y0){
			
			return kShapeBelow;	
		}
	}
	
	return kNoCollision;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Rect_setup(Rect this){

	// rect's center if placed on P(0, 0)
	this->rectangle.x1 = ITOFIX19_13(Entity_getWidth((Entity)this->owner) >> 1);
	this->rectangle.y1 = ITOFIX19_13(Entity_getHeight((Entity)this->owner) >> 1);
	
	this->rectangle.x0 = -this->rectangle.x1;
	this->rectangle.y0 = -this->rectangle.y1;
	
	// if owner does not move
	if(!this->moves){
		
		// position the shape to avoid in real time calculation
		VBVec3D ownerPosition = Entity_getPosition((Entity)this->owner);
		Gap ownerGap = InGameEntity_getGap(this->owner);
		
		// calculate gap on each side of the rectangle
		this->rectangle.x0 += ownerPosition.x + ITOFIX19_13(ownerGap.left);
		this->rectangle.x1 += ownerPosition.x - ITOFIX19_13(ownerGap.right);
		this->rectangle.y0 += ownerPosition.y + ITOFIX19_13(ownerGap.up);
		this->rectangle.y1 += ownerPosition.y - ITOFIX19_13(ownerGap.down);
	}
	
	this->positionedRectangle = this->rectangle;
	
	// no more setup needed
	this->ready = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prepare the shape to be checked
void Rect_positione(Rect this){
	
	Gap gap = InGameEntity_getGap(this->owner);

	// get owner's position
	VBVec3D myOwnerPosition = Entity_getPosition((Entity)this->owner);

	// calculate positioned rectangle	
	this->positionedRectangle = this->rectangle;
	
	this->positionedRectangle.x0 += myOwnerPosition.x + ITOFIX19_13(gap.left);
	this->positionedRectangle.x1 += myOwnerPosition.x - ITOFIX19_13(gap.right);
	this->positionedRectangle.y0 += myOwnerPosition.y + ITOFIX19_13(gap.up);
	this->positionedRectangle.y1 += myOwnerPosition.y - ITOFIX19_13(gap.down);

	// not checked yet
	this->checked = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve rectangle
Rectangle Rect_getPositionedRectangle(Rect this){
	
	return this->positionedRectangle;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// draw rect
void Rect_draw(Rect this){
	
	// create a polygon
	Polygon polygon = __NEW(Polygon);
	
	fix19_13 z = Container_getGlobalPosition((Container)this->owner).z;
	
	// add vertices
	Polygon_addVertice(polygon, this->positionedRectangle.x0, this->positionedRectangle.y0, z);
	Polygon_addVertice(polygon, this->positionedRectangle.x1, this->positionedRectangle.y0, z);
	Polygon_addVertice(polygon, this->positionedRectangle.x1, this->positionedRectangle.y1, z);
	Polygon_addVertice(polygon, this->positionedRectangle.x0, this->positionedRectangle.y1, z);
	
	// draw the polygon
	Polygon_draw(polygon, false);
	
	__DELETE(polygon);
}
