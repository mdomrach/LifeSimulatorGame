#include "TerrainDisplayManager.h"

#include "GameManager.h"
#include "TerrainCalculator.h"
#include "Terrain.h"
#include "TerrainDisplayMesh.h"

void FTerrainDisplayManager::Initialize(FGameManager* gameManager)
{
	mesh = gameManager->terrainDisplayMesh;
	terrain = gameManager->terrain;
}

void FTerrainDisplayManager::SetupDisplayMesh()
{
	int numberOfVertices = FTerrainCalculator::numberOfQuads;
	int numberOfQuads = FTerrainCalculator::numberOfQuads - 1;

	int vertexCount = numberOfVertices * numberOfVertices;
	//mesh->vertices.edgeIndices.resize(vertexCount);
	mesh->vertices.triangleIndices = new std::vector<int>[vertexCount];
	mesh->attachedVertices.vertexIndices = new int[vertexCount][3];
	mesh->attachedVertices.vertexWeights = new float[vertexCount][3];
	mesh->terrainVertices.resize(vertexCount);

	int triangleCount = 2 * numberOfQuads * numberOfQuads;
	//mesh->triangles.edgeIndices.resize(triangleCount);
	mesh->triangles.vertexIndicesCount = triangleCount * 3;
	mesh->triangles.vertexIndices = new int[triangleCount][3];

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

			LinkTriangleAndVertex(bottomLeftTriangleIndex, bottomLeftVertexIndex, bottomRightVertexIndex, topLeftVertexIndex);
			LinkTriangleAndVertex(topRightTriangleIndex, topRightVertexIndex, topLeftVertexIndex, bottomRightVertexIndex);
		}
	}

	for (int x = 0; x < numberOfVertices; x++)
	{
		for (int y = 0; y < numberOfVertices; y++)
		{
			int vertexIndex = GetVertexIndex(x, y);

			mesh->attachedVertices.vertexIndices[vertexIndex][0] =  FTerrainCalculator::GetVertexIndex(x, y);

			mesh->attachedVertices.vertexWeights[vertexIndex][0] = 1;
			mesh->attachedVertices.vertexWeights[vertexIndex][1] = 0;
			mesh->attachedVertices.vertexWeights[vertexIndex][2] = 0;
		}
	}

	UpdateTerrainDisplayFromTerrain();
}

void FTerrainDisplayManager::UpdateTerrainDisplayFromTerrain()
{
	for (int i = 0; i < mesh->terrainVertices.size(); i++)
	{
		int attachedVertexIndex = mesh->attachedVertices.vertexIndices[i][0];

		mesh->terrainVertices[i].pos = terrain->vertices[attachedVertexIndex].pos;
		mesh->terrainVertices[i].normal = terrain->vertices[attachedVertexIndex].normal;
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

	mesh->triangles.vertexIndices[triangleIndex][0] = vertexIndex1;
	mesh->triangles.vertexIndices[triangleIndex][1] = vertexIndex2;
	mesh->triangles.vertexIndices[triangleIndex][2] = vertexIndex3;
}