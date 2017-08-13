#include "InputManager.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "GameManager.h"
#include "VulkanApplication.h"

void FInputManager::Initialize(FGameManager* GameManager)
{
	this->window = GameManager->vulkanApplication->window;
}

void FInputManager::InitializeInput()
{
	double xPosition, yPosition;
	glfwGetCursorPos(window, &xPosition, &yPosition);

	currentXMousePos = (float)yPosition;
	currentYMousePos = (float)xPosition;
}

void FInputManager::ProcessInput()
{
	ProcessMouseInput();
	ProcessKeyboardInput();
}

void FInputManager::ProcessMouseInput()
{
	double xMousePosition, yMousePosition;
	glfwGetCursorPos(window, &xMousePosition, &yMousePosition);

	deltaXMousePos = (float)(currentXMousePos - xMousePosition);
	deltaYMousePos = (float)(currentYMousePos - yMousePosition);

	currentXMousePos = xMousePosition;
	currentYMousePos = yMousePosition;
}

void FInputManager::ProcessKeyboardInput()
{
	for (auto item : pressedKeys)
	{
		auto key = item.first;
		pressedKeys[key] = (glfwGetKey(window, key) == GLFW_PRESS);
	}
}

void FInputManager::MonitorKeyPress(int key)
{
	pressedKeys.insert(std::pair<int, bool>(key, false));
}

bool FInputManager::IsKeyPressed(int key)
{
	return pressedKeys[key];
}