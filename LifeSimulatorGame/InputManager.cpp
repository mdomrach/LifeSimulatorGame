#include "InputManager.h"
#include "Scene.h"
#include "Camera.h"

#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/quaternion.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "GameManager.h"
#include "VulkanApplication.h"

void FInputManager::Initialize(FGameManager* GameManager)
{
	this->scene = GameManager->scene;
	this->window = GameManager->vulkanApplication->window;
}

void FInputManager::InitializeInput()
{
	double xPosition, yPosition;
	glfwGetCursorPos(window, &xPosition, &yPosition);

	lastYPos = (float)yPosition;
	lastXPos = (float)xPosition;
}

void FInputManager::ProcessInput(float deltaFrameTime)
{
	ProcessMouseInput();
	ProcessKeyboardInput(deltaFrameTime);
	
	auto camera = scene->camera;
	glm::mat4 transform;
	transform = transform * glm::toMat4(camera->rotation);
	transform = glm::translate(transform, camera->position);
	camera->view = transform;
}

void FInputManager::ProcessMouseInput()
{
	auto camera = scene->camera;
	float speed = -0.002f;

	double xPosition, yPosition;
	glfwGetCursorPos(window, &xPosition, &yPosition);

	float deltaXPosition = (float)(lastXPos - xPosition);
	float deltaYPosition = (float)(lastYPos - yPosition);

	glm::vec3 eulAngles(speed*deltaYPosition, speed*deltaXPosition, 0);
	glm::quat orientationChange = glm::quat(eulAngles);
	camera->rotation = orientationChange * camera->rotation;

	lastYPos = (float)yPosition;
	lastXPos = (float)xPosition;
}

void FInputManager::ProcessKeyboardInput(float deltaFrameTime)
{
	auto camera = scene->camera;
	float cameraSpeed = 2.5f * deltaFrameTime;

	glm::vec3 translation = glm::vec3(0, 0, 0);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		translation.z += cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		translation.z -= cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		translation.x += cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		translation.x -= cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		translation.y -= cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		translation.y += cameraSpeed;
	}

	glm::vec3 cameraRight = translation * glm::mat3x3(camera->view);
	camera->position += cameraRight;


	glm::vec3 eulAngles;// (speed*deltaYPosition, speed*deltaXPosition, 0);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		eulAngles.z -= cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		eulAngles.z += cameraSpeed;
	}

	glm::quat orientationChange = glm::quat(eulAngles);
	camera->rotation = orientationChange * camera->rotation;
}