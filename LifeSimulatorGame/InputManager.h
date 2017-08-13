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

