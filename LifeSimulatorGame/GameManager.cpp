#include "GameManager.h"

#include "Scene.h"
#include "InputManager.h"
#include "VulkanApplication.h"
#include "TimeManager.h"
#include "SleepCalculator.h"
#include "VulkanDevice.h"
#include "CameraController.h"
#include "TerrainEditor.h"
#include "Terrain.h"
#include "VulkanScreenGrab.h"

FGameManager::FGameManager()
{
	scene = new FScene();
	inputManager = new FInputManager();
	vulkanApplication = new FVulkanApplication();
	timeManager = new FTimeManager();
	cameraController = new FCameraController();
	terrainEditor = new FTerrainEditor();
	terrain = new FTerrain();
	screenGrab = new FVulkanScreenGrab();

	vulkanApplication->Initialize(this);
	vulkanApplication->InitializeVulkan();

	inputManager->Initialize(this);
	cameraController->Initialize(this);
	terrainEditor->Initialize(this);
	screenGrab->Initialize(this);
}

void FGameManager::Run()
{
	MainLoop();
	CleanupGame();
}

void FGameManager::MainLoop()
{
	inputManager->InitializeInput();
	while (!glfwWindowShouldClose(vulkanApplication->window))
	{
		glfwPollEvents();
		timeManager->UpdateTime();
		inputManager->ProcessInput();
		cameraController->ProcessInput();
		terrainEditor->ProcessInput();
		screenGrab->ProcessInput();

		vulkanApplication->UpdateUniformBuffer();
		vulkanApplication->DrawFrame();

		//FSleepCalculator::SleepUntilWaitTime(timeManager->deltaFrameTime);
	}

	vkDeviceWaitIdle(vulkanApplication->vulkanDevice.logicalDevice);
}

void FGameManager::CleanupGame()
{
	vulkanApplication->Cleanup();
}
