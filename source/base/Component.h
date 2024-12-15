/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef COMPONENT_H_
#define COMPONENT_H_


//=========================================================================================================
// INCLUDES
//=========================================================================================================

#include <ListenerObject.h>


//=========================================================================================================
// FORWARD DECLARATIONS
//=========================================================================================================

class SpatialObject;
typedef const void ComponentSpec;


//=========================================================================================================
// CLASS' DECLARATION
//=========================================================================================================

///
/// Class Component
///
/// Inherits from ListenerObject
///
/// Serves as the base class for components of entities.
abstract class Component : ListenerObject 
{
	/// @protectedsection

	/// Object to which this component attaches to
	SpatialObject owner;

	/// Pointer to the spec that defines how to initialize the component 
	const ComponentSpec* componentSpec;

	/// Pointer to the transformation that the component attaches to
	const Transformation* transformation;

	/// @publicsection

	/// Class' constructor
	/// @param owner: SpatialObject to which the component attaches to
	/// @param componentSpec: Pointer to the spec that defines how to initialize the component
	void constructor(SpatialObject owner, const ComponentSpec* componentSpec);

	/// Class' destructor
	void destructor();

	/// Retrieve the spec pointer that defined how to initialized the component
	/// @return Component spec pointer
	ComponentSpec* getSpec();

	/// Retrieve the collider's owner	
	SpatialObject getOwner();
}

#endif
