/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef UI_CONTAINER_H_
#define UI_CONTAINER_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Container.h>
#include <Entity.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class UIContainer
///
/// Inherits from Container
///
/// Implements a container whose children are always fixed to the camera
class UIContainer : Container
{
	/// @publicsection

	/// Class' constructor
	/// @param childrenPositionedEntities: Array of specs that define how to configure the container's children
	void constructor(PositionedEntity* childrenPositionedEntities);

	/// Compute the container's global transformation.
	/// @param environmentTransform: Reference environment for the local transformation
	/// @param invalidateTransformationFlag: Flag that determines which transfomation's components 
	/// must be recomputed
	override void transform(const Transformation* environmentTransform, uint8 invalidateTransformationFlag);

	/// Spawn a new child and configure it with the provided positioned entity struct.
	/// @param positionedEntity: Struct that defines which entity spec to use to configure the new child
	Entity spawnChildEntity(const PositionedEntity* const positionedEntity);
}


#endif
