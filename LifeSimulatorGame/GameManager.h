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
struct FTerrainVisibleArea;

class FGameManager
{
public:
	FScene* scene;
	FTerrain* terrain;
	FTerrainDisplayMesh* terrainDisplayMesh;
	FTerrainVisibleArea* terrainVisibleArea;


	FInputManager* inputManager;
	FVulkanApplication* vulkanApplication;
	FTimeManager* timeManager;
	FCameraController* cameraController;
	FTerrainEditor* terrainEditor;
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

