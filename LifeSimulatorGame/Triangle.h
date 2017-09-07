#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "TerrainVertex.h"
#include "Terrain.h"

struct FVertices
{
	std::vector<std::vector<int>> edge;
	std::vector<std::vector<int>> triangle;
};

struct FAttachedVertices
{
	std::vector<std::vector<int[3]>> vertices;
	std::vector<std::vector<float>> weights;
};

struct FHalfEdges
{
	std::vector<int> otherHalfEdge;

	std::vector<int[2]> vertices;
	std::vector<int> triangle;
};

struct FTriangles
{
	std::vector<int[3]> vertices;
	std::vector<int[3]> halfEdges;
};

struct FTerrainDisplayMesh
{
	std::vector<FTerrainVertex> meshVertices;

	FVertices vertices;
	FHalfEdges edges;
	FTriangles triangles;

	FTerrain* heightMesh;
	FAttachedVertices attachedVertices;
};

