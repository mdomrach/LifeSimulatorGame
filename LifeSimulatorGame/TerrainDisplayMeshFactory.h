#pragma once
#include <map>
#include <unordered_set>
#include <glm/glm.hpp>

class FGameManager;
class FMesh;
struct FTerrainDisplayMesh;
struct FTerrainVisibleArea;

class FTerrainDisplayMeshFactory
{
public:
	FTerrainDisplayMeshFactory(FGameManager* gameManager);
	void SetupTerrainDisplayMesh();

	std::unordered_set<int> intersectedTiles;

	FMesh* heightMap;
	FTerrainVisibleArea* terrainVisibleArea;
	FTerrainDisplayMesh* terrainDisplayMesh;

private:
	std::map<int, int> terrainAreaVerticesToNewVertices;
	std::map<int, int> heightMapVerticesToNewVertices;

	std::map<int, std::vector<int>> intersectedTileToEdges;
	std::map<int, std::vector<int>> intersectedEdgeToVertices;

	FTerrainVisibleArea* terrainMeshGraph;

	void AddPolygonIntersections(int terrainVisiblePolygon);
	void AddEdgeIntersections(int terrainVisibleEdge);

	void CopyTerrainAreaVertices();
	void CopyHeightMapVertices();

	//int GetTerrainVisibleAreaVertex(int vertex);

	int previousVertex;
	int previousEdge;

	struct FVertexIntersection
	{
		int vertex;
		float distance;
		int intersectingEdge;
		int intersectingPolygon;
	};
};