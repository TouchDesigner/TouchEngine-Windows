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
	#include <OpenGL/OpenGL.h>
#endif
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

TE_ASSUME_NONNULL_BEGIN

typedef TEObject TEInstance;

#ifdef _WIN32
struct ID3D11Device;
typedef struct HGLRC__ *HGLRC;
typedef struct HDC__ *HDC;
#endif

typedef void (*TEInstanceEventCallback)(TEInstance *instance, TEEvent event, TEResult result, int64_t time_value, int32_t time_scale, void * TE_NULLABLE info);
typedef void (*TEInstanceParameterValueCallback)(TEInstance *instance, const char *identifier, void *info);


/*
 On return, extensions is a list of file extensions supported by TEInstanceCreate
 The caller is responsible for releasing the returned TEStringArray using TERelease()
 */
TE_EXPORT TEResult TEInstanceGetSupportedFileExtensions(struct TEStringArray * TE_NULLABLE * TE_NONNULL extensions);

/*
 Creates an instance for a .tox file located at 'path'.
 'path' is a UTF-8 encoded string. The file is loaded asynchronously after this function returns.
 'context' // TODO: 
 'mode' see TETimeMode in TETypes.h
 'event_callback' will be called to deliver TEEvents related to loading and rendering the instance.
 'callback_info' will be passed into the callbacks as the 'info' argument.
 'instance' will be set to a TEInstance on return, or NULL if an instance could not be created.
	The caller is responsible for releasing the returned TEInstance using TERelease()
 Returns TEResultSucccess or an error
 */
// TODO: TETimeMode is currently ignored
#ifdef __APPLE__

TE_EXPORT TEResult TEInstanceCreateGL(const char *path,
									  CGLContextObj context,
									  TETimeMode mode,
									  TEInstanceEventCallback event_callback,
									  TEInstanceParameterValueCallback prop_value_callback,
									  void * TE_NULLABLE callback_info,
									  TEInstance * TE_NULLABLE * TE_NONNULL instance);

TE_EXPORT CGLContextObj TEInstanceGetGLContext(TEInstance *instance);

#endif

#ifdef _WIN32

/*
 Creates an instance for a .tox file located at 'path'.
 Texture parameter values will be suitable for use with Direct3D:
 	A TED3DTexture or TEDXGITexture can be set on input parameters
	output parameters will emit a TEDXGITexture (which can be used to instantiate a TED3DTexture)
 
 'path' is a UTF-8 encoded string. The file is loaded asynchronously after this function returns.
 'device' is the Direct3D device to be used for texture creation.
 	If texture input and outputs will not be used, this value may be NULL.
 'mode' see TETimeMode in TETypes.h
 'event_callback' will be called to deliver TEEvents related to loading and rendering the instance.
 'callback_info' will be passed into the callbacks as the 'info' argument.
 'instance' will be set to a TEInstance on return, or NULL if an instance could not be created.
	The caller is responsible for releasing the returned TEInstance using TERelease()
 Returns TEResultSucccess or an error
 */
// TODO: TETimeMode is currently ignored
TE_EXPORT TEResult TEInstanceCreateD3D(const char *path,
									   ID3D11Device * TE_NULLABLE device,
									   TETimeMode mode,
									   TEInstanceEventCallback event_callback,
									   TEInstanceParameterValueCallback prop_value_callback,
									   void * TE_NULLABLE callback_info,
									   TEInstance * TE_NULLABLE * TE_NONNULL instance);

/*
 Creates an instance for a .tox file located at 'path'.
 Texture parameter values will be suitable for use with OpenGL:
 	Any TETexture type can be set on input parameters
	output parameters will emit a TEOpenGLTexture
 
 'path' is a UTF-8 encoded string. The file is loaded asynchronously after this function returns.
 'dc' is a valid device context to be used for OpenGL commands
	This value can be changed later using TEInstanceSetGLContext()
 'rc' is a valid OpenGL render context to be used for OpenGL commands
	This value can be changed later using TEInstanceSetGLContext()	
 'event_callback' will be called to deliver TEEvents related to loading and rendering the instance.
 'callback_info' will be passed into the callbacks as the 'info' argument.
 'instance' will be set to a TEInstance on return, or NULL if an instance could not be created.
	The caller is responsible for releasing the returned TEInstance using TERelease()
 Returns TEResultSucccess or an error
 */
// TODO: TETimeMode is currently ignored
TE_EXPORT TEResult TEInstanceCreateGL(const char *path,
									  HDC dc,
									  HGLRC rc,
									  TETimeMode mode,
									  TEInstanceEventCallback event_callback,
									  TEInstanceParameterValueCallback prop_value_callback,
									  void * TE_NULLABLE callback_info,
									  TEInstance * TE_NULLABLE * TE_NONNULL instance);

/*
Returns the ID3D11Device associated with a Direct3D instance, or NULL.
*/
TE_EXPORT ID3D11Device *TEInstanceGetD3DDevice(TEInstance *instance);

