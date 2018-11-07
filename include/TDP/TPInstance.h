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

#ifndef TPInstance_h
#define TPInstance_h

#include "TPBase.h"
#include "TPTypes.h"
#include "TPStructs.h"
#include "TPTexture.h"
#ifdef __APPLE__
#import <OpenGL/OpenGL.h>
#endif
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

TP_ASSUME_NONNULL_BEGIN

typedef TPObject TPInstance;

typedef void (*TPInstanceEventCallback)(TPInstance *instance, TPEvent event, TPResult result, int64_t time_value, int32_t time_scale, void * TP_NULLABLE info);
typedef void (*TPInstanceParameterValueCallback)(TPInstance *instance, const char *identifier, void *info);


/*
 On return, extensions is a list of file extensions supported by TPInstanceCreate
 The caller is responsible for releasing the returned TPStringArray using TPRelease()
 */
TP_EXPORT TPResult TPInstanceCopySupportedFileExtensions(struct TPStringArray * TP_NULLABLE * TP_NONNULL extensions);

/*
 Creates an instance for a .tox file localted at 'path'.
 'path' is assumed to be UTF-8 encoded.
 The file is loaded asynchronously after this function returns.
 'event_callback' will be called with TPEventInstanceDidLoad when file loading is complete, or a TPResult indicating an error.
 'callback_info' will be passed into the callbacks as the 'info' argument.
 The caller is responsible for releasing the returned TPInstance using TPRelease()
 */
// TODO: TPTimeMode is currently ignored
#ifdef __APPLE__
TP_EXPORT TPResult TPInstanceCreate(const char *path,
								   CGLContextObj context,
								   TPTimeMode mode,
								   TPInstanceEventCallback event_callback,
								   TPInstanceParameterValueCallback prop_value_callback,
								   void * TP_NULLABLE callback_info,
								   TPInstance **instanceOut);
#else
// TODO: currently an instance on Windows is not tied to a specific DX or GL renderer
TP_EXPORT TPResult TPInstanceCreate(const char *path,
								   TPTimeMode mode,
								   TPInstanceEventCallback frame_callback,
								   TPInstanceParameterValueCallback prop_value_callback,
								   void * TP_NULLABLE callback_info,
								   TPInstance **instanceOut);
#endif

TP_EXPORT const char *TPInstanceGetPath(TPInstance *instance);

#ifdef __APPLE__
TP_EXPORT CGLContextObj TPInstanceGetGLContext(TPInstance *instance);
#else
// TODO: Win32 DX/GL version
#endif

TP_EXPORT TPTimeMode TPInstanceGetTimeMode(TPInstance *instance);

/*
 Rendering
 */

/*
 Initiates rendering of a frame. 
 time_value, time_scale are ignored for TPTimeInternal
 The frame is rendered asynchronously after this function returns.
 TPInstanceEventCallback is called with TPEventFrameDidFinish when frame completes
 */
TP_EXPORT TPResult TPInstanceStartFrameAtTime(TPInstance *instance, int64_t time_value, int32_t time_scale);

/*
 Requests the cancellation of an in-progress frame.
 If the frame was successfully cancelled, TPInstanceEventCallback is called with TPEventFrameDidFinish and TPResultCancelled,
 alternatively the frame may complete as usual before the cancellation request is delivered.
*/
TP_EXPORT TPResult TPInstanceCancelFrame(TPInstance *instance);

/*
 Parameter Layout
 */

/*
 On return children is a list of parameter identifiers for the children of the parent parameter denoted by identifier.
 If identifier is NULL or an empty string, the top level parameters are returned.
 identifier should denote a parameter of type TPParameterTypeGroup or TPParameterTypeComplex.
 The caller is responsible for releasing the returned TPStringArray using TPRelease().
 */
TP_EXPORT TPResult TPInstanceParameterCopyChildren(TPInstance *instance, const char * TP_NULLABLE identifier, struct TPStringArray * TP_NULLABLE * TP_NONNULL children);

/*
 On return groups is a list of parameter identifiers for the top level parameters in the given scope.
 The caller is responsible for releasing the returned TPStringArray using TPRelease().
 */
