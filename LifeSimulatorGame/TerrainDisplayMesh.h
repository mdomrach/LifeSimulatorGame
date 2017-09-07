#pragma once

#include "Triangles.h"
#include "Vertices.h"
#include "Edges.h"
#include "AttachedVertices.h"
#include "TerrainVertex.h"

struct FTerrainDisplayMesh
{
	FTriangles triangles;
	FVertices vertices;
	FEdges edges;
	FAttachedVertices attachedVertices;
	std::vector<FTerrainVertex> terrainVertices;
};