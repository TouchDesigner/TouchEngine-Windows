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

#ifndef TETypes_h
#define TETypes_h
#include "TEBase.h"
#include <stdint.h>

typedef TE_ENUM(TEResult, int32_t) 
{

	TEResultSuccess,

	TEResultInsufficientMemory,

	TEResultInternalError,

	TEResultMissingResource,

	/*
	Samples were dropped during rendering due to full output stream buffers.
	*/
	TEResultDroppedSamples,

	TEResultBadUsage,

	TEResultNoMatchingParameter,

	TEResultCancelled,

	/*
	A TouchDesigner/Player key was found, but its update date is older than the build of TouchEngine being used.

	This result should be considered a warning and operation will continue, potentially with limited functionality.
	*/
	TEResultExpiredKey,

	/*
	No TouchDesigner or TouchPlayer key was found.
	*/
	TEResultNoKey,

	/*
	General error trying to determine installed license. Open TouchDesigner to diagnose further.
	*/
	TEResultKeyError,

	TEResultFileError,

	/*
	The version/build of TouchEngine being used is older than the one used to create the file being opened.
	*/
	TEResultVersionError,

	/*
	Unable to find TouchEngine executable to launch.
	*/
	TEResultTouchEngineNotFound,

	/*
	Launching the TouchEngine executable failed.
	*/
	TEResultFailedToLaunchTouchEngine,

	/* 
    Invalid argument given to function.
    */
	TEResultInvalidArgument,

};

typedef TE_ENUM(TEEvent, int32_t) 
{
	TEEventInstanceDidLoad,
	TEEventParameterLayoutDidChange,
	TEEventFrameDidFinish
};

typedef TE_ENUM(TETimeMode, int32_t) 
{
	TETimeExternal,
	TETimeInternal
};

typedef TE_ENUM(TEScope, int32_t) 
{
	TEScopeInput,
	TEScopeOutput
};

typedef TE_ENUM(TEParameterType, int32_t) 
{
	/*
	 Multiple parameters collected according to user preference
	 */
	TEParameterTypeGroup,

	/*
	 Multiple parameters which collectively form a single complex parameter
	 */
	TEParameterTypeComplex,

	/*
	 double
	 */
	TEParameterTypeDouble,

	/*
	 int32_t
	 */
	TEParameterTypeInt,

	/*
	 UTF-8 char *
	 */
	TEParameterTypeString,


	/*
	 TETexture *
	 */
	TEParameterTypeTexture,

	/*
	 planar float sample buffers
	 */
	TEParameterTypeFloatStream
};

typedef TE_ENUM(TEParameterIntent, int32_t) 
{
	TEParameterIntentNotSpecified,
	TEParameterIntentColorRGBA,
	TEParameterIntentPositionXYZW,
	TEParameterIntentFilePath,
	TEParameterIntentDirectoryPath
};

typedef TE_ENUM(TEParameterValue, int32_t) 
{
	TEParameterValueMinimum,
	TEParameterValueMaximum,
	TEParameterValueDefault,
	TEParameterValueCurrent
};

typedef TE_ENUM(TETextureType, int32_t) 
{
	TETextureTypeOpenGL,
	TETextureTypeD3D,
	TETextureTypeDXGI
};

#endif /* TETypes_h */
