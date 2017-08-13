#pragma once

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
	void ProcessMouseInput();
	void ProcessKeyboardInput();
};

