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

	currentXMousePos = (float) xMousePosition;
	currentYMousePos = (float) yMousePosition;


	for (auto item : mouseStates)
	{
		auto mouseButton = item.first;
		mouseStates[mouseButton] = glfwGetMouseButton(window, mouseButton);
	}
}

void FInputManager::ProcessKeyboardInput()
{
	for (auto item : keyStates)
	{
		auto key = item.first;
		keyStates[key] = glfwGetKey(window, key);
	}
}

void FInputManager::MonitorKeyState(int key)
{
	keyStates.insert(std::pair<int, int>(key, GLFW_RELEASE));
}

int FInputManager::GetKeyState(int key)
{
	return keyStates[key];
}

void FInputManager::MonitorMouseState(int mouseButton)
{
	mouseStates.insert(std::pair<int, int>(mouseButton, GLFW_RELEASE));
}

int FInputManager::GetMouseState(int mouseButton)
{
	return mouseStates[mouseButton];
}