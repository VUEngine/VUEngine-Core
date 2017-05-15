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

#include <string.h>
#include <CollisionsContainerEntity.h>
#include <Optics.h>


//---------------------------------------------------------------------------------------------------------
//											CLASS'S DEFINITION
//---------------------------------------------------------------------------------------------------------

/**
 * @class	CollisionsContainerEntity
 * @extends Entity
 * @ingroup stage-entities
 */
__CLASS_DEFINITION(CollisionsContainerEntity, Entity);


//---------------------------------------------------------------------------------------------------------
//												PROTOTYPES
//---------------------------------------------------------------------------------------------------------

// global


//---------------------------------------------------------------------------------------------------------
//												CLASS'S METHODS
//---------------------------------------------------------------------------------------------------------

// always call these two macros next to each other
__CLASS_NEW_DEFINITION(CollisionsContainerEntity, CollisionsContainerEntityDefinition* collisionsContainerEntityDefinition, s16 id, s16 internalId, const char* const name)
__CLASS_NEW_END(CollisionsContainerEntity, collisionsContainerEntityDefinition, id, internalId, name);


// class's constructor
void CollisionsContainerEntity_constructor(CollisionsContainerEntity this, CollisionsContainerEntityDefinition* collisionsContainerEntityDefinition, s16 id, s16 internalId, const char* const name)
{
	ASSERT(this, "CollisionsContainerEntity::constructor: null this");

	// construct base Container
	__CONSTRUCT_BASE(Entity, collisionsContainerEntityDefinition, id, internalId, name);
}

// class's destructor
void CollisionsContainerEntity_destructor(CollisionsContainerEntity this)
{
	ASSERT(this, "CollisionsContainerEntity::destructor: null this");

	// destroy the super Container
	// must always be called at the end of the destructor
	__DESTROY_BASE;
}

void CollisionsContainerEntity_initialize(CollisionsContainerEntity this __attribute__ ((unused)), bool recursive __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::initialize: null this");
}

void CollisionsContainerEntity_ready(CollisionsContainerEntity this __attribute__ ((unused)), bool recursive __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::ready: null this");
}

void CollisionsContainerEntity_update(CollisionsContainerEntity this __attribute__ ((unused)), u32 elapsedTime __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::update: null this");
}

void CollisionsContainerEntity_transform(CollisionsContainerEntity this __attribute__ ((unused)), const Transformation* environmentTransform __attribute__ ((unused)), u8 invalidateTransformationFlag __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::transform: null this");
}

void CollisionsContainerEntity_updateVisualRepresentation(CollisionsContainerEntity this __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::updateVisualRepresentation: null this");
}

bool CollisionsContainerEntity_handleMessage(CollisionsContainerEntity this __attribute__ ((unused)), Telegram telegram __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::handleMessage: null this");

	return true;
}

bool CollisionsContainerEntity_handlePropagatedMessage(CollisionsContainerEntity this __attribute__ ((unused)), int message __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::handlePropagatedMessage: null this");

	return false;
}

int CollisionsContainerEntity_passMessage(CollisionsContainerEntity this __attribute__ ((unused)), int (*propagatedMessageHandler)(Container this, va_list args) __attribute__ ((unused)), va_list args __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::passMessage: null this");

	return false;
}


/**
 * Whether it is visible
 *
 * @memberof		CollisionsContainerEntity
 * @public
 *
 * @param this		Function scope
 * @param pad
 * @param recursive
 *
 * @return			Boolean if visible
 */
bool CollisionsContainerEntity_isVisible(CollisionsContainerEntity this, int pad, bool recursive __attribute__ ((unused)))
{
	ASSERT(this, "CollisionsContainerEntity::isVisible: null this");

	int x = 0;
	int y = 0;
	int z = 0;

	VBVec3D position3D = this->transform.globalPosition;

	if(this->centerDisplacement)
	{
		position3D.x += this->centerDisplacement->x;
		position3D.y += this->centerDisplacement->y;
		position3D.z += this->centerDisplacement->z;
	}

	// normalize the position to screen coordinates
	__OPTICS_NORMALIZE(position3D);

	VBVec2D position2D;
	__OPTICS_PROJECT_TO_2D(position3D, position2D);

	int halfWidth = (int)this->size.x >> 1;
	int halfHeight = (int)this->size.y >> 1;
	int halfDepth = (int)this->size.z >> 1;

	x = FIX19_13TOI(position2D.x);
	y = FIX19_13TOI(position2D.y);
	z = FIX19_13TOI(position2D.z);

	if(x + halfWidth > -pad && x - halfWidth < __SCREEN_WIDTH + pad)
	{
		// check y visibility
		if(y + halfHeight > -pad && y - halfHeight < __SCREEN_HEIGHT + pad)
		{
			// check z visibility
			if(z + halfDepth > -pad && z - halfDepth < __SCREEN_DEPTH + pad)
			{
				return true;
			}
		}
	}

	return false;
}
