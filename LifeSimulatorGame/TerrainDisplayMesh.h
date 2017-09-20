#pragma once

#include "Triangles.h"
#include "Vertices.h"
#include "Edges.h"
#include "AttachedVertices.h"
#include "MeshVertex.h"

struct FTerrainDisplayMesh
{
	FMesh mesh;
	FAttachedVertices attachedVertices;
};