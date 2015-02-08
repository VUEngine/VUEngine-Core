/* VBJaEngine: bitmap graphics engine for the Nintendo Virtual Boy
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

//---------------------------------------------------------------------------------------------------------
// 												INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <ManagedEntity.h>
#include <Prototypes.h>
#include <Optics.h>
#include <AnimatedSprite.h>
#include <Shape.h>
#include <CollisionManager.h>


//---------------------------------------------------------------------------------------------------------
// 											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

// define the ManagedEntity
__CLASS_DEFINITION(ManagedEntity, Entity);
__CLASS_FRIEND_DEFINITION(Entity);


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
__CLASS_NEW_DEFINITION(ManagedEntity, ManagedEntityDefinition* managedEntityDefinition, s16 id)
__CLASS_NEW_END(ManagedEntity, managedEntityDefinition, id);

// class's constructor
void ManagedEntity_constructor(ManagedEntity this, ManagedEntityDefinition* managedEntityDefinition, s16 id)
{
	ASSERT(this, "ManagedEntity::constructor: null this");

	// construct base Entity
	__CONSTRUCT_BASE((EntityDefinition*)managedEntityDefinition, id);

	/* the sprite must be initializated in the derivated class */
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
	__DESTROY_BASE;
}

static void ManagedEntity_registerSprites(ManagedEntity this, Entity child)
{
	ASSERT(this, "ManagedEntity::registerSprites: null this");
	ASSERT(child, "ManagedEntity::registerSprites: null child");

	if (child)
	{
		if(child->sprites)
		{
			VirtualNode spriteNode = VirtualList_begin(child->sprites);

			for(; spriteNode; spriteNode = VirtualNode_getNext(spriteNode))
			{
				Sprite sprite = __UPCAST(Sprite, VirtualNode_getData(spriteNode));
				VirtualList_pushBack(this->managedSprites, sprite);
				DrawSpec drawSpec = Sprite_getDrawSpec(sprite);
				
				// eliminate fractions to avoid rounding problems later
				drawSpec.position.x &= 0xFFFFE000;
				drawSpec.position.y &= 0xFFFFE000;
				// don't round z coordinate since it is used for layer sorting
				Sprite_setDrawSpec(sprite, &drawSpec);
			}
		}
		
		if(child->children)
		{
			VirtualNode childNode = VirtualList_begin(child->children);
			
			for(; childNode; childNode = VirtualNode_getNext(childNode))
			{
				ManagedEntity_registerSprites(this, __UPCAST(Entity, VirtualNode_getData(childNode)));
			}
		}
	}
}

// transform class
void ManagedEntity_initialTransform(ManagedEntity this, Transformation* environmentTransform)
{
	ASSERT(this, "ManagedEntity::initialTransform: null this");
	
	Entity_initialTransform(__UPCAST(Entity, this), environmentTransform);
	
	VBVec3D position3D = this->transform.globalPosition;
	
	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	// project position to 2D space
	__OPTICS_PROJECT_TO_2D(position3D, this->previous2DPosition);
	
	ManagedEntity_registerSprites(this, __UPCAST(Entity, this));
}

// transform class
void ManagedEntity_transform(ManagedEntity this, Transformation* environmentTransform)
{
	ASSERT(this, "ManagedEntity::transform: null this");

	int updateSpritePosition = Entity_updateSpritePosition(__UPCAST(Entity, this));
	
	// TODO: take into account scaling
	//int updateSpriteScale = Entity_updateSpriteScale(__UPCAST(Entity, this));

	if(updateSpritePosition)
	{
		// concatenate environment transform
		Transformation environmentTransformCopy =
		{
			// local position
			{
				0,
				0,
				0
			},
			// global position
			{
				environmentTransform->globalPosition.x + this->transform.localPosition.x,
				environmentTransform->globalPosition.y + this->transform.localPosition.y,
				environmentTransform->globalPosition.z + this->transform.localPosition.z
			},
			// scale
			{
				environmentTransform->scale.x * this->transform.scale.x,
				environmentTransform->scale.y * this->transform.scale.y
			},
			// rotation
			{
				environmentTransform->rotation.x + this->transform.rotation.x,
				environmentTransform->rotation.y + this->transform.rotation.y,
				environmentTransform->rotation.z + this->transform.rotation.z
			}
		};
	
		// save new global position
		this->transform.globalPosition = environmentTransformCopy.globalPosition;
	
		// save new global position
		VBVec3D position3D = this->transform.globalPosition;
		VBVec2D position2D;
		
		// normalize the position to screen coordinates
		__OPTICS_NORMALIZE(position3D);
	
		// project position to 2D space
		__OPTICS_PROJECT_TO_2D(position3D, position2D);
	
		VirtualNode spriteNode = VirtualList_begin(this->managedSprites);
		
		for(; spriteNode; spriteNode = VirtualNode_getNext(spriteNode))
		{
			Sprite sprite = __UPCAST(Sprite, VirtualNode_getData(spriteNode));
			DrawSpec drawSpec = Sprite_getDrawSpec(sprite);
			drawSpec.position.x += position2D.x - this->previous2DPosition.x;
			drawSpec.position.y += position2D.y - this->previous2DPosition.y;
			drawSpec.position.z += position2D.z - this->previous2DPosition.z;
	
			Sprite_setDrawSpec(sprite, &drawSpec);
			Sprite_setRenderFlag(sprite, __UPDATE_G);
		}
		
		this->previous2DPosition = position2D;
	}
	else
	{
		// allow children's global positions to be updated properly
		Entity_transform(__UPCAST(Entity, this), environmentTransform);
	}
}

