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


#ifndef TEAllocation_h
#define TEAllocation_h

#include <TouchEngine/TEObject.h>
#include <TouchEngine/TEResult.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

TE_ASSUME_NONNULL_BEGIN

typedef TEObject TEAllocation;

typedef TE_ENUM(TEAllocationType, int32_t) 
{
	TEAllocationTypeVulkan,
	TEAllocationTypeD3D,
	TEAllocationTypeMetalBuffer,
	TEAllocationTypeHost
};

/*
 Returns the type of the allocation
 */
TE_EXPORT TEAllocationType TEAllocationGetType(const TEAllocation *allocation);

/*
 Returns the size of the allocation
 */
TE_EXPORT size_t TEAllocationGetSize(const TEAllocation *allocation);

/*
 Host memory allocations - these are created for you by TEHostBuffer
 */

typedef struct TEHostAllocation_ TEHostAllocation;

typedef void (*TEHostAllocationCallback)(const void *, size_t size, TEObjectEvent event, void * TE_NULLABLE info);

/*
 Returns a pointer to the start of the allocation
 */
TE_EXPORT const void *TEHostAllocationGetData(const TEHostAllocation *allocation);

TE_EXPORT TEResult TEHostAllocationSetCallback(TEHostAllocation *allocation, TEHostAllocationCallback TE_NULLABLE callback, void * TE_NULLABLE info);

TE_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif /* TEAllocation_h */
