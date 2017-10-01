#pragma once
#include <unordered_set>

class FGameManager;
struct FTerrainDisplayMesh;
class FMesh;
struct FTerrainVisibleArea;
class FScene;

#include <glm/glm.hpp>

class FTerrainDisplayManager
{
public:
	void Initialize(FGameManager* gameManager);
	void SetupTerrainVisibleArea();
	void SetupDisplayMesh();
	void UpdateTerrainDisplayFromTerrain();

private:
	int GetVertexIndex(int xGrid, int yGrid);

	const int numberOfQuads = 16;
	const int numberOfVertices = 17;

	FGameManager* gameManager;
	FTerrainDisplayMesh* terrainDisplayMesh;
	FMesh* terrain;
	FTerrainVisibleArea* terrainVisibleArea;
	FScene* scene;
};