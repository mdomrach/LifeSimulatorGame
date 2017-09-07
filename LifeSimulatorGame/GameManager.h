#pragma once

#include <vector>

class FScene;
class FInputManager;
class FVulkanApplication;
class FTimeManager;
class FCameraController;
class FTerrainEditor;
class FTerrain;
class FVulkanScreenGrab;
class FVulkanApplicationData;
class FTextOverlay;
class FFPSTextOverlay;
struct FTerrainDisplayMesh;
class FTerrainManager;
class FTerrainDisplayManager;

class FGameManager
{
public:
	FScene* scene;
	FInputManager* inputManager;
	FVulkanApplication* vulkanApplication;
	FTimeManager* timeManager;
	FCameraController* cameraController;
	FTerrainEditor* terrainEditor;
	FTerrain* terrain;
	FTerrainDisplayMesh* terrainDisplayMesh;
	FVulkanScreenGrab* screenGrab;
	FVulkanApplicationData* applicationData;
	FTerrainManager* terrainManager;
	FTerrainDisplayManager* terrainDisplayManager;

	std::vector<FTextOverlay*> textOverlay;

	FFPSTextOverlay* fpsTextOverlay;

	FGameManager();
	
	void Run();
private:
	void MainLoop();
	void CleanupGame();
};

