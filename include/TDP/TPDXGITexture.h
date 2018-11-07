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

#ifndef TPDXGITexture_h
#define TPDXGITexture_h

#include "TPBase.h"
#include "TPTexture.h"

typedef void *HANDLE;

#ifdef __cplusplus
extern "C" {
#endif

TP_ASSUME_NONNULL_BEGIN

typedef TPTexture TPDXGITexture;
typedef TPTexture TPD3D11Texture;

typedef void (*TPDXGITextureReleaseCallback)(HANDLE handle, void * TP_NULLABLE info);

/*
Create a texture from a shared handle
This must come from IDXGIResource::GetSharedHandle(), and not IDXGIResource1::CreateSharedHandle()
callback will be called with the values passed to handle and info when the texture is released
The caller is responsible for releasing the returned TPDXGITexture using TPRelease()
*/
TP_EXPORT TPDXGITexture *TPDXGITextureCreate(HANDLE handle, TPDXGITextureReleaseCallback TP_NULLABLE callback, void *info);

/*
 Create a texture from a TPD3D11Texture. Depending on the source texture, this may involve copying
 the texture to permit sharing.
 The caller is responsible for releasing the returned TPDXGITexture using TPRelease()
 */
// TODO: this function may be removed from the API
TP_EXPORT TPDXGITexture *TPDXGITextureCreateFromD3D(TPD3D11Texture *texture);

TP_EXPORT HANDLE TPDXGITextureGetHandle(TPDXGITexture *texture);

TP_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif /* TPDXGITexture_h */
