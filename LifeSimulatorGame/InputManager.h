#pragma once
#include <map>

struct GLFWwindow;
class FGameManager;

class FInputManager
{
private:
	GLFWwindow* window;

public:

	float deltaXMousePos;
	float deltaYMousePos;

	float currentXMousePos;
	float currentYMousePos;

	void Initialize(FGameManager* GameManager);

	void InitializeInput();
	void ProcessInput();

	void MonitorKeyPress(int key);
	bool IsKeyPressed(int key);

private:
	std::map<int, bool> pressedKeys;
	void ProcessMouseInput();
	void ProcessKeyboardInput();
};

