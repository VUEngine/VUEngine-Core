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

class SpatialObject : Object
{
	void constructor(SpatialObject this);
	void destructor(SpatialObject this);
	virtual VirtualList getShapes(SpatialObject this);
	virtual bool isMoving(SpatialObject this);
	virtual bool isSubjectToGravity(SpatialObject this, Acceleration gravity);
	virtual fix10_6 getWidth(SpatialObject this);
	virtual fix10_6 getHeight(SpatialObject this);
	virtual fix10_6 getDepth(SpatialObject this);
	virtual const Vector3D* getPosition(SpatialObject this);
	virtual void setPosition(SpatialObject this, const Vector3D* position);
	virtual const Rotation* getRotation(SpatialObject this);
	virtual void setRotation(SpatialObject this, const Rotation* rotation);
	virtual const Scale* getScale(SpatialObject this);
	virtual void setScale(SpatialObject this, const Scale* scale);
	virtual fix10_6 getBounciness(SpatialObject this);
	virtual fix10_6 getFrictionCoefficient(SpatialObject this);
	virtual Velocity getVelocity(SpatialObject this);
	virtual bool isAffectedByRelativity(SpatialObject this);
	virtual bool enterCollision(SpatialObject this, const CollisionInformation* collisionInformation);
	virtual bool updateCollision(SpatialObject this, const CollisionInformation* collisionInformation);
	virtual void exitCollision(SpatialObject this, Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	virtual void collidingShapeOwnerDestroyed(SpatialObject this, Shape shape, Shape shapeNotCollidingAnymore, bool isShapeImpenetrable);
	virtual u16 getMovementState(SpatialObject this);
	virtual u32 getInGameType(SpatialObject this);
}


#endif
