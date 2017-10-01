#include "TerrainDisplayMeshFactory.h"
#include "GameManager.h"
#include "TerrainCalculator.h"
#include "Mesh.h"
#include "MeshVertex.h"
//#include "TerrainDisplayMesh.h"
#include "TerrainVisibleArea.h"
#include "GeometryCalculator.h"
//#include "Scene.h"
//#include "MeshCalculator.h"


FTerrainDisplayMeshFactory::FTerrainDisplayMeshFactory(FGameManager* gameManager)
{
	terrainDisplayMesh = gameManager->terrainDisplayMesh;
	heightMap = gameManager->terrain;
	terrainVisibleArea = gameManager->terrainVisibleArea;

	terrainMeshGraph = new FTerrainVisibleArea();
}

void FTerrainDisplayMeshFactory::SetupTerrainDisplayMesh()
{
	CopyTerrainAreaVertices();
	CopyHeightMapVertices();

	for (int i = 0; i < terrainVisibleArea->triangles.edgeIndices.size(); i++)
	{
		AddPolygonIntersections(i);
	}
}

void FTerrainDisplayMeshFactory::CopyTerrainAreaVertices()
{
	auto oldSize = terrainMeshGraph->vertexLocations.size();
	auto newSize = terrainMeshGraph->vertexLocations.size() + terrainVisibleArea->vertexLocations.size();
	terrainMeshGraph->vertexLocations.resize(newSize);
	terrainMeshGraph->vertices.edgeIndices.resize(newSize);
	terrainMeshGraph->vertices.triangleIndices.resize(newSize);

	for (int i = 0; i < terrainVisibleArea->vertexLocations.size(); i++)
	{
		int newVertex = i + oldSize;
		//terrainAreaVerticesToNewVertices.insert(i, newVertex);
		terrainMeshGraph->vertexLocations[newVertex] = terrainVisibleArea->vertexLocations[i];
	}
}

void FTerrainDisplayMeshFactory::CopyHeightMapVertices()
{
	auto oldSize = terrainMeshGraph->vertexLocations.size();
	auto newSize = terrainMeshGraph->vertexLocations.size() + heightMap->vertices.size();
	terrainMeshGraph->vertexLocations.resize(newSize);
	terrainMeshGraph->vertices.edgeIndices.resize(newSize);
	terrainMeshGraph->vertices.triangleIndices.resize(newSize);

	for (int i = 0; i < heightMap->vertices.size(); i++)
	{
		int newVertex = i + oldSize;
		//heightMapVerticesToNewVertices.insert(i, newVertex);
		terrainMeshGraph->vertexLocations[newVertex] = heightMap->vertices[i].pos;
	}
}

void FTerrainDisplayMeshFactory::AddPolygonIntersections(int terrainVisiblePolygon)
{
	for (auto edge : terrainVisibleArea->triangles.edgeIndices[terrainVisiblePolygon])
	{
		AddEdgeIntersections(edge);
	}
}

void FTerrainDisplayMeshFactory::AddEdgeIntersections(int terrainVisibleEdge)
{
	auto startIndex = terrainVisibleArea->edges.vertexIndices[terrainVisibleEdge][0];
	auto endIndex = terrainVisibleArea->edges.vertexIndices[terrainVisibleEdge][1];

	auto startPosition = terrainVisibleArea->vertexLocations[startIndex];
	auto endPosition = terrainVisibleArea->vertexLocations[endIndex];
	auto deltaPosition = endPosition - startPosition;

	std::vector<int> xIntersectionVertices;
	std::vector<float> xDistances;
	if (deltaPosition.x > 0)
	{
		int xStart = (int)startPosition.x+1;
		int xEnd = (int)endPosition.x;
		for (int x = xStart; x <= xEnd; x++)
		{
			float t = (x - startPosition.x) / deltaPosition.x;
			float y = startPosition.y + t * deltaPosition.y;

			xIntersectionVertices.push_back(terrainMeshGraph->vertexLocations.size());
			xDistances.push_back(t);

			terrainMeshGraph->vertexLocations.push_back(glm::vec2(x, y));
			terrainMeshGraph->vertices.edgeIndices.push_back(std::vector<int>());
			terrainMeshGraph->vertices.triangleIndices.push_back(std::vector<int>());
		}
	}
	else
	{
		int xStart = (int)startPosition.x;
		int xEnd = (int)endPosition.x+1;
		for (int x = xStart; x >= xEnd; x--)
		{
			float t = (x - startPosition.x) / deltaPosition.x;
			float y = startPosition.y + t * deltaPosition.y;

			xIntersectionVertices.push_back(terrainMeshGraph->vertexLocations.size());
			xDistances.push_back(t);

			terrainMeshGraph->vertexLocations.push_back(glm::vec2(x, y));
			terrainMeshGraph->vertices.edgeIndices.push_back(std::vector<int>());
			terrainMeshGraph->vertices.triangleIndices.push_back(std::vector<int>());
		}
	}

	std::vector<int> yIntersectionVertices;
	std::vector<float> yDistances;
	if (deltaPosition.y > 0)
	{
		int yStart = (int)startPosition.y+1;
		int yEnd = (int)endPosition.y;
		for (int y = yStart; y <= yEnd; y++)
		{
			float t = (y - startPosition.y) / deltaPosition.y;
			float x = startPosition.x + t * deltaPosition.x;

			yIntersectionVertices.push_back(terrainMeshGraph->vertexLocations.size());
			yDistances.push_back(t);

			terrainMeshGraph->vertexLocations.push_back(glm::vec2(x, y));
			terrainMeshGraph->vertices.edgeIndices.push_back(std::vector<int>());
			terrainMeshGraph->vertices.triangleIndices.push_back(std::vector<int>());
		}
	}
	else
	{
		int yStart = (int)startPosition.y;
		int yEnd = (int)endPosition.y+1;
		for (int y = yStart; y >= yEnd; y--)
		{
			float t = (y - startPosition.y) / deltaPosition.y;
			float x = startPosition.x + t * deltaPosition.x;

			yDistances.push_back(t);
			yIntersectionVertices.push_back(terrainMeshGraph->vertexLocations.size());

			terrainMeshGraph->vertexLocations.push_back(glm::vec2(x, y));
			terrainMeshGraph->vertices.edgeIndices.push_back(std::vector<int>());
			terrainMeshGraph->vertices.triangleIndices.push_back(std::vector<int>());
		}
	}

	int xVertexIndex = 0;
	int yVertexIndex = 0;
	auto currentVertexIndex = startIndex;
	while (xVertexIndex < xIntersectionVertices.size() && yVertexIndex < yIntersectionVertices.size())
	{
		if (xDistances[xVertexIndex] < yDistances[yVertexIndex])
		{
			int edgeIndex = terrainMeshGraph->edges.triangleIndices.size();

			terrainMeshGraph->edges.triangleIndices.push_back(-1);
			terrainMeshGraph->edges.vertexIndices.push_back({ -1, -1 });

			FGeometryCalculator::LinkVerticesAndEdge(currentVertexIndex, edgeIndex, xIntersectionVertices[xVertexIndex], terrainMeshGraph->vertices, terrainMeshGraph->edges);

			xVertexIndex++;
		}
		else
		{
			yVertexIndex++;
		}
	}
}