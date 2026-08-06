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


#ifndef TED3D_h
#define TED3D_h

#include <TouchEngine/TETexture.h>
#include <TouchEngine/TESemaphore.h>
#include <TouchEngine/TEInstance.h>

#ifdef __cplusplus
extern "C" {
#endif

TE_ASSUME_NONNULL_BEGIN

#ifdef _WIN32

#include <dxgiformat.h>

typedef TE_ENUM(TED3DHandleType, int32_t) 
{
	/*
	 A global handle from a call to IDXGIResource::GetSharedHandle()
	 */
	TED3DHandleType11Global,
	/*
	 A NT handle from a call to IDXGIResource1::CreateSharedHandle()
	 */
	TED3DHandleType11NT,

	/*
	A NT handle referring to a committed resource from a call to ID3D12Device::CreateSharedHandle()
	 */
	TED3DHandleType12CommittedNT,

	/*
	A NT handle referring to a heap resource from a call to ID3D12Device::CreateSharedHandle()
	 */
	TED3DHandleType12HeapNT,

	/*
	 Deprecated names
	 */
	TED3DHandleTypeD3D11Global = TED3DHandleType11Global,
	TED3DHandleTypeD3D11NT = TED3DHandleType11NT,
	TED3DHandleTypeD3D12ResourceNT = TED3DHandleType12CommittedNT
};


/*
 Returns via 'types' the TED3DHandleType supported by the instance.
 This may change during configuration of an instance, and must be queried after receiving TEEventInstanceDidConfigure
 'types' is an array of TED3DHandleType, or NULL, in which case the value at counts is set to the number of available types
 'count' is a pointer to an int32_t which should be set to the number of available elements in 'types'.
 If this function returns TEResultSuccess, 'count' is set to the number of TED3DHandleTypes filled in 'types'
 If this function returns TEResultInsufficientMemory, the value at 'count' was too small to return all the types, and
 	'count' has been set to the number of available types. Resize 'types' appropriately and call the function again to
 	retrieve the full array of types. 
 Where multiple types are returned, they may be selected for output by association of a suitable TEGraphicsContext
 */
TE_EXPORT TEResult TEInstanceGetSupportedD3DHandleTypes(TEInstance *instance, TED3DHandleType types[TE_NULLABLE], int32_t *count);

/*
 Returns via 'formats' the DXGI_FORMATs supported by the instance.
 This may change during configuration of an instance, and must be queried after receiving TEEventInstanceDidConfigure
 'formats' is an array of DXGI_FORMAT, or NULL, in which case the value at counts is set to the number of available formats
 'count' is a pointer to an int32_t which should be set to the number of available elements in 'formats'.
 If this function returns TEResultSuccess, 'count' is set to the number of DXGI_FORMATs filled in 'formats'
 If this function returns TEResultInsufficientMemory, the value at 'count' was too small to return all the formats, and
 	'count' has been set to the number of available formats. Resize 'formats' appropriately and call the function again to
 	retrieve the full array of formats. 
 */
TE_EXPORT TEResult TEInstanceGetSupportedD3DFormats(TEInstance *instance, DXGI_FORMAT formats[TE_NULLABLE], int32_t *count);

typedef struct TED3DSharedTexture_ TED3DSharedTexture;

/*
 D3D Shared Allocations - see TEAllocation.h for functions common to all allocations
 */

typedef struct TED3DAllocation_ TED3DAllocation;

typedef void (*TED3DAllocationCallback)(HANDLE allocation, size_t size, TEObjectEvent event, void * TE_NULLABLE info);

/*
 Create an allocation from a D3D allocation
'handle' is the HANDLE for the exportable memory
	If the handle is a NT handle, it will be duplicated and TouchEngine will manage its lifetime - consequently the value returned from
	TED3DAllocationGetHandle() will not match this value
	If the handle is not an NT handle, the underlying resource must remain valid until receiving TEObjectEventRelease via 'callback'
'type' is one of the TED3DHandleTypes
'size' is the size of the allocation for TED3DHandleType12HeapNT, otherwise it will be ignored
'callback' will be invoked for object use and lifetime events - see TEObjectEvent in TEObject.h
'info' will be passed as the 'info' argument when 'callback' is invoked
The caller is responsible for releasing the returned TED3DAllocation using TERelease()
*/
TE_EXPORT TED3DAllocation *TED3DAllocationCreate(HANDLE handle, TED3DHandleType type, size_t size, TED3DAllocationCallback TE_NULLABLE callback, void * TE_NULLABLE info);

/*
 Returns the HANDLE to be imported to instantiate a Vulkan allocation.
 For allocations you have received from TouchEngine, and all allocations backed by NT handles, the lifetime
 	of this handle is managed by TouchEngine and it should not be passed to a call to CloseHandle()
 */
TE_EXPORT HANDLE TED3DAllocationGetHandle(const TED3DAllocation *allocation);

/*
 Returns the type of the allocation
 */
TE_EXPORT TED3DHandleType TED3DAllocationGetHandleType(const TED3DAllocation *allocation);

/*
 Sets 'callback' to be invoked for object use and lifetime events - see TEObjectEvent in TEObject.h.
 This replaces (or cancels) any callback previously set on the TED3DAllocation
 */
TE_EXPORT TEResult TED3DAllocationSetCallback(TED3DAllocation *allocation, TED3DAllocationCallback callback, void *info);

/*
 D3D Shared Textures - see TETexture.h for functions common to all textures
 */

/*
 Event callback for D3D Shared Textures
 */
typedef void (*TED3DSharedTextureCallback)(TED3DAllocation *allocation, size_t offset, TEObjectEvent event, void * TE_NULLABLE info);

/*
Create a texture from a shared handle
'allocation' a TED3DAllocation backing the texture
'heapOffset' if the texture is backed by a D3D12 heap, this is an offset into the heap
	for allocations other than TED3DHandleType12HeapNT this value is ignored
'origin' is the position in 2D space of the 0th texel of the texture
'map' describes how components are to be mapped when the texture is read. If components are not swizzled, you
	can pass kTETextureComponentMapIdentity
'callback' will be invoked for object use and lifetime events - see TEObjectEvent in TEObject.h
The caller is responsible for releasing the returned TED3DSharedTexture using TERelease()
*/
TE_EXPORT TED3DSharedTexture *TED3DSharedTextureCreate(TED3DAllocation *allocation,
											size_t heapOffset,
											DXGI_FORMAT format,
											uint64_t width,
											uint32_t height,
											TETextureOrigin origin,
											TETextureComponentMap map,
											TED3DSharedTextureCallback TE_NULLABLE callback,
											void * TE_NULLABLE info);

TE_EXPORT TED3DAllocation *TED3DSharedTextureGetAllocation(const TED3DSharedTexture *texture);

/*
 Returns the offset into the allocation backing this texture
 	For allocations other than TED3DHandleType12HeapNT this value will be 0 
 */
TE_EXPORT size_t TED3DSharedTextureGetHeapOffset(const TED3DSharedTexture *texture);

/*
 Returns the width of the texture, if known
 	Some older TouchDesigner installations may output textures which report a width and height of 0 and a format
 	of DXGI_FORMAT_UNKNOWN - these properties can be discovered from the instantiated Direct3D texture
 */
TE_EXPORT uint64_t TED3DSharedTextureGetWidth(const TED3DSharedTexture *texture);

/*
 Returns the height of the texture, if known
 	Some older TouchDesigner installations may output textures which report a width and height of 0 and a format
 	of DXGI_FORMAT_UNKNOWN - these properties can be discovered from the instantiated Direct3D texture
 */
TE_EXPORT uint32_t TED3DSharedTextureGetHeight(const TED3DSharedTexture *texture);

/*
 Returns the format of the texture, if known
 	Some older TouchDesigner installations may output textures which report a width and height of 0 and a format
 	of DXGI_FORMAT_UNKNOWN - these properties can be discovered from the instantiated Direct3D texture
 */
TE_EXPORT DXGI_FORMAT TED3DSharedTextureGetFormat(const TED3DSharedTexture *texture);

/*
 Sets 'callback' to be invoked for object use and lifetime events - see TEObjectEvent in TEObject.h.
 This replaces (or cancels) any callback previously set on the TED3DSharedTexture
 */
TE_EXPORT TEResult TED3DSharedTextureSetCallback(TED3DSharedTexture *texture, TED3DSharedTextureCallback TE_NULLABLE callback, void * TE_NULLABLE info);

/*
 D3D Buffers - see TEBuffer.h for functions common to all buffer types
 */

typedef struct TED3DSharedBuffer_ TED3DSharedBuffer;

typedef void (*TED3DSharedBufferCallback)(TED3DAllocation *allocation, size_t offset, size_t size, TEObjectEvent event, void * TE_NULLABLE info);

/*
 Create a D3D buffer
 'allocation' is a TED3DAllocation backing the buffer
 'offset' is the offset into the allocation for the start of this buffer
 'size' is the size of the buffer
 'callback' will be invoked for object use and lifetime events - see TEObjectEvent in TEObject.h
 The caller is responsible for releasing the returned TED3DSharedBuffer using TERelease()
 */
TE_EXPORT TED3DSharedBuffer *TED3DSharedBufferCreate(TED3DAllocation *allocation, size_t offset, size_t size,
														TED3DSharedBufferCallback TE_NULLABLE callback, void * TE_NULLABLE info);

/*
 Sets 'callback' to be invoked for object use and lifetime events - see TEObjectEvent in TEObject.h.
 This replaces (or cancels) any callback previously set on the TED3DSharedBuffer
 */
TE_EXPORT TEResult TED3DSharedBufferSetCallback(TED3DSharedBuffer *buffer, TED3DSharedBufferCallback TE_NULLABLE callback, void * TE_NULLABLE info);


/*
 D3D Fences - see TESemaphore.h for functions common to all semaphore types
 */

typedef struct TED3DSharedFence_ TED3DSharedFence;

typedef void (*TED3DSharedFenceCallback)(HANDLE handle, TEObjectEvent event, void * TE_NULLABLE info);

/*
 Creates a fence from a handle exported from a Direct3D 11 or 12 fence

 'handle' is a HANDLE exported from a Direct3D 11 or 12 fence object (using ID3D11Fence::CreateSharedHandle() or
 	ID3D12Device::CreateSharedHandle())
	The handle will be duplicated and TouchEngine will manage its lifetime - consequently the value returned from
	TED3DSharedFenceGetHandle() will not match this value
'callback' will be invoked for object use and lifetime events - see TEObjectEvent in TEObject.h
The caller is responsible for releasing the returned TED3DSharedFence using TERelease()
 */
TE_EXPORT TED3DSharedFence *TED3DSharedFenceCreate(HANDLE handle, TED3DSharedFenceCallback callback, void * TE_NULLABLE info);

/*
 Returns the HANDLE associated with the fence. The lifetime of this handle is managed by TouchEngine and it should not be passed
 to a call to CloseHandle().
 */
TE_EXPORT HANDLE TED3DSharedFenceGetHandle(TED3DSharedFence *fence);

/*
 Sets 'callback' to be invoked for object use and lifetime events - see TEObjectEvent in TEObject.h.
 This replaces (or cancels) any callback previously set on the TED3DSharedFence
 */
TE_EXPORT TEResult TED3DSharedFenceSetCallback(TED3DSharedFence *fence, TED3DSharedFenceCallback TE_NULLABLE callback, void * TE_NULLABLE info);

#endif // _WIN32

TE_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif // TED3D_h
