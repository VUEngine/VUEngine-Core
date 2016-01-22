/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
 *
 * Copyright (C) 2007 Jorge Eremiev <jorgech3@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not,
 * see <http://www.gnu.org/licenses/>.
 */


//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Shape.h>
#include <Game.h>
#include <CollisionManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the Shape
__CLASS_DEFINITION(Shape, Object);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// class's constructor
void Shape_constructor(Shape this, SpatialObject owner)
{
	ASSERT(this, "Shape::constructor: null this");

	// construct base object
	__CONSTRUCT_BASE();

	// set the owner
	this->owner = owner;

	// do I move?
	this->moves = __VIRTUAL_CALL(bool, SpatialObject, moves, owner);

	// not checked yet
	this->checked = false;

	// not setup yet
	this->ready = false;

	// set flag
	this->checkForCollisions = true;

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

// is active?
bool Shape_isActive(Shape this)
{
	ASSERT(this, "Shape::isActive: null this");

	return false;
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