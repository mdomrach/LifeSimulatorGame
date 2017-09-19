#pragma once

#include "Vertices.h"
#include "Edges.h"
#include "Triangles.h"

#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

struct FTerrainVisibleArea
{
	std::vector<glm::vec2> vertexLocations;

	FVertices vertices;
	FEdges edges;
	FTriangles triangles;
};