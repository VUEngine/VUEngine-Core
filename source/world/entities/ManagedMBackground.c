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

#include <MBackground.h>
#include <Prototypes.h>
#include <Optics.h>
#include <Shape.h>
#include <MBgmapSprite.h>
#include <ManagedMBackground.h>
#include <debugUtilities.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

__CLASS_DEFINITION(ManagedMBackground, MBackground);

__CLASS_FRIEND_DEFINITION(Entity);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

extern const VBVec3D* _screenPosition;
extern const Optical* _optical;


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ManagedMBackground, MBackgroundDefinition* definition, int id, const char* const name)
__CLASS_NEW_END(ManagedMBackground, definition, id, name);

// class's constructor
void ManagedMBackground_constructor(ManagedMBackground this, MBackgroundDefinition* definition, int id, const char* const name)
{
	ASSERT(this, "ManagedMBackground::constructor: null this");

	// construct base
	__CONSTRUCT_BASE(MBackground, definition, id, name);

	// the sprite must be initialized in the derived class
	this->managedSprites = __NEW(VirtualList);

	this->previous2DPosition.x = 0;
	this->previous2DPosition.y = 0;
	this->previous2DPosition.z = 0;
	this->previous2DPosition.parallax = 0;
}

// class's destructor
void ManagedMBackground_destructor(ManagedMBackground this)
{
	ASSERT(this, "ManagedMBackground::destructor: null this");

	if(this->managedSprites)
	{
		__DELETE(this->managedSprites);
		this->managedSprites = NULL;
	}

	// delete the super object
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

static void ManagedMBackground_registerSprites(ManagedMBackground this, Entity child)
{
	ASSERT(this, "ManagedMBackground::registerSprites: null this");
	ASSERT(child, "ManagedMBackground::registerSprites: null child");

	if(child)
	{
		if(child->sprites)
		{
			VirtualNode spriteNode = child->sprites->head;

			for(; spriteNode; spriteNode = spriteNode->next)
			{
				Sprite sprite = __SAFE_CAST(Sprite, spriteNode->data);
				VirtualList_pushBack(this->managedSprites, sprite);
			}
		}

		if(child->children)
		{
			VirtualNode childNode = child->children->head;

			for(; childNode; childNode = childNode->next)
			{
				ManagedMBackground_registerSprites(this, __SAFE_CAST(Entity, childNode->data));
			}
		}
	}
}

// transform class
void ManagedMBackground_initialTransform(ManagedMBackground this, Transformation* environmentTransform)
{
	ASSERT(this, "ManagedMBackground::initialTransform: null this");

	Entity_initialTransform(__SAFE_CAST(Entity, this), environmentTransform);

	VirtualList_clear(this->managedSprites);
	ManagedMBackground_registerSprites(this, __SAFE_CAST(Entity, this));

	// save new global position
	VBVec3D position3D = this->transform.globalPosition;
	VBVec2D position2D;

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, position2D);

	position2D.parallax = 0;

	this->previous2DPosition = position2D;
}

// transform class
void ManagedMBackground_transform(ManagedMBackground this, const Transformation* environmentTransform)
{
	ASSERT(this, "ManagedMBackground::transform: null this");

	// allow normal transformation while not visible to avoid projection errors
	// at the initial transformation
//	if(!Entity_isVisible(__SAFE_CAST(Entity, this), 0, false) || Entity_updateSpriteTransformations(__SAFE_CAST(Entity, this)))
	if(Entity_updateSpriteTransformations(__SAFE_CAST(Entity, this)))
	{
		Entity_transform(__SAFE_CAST(Entity, this), environmentTransform);
		this->invalidateGlobalTransformation = 0;

		// save the 2d position
		VBVec3D position3D = this->transform.globalPosition;
		VBVec2D position2D;

		// normalize the position to screen coordinates
		__OPTICS_NORMALIZE(position3D);

		// project position to 2D space
		__OPTICS_PROJECT_TO_2D(position3D, position2D);

		position2D.parallax = 0;

		this->previous2DPosition = position2D;

		this->updateSprites = __UPDATE_SPRITE_POSITION | __UPDATE_SPRITE_TRANSFORMATIONS;

		return;
	}

	if(__INVALIDATE_POSITION & this->invalidateGlobalTransformation ||
		this->children)
	{
		// call base class's transform method
		Container_transformNonVirtual(__SAFE_CAST(Container, this), environmentTransform);
	}

	// apply environment transform
	Container_applyEnvironmentToTranformation(__SAFE_CAST(Container, this), environmentTransform);

	this->updateSprites = __UPDATE_SPRITE_POSITION;

	this->invalidateGlobalTransformation = 0;
}

void ManagedMBackground_updateVisualRepresentation(ManagedMBackground this)
{
	ASSERT(this, "ManagedMBackground::updateVisualRepresentation: null this");

	if(this->updateSprites)
	{
		if(this->updateSprites & __UPDATE_SPRITE_TRANSFORMATIONS)
		{
			Entity_updateVisualRepresentation(__SAFE_CAST(Entity, this));
		}

		// save new global position
		VBVec3D position3D = this->transform.globalPosition;
		VBVec2D position2D;
		position2D.parallax = 0;

		// normalize the position to screen coordinates
		__OPTICS_NORMALIZE(position3D);

		// project position to 2D space
		__OPTICS_PROJECT_TO_2D(position3D, position2D);

		VirtualNode spriteNode = this->managedSprites->head;

		VBVec2D displacement;

		displacement.x = position2D.x - this->previous2DPosition.x;
		displacement.y = position2D.y - this->previous2DPosition.y;
		displacement.z = 0;
		displacement.parallax = 0;

		for(; spriteNode; spriteNode = spriteNode->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, spriteNode->data);

			__VIRTUAL_CALL(Sprite, addDisplacement, sprite, &displacement);
		}

		this->previous2DPosition = position2D;
	}

	this->updateSprites = 0;
}

// execute logic
void ManagedMBackground_update(ManagedMBackground this __attribute__ ((unused)))
{
	ASSERT(this, "ManagedMBackground::update: null this");
}

int ManagedMBackground_passMessage(ManagedMBackground this __attribute__ ((unused)), int (*propagatedMessageHandler)(Container this, va_list args) __attribute__ ((unused)), va_list args __attribute__ ((unused)))
{
	ASSERT(this, "ManagedMBackground::passMessage: null this");

	return false;
}
