/*
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef LINE_FIELD_H_
#define LINE_FIELD_H_


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// INCLUDES
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Collider.h>
#include <Mesh.h>


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' MACROS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

#define __LINE_FIELD_VERTEXES		2


//——————————————————————————————————————————————————————————————————————————————————————————————————————————
// CLASS' DECLARATION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————

///
/// Class LineField
///
/// Inherits from Collider
///
/// Defines a line that is sensible to collision on one of its sides.
class LineField : Collider
{
	/// @protectedsection

	/// Mesh used to draw the collider
	MeshSpec* meshSpec;

	/// Extreme of the line that defines the line field
	Vector3D a;

	/// Extreme of the line that defines the line field
	Vector3D b;

	/// Normal to the line defined by the points a and b
	Vector3D normal;

	/// Length of the line field's normal
	fixed_t normalLength;

	/// @publicsection

	/// Class' constructor
	void constructor(GameObject owner, const ColliderSpec* colliderSpec);

	/// Retrieve the normal to the collider.
	/// @return Normal to the collider
	override Vector3D getNormal();

	/// Configure the wireframe used to show the collider.
	override void configureWireframe();

	/// Print collider's state.
	/// @param x: Screen x coordinate where to print
	/// @param y: Screen y coordinate where to print
	override void print(int32 x, int32 y);

	/// Displace the line filed.
	/// @param displacement: Displacement vector
	void displace(fixed_t displacement);

	/// Retrieve the line field's center point.
	/// @return Line field's center point
	Vector3D getCenter();

	/// Retrieve the vertexes that define the line field.
	/// @param vertexes: Array of vectors that define the linefied
	void getVertexes(Vector3D vertexes[__LINE_FIELD_VERTEXES]);

	/// Set the length of the line field's normal.
	/// @param normalLength: Length of the line field's normal
	void setNormalLength(fixed_t normalLength);
}


#endif
