#pragma once
#include <string>

class FScene;
class FMesh;
class FCamera;

class FSceneCalculator
{
public:
	static void LoadScene(FScene* scene, int width, int height);
private:
	static FMesh* LoadMesh();
	static FMesh* LoadTerrain();
	static FCamera* LoadCamera(int width, int height);
	const std::string MODEL_PATH = "Models/chalet.obj";
};

