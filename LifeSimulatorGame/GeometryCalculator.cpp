#include "GeometryCalculator.h"

#include "Vertices.h"
#include "Edges.h"
#include "Triangles.h"

void FGeometryCalculator::LinkVerticesAndEdge(int vertexIndex1, int vertexIndex2, int edgeIndex, FVertices& vertices, FEdges& edges)
{
	vertices.edgeIndices[vertexIndex1].push_back(edgeIndex);
	vertices.edgeIndices[vertexIndex2].push_back(edgeIndex);
	edges.vertexIndices[edgeIndex] = { vertexIndex1, vertexIndex2 };
}

void FGeometryCalculator::LinkVerticesAndTriangle(int vertexIndex1, int vertexIndex2, int vertexIndex3, int triangleIndex, FVertices& vertices, FTriangles& triangles)
{
	vertices.triangleIndices[vertexIndex1].push_back(triangleIndex);
	vertices.triangleIndices[vertexIndex2].push_back(triangleIndex);
	vertices.triangleIndices[vertexIndex3].push_back(triangleIndex);

	triangles.vertexIndices[triangleIndex] = { vertexIndex1, vertexIndex2, vertexIndex3 };
}

void FGeometryCalculator::LinkEdgesAndTriangle(int edgeIndex1, int edgeIndex2, int edgeIndex3, int triangleIndex, FEdges& edges, FTriangles& triangles)
{
	edges.triangleIndices[edgeIndex1] = triangleIndex;
	edges.triangleIndices[edgeIndex2] = triangleIndex;
	edges.triangleIndices[edgeIndex3] = triangleIndex;

	triangles.edgeIndices[triangleIndex] = { edgeIndex1, edgeIndex2, edgeIndex3 };
}