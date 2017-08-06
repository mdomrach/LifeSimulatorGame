#include "TimeManager.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

void FTimeManager::UpdateTime()
{
	float currentFrameTime = (float) glfwGetTime();
	deltaFrameTime = currentFrameTime - startFrameTime;
	startFrameTime = currentFrameTime;
}