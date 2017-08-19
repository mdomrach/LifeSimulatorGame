#pragma once
#include <map>

struct GLFWwindow;
class FGameManager;

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class FInputManager
{
private:
	GLFWwindow* window;

public:

	float deltaXMousePos;
	float deltaYMousePos;

	float currentXMousePos;
	float currentYMousePos;

	glm::vec3 HitPoint;

	void Initialize(FGameManager* GameManager);

	void InitializeInput();
	void ProcessInput();

	void MonitorKeyState(int key);
	int GetKeyState(int key);

	void MonitorMouseState(int mouseButton);
	int GetMouseState(int mouseButton);

private:
	std::map<int, int> keyStates;
	std::map<int, int> mouseStates;
	void ProcessMouseInput();
	void ProcessKeyboardInput();
};

