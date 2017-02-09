/* VUEngine - Virtual Utopia Engine <http://vuengine.planetvb.com/>
 * A universal game engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007, 2017 by Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <chris@vr32.de>
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


//---------------------------------------------------------------------------------------------------------
//												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Shape.h>
#include <Game.h>
#include <CollisionManager.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	Shape
 * @extends Object
 */
__CLASS_DEFINITION(Shape, Object);


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Shape_constructor(Shape this, SpatialObject owner)
{
	ASSERT(this, "Shape::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE(Object);

	// set the owner
	this->owner = owner;

	// do I move?
	this->moves = __VIRTUAL_CALL(SpatialObject, moves, owner);

	// not checked yet
	this->checked = false;

	// not setup yet
	this->ready = false;

	// set flag
	this->checkForCollisions = false;

	Shape_setActive(this, false);
}

// class's destructor
void Shape_destructor(Shape this)
{
	ASSERT(this, "Shape::destructor: null this");

	// destroy the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

// retrieve owner
SpatialObject Shape_getOwner(Shape this)
{
	ASSERT(this, "Shape::getOwner: null this");

	return this->owner;
}

// set active
void Shape_setActive(Shape this, bool active)
{
	ASSERT(this, "Shape::setActive: null this");

	if(active)
	{
		CollisionManager_shapeBecameActive(Game_getCollisionManager(Game_getInstance()), this);
	}
	else
	{
		CollisionManager_shapeBecameInactive(Game_getCollisionManager(Game_getInstance()), this);
	}
}

// do I move?
bool Shape_moves(Shape this)
{
	ASSERT(this, "Shape::moves: null this");

	return this->moves;
}

// has been checked
bool Shape_isChecked(Shape this)
{
	ASSERT(this, "Shape::isChecked: null this");

	return this->checked;
}

// set check status
void Shape_checked(Shape this, bool checked)
{
	ASSERT(this, "Shape::checked: null this");

	this->checked = checked;
}

// has been configured?
bool Shape_isReady(Shape this)
{
	return this->ready;
}

// set configured flag
void Shape_setReady(Shape this, bool ready)
{
	this->ready = ready;
}

// set flag
void Shape_setCheckForCollisions(Shape this, bool checkForCollisions)
{
	this->checkForCollisions = checkForCollisions;
}

// get flag
bool Shape_checkForCollisions(Shape this)
{
	return this->checkForCollisions;
}
