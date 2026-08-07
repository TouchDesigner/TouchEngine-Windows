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

#include "stdafx.h"
#include "Geometry.h"
#include "Color.h"
#include <vector>
#include <numbers>
#include <cmath>
#include "Vec.h"

namespace Geometry {

static constexpr int getPointCount(int divisions)
{
	return divisions + 1;
}

static constexpr int getPrimitiveCount(int divisions)
{
	return divisions;
}

static constexpr int getVertexCount(int divisions)
{
	return divisions * 3;
}

static void generateCircleTriangleList(float centerX, float centerY,
	float radius,
	uint32_t divisions,
	std::vector<Vec3>& outPoints,
	std::vector<uint32_t>& outIndices,
	std::vector<Vec4>& outPrimitiveColors,
	Vec4 color1, Vec4 color2)
{
	outPoints.clear();
	outIndices.clear();
	outPoints.reserve(getPointCount(divisions));
	outIndices.reserve(getVertexCount(divisions));
	outPrimitiveColors.reserve(getPrimitiveCount(divisions));

	// Point 0 = center
	outPoints.push_back({ centerX, centerY, 0.0f });

	// Ring points: indices 1..segments
	for (uint32_t i = 0; i < divisions; ++i) {
		double angle = (static_cast<double>(i) / static_cast<double>(divisions)) * std::numbers::pi * 2.0;
		float x = static_cast<float>(centerX + radius * std::cos(angle));
		float y = static_cast<float>(centerY + radius * std::sin(angle));
		outPoints.push_back({ x, y, 0.0f });
		outPrimitiveColors.push_back(i % 2 == 0 ? color1 : color2);
	}

	// Triangles: center, ring[i], ring[i+1] (wrapping last back to first)
	for (uint32_t i = 0; i < divisions; ++i) {
		uint32_t current = 1 + i;
		uint32_t next = 1 + ((i + 1) % divisions);

		outIndices.push_back(0);       // center
		outIndices.push_back(current);
		outIndices.push_back(next);
	}
}

}

TouchObject<TEGeometry> Geometry::getCircleGeometry(float radius, int divisions, const Color& color1, const Color& color2, BufferProvider& provider)
{
	TouchObject<TEGeometry> geometry = TouchObject<TEMutableGeometry>::make_take(TEMutableGeometryCreate());

	static constexpr const char* kPointsAttribName = "P";
	static constexpr const char* kColorAttribName = "Color";

	uint32_t pointCount = getPointCount(divisions);
	uint32_t primitiveCount = getPrimitiveCount(divisions);
	uint32_t vertexCount = getVertexCount(divisions);

	Vec4 c1 = { color1.red, color1.green, color1.blue, color1.alpha };
	Vec4 c2 = { color2.red, color2.green, color2.blue, color2.alpha };

	struct TEGeometryAttributeInfo attributeP = {
		.name = kPointsAttribName,
		.type = TEGeometryAttributeTypeFloat,
		.intent = TEGeometryAttributeIntentNone,
		.components = 3,
		.columns = 1,
		.arrayCount = 1,
		.offset = 0,
		.stride = sizeof(float) * 3,
		.arrayStride = sizeof(float) * 3
	};

	TEMutableGeometryAddAttribute(geometry, TEGeometryScopePoints, &attributeP);

	struct TEGeometryAttributeInfo attributeColor = {
		.name = kColorAttribName,
		.type = TEGeometryAttributeTypeFloat,
		.intent = TEGeometryAttributeIntentColor,
		.components = 4,
		.columns = 1,
		.arrayCount = 1,
		.offset = 0,
		.stride = sizeof(float) * 4,
		.arrayStride = sizeof(float) * 4
	};
	
	TEMutableGeometryAddAttribute(geometry, TEGeometryScopePrimitives, &attributeColor);
	
	TEMutableGeometrySetMaxElements(geometry, TEGeometryScopePoints, pointCount);
	
	TEMutableGeometrySetMaxElements(geometry, TEGeometryScopeVertices, vertexCount);
	
	TEMutableGeometrySetMaxElements(geometry, TEGeometryScopePrimitives, primitiveCount);
	
	TEMutableGeometrySetMaxVertices(geometry, TEGeometryPrimitiveTypeTriangles, vertexCount);
	
	std::vector<Vec3> points;
	std::vector<uint32_t> indices;
	std::vector<Vec4> colors;
	generateCircleTriangleList(0.0f, 0.0f, radius, divisions, points, indices, colors, c1, c2);
	
	// In this example we use device buffers for attributes and the index buffer
	// but they could be host buffers too

	// Points attribute buffer
	TouchObject<TEBuffer> pointsBuffer = provider.getDeviceBuffer(points);
	TEMutableGeometrySetAttributeBufferByName(geometry, TEGeometryScopePoints, kPointsAttribName, pointsBuffer);

	// Primitives attribute buffer
	TouchObject<TEBuffer> colorBuffer = provider.getDeviceBuffer(colors);
	TEMutableGeometrySetAttributeBufferByName(geometry, TEGeometryScopePrimitives, kColorAttribName, colorBuffer);
	
	// Index buffer
	TouchObject<TEBuffer> indexBuffer = provider.getDeviceBuffer(indices);
	TEMutableGeometrySetBuffer(geometry, TEGeometryBufferIDIndex, indexBuffer);
	
	// Topology buffer
	TEGeometryTopology topo = {
		.trianglesOffset = 0,
		.trianglesCount = primitiveCount,
		.quadsOffset = topo.trianglesOffset + topo.trianglesCount * 3,
		.quadsCount = 0,
		.lineStripsOffset = topo.quadsOffset + topo.quadsCount * 4,
		.lineStripsCount = 0,
		.lineStripsVertexCount = 0,
		.linesOffset = topo.lineStripsOffset + topo.lineStripsVertexCount,
		.linesCount = 0,
		.pointPrimsOffset = topo.linesOffset + topo.linesCount * 2,
		.pointPrimsCount = 0
	};
	
	// These could be device buffers too
	TouchObject<TEBuffer> topoBuffer = provider.getHostBuffer(&topo, sizeof(topo));
	TEMutableGeometrySetBuffer(geometry, TEGeometryBufferIDTopology, topoBuffer);
	
	TouchObject<TEBuffer> pointsCountBuffer = provider.getHostBuffer(&pointCount, sizeof(pointCount));
	TEMutableGeometrySetBuffer(geometry, TEGeometryBufferIDPointsCount, pointsCountBuffer);
	
	return geometry;
}
