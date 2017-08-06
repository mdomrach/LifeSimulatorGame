#pragma once

struct GLFWwindow;
class FScene;
class FGameManager;

class FInputManager
{
private:
	GLFWwindow* window;
	FScene* scene;

public:
	void Initialize(FGameManager* GameManager);

	void InitializeInput();
	void ProcessInput(float deltaFrameTime);

private:
	// should be moved to input manager
	float lastXPos;
	float lastYPos;
	
	void ProcessMouseInput();
	void ProcessKeyboardInput(float deltaFrameTime);
};

