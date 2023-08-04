/**
 * VUEngine Core
 *
 * © Jorge Eremiev <jorgech3@gmail.com> and Christian Radke <c.radke@posteo.de>
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_


//---------------------------------------------------------------------------------------------------------
//											 DECLARATIONS
//---------------------------------------------------------------------------------------------------------

extern bool _stackHeadroomViolation;
extern int32 _vuengineLinkPointer;
extern int32 _vuengineStackPointer;
extern bool _triggeringException;
extern uint32 _bssEnd;
extern const Vector3D* _cameraPosition;
extern const Vector3D* _cameraPreviousPosition;
extern const CameraFrustum* _cameraFrustum;
extern const Rotation* _cameraRotation;
extern const Rotation* _cameraInvertedRotation;
extern const Optical* _optical;

#endif
