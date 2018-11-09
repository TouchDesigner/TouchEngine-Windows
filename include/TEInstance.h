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

#ifndef TEInstance_h
#define TEInstance_h

#include "TEBase.h"
#include "TETypes.h"
#include "TEStructs.h"
#include "TETexture.h"
#ifdef __APPLE__
#import <OpenGL/OpenGL.h>
#endif
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

TE_ASSUME_NONNULL_BEGIN

typedef TEObject TEInstance;

typedef void (*TEInstanceEventCallback)(TEInstance *instance, TEEvent event, TEResult result, int64_t time_value, int32_t time_scale, void * TE_NULLABLE info);
typedef void (*TEInstanceParameterValueCallback)(TEInstance *instance, const char *identifier, void *info);


/*
 On return, extensions is a list of file extensions supported by TEInstanceCreate
 The caller is responsible for releasing the returned TEStringArray using TERelease()
 */
TE_EXPORT TEResult TEInstanceGetSupportedFileExtensions(struct TEStringArray * TE_NULLABLE * TE_NONNULL extensions);

/*
 Creates an instance for a .tox file localted at 'path'.
 'path' is assumed to be UTF-8 encoded.
 The file is loaded asynchronously after this function returns.
 'event_callback' will be called with TEEventInstanceDidLoad when file loading is complete, or a TEResult indicating an error.
 'callback_info' will be passed into the callbacks as the 'info' argument.
 The caller is responsible for releasing the returned TEInstance using TERelease()
 */
// TODO: TETimeMode is currently ignored
#ifdef __APPLE__
TE_EXPORT TEResult TEInstanceCreate(const char *path,
								   CGLContextObj context,
								   TETimeMode mode,
								   TEInstanceEventCallback event_callback,
								   TEInstanceParameterValueCallback prop_value_callback,
								   void * TE_NULLABLE callback_info,
								   TEInstance **instanceOut);
#else
// TODO: currently an instance on Windows is not tied to a specific DX or GL renderer
TE_EXPORT TEResult TEInstanceCreate(const char *path,
								   TETimeMode mode,
								   TEInstanceEventCallback frame_callback,
								   TEInstanceParameterValueCallback prop_value_callback,
								   void * TE_NULLABLE callback_info,
								   TEInstance **instanceOut);
#endif

TE_EXPORT const char *TEInstanceGetPath(TEInstance *instance);

#ifdef __APPLE__
TE_EXPORT CGLContextObj TEInstanceGetGLContext(TEInstance *instance);
#else
// TODO: Win32 DX/GL version
#endif

TE_EXPORT TETimeMode TEInstanceGetTimeMode(TEInstance *instance);

/*
 Rendering
 */

/*
 Initiates rendering of a frame. 
 time_value, time_scale are ignored for TETimeInternal
 The frame is rendered asynchronously after this function returns.
 TEInstanceEventCallback is called with TEEventFrameDidFinish when frame completes
 */
TE_EXPORT TEResult TEInstanceStartFrameAtTime(TEInstance *instance, int64_t time_value, int32_t time_scale);

/*
 Requests the cancellation of an in-progress frame.
 If the frame was successfully cancelled, TEInstanceEventCallback is called with TEEventFrameDidFinish and TEResultCancelled,
 alternatively the frame may complete as usual before the cancellation request is delivered.
*/
TE_EXPORT TEResult TEInstanceCancelFrame(TEInstance *instance);

/*
 Parameter Layout
 */

/*
 On return children is a list of parameter identifiers for the children of the parent parameter denoted by identifier.
 If identifier is NULL or an empty string, the top level parameters are returned.
 identifier should denote a parameter of type TEParameterTypeGroup or TEParameterTypeComplex.
 The caller is responsible for releasing the returned TEStringArray using TERelease().
 */
TE_EXPORT TEResult TEInstanceParameterGetChildren(TEInstance *instance, const char * TE_NULLABLE identifier, struct TEStringArray * TE_NULLABLE * TE_NONNULL children);

/*
 On return groups is a list of parameter identifiers for the top level parameters in the given scope.
 The caller is responsible for releasing the returned TEStringArray using TERelease().
 */
