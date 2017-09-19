#pragma once

class FGameManager;
struct FTerrainDisplayMesh;
class FTerrain;
struct FTerrainVisibleArea;

class FTerrainDisplayManager
{
public:
	void Initialize(FGameManager* gameManager);
	void SetupTerrainVisibleArea();
	void SetupDisplayMesh();
	void UpdateTerrainDisplayFromTerrain();

private:
	int GetVertexIndex(int xGrid, int yGrid);
	void LinkTriangleAndVertex(int triangleIndex, int vertexIndex1, int vertexIndex2, int vertexIndex3);

	const int numberOfQuads = 16;
	const int numberOfVertices = 17;

	FTerrainDisplayMesh* mesh;
	FTerrain* terrain;
	FTerrainVisibleArea* terrainVisibleArea;
};