/*
Change the device associated with a Direct3D instance.
'device' must be a valid Direct3D device.
Returns TEResultSuccess on success, or an error.
*/
TE_EXPORT TEResult TEInstanceSetD3DDevice(TEInstance *instance, ID3D11Device *device);

/*
Returns the device context associated with an OpenGL instance, or NULL.
*/
TE_EXPORT HDC TEInstanceGetGLDC(TEInstance *instance);

/*
Returns the OpenGL render context associated with an OpenGL instance, or NULL.
*/
TE_EXPORT HGLRC TEInstanceGetGLRC(TEInstance *instance);

/*
Change the device and/or render contexts used by an OpenGL instance.
If the render context changes, this function may do work in both the previously set and new render contexts.
Returns TEResultSuccess on success, or an error.
*/
TE_EXPORT TEResult TEInstanceSetGLContext(TEInstance *instance, HDC dc, HGLRC rc);

#endif

TE_EXPORT const char *TEInstanceGetPath(TEInstance *instance);

TE_EXPORT TETimeMode TEInstanceGetTimeMode(TEInstance *instance);

/*
 Rendering
 */

/*
 Initiates rendering of a frame. 
 'time_value', 'time_scale' are ignored for TETimeInternal
 'discontinuity' if true indicates the frame does not follow naturally from the previously requested frame
 The frame is rendered asynchronously after this function returns.
 TEInstanceParameterValueCallback is called for any outputs affected by the rendered frame.
 TEInstanceEventCallback is called with TEEventFrameDidFinish when the frame completes.
 */
TE_EXPORT TEResult TEInstanceStartFrameAtTime(TEInstance *instance, int64_t time_value, int32_t time_scale, bool discontinuity);

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
 On return 'children' is a list of parameter identifiers for the children of the parent parameter denoted by 'identifier'.
 If 'identifier' is NULL or an empty string, the top level parameters are returned.
 'identifier' should denote a parameter of type TEParameterTypeGroup or TEParameterTypeComplex.
 The caller is responsible for releasing the returned TEStringArray using TERelease().
 */
TE_EXPORT TEResult TEInstanceParameterGetChildren(TEInstance *instance, const char * TE_NULLABLE identifier, struct TEStringArray * TE_NULLABLE * TE_NONNULL children);

/*
 On return 'groups' is a list of parameter identifiers for the top level parameters in the given scope.
 The caller is responsible for releasing the returned TEStringArray using TERelease().
 */
TE_EXPORT TEResult TEInstanceGetParameterGroups(TEInstance *instance, TEScope scope, struct TEStringArray * TE_NULLABLE * TE_NONNULL groups);

/*
 Parameter Basics
 */

/*
 On return 'info' describes the parameter denoted by 'identifier'.
 The caller is responsible for releasing the returned TEParameterInfo using TERelease().
 */
TE_EXPORT TEResult TEInstanceParameterGetInfo(TEInstance *instance, const char *identifier, TEParameterInfo * TE_NULLABLE * TE_NONNULL info);

/*
 Stream Parameter Configuration
 A stream is an array of float values that are buffered both on input and output
 from the Engine. They are used to pass data such as motion or audio data
 that needs continuity in it's data.
 */

/*
 On return 'description' describes the stream parameter denoted by 'identifier'.
 The caller is responsible for releasing the returned TEStreamDescription using TERelease().
 */
TE_EXPORT TEResult TEInstanceParameterGetStreamDescription(TEInstance *instance, const char *identifier, TEStreamDescription * TE_NULLABLE * TE_NONNULL description);

/*
 Configures the input stream denoted by 'identifier' according to the content of 'description'.
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
 'buffers' is an array of pointers to 'count' buffers where each buffer receives values for one channel
 Prior to calling this function, set 'length' to the capacity (in samples) of a single channel buffer
 On return, 'length' is set to the actual number of samples copied to each channel buffer
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
 'value' is a UTF-8 encoded string
 */
TE_EXPORT TEResult TEInstanceParameterSetStringValue(TEInstance *instance, const char *identifier, const char * TE_NULLABLE value);

/*
 Sets the value of a texture input parameter
 'texture' may be retained by the instance
 Work may be done in the graphics context associated with the instance by this call.
 An OpenGL instance may change the current framebuffer binding during this call.
 */
TE_EXPORT TEResult TEInstanceParameterSetTextureValue(TEInstance *instance, const char *identifier, TETexture *TE_NULLABLE texture);

/*
 Copies stream samples to an input stream.
 'buffers' is an array of pointers to 'count' buffers where each buffer contains values for one channel
 Prior to calling this function, set 'length' to the size (in samples) of a single channel buffer
 On return, 'length' is set to the actual number of samples copied to each channel buffer
 Calls to this function and TEInstanceParameterGetOutputStreamValues() can be made at any point after an instance has loaded, including during rendering
*/
TE_EXPORT TEResult TEInstanceParameterAppendStreamValues(TEInstance *instance, const char *identifier, const float * TE_NONNULL * TE_NONNULL buffers, int32_t count, int64_t *length);

TE_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif /* TEInstance_h */
