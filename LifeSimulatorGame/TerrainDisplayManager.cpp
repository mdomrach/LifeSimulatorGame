#include "TerrainDisplayManager.h"

#include "GameManager.h"
#include "TerrainCalculator.h"
#include "Mesh.h"
#include "TerrainDisplayMesh.h"
#include "TerrainVisibleArea.h"
#include "GeometryCalculator.h"

void FTerrainDisplayManager::Initialize(FGameManager* gameManager)
{
	terrainDisplayMesh = gameManager->terrainDisplayMesh;
	terrain = gameManager->terrain;
	terrainVisibleArea = gameManager->terrainVisibleArea;
}

void FTerrainDisplayManager::SetupTerrainVisibleArea()
{
	terrainVisibleArea->vertexLocations.resize(3);
	terrainVisibleArea->vertexLocations[0] = { -0.5f - numberOfQuads / 2, -0.5f - numberOfQuads / 2 };
	terrainVisibleArea->vertexLocations[1] = { -0.5f - numberOfQuads / 2, 0.5f - numberOfQuads / 2 };
	terrainVisibleArea->vertexLocations[2] = { 0.5f - numberOfQuads / 2, -0.5f - numberOfQuads / 2 };

	terrainVisibleArea->vertices.edgeIndices.resize(3);
	terrainVisibleArea->vertices.triangleIndices.resize(3);

	terrainVisibleArea->edges.vertexIndices.resize(3);
	terrainVisibleArea->edges.triangleIndices.resize(3);

	terrainVisibleArea->triangles.vertexIndices.resize(1);
	terrainVisibleArea->triangles.edgeIndices.resize(1);

	FGeometryCalculator::LinkVerticesAndEdge(0, 1, 0, terrainVisibleArea->vertices, terrainVisibleArea->edges);
	FGeometryCalculator::LinkVerticesAndEdge(1, 2, 1, terrainVisibleArea->vertices, terrainVisibleArea->edges);
	FGeometryCalculator::LinkVerticesAndEdge(2, 0, 2, terrainVisibleArea->vertices, terrainVisibleArea->edges);
	FGeometryCalculator::LinkVerticesAndTriangle(0, 1, 2, 0, terrainVisibleArea->vertices, terrainVisibleArea->triangles);
	FGeometryCalculator::LinkEdgesAndTriangle(0, 1, 2, 0, terrainVisibleArea->edges, terrainVisibleArea->triangles);
}

void FTerrainDisplayManager::SetupDisplayMesh()
{
	int numberOfVertices = FTerrainCalculator::numberOfQuads;
	int numberOfQuads = FTerrainCalculator::numberOfQuads - 1;

	int vertexCount = numberOfVertices * numberOfVertices;
	//mesh->vertices.edgeIndices.resize(vertexCount);
	terrainDisplayMesh->attachedVertices.vertexIndices = new int[vertexCount][3];
	terrainDisplayMesh->attachedVertices.vertexWeights = new float[vertexCount][3];
	terrainDisplayMesh->mesh.vertices.resize(vertexCount);

	int triangleCount = 2 * numberOfQuads * numberOfQuads;
	terrainDisplayMesh->mesh.vertices.resize(triangleCount);
	//mesh->triangles.edgeIndices.resize(triangleCount);
	//mesh->triangles.vertexIndicesCount = triangleCount * 3;

	//int edgeCount = triangleCount * 3;
	//mesh->edges.triangleIndex.resize(edgeCount);
	//mesh->edges.vertexIndices.resize(edgeCount);

	for (int x = 0; x < numberOfQuads; x++)
	{
		for (int y = 0; y < numberOfQuads; y++)
		{
			int bottomLeftTriangleIndex = 2 * (x * numberOfQuads + y);
			int topRightTriangleIndex = 2 * (x * numberOfQuads + y) + 1;

			int bottomLeftVertexIndex = GetVertexIndex(x, y);
			int topLeftVertexIndex = GetVertexIndex(x, y + 1);
			int bottomRightVertexIndex = GetVertexIndex(x + 1, y);
			int topRightVertexIndex = GetVertexIndex(x + 1, y + 1);
		}
	}

	for (int x = 0; x < numberOfVertices; x++)
	{
		for (int y = 0; y < numberOfVertices; y++)
		{
			int vertexIndex = GetVertexIndex(x, y);

			terrainDisplayMesh->attachedVertices.vertexIndices[vertexIndex][0] = FTerrainCalculator::GetVertexIndex(x, y);

			terrainDisplayMesh->attachedVertices.vertexWeights[vertexIndex][0] = 1;
			terrainDisplayMesh->attachedVertices.vertexWeights[vertexIndex][1] = 0;
			terrainDisplayMesh->attachedVertices.vertexWeights[vertexIndex][2] = 0;
		}
	}

	UpdateTerrainDisplayFromTerrain();
}

void FTerrainDisplayManager::UpdateTerrainDisplayFromTerrain()
{
	for (int i = 0; i < terrainDisplayMesh->mesh.vertices.size(); i++)
	{
		int attachedVertexIndex = terrainDisplayMesh->attachedVertices.vertexIndices[i][0];
		terrainDisplayMesh->mesh.vertices[i].pos = terrainDisplayMesh->mesh.vertices[attachedVertexIndex].pos;
		terrainDisplayMesh->mesh.vertices[i].normal = terrainDisplayMesh->mesh.vertices[attachedVertexIndex].normal;
	}
}

int FTerrainDisplayManager::GetVertexIndex(int xGrid, int yGrid)
{
	return xGrid * FTerrainCalculator::numberOfQuads + yGrid;
}

void FTerrainDisplayManager::LinkTriangleAndVertex(int triangleIndex, int vertexIndex1, int vertexIndex2, int vertexIndex3)
{
	//mesh->vertices.triangleIndices[vertexIndex1].push_back(triangleIndex);
	//mesh->vertices.triangleIndices[vertexIndex2].push_back(triangleIndex);
	//mesh->vertices.triangleIndices[vertexIndex3].push_back(triangleIndex);

	terrainDisplayMesh->mesh.indices[3 * triangleIndex + 0] = vertexIndex1;
	terrainDisplayMesh->mesh.indices[3 * triangleIndex + 1] = vertexIndex2;
	terrainDisplayMesh->mesh.indices[3 * triangleIndex + 2] = vertexIndex3;
}

