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

	inputManager->MonitorKeyState(GLFW_KEY_W);
	inputManager->MonitorKeyState(GLFW_KEY_S);
	inputManager->MonitorKeyState(GLFW_KEY_A);
	inputManager->MonitorKeyState(GLFW_KEY_D);
	inputManager->MonitorKeyState(GLFW_KEY_R);
	inputManager->MonitorKeyState(GLFW_KEY_F);
	inputManager->MonitorKeyState(GLFW_KEY_Q);
	inputManager->MonitorKeyState(GLFW_KEY_E);

	inputManager->MonitorMouseState(GLFW_MOUSE_BUTTON_RIGHT);
	isCameraRotating = false;
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
	int isMouseDown = inputManager->GetMouseState(GLFW_MOUSE_BUTTON_RIGHT);
	if (!isCameraRotating && isMouseDown)
	{
		auto camera = scene->camera;
		initialXMousePosition = inputManager->currentXMousePos;
		initialYMousePosition = inputManager->currentYMousePos;
		initialCameraRotation = camera->rotation;
		isCameraRotating = true;
	}
	else if (isCameraRotating && isMouseDown)
	{
		auto camera = scene->camera;
		const float rotateSpeed = 0.002f;

		float deltaXMousePosition = inputManager->currentXMousePos - initialXMousePosition;
		float deltaYMousePosition = inputManager->currentYMousePos - initialYMousePosition;

		glm::vec3 eulAngles(rotateSpeed*deltaYMousePosition, rotateSpeed*deltaXMousePosition, 0);
		glm::quat orientationChange = glm::quat(eulAngles);
		camera->rotation = orientationChange * initialCameraRotation;
	}
	else if (isCameraRotating && !isMouseDown)
	{
		isCameraRotating = false;
	}
}

void FCameraController::ProcessKeyboardInput()
{
	auto camera = scene->camera;
	float moveSpeed = 2.5f * timeManager->deltaFrameTime;

	glm::vec3 translation = glm::vec3(0, 0, 0);

	if (inputManager->GetKeyState(GLFW_KEY_W))
	{
		translation.z += moveSpeed;
	}
	if (inputManager->GetKeyState(GLFW_KEY_S))
	{
		translation.z -= moveSpeed;
	}
	if (inputManager->GetKeyState(GLFW_KEY_A))
	{
		translation.x += moveSpeed;
	}
	if (inputManager->GetKeyState(GLFW_KEY_D))
	{
		translation.x -= moveSpeed;
	}
	if (inputManager->GetKeyState(GLFW_KEY_R))
	{
		translation.y -= moveSpeed;
	}
	if (inputManager->GetKeyState(GLFW_KEY_F))
	{
		translation.y += moveSpeed;
	}

	glm::vec3 cameraRight = translation * glm::mat3x3(camera->view);
	camera->position += cameraRight;


	glm::vec3 eulAngles;
	float rotateSpeed = moveSpeed;
	if (inputManager->GetKeyState(GLFW_KEY_Q))
	{
		eulAngles.z -= rotateSpeed;
	}
	if (inputManager->GetKeyState(GLFW_KEY_E))
	{
		eulAngles.z += rotateSpeed;
	}

	glm::quat orientationChange = glm::quat(eulAngles);
	camera->rotation = orientationChange * camera->rotation;
}