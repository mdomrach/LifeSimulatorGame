#include "CameraController.h"
#include "Scene.h"
#include "Camera.h"
#include "TimeManager.h"
#include "InputManager.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GameManager.h"
#include "VulkanApplication.h"

void FCameraController::Initialize(FGameManager* GameManager)
{
	this->scene = GameManager->scene;
	this->inputManager = GameManager->inputManager;
	this->timeManager = GameManager->timeManager;

	inputManager->MonitorKeyPress(GLFW_KEY_W);
	inputManager->MonitorKeyPress(GLFW_KEY_S);
	inputManager->MonitorKeyPress(GLFW_KEY_A);
	inputManager->MonitorKeyPress(GLFW_KEY_D);
	inputManager->MonitorKeyPress(GLFW_KEY_R);
	inputManager->MonitorKeyPress(GLFW_KEY_F);
	inputManager->MonitorKeyPress(GLFW_KEY_Q);
	inputManager->MonitorKeyPress(GLFW_KEY_E);
}

void FCameraController::ProcessInput()
{
	ProcessMouseInput();
	ProcessKeyboardInput();

	auto camera = scene->camera;
	glm::mat4 transform;
	transform = transform * glm::toMat4(camera->rotation);
	transform = glm::translate(transform, camera->position);
	camera->view = transform;
}

void FCameraController::ProcessMouseInput()
{
	auto camera = scene->camera;
	float speed = -0.002f;
	
	glm::vec3 eulAngles(speed*inputManager->deltaYMousePos, speed*inputManager->deltaXMousePos, 0);
	glm::quat orientationChange = glm::quat(eulAngles);
	camera->rotation = orientationChange * camera->rotation;
}

void FCameraController::ProcessKeyboardInput()
{
	auto camera = scene->camera;
	float cameraSpeed = 2.5f * timeManager->deltaFrameTime;

	glm::vec3 translation = glm::vec3(0, 0, 0);

	if (inputManager->IsKeyPressed(GLFW_KEY_W))
	{
		translation.z += cameraSpeed;
	}
	if (inputManager->IsKeyPressed(GLFW_KEY_S))
	{
		translation.z -= cameraSpeed;
	}
	if (inputManager->IsKeyPressed(GLFW_KEY_A))
	{
		translation.x += cameraSpeed;
	}
	if (inputManager->IsKeyPressed(GLFW_KEY_D))
	{
		translation.x -= cameraSpeed;
	}
	if (inputManager->IsKeyPressed(GLFW_KEY_R))
	{
		translation.y -= cameraSpeed;
	}
	if (inputManager->IsKeyPressed(GLFW_KEY_F))
	{
		translation.y += cameraSpeed;
	}

	glm::vec3 cameraRight = translation * glm::mat3x3(camera->view);
	camera->position += cameraRight;


	glm::vec3 eulAngles;// (speed*deltaYPosition, speed*deltaXPosition, 0);
	if (inputManager->IsKeyPressed(GLFW_KEY_Q))
	{
		eulAngles.z -= cameraSpeed;
	}
	if (inputManager->IsKeyPressed(GLFW_KEY_E))
	{
		eulAngles.z += cameraSpeed;
	}

	glm::quat orientationChange = glm::quat(eulAngles);
	camera->rotation = orientationChange * camera->rotation;
}