#pragma once

struct FVertices;
struct FEdges;
struct FTriangles;

class FGeometryCalculator
{
public:
	static int AddVertex(FVertices& vertices);
	static int AddEdge(FEdges& edges);

	static void LinkVerticesAndEdge(int vertexIndex1, int vertexIndex2, int edgeIndex, FVertices& vertices, FEdges& edges);
	static void LinkVerticesAndTriangle(int vertexIndex1, int vertexIndex2, int vertexIndex3, int triangleIndex, FVertices& vertices, FTriangles& triangles);
	static void LinkEdgesAndTriangle(int edgeIndex1, int edgeIndex2, int edgeIndex3, int triangleIndex, FEdges& edges, FTriangles& triangles);
};

