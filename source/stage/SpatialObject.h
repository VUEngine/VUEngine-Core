/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2018 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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

#ifndef SPATIAL_OBJECT_H_
#define SPATIAL_OBJECT_H_


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Object.h>
#include <VirtualList.h>
#include <Shape.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DECLARATION
//---------------------------------------------------------------------------------------------------------

/// @ingroup stage
class SpatialObject : Object
{
	/// @publicsection
	void constructor();
	void destructor();
	virtual VirtualList getShapes();
	virtual bool isMoving();
	virtual bool isSubjectToGravity(Acceleration gravity);
	virtual fix10_6 getRadius();
	virtual fix10_6 getWidth();
	virtual fix10_6 getHeight();
	virtual fix10_6 getDepth();
	virtual const Vector3D* getPosition();
	virtual void setPosition(const Vector3D* position);
	virtual const Rotation* getRotation();
	virtual void setRotation(const Rotation* rotation);
	virtual const Scale* getScale();
	virtual void setScale(const Scale* scale);
	virtual fix10_6 getBounciness();
	virtual fix10_6 getFrictionCoefficient();
	virtual Velocity getVelocity();
	virtual fix10_6 getSpeed();
	virtual fix10_6 getMaximumSpeed();
	virtual bool isAffectedByRelativity();
	virtual bool enterCollision(const CollisionInformation* collisionInformation);
	virtual bool updateCollision(const CollisionInformation* collisionInformation);
	virtual void exitCollision(Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	virtual void collidingShapeOwnerDestroyed(Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	virtual u16 getMovementState();
	virtual u32 getInGameType();
}


#endif
