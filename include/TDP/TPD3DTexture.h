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

#ifndef TPD3DTexture_h
#define TPD3DTexture_h

#include "TPBase.h"
#include "TPTexture.h"
struct ID3D11Texture2D;
struct ID3D11Device;

#ifdef __cplusplus
extern "C" {
#endif

TP_ASSUME_NONNULL_BEGIN

typedef TPTexture TPD3DTexture;
typedef TPTexture TPDXGITexture;

// TODO: document only one mip level is allowed
// TODO: right now we require DXGI_FORMAT_B8G8R8A8_UNORM
/*
 The caller is responsible for releasing the returned TPD3DTexture using TPRelease()
 */
TP_EXPORT TPD3DTexture *TPD3DTextureCreate(ID3D11Texture2D *texture);

/*
 Creates a TPD3DTexture from a TPDXGITexture
 The caller is responsible for releasing the returned TPD3DTexture using TPRelease()
 */
TP_EXPORT TPD3DTexture *TPD3DTextureCreateFromDXGI(ID3D11Device *device, TPDXGITexture *texture);

/*
 Returns the underlying ID3D11Texture2D.
 This texture should be considered to be owned by the TPD3DTexture and should not be retained beyond
 the lifetime of its owner.
 */
TP_EXPORT ID3D11Texture2D *TPD3DTextureGetTexture(TPD3DTexture *texture);

TP_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif /* TPD3DTexture_h */
