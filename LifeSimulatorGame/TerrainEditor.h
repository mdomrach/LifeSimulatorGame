#pragma once

class FInputManager;
class FTerrain;
class FGameManager;
class FTimeManager;
class FScene;

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class FTerrainEditor
{
private:
	enum TerraformMode { raise, lower, flatten };

	FScene* scene;
	FInputManager* inputManager;
	FTerrain* terrain;
	FTimeManager* timeManager;

	const int numberOfQuads = 16;
	const int numberOfVertices = 17;
public:
	void Initialize(FGameManager* GameManager);
	void ProcessInput();

private:
	glm::vec2 GetHitPosition();
	void Lower(glm::vec2 hitPosition, float maxHeightChange);
	void Raise(glm::vec2 hitPosition, float maxHeightChange);
	void Flatten(glm::vec2 hitPosition, float maxHeightChange);

	float Lerp(float from, float to, float t);
	float MoveTo(float value, float target, float maxDistance);
	TerraformMode terraformMode;
	const float maxDistance = 6;
};

