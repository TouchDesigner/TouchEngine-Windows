# Changes

## 2.2

* Fix an issue which caused GPU synchronization to fail for Direct3D 11 hosts on some hardware (there is a corresponding fix in TouchDesigner 2022.31800+)

## 2.1

* Add detailed reporting for component errors via TEInstanceGetErrors()
* Fix an issue which could upset COM state in hosts in particular circumstances
* A subsequent call to TEInstanceConfigure() before a previous configuration has completed now completes the initial configuration with TEResultCancelled
* Other fixes for stability

## 2.0

 * Adds support for Vulkan textures and semaphores (see TEVulkan.h)
 * Adds support for Direct3D 12 textures and fences (see TED3D12.h and TED3D.h)
 * Adds support for Direct3D 11 fences (see TED3D.h)
 * Adds support for macOS (SDK available at https://github.com/TouchDesigner/TouchEngine-macOS)
 * Functionality for each graphics API is no longer included from the umbrella header and should be explicitly included from TED3D11.h, TED3D12.h, TEOpenGL.h, TEMetal.h or TEVulkan.h
 * Replaces 'TETextureIsVerticallyFlipped()' with 'TETextureGetOrigin()'
 * Adds 'TEInstanceSetOutputTextureOrigin()' and 'TEInstanceGetOutputTextureOrigin()'
 * Adds 'TESemaphore' and associated functions (see TESemaphore.h)
 * Removes TEDXGITexture and replaces it with TED3DSharedTexture (see TED3D.h)
 * Exposes synchronization of texture usage with texture transfers (see TEInstance.h and Vulkan-specific transfers in TEVulkan.h)
 * TEObjects which previously had a release callback now have an event callback for richer lifetime event notifications - see TEObjectEvent in TEObject.h
 * Some TEObjects vended by TouchEngine now have configurable callbacks to allow hosts to track resource lifetime
 * Adds TEInstanceLinkSetInterest() to allow hosts to express disinterest in input or output links
 * Adds support for more texture formats, including some compressed formats, and functions to interrogate support (see TEVulkan.h and TED3D.h)
 * Adds TEInstanceLinkHasChoices() to simplify discovery of link menus
 * Numerous fixes for stability

### 2.0 General Changes


#### Windows Libraries

TouchEngine is now distributed as a single DLL. If you have an existing project with libIPM.dll and libTPC.dll alongside TouchEngine.dll, they should be deleted on updating TouchEngine.dll.


#### Resource Lifetime

TouchEngine's performance can be considerably improved by reusing GPU resources.

For textures and semaphore types you supply to TouchEngine, TouchEngine uses the lifetime of the TEObjects you create to manage internal resource lifetime. For example, to improve texture performance, recycle input textures from a pool, keeping the associated TETexture alive for as long as the texture is reused. Callbacks for textures now include a TEObjectEvent which will signal TEObjectEventEndUse when TouchEngine is no longer using the resource - at this point, it can be recycled to the pool. Reusing semaphores brings a similar gain.

For textures and semaphores you receive from TouchEngine as outputs, which will be re-used when possible, you can now register a callback with the TETexture or TESemaphore which allows you to keep associated resources alive until your callback receives TEObjectEventRelease. Note that you must release the TETexture you receive from an output to allow it to be recycled - retain it for as long as you are using it for GPU work, then release it. The TouchEngine example project demonstrates doing this for Direct3D 12.


#### GPU Synchronization

Previously GPU synchronization was managed for you. To accommodate more modern graphics APIs synchronization is now exposed through the concept of texture transfers. Check for support for texture transfers after instance configuration has completed (TEEventInstanceReady).

For transferring textures **to** TouchEngine, the sequence would normally be:

	TEInstanceLinkSetTextureValue(instance, identifier, texture)
	TEInstanceAddTextureTransfer(instance, texture, semaphore, value)
	// GPU signals the semaphore you used in the texture transfer

For transferring textures **from** TouchEngine:

	TEInstanceLinkGetTextureValue(instance, identifier, &texture)
	if (TEInstanceHasTextureTransfer(instance, texture))
	{
		TESemaphore *semaphore;
		uint64_t value;
		TEInstanceLinkGetTextureTransfer(instance, texture, &semaphore, &value)
		// GPU waits for the returned semaphore before using the texture
	}


A TED3D11Context will handle sync of TED3D11Texture inputs for you. You must use texture transfer operations to acquire and release the DXGI keyed mutexes associated with output textures. In this case, the `semaphore` parameter to `TEInstanceAddTextureTransfer()` and `TEInstanceAddTextureTransfer()` will be `NULL` and the wait-value the value to be used for mutex acquire or release.

A TEOpenGLContext will handle sync of TEOpenGLTexture inputs for you. You must use `TEOpenGLTextureLock()` to sync output textures before using them, and `TEOpenGLTextureUnlock()` to release them when you are finished with them.


#### Vulkan

To opt in to using Vulkan texture and semaphore types, associate a TEVulkanContext with an instance prior to configuring it, using `TEInstanceAssociateGraphicsContext()`.

Vulkan is only supported when TouchDesigner 2022 or later is installed. Check for support using `TEInstanceGetSupportedTextureTypes()` and `TEInstanceGetSupportedSemaphoreTypes()` after instance configuration is complete.

For Vulkan texture transfers, see TEVulkan.h which deals with the image layouts required for image memory barriers - when performing a texture transfer for a texture whose contents you wish to preserve, do the full Vulkan image transfer including performing the memory barrier operation. When texture content is disposable, use the regular texture transfer mechanism from TEInstance.h.

TouchEngine provides you with the importable handles for Vulkan GPU resources and it is up to the host to instantiate these objects and manage their lifetime. Use the advice in "Resource Lifetime" above to allow resources to be recycled by the host and TouchEngine.


#### Direct3D 12

To opt in to using D3D12 textures and fences, associate a TED3D12Context with an instance prior to configuring it, using `TEInstanceAssociateGraphicsContext()`.

D3D12 is only supported when TouchDesigner 2022.28580 or later is installed. Check for support using `TEInstanceGetSupportedD3DHandleTypes()` and `TEInstanceGetSupportedSemaphoreTypes()` after instance configuration is complete.

TouchEngine provides you with the importable handles for D3D12 GPU resources (via TED3DSharedTexture and TED3DSharedFence) and it is up to the host to instantiate the D3D objects and manage their lifetime. Use the advice in "Resource Lifetime" above to allow resources to be recycled by the host and TouchEngine.


#### Direct3D 11

If you use TED3DSharedTexture for inputs or to instantiate outputs yourself, DXGI shared texture handles (eg from a D3D11 texture) may be backed by a DXGI Keyed Mutex. Due to limitations in some versions of TouchDesigner, textures transferred **to** TouchEngine must be released to the value 0 - use `TEInstanceRequiresDXGIReleaseToZero()` to check after instance configuration has completed. To perform a texture transfer for a DXGI Keyed Mutex, pass a `NULL` semaphore to `TEInstanceAddTextureTransfer()`, passing the value you have released the mutex to as the wait value. `TEInstanceLinkGetTextureTransfer()` will return a `NULL` semaphore and a wait value which you must use when acquiring the DXGI Keyed Mutex.


## 1.9

 * Fix a crash which could occur on systems with TouchDesigner 2022 installed.

## 1.8

 * Fix issue which could cause `TEInstanceAssociateGraphicsContext()` or `TEInstanceAssociateGraphicsContext()` to ignore device selection on multi-GPU machines
 * Where a link has TELinkIntentPulse, setting `false` will not cancel a previous `true` for the current frame
 * When a link with TELinkIntentPulse is set to true, it will revert to `false` immediately afterwards

## 1.7

* Fix a performance issue which affected texture inputs.

## 1.6

* Fix an issue which could cause a long delay before reporting an error in some situations.
 
## 1.5

* Add `TEInstanceConfigure()` and change arguments to `TEInstanceLoad()`
* Add `TEEventInstanceReady` and `TEEventInstanceDidUnload`
* `TEInstanceGetPath()` now returns a `TEString` which the caller should release

## 1.4

* Add `TEInstanceSetStatisticsCallback()`.

## 1.3

 * Add `TETextureComponentMap` to support swizzled texture formats.
 * Fixes for working directly with DXGI textures.

## 1.2

### Fixes

 * Passing a null texture to `TEInstanceLinkSetTextureValue()` correctly clears any previous texture.
 * `TEInstanceHasFile()` correctly returns false after an unload following an unrecoverable error.

## 1.1

### Fixes

 * Fix a crash which could occur using `TEInstance...` functions from an unload callback after the final TERelease of an instance.
