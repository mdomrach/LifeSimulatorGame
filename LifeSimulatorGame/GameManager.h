#pragma once

class FScene;
class FInputManager;
class FVulkanApplication;
class FTimeManager;
class FCameraController;

class FGameManager
{
public:
	FScene* scene;
	FInputManager* inputManager;
	FVulkanApplication* vulkanApplication;
	FTimeManager* timeManager;
	FCameraController* cameraController;

	FGameManager();
	
	void Run();
private:
	void MainLoop();
	void CleanupGame();
};

