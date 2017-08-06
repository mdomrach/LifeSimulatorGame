#pragma once

class FScene;
class FInputManager;
class FVulkanApplication;
class FTimeManager;

class FGameManager
{
public:
	FScene* scene;
	FInputManager* inputManager;
	FVulkanApplication* vulkanApplication;
	FTimeManager* timeManager;

	FGameManager();
	
	void Run();
private:
	void InitializeGame();
	void MainLoop();
	void CleanupGame();
};

