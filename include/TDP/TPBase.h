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

#ifndef TPBase_h
#define TPBase_h

#if defined(__APPLE__)
	#define TP_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
	#define TP_ASSUME_NONNULL_END	_Pragma("clang assume_nonnull end")
	#define TP_NONNULL _Nonnull
	#define TP_NULLABLE _Nullable
	#if !defined(TP_EXPORT)
		#define TP_EXPORT __attribute__((visibility("default")))
	#endif
#else
	#define TP_ASSUME_NONNULL_BEGIN
	#define TP_ASSUME_NONNULL_END
	#define TP_NONNULL
	#define TP_NULLABLE
	#if !defined(TP_EXPORT)
		#if defined (TP_BUILD_DLL)
			#define TP_EXPORT __declspec(dllexport)
		#else
			#define TP_EXPORT __declspec(dllimport)
		#endif
	#endif
#endif

// TODO: This form is supported for C by MSVC and LLVM, may need rethink if we require support for other compilers
#define TP_ENUM(_name, _type) enum _name : _type _name; enum _name : _type

TP_ASSUME_NONNULL_BEGIN

typedef struct TPObject_ TPObject;

/*
 Retains a TPObject, incrementing its reference count. If you retain an object,
 you are responsible for releasing it (using TPRelease()).
 Returns the input value.
 */
#define TPRetain(x) TPRetain_((TPObject *)(x))
TP_EXPORT TPObject *TPRetain_(TPObject *object);

/*
 Releases a TPObject, decrementing its reference count. When the last reference
 to an object is released, it is deallocated and destroyed.
 Sets the passed pointer to NULL.
 */
#define TPRelease(x) TPRelease_((TPObject **)(x))
TP_EXPORT void TPRelease_(TPObject * TP_NULLABLE * TP_NULLABLE object);

TP_ASSUME_NONNULL_END

#endif /* TPBase_h */
