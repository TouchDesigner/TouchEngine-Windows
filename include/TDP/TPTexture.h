/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with 
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 *
 * TouchPlugIn
 *
 * Copyright © 2018 Derivative. All rights reserved.
 *
 */

#ifndef TPTexture_h
#define TPTexture_h

#include "TPBase.h"
#include "TPTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

TP_ASSUME_NONNULL_BEGIN

typedef TPObject TPTexture;

/*
 Returns the type (OpenGL, D3D, DXGI) of texture
 */
TP_EXPORT TPTextureType TPTextureGetType(TPTexture *texture);

TP_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif /* TPTexture_h */
