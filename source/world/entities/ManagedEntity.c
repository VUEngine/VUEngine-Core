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

#include <ManagedEntity.h>
#include <Prototypes.h>
#include <Optics.h>
#include <Shape.h>
#include <MBgmapSprite.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the ManagedEntity
__CLASS_DEFINITION(ManagedEntity, Entity);

__CLASS_FRIEND_DEFINITION(Entity);
__CLASS_FRIEND_DEFINITION(VirtualNode);
__CLASS_FRIEND_DEFINITION(VirtualList);


//---------------------------------------------------------------------------------------------------------
// 												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global
extern const VBVec3D* _screenPosition;
extern const VBVec3D* _screenDisplacement;
extern const Optical* _optical;

static void ManagedEntity_registerSprites(ManagedEntity this, Entity child);


//---------------------------------------------------------------------------------------------------------
// 												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(ManagedEntity, ManagedEntityDefinition* managedEntityDefinition, s16 id, const char* const name)
__CLASS_NEW_END(ManagedEntity, managedEntityDefinition, id, name);

// class's constructor
void ManagedEntity_constructor(ManagedEntity this, ManagedEntityDefinition* managedEntityDefinition, s16 id, const char* const name)
{
	ASSERT(this, "ManagedEntity::constructor: null this");

	// construct base Entity
	__CONSTRUCT_BASE(Entity, (EntityDefinition*)managedEntityDefinition, id, name);

	// the sprite must be initialized in the derived class
	this->managedSprites = __NEW(VirtualList);

	this->previous2DPosition.x = 0;
	this->previous2DPosition.y = 0;
	this->previous2DPosition.z = 0;
	this->previous2DPosition.parallax = 0;
}

// class's destructor
void ManagedEntity_destructor(ManagedEntity this)
{
	ASSERT(this, "ManagedEntity::destructor: null this");

	if(this->managedSprites)
	{
		__DELETE(this->managedSprites);
		this->managedSprites = NULL;
	}

	// destroy the super Entity
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

static void ManagedEntity_registerSprites(ManagedEntity this, Entity child)
{
	ASSERT(this, "ManagedEntity::registerSprites: null this");
	ASSERT(child, "ManagedEntity::registerSprites: null child");

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
				ManagedEntity_registerSprites(this, __SAFE_CAST(Entity, childNode->data));
			}
		}
	}
}

// transform class
void ManagedEntity_initialTransform(ManagedEntity this, Transformation* environmentTransform, u32 recursive)
{
	ASSERT(this, "ManagedEntity::initialTransform: null this");

	Entity_initialTransform(__SAFE_CAST(Entity, this), environmentTransform, recursive);

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

void ManagedEntity_ready(ManagedEntity this, u32 recursive)
{
	ASSERT(this, "ManagedEntity::ready: null this");

    Entity_ready(__SAFE_CAST(Entity, this), recursive);

	if(!VirtualList_getSize(this->managedSprites))
	{
    	ManagedEntity_registerSprites(this, __SAFE_CAST(Entity, this));
    }
}

// transform class
void ManagedEntity_transform(ManagedEntity this, const Transformation* environmentTransform)
{
	ASSERT(this, "ManagedEntity::transform: null this");

	// allow normal transformation while not visible to avoid projection errors
	// at the initial transformation
//	if(!Entity_isVisible(__SAFE_CAST(Entity, this), 0, false) || Entity_updateSpriteScale(__SAFE_CAST(Entity, this)))
	if(Entity_updateSpriteScale(__SAFE_CAST(Entity, this)))
	{
		Entity_transform(__SAFE_CAST(Entity, this), environmentTransform);

		// save the 2d position
		VBVec3D position3D = this->transform.globalPosition;
		VBVec2D position2D;

		// normalize the position to screen coordinates
		__OPTICS_NORMALIZE(position3D);

		// project position to 2D space
		__OPTICS_PROJECT_TO_2D(position3D, position2D);

		position2D.parallax = 0;

		this->previous2DPosition = position2D;

		this->updateSprites = __UPDATE_SPRITE_TRANSFORMATION;

		return;
	}

    this->updateSprites = (_screenDisplacement->x || _screenDisplacement->y || _screenDisplacement->z)  || (__INVALIDATE_POSITION & this->invalidateGlobalTransformation)? __UPDATE_SPRITE_POSITION : 0;

    // call base class's transform method
    Container_transformNonVirtual(__SAFE_CAST(Container, this), environmentTransform);
}

void ManagedEntity_updateVisualRepresentation(ManagedEntity this)
{
	ASSERT(this, "ManagedEntity::updateVisualRepresentation: null this");

	if(this->updateSprites)
	{
		if(this->updateSprites & __UPDATE_SPRITE_SCALE)
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

int ManagedEntity_passMessage(ManagedEntity this __attribute__ ((unused)), int (*propagatedMessageHandler)(Container this, va_list args) __attribute__ ((unused)), va_list args __attribute__ ((unused)))
{
	ASSERT(this, "ManagedEntity::passMessage: null this");

	return false;
}
