#ifndef AFFINE_H_
#define AFFINE_H_


//---------------------------------------------------------------------------------------------------------
// 											INCLUDES
//---------------------------------------------------------------------------------------------------------

#include <Types.h>
#include <Math.h>
#include <MiscStructs.h>


//---------------------------------------------------------------------------------------------------------
// 											DECLARATIONS
//---------------------------------------------------------------------------------------------------------

fix19_13 Affine_applyAll(u16 param, fix19_13 paramTableRow, const Scale* scale, const Rotation* rotation, const TextureSource* textureSource, s16 width, s16 height);

#endif