#pragma once

class FGameManager;
class FMesh;

class FTerrainManager
{
public:
	void Initialize(FGameManager* GameManager);
	void RecalculateNormals();

private:
	void LoadAssets();

	const int numberOfQuads = 16;
	const int numberOfVertices = 17;
	FMesh* terrain;
	int GetVertexIndex(int x, int y);
};