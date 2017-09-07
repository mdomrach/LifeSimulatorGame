#pragma once

class FGameManager;
struct FTerrainDisplayMesh;
class FTerrain;

class FTerrainDisplayManager
{
public:
	void Initialize(FGameManager* gameManager);
	void SetupDisplayMesh();
	void UpdateTerrainDisplayFromTerrain();

private:
	int GetVertexIndex(int xGrid, int yGrid);
	void LinkTriangleAndVertex(int triangleIndex, int vertexIndex1, int vertexIndex2, int vertexIndex3);

	FTerrainDisplayMesh* mesh;
	FTerrain* terrain;
};