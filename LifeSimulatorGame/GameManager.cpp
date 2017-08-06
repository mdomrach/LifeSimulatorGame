#include "GameManager.h"

#include "Scene.h"
#include "InputManager.h"
#include "VulkanApplication.h"
#include "TimeManager.h"
#include "SleepCalculator.h"
#include "VulkanDevice.h"

FGameManager::FGameManager()
{
	scene = new FScene();
	inputManager = new FInputManager();
	vulkanApplication = new FVulkanApplication();
	timeManager = new FTimeManager();

	vulkanApplication->Initialize(this);
	vulkanApplication->InitializeVulkan();

	inputManager->Initialize(this);
}

void FGameManager::Run()
{
	InitializeGame();
	MainLoop();
	CleanupGame();
}

void FGameManager::InitializeGame()
{
	//vulkanApplication->InitializeVulkan();
}

void FGameManager::MainLoop()
{
	inputManager->InitializeInput();
	while (!glfwWindowShouldClose(vulkanApplication->window))
	{
		//float startLoopTime = startFrameTime;
		//for (int i = 0; i < 1000 && !glfwWindowShouldClose(window); i++)
		//{
		glfwPollEvents();
		timeManager->UpdateTime();
		inputManager->ProcessInput(timeManager->deltaFrameTime);

		vulkanApplication->UpdateUniformBuffer();
		vulkanApplication->DrawFrame();

		FSleepCalculator::SleepUntilWaitTime(timeManager->deltaFrameTime);
		//}
		//float endLoopTime = startFrameTime;
		//float loopTime = (endLoopTime - startLoopTime);
		//float loopFramePerSecond = 1000 / loopTime;
		//std::cout << loopFramePerSecond << " "<< loopTime<< "\n";
	}

	vkDeviceWaitIdle(vulkanApplication->vulkanDevice.logicalDevice);
}

void FGameManager::CleanupGame()
{
	vulkanApplication->Cleanup();
}
