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


#ifndef TEGeometry_h
#define TEGeometry_h

#include <TouchEngine/TEObject.h>
#include <TouchEngine/TEResult.h>
#include <TouchEngine/TEStrings.h>

#ifdef __cplusplus
extern "C" {
#endif

TE_ASSUME_NONNULL_BEGIN

typedef struct TEGeometry_ TEGeometry;
typedef TEGeometry TEMutableGeometry;
typedef TEObject TEBuffer;

typedef TE_ENUM(TEGeometryScope, int32_t) 
{
	TEGeometryScopePoints,
	TEGeometryScopeVertices,
	TEGeometryScopePrimitives
};

typedef TE_ENUM(TEGeometryAttributeType, int32_t)
{
	/*
	 float
	 */
	TEGeometryAttributeTypeFloat,

	/*
	 double
	 */
	TEGeometryAttributeTypeDouble,

	/*
	 int32_t
	 */
	TEGeometryAttributeTypeInt32,

	/*
	 uint32_t
	 */
	TEGeometryAttributeTypeUInt32
};

typedef TE_ENUM(TEGeometryPrimitiveType, int32_t)
{
	TEGeometryPrimitiveTypeTriangles,
	TEGeometryPrimitiveTypeQuads,
	TEGeometryPrimitiveTypeLineStrips,
	TEGeometryPrimitiveTypeLines,
	TEGeometryPrimitiveTypePoints 
};

typedef TE_ENUM(TEGeometryAttributeIntent, int32_t)
{
	TEGeometryAttributeIntentNone,
	TEGeometryAttributeIntentDirection,
	TEGeometryAttributeIntentTransform,
	TEGeometryAttributeIntentColor,
	TEGeometryAttributeIntentQuaternion
};

typedef TE_ENUM(TEGeometryBufferID, int32_t)
{
	/*
	 The contents is an array of uint32_t being indices into the attribute buffers for TEGeometryScopePoints
	 */
	TEGeometryBufferIDIndex,

	/*
	 The contents is a single uint32_t being the total number of points
	 */
	TEGeometryBufferIDPointsCount,

	/*
	 The contents is a single TEGeometryTopology struct
	 */
	TEGeometryBufferIDTopology,

	/*
	 An array of TEGeometryLineStripInfo, one for each line strip
	 */
	TEGeometryBufferIDLineStripInfo,

	/*
	 The contents is an array of uint32_t being the index of the associated line strip primitive for each
	  line strip index
	 */
	TEGeometryBufferIDLineStripIndex
};

struct TEGeometryAttributeInfo
{
	const char *				name;
	TEGeometryAttributeType		type;
	TEGeometryAttributeIntent	intent;
	uint32_t					components;
	uint32_t					columns;

	/*
	 The attribute is an array of 'arrayCount' repeated values, or 0 if not an array
	 */
	uint32_t					arrayCount;

	/*
	 Bytes before the first attribute data
	 */
	uint32_t					offset;

	/*
	 Bytes between the start of an attribute and the start of the next point's data for this attribute
	 */
	uint32_t					stride;

	/*
	 Bytes between the start of an element in the array and the start of the next element
	*/
	uint32_t					arrayStride;
};

struct TEGeometryTopology
{
	/*
	 The offset (in indices) from the start of the index buffer
	 to the first triangle primitive
	 Triangle primitives must be ordered first in the index buffer
	 */
	uint32_t trianglesOffset;
	/*
	 The number of triangles in the index buffer
	 */
	uint32_t trianglesCount;

	/*
	 The offset (in indices) from the start of the index buffer
	 to the first quad primitive
	 Quad primitives must be ordered after triangle primitives in the index buffer
	 */
	uint32_t quadsOffset;
	/*
	 The number of quads in the index buffer
	 */
	uint32_t quadsCount;

	/*
	 The offset (in indices) from the start of the index buffer
	 to the first line strip primitive
	 Line strip primitives must be ordered after quad primitives in the index buffer
	 */
	uint32_t lineStripsOffset;
	/*
	 The number of line strips in the index buffer
	 */
	uint32_t lineStripsCount;
	/*
	 The number of line strip vertices in the index buffer
	 */
	uint32_t lineStripsVertexCount;

	/*
	 The offset (in indices) from the start of the index buffer
	 to the first line primitive
	 Line primitives must be ordered after line strip primitives in the index buffer
	 */
	uint32_t linesOffset;
	/*
	 The number of lines in the index buffer
	 */
	uint32_t linesCount;

	/*
	 The offset (in indices) from the start of the index buffer
	 to the first point primitive
	 */
	uint32_t pointPrimsOffset;
	/*
	 The number of point primitives in the index buffer
	 Point primitives must be ordered after line primitives in the index buffer
	 */
	uint32_t pointPrimsCount;
};

struct TEGeometryLineStripInfo {
	/*
	 The offset (in indices) from the start of the index buffer to
	 the first vertex of this line strip
	 */
	uint32_t offset;

	/*
	 The number of vertices in this line strip
	 */
	uint32_t count;
};

TE_EXPORT int32_t TEGeometryGetAttributeCount(const TEGeometry *geometry, TEGeometryScope scope);

/*
 On successful return, sets 'info' to a TEGeometryAttributeInfo for the attribute identified by 'scope' and 'index'
  The caller is responsible for releasing the returned TEGeometryAttributeInfo using TERelease()
 Returns TEResultSucccess or an error
 */
TE_EXPORT TEResult TEGeometryGetAttributeInfo(const TEGeometry *geometry,
											TEGeometryScope scope,
											int32_t index,
											struct TEGeometryAttributeInfo * TE_NULLABLE * TE_NONNULL info);

/*
 On successful return, sets 'info' to a TEGeometryAttributeInfo for the attribute identified by 'scope' and 'name'
  The caller is responsible for releasing the returned TEGeometryAttributeInfo using TERelease()
 Returns TEResultSucccess or an error
 */
TE_EXPORT TEResult TEGeometryGetAttributeInfoByName(const TEGeometry *geometry,
													TEGeometryScope scope,
													const char *name,
													struct TEGeometryAttributeInfo * TE_NULLABLE * TE_NONNULL info);

/*
 Returns true if the attribute layout of 'a' and 'b' are equal, otherwise false
 */
TE_EXPORT bool TEGeometryGetAttributeInfoEqual(const TEGeometry *a, const TEGeometry *b);

/*
 Returns the maximum possible number of elements per attribute for 'scope'
 */
TE_EXPORT uint32_t TEGeometryGetMaxElements(const TEGeometry *a, TEGeometryScope scope);

/*
 Returns the maximum possible number of vertices for 'type'
 */
TE_EXPORT uint32_t TEGeometryGetMaxVertices(const TEGeometry *geometry, TEGeometryPrimitiveType type);

/*
 Gets the maximum possible number of primitives of 'type'
  This may be equal to or greater than the entry in the topology buffer
 */
TE_EXPORT uint32_t TEGeometryGetMaxPrimitives(const TEGeometry *geometry, TEGeometryPrimitiveType type);

/*
 On successful return, sets 'buffer' to a TEBuffer for the attribute identified by 'scope' and 'index'
  The caller is responsible for releasing the returned TEBuffer using TERelease()
 Returns TEResultSucccess or an error
 */
TE_EXPORT TEResult TEGeometryGetAttributeBuffer(const TEGeometry *geometry, TEGeometryScope scope, int32_t index, TEBuffer * TE_NULLABLE * TE_NONNULL buffer);

/*
 On successful return, sets 'buffer' to a TEBuffer for the attribute identified by 'scope' and 'name'
  The caller is responsible for releasing the returned TEBuffer using TERelease()
 Returns TEResultSucccess or an error
 */
TE_EXPORT TEResult TEGeometryGetAttributeBufferByName(const TEGeometry *geometry,
													TEGeometryScope scope,
													const char *name,
													TEBuffer * TE_NULLABLE * TE_NONNULL buffer);

/*
 On successful return, sets 'buffer' to a TEBuffer for the buffer identified by 'ident'
  The caller is responsible for releasing the returned TEBuffer using TERelease()
 Returns TEResultSucccess or an error
 */
TE_EXPORT TEResult TEGeometryGetBuffer(const TEGeometry *geometry, TEGeometryBufferID ident, TEBuffer * TE_NULLABLE * TE_NONNULL buffer);

/*
 On successful return, sets 'names' to a TEStringArray with the names of any attribute groups
  There may be empty names in the TEStringArray, indicating no group for that index
  The caller is responsible for releasing the returned TEStringArray using TERelease().
 */
TE_EXPORT TEResult TEGeometryGetGroupNames(const TEGeometry *geometry, TEGeometryScope scope, struct TEStringArray * TE_NULLABLE * TE_NONNULL names);

/*
 On successful return, sets 'buffer' to a TEBuffer for the attribute groups for 'scope'
  The content of this buffer is an array of uint32_t per element in the associated attribute buffer,
   each entry to be treated as a bitmask indicating membership of the attribute groups
  The caller is responsible for releasing the returned TEBuffer using TERelease()
 Returns TEResultSucccess or an error
 */
TE_EXPORT TEResult TEGeometryGetGroupsBuffer(const TEGeometry *geometry, TEGeometryScope scope, TEBuffer * TE_NULLABLE * TE_NONNULL buffer);

/*
 Returns via 'count' the number of dimensions, and via dimensions' the number of points for each dimension
 Dimension information may be missing - if 'count' is zero after a successful return, consider the TEGeometry to have
 a single dimension sized for the number of points.
'count' is a pointer to an int32_t which should be set to the number of available elements in 'dimensions'.
 If this function returns TEResultSuccess, 'count' is set to the number of uint32_t filled in 'dimensions'
 If this function returns TEResultInsufficientMemory, the value at 'count' was too small to return the dimensions, and
 	'count' has been set to the number of available dimensions. Resize 'dimensions' appropriately and call the function again to
 	retrieve the full array. 
 */
TE_EXPORT TEResult TEGeometryGetDimensions(const TEGeometry *geometry, uint32_t dimensions[TE_NULLABLE], int32_t *count);

/*
 Mutable Geometry
 */

/*
 Creates a TEMutableGeometry from a TEGeometry
  The caller is responsible for releasing the returned TEMutableGeometry using TERelease()
 */
TE_EXPORT TEMutableGeometry *TEGeometryCreateMutableCopy(const TEGeometry *geometry);

/*
 Creates a new empty TEMutableGeometry
  The caller is responsible for releasing the returned TEMutableGeometry using TERelease()
 */
TE_EXPORT TEMutableGeometry *TEMutableGeometryCreate(void);

/*
 Adds an attribute for 'scope' to 'geometry'
  The contents of 'info' will be copied before this function returns
 */
TE_EXPORT TEResult TEMutableGeometryAddAttribute(TEMutableGeometry *geometry, TEGeometryScope scope, const struct TEGeometryAttributeInfo *info);

/*
 Sets the maximum number of elements for 'scope' to 'n'
  If the actual number of elements is known when the geometry is created, use that value
  If the number is the result of a pending operation, use the maximum possible number emitted by the operation
  The default value is 0 
 */
TE_EXPORT TEResult TEMutableGeometrySetMaxElements(TEMutableGeometry *geometry, TEGeometryScope scope, uint32_t n);

/*
 Sets an upper limit for the number of vertices of 'type' present in the TEMutableGeometry
  If the actual number of vertices is known when the geometry is created, use that value
  If the number is the result of a pending operation, use the maximum possible number emitted by the operation
  The default value is 0
 */
TE_EXPORT TEResult TEMutableGeometrySetMaxVertices(TEMutableGeometry *geometry, TEGeometryPrimitiveType type, uint32_t n);

/*
 Sets an upper limit for the number of primitives for 'type'
  This must be set for TEGeometryPrimitiveTypeLineStrips, for other types it will be calculated from the value passed to
   TEMutableGeometrySetMaxVertices()
  If the actual number of primitive is known when the geometry is created, use that value
  If the number is the result of a pending operation, use the maximum possible number emitted by the operation
  The default value is 0
 */
TE_EXPORT TEResult TEMutableGeometrySetMaxPrimitives(TEMutableGeometry *geometry, TEGeometryPrimitiveType type, uint32_t n);

/*
 Sets the attribute buffer identified by 'scope' and 'index'
  'buffer' may be retained by the TEMutableGeometry
  The layout of the content of this buffer must match that described in a call to TEMutablePoitnsAddAttribute
  The buffer must be sized to accomodate at least as many elements as described in a call to TEMutableGeometrySetMaxElements
 */
TE_EXPORT TEResult TEMutableGeometrySetAttributeBuffer(TEMutableGeometry *geometry, TEGeometryScope scope, int32_t index, TEBuffer * TE_NULLABLE buffer);

/*
 Sets the attribute buffer identified by 'scope' and 'name'
  'buffer' may be retained by the TEMutableGeometry
  The layout of the content of this buffer must match that described in a call to TEMutablePoitnsAddAttribute
  The buffer must be sized to accomodate at least as many elements as described in a call to TEMutableGeometrySetMaxElements
 */
TE_EXPORT TEResult TEMutableGeometrySetAttributeBufferByName(TEMutableGeometry *geometry, TEGeometryScope scope, const char *name, TEBuffer * TE_NULLABLE buffer);

/*
 Sets the buffer identified by 'ident'
 */
TE_EXPORT TEResult TEMutableGeometrySetBuffer(TEMutableGeometry *geometry, TEGeometryBufferID ident, TEBuffer * TE_NULLABLE buffer);

/*
 Sets a name for the attribute group at 'index' for 'scope'
  There can be a maximum of 32 groups for any scope
  Not all indices need be named
 */
TE_EXPORT TEResult TEMutableGeometrySetGroupName(TEMutableGeometry *geometry, TEGeometryScope scope, int32_t index, const char * TE_NULLABLE name);

/*
 Sets a groups buffer for 'scope'
  See TEGeometryGetGroupsBuffer() for a description of the layout of this buffer
 */
TE_EXPORT TEResult TEMutableGeometrySetGroupsBuffer(TEMutableGeometry *geometry, TEGeometryScope scope, TEBuffer * TE_NULLABLE buffer);

/*
 Sets the number of dimensions and the count of points per dimension
 'dimensions' is an array of uint32_t, each entry being the number of points for that dimension
 'count' is the number of entries in the array, being the number of dimensions
 */
TE_EXPORT TEResult TEMutableGeometrySetDimensions(TEMutableGeometry *geometry, uint32_t dimensions[TE_NONNULL], int32_t count);

TE_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif
