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

#ifndef TPOpenGLTexture_h
#define TPOpenGLTexture_h

#include "TPBase.h"
#include "TPTexture.h"
#ifdef __APPLE__
#include <OpenGL/gltypes.h>
#else
typedef unsigned int GLuint;
#endif
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

TP_ASSUME_NONNULL_BEGIN

typedef TPTexture TPOpenGLTexture;

typedef void (*TPOpenGLTextureReleaseCallback)(GLuint texture, void * TP_NULLABLE info);

// TODO: target as requirement or argument

/*
Create a texture from an OpenGL texture
callback will be called with the values passed to texture and info when the texture is released - the 
texture should remain valid until that happens.
The caller is responsible for releasing the returned TPOpenGLTexture using TPRelease()
*/
TP_EXPORT TPOpenGLTexture *TPOpenGLTextureCreate(GLuint texture, int32_t width, int32_t height, TPOpenGLTextureReleaseCallback TP_NULLABLE callback, void * TP_NULLABLE info);

/*
 Returns the underlying OpenGL texture.
 This texture is owned by the TPOpenGLTexture and should not be used beyond the lifetime of its owner.
 */
TP_EXPORT GLuint TPOpenGLTextureGetName(TPOpenGLTexture *texture);

TP_EXPORT int32_t TPOpenGLTextureGetWidth(TPTexture *texture);

TP_EXPORT int32_t TPOpenGLTextureGetHeight(TPTexture *texture);

TP_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif /* TPOpenGLTexture_h */
