# Changes

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