TE_EXPORT TEResult TEInstanceGetParameterGroups(TEInstance *instance, TEScope scope, struct TEStringArray * TE_NULLABLE * TE_NONNULL groups);

/*
 Parameter Basics
 */

/*
 On return info describes the parameter denoted by identifier.
 The caller is responsible for releasing the returned TEParameterInfo using TERelease().
 */
TE_EXPORT TEResult TEInstanceParameterGetInfo(TEInstance *instance, const char *identifier, TEParameterInfo * TE_NULLABLE * TE_NONNULL info);

/*
 Stream Parameter Configuration
 */

/*
 On return description describes the stream parameter denoted by identifier.
 The caller is responsible for releasing the returned TEStreamDescription using TERelease().
 */
TE_EXPORT TEResult TEInstanceParameterGetStreamDescription(TEInstance *instance, const char *identifier, TEStreamDescription * TE_NULLABLE * TE_NONNULL description);

/*
 Configures the input stream denoted by identifier according to the content of description.
 Input streams must have their description set prior to use.
 Changing the description once rendering has begun may result in dropped samples.
 */
TE_EXPORT TEResult TEInstanceParameterSetInputStreamDescription(TEInstance *instance, const char *identifier, const TEStreamDescription *description);

/*
 Getting Parameter Values
 */

TE_EXPORT TEResult TEInstanceParameterGetDoubleValue(TEInstance *instance, const char *identifier, TEParameterValue which, double *value, int32_t count);

TE_EXPORT TEResult TEInstanceParameterGetIntValue(TEInstance *instance, const char *identifier, TEParameterValue which, int32_t *value, int32_t count);

/*
 The caller is responsible for releasing the returned TEString using TERelease().
 */
TE_EXPORT TEResult TEInstanceParameterGetStringValue(TEInstance *instance, const char *identifier, TEParameterValue which, TEString * TE_NULLABLE * TE_NONNULL string);

/*
 The caller is responsible for releasing the returned TETexture using TERelease().
 */
TE_EXPORT TEResult TEInstanceParameterGetTextureValue(TEInstance *instance, const char *identifier, TEParameterValue which, TETexture * TE_NULLABLE * TE_NONNULL value);


/*
 Copies stream samples from an output stream.
 buffers is an array of pointers to count buffers where each buffer receives values for one channel
 Prior to calling this function, set length to the capacity (in samples) of a single channel buffer
 On return, length is set to the actual number of samples copied to each channel buffer
 Calls to this function and TEInstanceParameterAppendStreamValues() can be made at any point after an instance has loaded, including during rendering
*/
TE_EXPORT TEResult TEInstanceParameterGetOutputStreamValues(TEInstance *instance, const char *identifier, float * TE_NONNULL * TE_NONNULL buffers, int32_t count, int64_t *length);

/*
 Setting Input Parameter Values
 */

TE_EXPORT TEResult TEInstanceParameterSetDoubleValue(TEInstance *instance, const char *identifier, const double *value, int32_t count);

TE_EXPORT TEResult TEInstanceParameterSetIntValue(TEInstance *instance, const char *identifier, const int32_t *value, int32_t count);

/*
 Sets the value of a string input parameter.
 value is a UTF-8 encoded string
 */
TE_EXPORT TEResult TEInstanceParameterSetStringValue(TEInstance *instance, const char *identifier, const char * TE_NULLABLE value);

/*
 Sets the value of a texture input parameter
 texture may be retained by the instance
 */
TE_EXPORT TEResult TEInstanceParameterSetTextureValue(TEInstance *instance, const char *identifier, TETexture *TE_NULLABLE texture);

/*
 Copies stream samples to an input stream.
 buffers is an array of pointers to count buffers where each buffer contains values for one channel
 Prior to calling this function, set length to the size (in samples) of a single channel buffer
 On return, length is set to the actual number of samples copied to each channel buffer
 Calls to this function and TEInstanceParameterGetOutputStreamValues() can be made at any point after an instance has loaded, including during rendering
*/
TE_EXPORT TEResult TEInstanceParameterAppendStreamValues(TEInstance *instance, const char *identifier, const float * TE_NONNULL * TE_NONNULL buffers, int32_t count, int64_t *length);

TE_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif /* TEInstance_h */
