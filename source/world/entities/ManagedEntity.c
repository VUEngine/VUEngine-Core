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
#include <CollisionManager.h>
#include <VPUManager.h>
#include <debugConfig.h>


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
const extern VBVec3D* _screenPosition;
const extern VBVec3D* _screenDisplacement;
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
	__CONSTRUCT_BASE((EntityDefinition*)managedEntityDefinition, id, name);

	/* the sprite must be initializated in the derived class */
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
				VBVec2D position = *__VIRTUAL_CALL_UNSAFE(const VBVec2D*, Sprite, getPosition, sprite);
				
				// eliminate fractions to avoid rounding problems later
				position.x &= 0xFFFFE000;
				position.y &= 0xFFFFE000;
				
				// don't round z coordinate since it is used for layer sorting
				__VIRTUAL_CALL(void, Sprite, setPosition, sprite, &position);
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
void ManagedEntity_initialTransform(ManagedEntity this, Transformation* environmentTransform)
{
	ASSERT(this, "ManagedEntity::initialTransform: null this");
	
	Entity_initialTransform(__SAFE_CAST(Entity, this), environmentTransform);
	
	VBVec3D position3D = this->transform.globalPosition;
	
	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, this->previous2DPosition);
	
	VirtualList_clear(this->managedSprites);
	ManagedEntity_registerSprites(this, __SAFE_CAST(Entity, this));
}

// transform class
void ManagedEntity_transform(ManagedEntity this, const Transformation* environmentTransform)
{
	ASSERT(this, "ManagedEntity::transform: null this");

	int updateSpritePosition = Entity_updateSpritePosition(__SAFE_CAST(Entity, this));
	
	// TODO: take into account scaling
	//int updateSpriteScale = Entity_updateSpriteScale(__SAFE_CAST(Entity, this));

	if(updateSpritePosition)
	{
		if(this->invalidateGlobalPosition.x ||
			this->invalidateGlobalPosition.y ||
			this->invalidateGlobalPosition.z ||
			this->children)
		{
			// call base class's transform method
			Container_transformNonVirtual(__SAFE_CAST(Container, this), environmentTransform);
		}

		// concatenate transform
		this->transform.globalPosition.x = environmentTransform->globalPosition.x + this->transform.localPosition.x;
		this->transform.globalPosition.y = environmentTransform->globalPosition.y + this->transform.localPosition.y;
		this->transform.globalPosition.z = environmentTransform->globalPosition.z + this->transform.localPosition.z;

		// propagate rotation
		this->transform.globalRotation.x = environmentTransform->globalRotation.x + this->transform.localRotation.x;
		this->transform.globalRotation.y = environmentTransform->globalRotation.x + this->transform.localRotation.y;
		this->transform.globalRotation.z = environmentTransform->globalRotation.x + this->transform.localRotation.z;
		
		// propagate scale
		this->transform.globalScale.x = FIX7_9_MULT(environmentTransform->globalScale.x, this->transform.localScale.x);
		this->transform.globalScale.y = FIX7_9_MULT(environmentTransform->globalScale.y, this->transform.localScale.y);
	
		// save new global position
		VBVec3D position3D = this->transform.globalPosition;
		VBVec2D position2D;
		position2D.parallax = 0;
		
		// normalize the position to screen coordinates
		__OPTICS_NORMALIZE(position3D);
	
		// project position to 2D space
		__OPTICS_PROJECT_TO_2D(position3D, position2D);
	
		position2D.x &= 0xFFFFE000;
		position2D.y &= 0xFFFFE000;

		VirtualNode spriteNode = this->managedSprites->head;
		
		fix19_13 xDisplacement = position2D.x - this->previous2DPosition.x;
		fix19_13 yDisplacement = position2D.y - this->previous2DPosition.y;

		for(; spriteNode; spriteNode = spriteNode->next)
		{
			Sprite sprite = __SAFE_CAST(Sprite, spriteNode->data);
			
			VBVec2D position = *__VIRTUAL_CALL_UNSAFE(const VBVec2D*, Sprite, getPosition, sprite);
			
			position.x += xDisplacement;
			position.y += yDisplacement;
//			position.z += position2D.z - this->previous2DPosition.z;
	
			// don't round z coordinate since it is used for layer sorting
			__VIRTUAL_CALL(void, Sprite, setPosition, sprite, &position);
			
			Sprite_setRenderFlag(sprite, __UPDATE_G);
		}
		
		VPUManager_disableInterrupt(VPUManager_getInstance());

		for(spriteNode = this->managedSprites->head; spriteNode; spriteNode = spriteNode->next)
		{
			__VIRTUAL_CALL(void, Sprite, render, __SAFE_CAST(Sprite, spriteNode->data));

		}
		
		VPUManager_enableInterrupt(VPUManager_getInstance());

		this->previous2DPosition = position2D;
	}
	else
	{
		// allow children's global positions to be updated properly
	//	Entity_transform(__SAFE_CAST(Entity, this), environmentTransform);
	}
}