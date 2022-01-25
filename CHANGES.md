# Changes

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