TP_EXPORT TPResult TPInstanceCopyParameterGroups(TPInstance *instance, TPScope scope, struct TPStringArray * TP_NULLABLE * TP_NONNULL groups);

/*
 Parameter Basics
 */

/*
 On return info describes the parameter denoted by identifier.
 The caller is responsible for releasing the returned TPParameterInfo using TPRelease().
 */
TP_EXPORT TPResult TPInstanceParameterCopyInfo(TPInstance *instance, const char *identifier, TPParameterInfo * TP_NULLABLE * TP_NONNULL info);

/*
 Stream Parameter Configuration
 */

/*
 On return description describes the stream parameter denoted by identifier.
 The caller is responsible for releasing the returned TPStreamDescription using TPRelease().
 */
TP_EXPORT TPResult TPInstanceParameterCopyStreamDescription(TPInstance *instance, const char *identifier, TPStreamDescription * TP_NULLABLE * TP_NONNULL description);

/*
 Configures the input stream denoted by identifier according to the content of description.
 Input streams must have their description set prior to use.
 Changing the description once rendering has begun may result in dropped samples.
 */
TP_EXPORT TPResult TPInstanceParameterSetInputStreamDescription(TPInstance *instance, const char *identifier, const TPStreamDescription *description);

/*
 Getting Parameter Values
 */

TP_EXPORT TPResult TPInstanceParameterGetDoubleValue(TPInstance *instance, const char *identifier, TPParameterValue which, double *value, int32_t count);

TP_EXPORT TPResult TPInstanceParameterGetIntValue(TPInstance *instance, const char *identifier, TPParameterValue which, int32_t *value, int32_t count);

/*
 The caller is responsible for releasing the returned TPString using TPRelease().
 */
TP_EXPORT TPResult TPInstanceParameterCopyStringValue(TPInstance *instance, const char *identifier, TPParameterValue which, TPString * TP_NULLABLE * TP_NONNULL string);

/*
 The caller is responsible for releasing the returned TPTexture using TPRelease().
 */
TP_EXPORT TPResult TPInstanceParameterCopyTextureValue(TPInstance *instance, const char *identifier, TPParameterValue which, TPTexture * TP_NULLABLE * TP_NONNULL value);


/*
 Copies stream samples from an output stream.
 buffers is an array of pointers to count buffers where each buffer receives values for one channel
 Prior to calling this function, set length to the capacity (in samples) of a single channel buffer
 On return, length is set to the actual number of samples copied to each channel buffer
 Calls to this function and TPInstanceParameterAppendStreamValues() can be made at any point after an instance has loaded, including during rendering
*/
TP_EXPORT TPResult TPInstanceParameterGetOutputStreamValues(TPInstance *instance, const char *identifier, float * TP_NONNULL * TP_NONNULL buffers, int32_t count, int64_t *length);

/*
 Setting Input Parameter Values
 */

TP_EXPORT TPResult TPInstanceParameterSetDoubleValue(TPInstance *instance, const char *identifier, const double *value, int32_t count);

TP_EXPORT TPResult TPInstanceParameterSetIntValue(TPInstance *instance, const char *identifier, const int32_t *value, int32_t count);

/*
 Sets the value of a string input parameter.
 value is a UTF-8 encoded string
 */
TP_EXPORT TPResult TPInstanceParameterSetStringValue(TPInstance *instance, const char *identifier, const char * TP_NULLABLE value);

/*
 Sets the value of a texture input parameter
 texture may be retained by the instance
 */
TP_EXPORT TPResult TPInstanceParameterSetTextureValue(TPInstance *instance, const char *identifier, TPTexture *TP_NULLABLE texture);

/*
 Copies stream samples to an input stream.
 buffers is an array of pointers to count buffers where each buffer contains values for one channel
 Prior to calling this function, set length to the size (in samples) of a single channel buffer
 On return, length is set to the actual number of samples copied to each channel buffer
 Calls to this function and TPInstanceParameterGetOutputStreamValues() can be made at any point after an instance has loaded, including during rendering
*/
TP_EXPORT TPResult TPInstanceParameterAppendStreamValues(TPInstance *instance, const char *identifier, const float * TP_NONNULL * TP_NONNULL buffers, int32_t count, int64_t *length);

TP_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif /* TPInstance_h */
