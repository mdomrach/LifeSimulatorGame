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
	FVulkanScreenGrab* screenGrab;
	FVulkanApplicationData* applicationData;

	std::vector<FTextOverlay*> textOverlay;

	FFPSTextOverlay* fpsTextOverlay;

	FGameManager();
	
	void Run();
private:
	void MainLoop();
	void CleanupGame();
};

