/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/


#ifndef TEBuffer_h
#define TEBuffer_h

#include <TouchEngine/TEObject.h>
#include <TouchEngine/TEResult.h>
#include <TouchEngine/TEAllocation.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

TE_ASSUME_NONNULL_BEGIN

typedef TEObject TEBuffer;
typedef TEObject TEAllocation;

typedef TE_ENUM(TEBufferType, int32_t) 
{
	TEBufferTypeVulkan,
	TEBufferTypeD3D,
	TEBufferTypeMetal,
	TEBufferTypeHost
};

/*
 Returns the type of the buffer
 */
TE_EXPORT TEBufferType TEBufferGetType(const TEBuffer *buffer);

/*
 Returns the size of the buffer
 Access to the content of the buffer must only occur within the region defined by
  TEBufferGetOffset() and TEBufferGetSize()
 */
TE_EXPORT size_t TEBufferGetSize(const TEBuffer *buffer);

/*
 Returns the offset of the buffer within the underlying allocation
 Access to the content of the buffer must only occur within the region defined by
  TEBufferGetOffset() and TEBufferGetSize()
 */
TE_EXPORT size_t TEBufferGetOffset(const TEBuffer *buffer);

/*
 Returns the underlying allocation
 */
TE_EXPORT TEAllocation *TEBufferGetAllocation(const TEBuffer *buffer);

/*
 Host memory buffers
 */

typedef struct TEHostBuffer_ TEHostBuffer;
typedef TEHostBuffer TEMutableHostBuffer;

typedef void (*TEHostBufferCallback)(TEHostAllocation *allocation, size_t offset, size_t size, TEObjectEvent event, void * TE_NULLABLE info);

/*
 Returns a pointer to the content of the buffer
  The buffer may be backed by a larger allocation. This is the start of the
   region defined by TEBufferGetOffset() and TEBufferGetSize() and not the
   start of the underlying allocation
 */
TE_EXPORT const void *TEHostBufferGetData(const TEHostBuffer *buffer);

TE_EXPORT TEResult TEHostBufferSetCallback(TEHostBuffer *buffer, TEHostBufferCallback TE_NULLABLE callback, void * TE_NULLABLE info);

/*
 Creates a modifiable buffer in host memory
  'callback' will be called to indicate lifetime events
 */
TE_EXPORT TEMutableHostBuffer *TEMutableHostBufferCreate(size_t size, TEHostBufferCallback TE_NULLABLE callback, void * TE_NULLABLE info);

/*
 Returns a pointer to the content of the buffer, which may be modified
  The buffer may be backed by a larger allocation. This is the start of the
   region defined by TEBufferGetOffset() and TEBufferGetSize() and not the
   start of the underlying allocation
 */
TE_EXPORT void *TEMutableHostBufferGetData(const TEMutableHostBuffer *buffer);

TE_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif /* TEBuffer_h */
