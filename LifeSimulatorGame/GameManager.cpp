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
#include "VulkanApplicationData.h"
#include "TextOverlay.h"
#include "FPSTextOverlay.h"

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
	applicationData = new FVulkanApplicationData();
	textOverlay = new FTextOverlay();
	fpsTextOverlay = new FFPSTextOverlay();

	screenGrab->Initialize(this);
	cameraController->Initialize(this);
	terrainEditor->Initialize(this);

	vulkanApplication->Initialize(this);
	vulkanApplication->InitializeVulkan();
	fpsTextOverlay->Initialize(this);

	inputManager->Initialize(this);

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
		fpsTextOverlay->UpdateFrame();
		cameraController->ProcessInput();
		screenGrab->ProcessInput();
		terrainEditor->ProcessInput();

		vulkanApplication->UpdateFrame();
		vulkanApplication->DrawFrame();

		FSleepCalculator::SleepUntilWaitTime(timeManager->deltaFrameTime);
	}

	vkDeviceWaitIdle(applicationData->vulkanDevice.logicalDevice);
}

void FGameManager::CleanupGame()
{
	vulkanApplication->Cleanup();
}
