#pragma once

#include <glm/gtx/quaternion.hpp>

class FInputManager;
class FScene;
class FGameManager;
class FTimeManager;

class FCameraController
{
private:
	FInputManager* inputManager;
	FScene* scene;
	FTimeManager* timeManager;

public:
	void Initialize(FGameManager* GameManager);
	void ProcessInput();

private:
	bool isCameraRotating;
	float initialXMousePosition;
	float initialYMousePosition;
	glm::quat initialCameraRotation;

	void ProcessMouseInput();
	void ProcessKeyboardInput();
};

