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

#ifndef TPTypes_h
#define TPTypes_h
#include "TPBase.h"
#include <stdint.h>

typedef TP_ENUM(TPResult, int32_t) 
{

	TPResultSuccess,

	TPResultInsufficientMemory,

	TPResultInternalError,

	TPResultMissingResource,

	/*
	Samples were dropped during rendering due to full output stream buffers.
	*/
	TPResultDroppedSamples,

	TPResultBadUsage,

	TPResultNoMatchingParameter,

	TPResultCancelled,

	/*
	A TouchDesigner/Player key was found, but its update date is older than the build of TouchEngine being used.

	This result should be considered a warning and operation will continue, potentially with limited functionality.
	*/
	TPResultExpiredKey,

	/*
	No TouchDesigner or TouchPlayer key was found.
	*/
	TPResultNoKey,

	/*
	General error trying to determine installed license. Open TouchDesigner to diagnose further.
	*/
	TPResultKeyError,

	TPResultFileError,

	/*
	The version/build of TouchEngine being used is older than the one used to create the file being opened.
	*/
	TPResultVersionError,

	/*
	Unable to find TouchEngine executable to launch.
	*/
	TPResultTouchEngineNotFound,

	/*
	Launching the TouchEngine executable failed.
	*/
	TPResultFailedToLaunchTouchEngine,

	/* 
    Invalid argument given to function.
    */
	TPResultInvalidArgument,

};

typedef TP_ENUM(TPEvent, int32_t) 
{
	TPEventInstanceDidLoad,
	TPEventParameterLayoutDidChange,
	TPEventFrameDidFinish
};

typedef TP_ENUM(TPTimeMode, int32_t) 
{
	TPTimeExternal,
	TPTimeInternal
};

typedef TP_ENUM(TPScope, int32_t) 
{
	TPScopeInput,
	TPScopeOutput
};

typedef TP_ENUM(TPParameterType, int32_t) 
{
	/*
	 Multiple parameters collected according to user preference
	 */
	TPParameterTypeGroup,

	/*
	 Multiple parameters which collectively form a single complex parameter
	 */
	TPParameterTypeComplex,

	/*
	 double
	 */
	TPParameterTypeDouble,

	/*
	 int32_t
	 */
	TPParameterTypeInt,

	/*
	 UTF-8 char *
	 */
	TPParameterTypeString,


	/*
	 TPTexture *
	 */
	TPParameterTypeTexture,

	/*
	 planar float sample buffers
	 */
	TPParameterTypeFloatStream
};

typedef TP_ENUM(TPParameterIntent, int32_t) 
{
	TPParameterIntentNotSpecified,
	TPParameterIntentColorRGBA,
	TPParameterIntentPositionXYZW,
	TPParameterIntentFilePath,
	TPParameterIntentDirectoryPath
};

typedef TP_ENUM(TPParameterValue, int32_t) 
{
	TPParameterValueMinimum,
	TPParameterValueMaximum,
	TPParameterValueDefault,
	TPParameterValueCurrent
};

typedef TP_ENUM(TPTextureType, int32_t) 
{
	TPTextureTypeOpenGL,
	TPTextureTypeD3D,
	TPTextureTypeDXGI
};

#endif /* TPTypes_h */
