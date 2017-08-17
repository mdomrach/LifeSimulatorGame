#include "TerrainEditor.h"

#include "GameManager.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "VulkanApplication.h"
#include "Terrain.h"

void FTerrainEditor::Initialize(FGameManager* GameManager)
{
	inputManager = GameManager->inputManager;
	terrain = GameManager->terrain;
	timeManager = GameManager->timeManager;


	inputManager->MonitorMouseState(GLFW_MOUSE_BUTTON_LEFT);
	inputManager->MonitorKeyState(GLFW_KEY_1);
	inputManager->MonitorKeyState(GLFW_KEY_2);
	inputManager->MonitorKeyState(GLFW_KEY_3);
}

void FTerrainEditor::ProcessInput()
{
	if (inputManager->GetKeyState(GLFW_KEY_1))
	{
		terraformMode = TerraformMode::lower;
	}
	if (inputManager->GetKeyState(GLFW_KEY_2))
	{
		terraformMode = TerraformMode::raise;
	}

	if (inputManager->GetKeyState(GLFW_KEY_3))
	{
		terraformMode = TerraformMode::flatten;
	}


	if (inputManager->GetMouseState(GLFW_MOUSE_BUTTON_LEFT))
	{
		float maxHeightChange = timeManager->deltaFrameTime;
		glm::vec2 hitPosition = GetHitPosition();

		switch (terraformMode)
		{
		case TerraformMode::lower:
			Lower(hitPosition, maxHeightChange);
			break;
		case TerraformMode::raise:
			Raise(hitPosition, maxHeightChange);
			break;
		case TerraformMode::flatten:
			Flatten(hitPosition, maxHeightChange);
			break;
		}
	}
}

void FTerrainEditor::Flatten(glm::vec2 hitPosition, float maxHeightChange)
{
	int flattenVertCount = 0;
	float flattenHeightSum = 0.0f;
	
	for (int i = 0; i < terrain->vertices.size(); i++)
	{
		auto vertexPosition = terrain->vertices[i].pos;
		auto vertexPosition2D = glm::vec2(vertexPosition.x, vertexPosition.y);
		float distance = glm::distance(vertexPosition2D, hitPosition);
		if (distance < maxDistance)
		{
			flattenVertCount++;
			flattenHeightSum += vertexPosition.z;
		}
	}

	if (flattenVertCount == 0)
		return;

	float flattenAverage = flattenHeightSum / flattenVertCount;
	for (int i = 0; i < terrain->vertices.size(); i++)
	{
		auto vertexPosition = terrain->vertices[i].pos;
		auto vertexPosition2D = glm::vec2(vertexPosition.x, vertexPosition.y);
		float distance = glm::distance(vertexPosition2D, hitPosition);
		if (distance < maxDistance)
		{
			terrain->vertices[i].pos.z = MoveTo(terrain->vertices[i].pos.z, flattenAverage, maxHeightChange);
		}
	}
}

void FTerrainEditor::Lower(glm::vec2 hitPosition, float maxHeightChange)
{
	for (int i = 0; i < terrain->vertices.size(); i++)
	{
		auto vertexPosition = terrain->vertices[i].pos;
		auto vertexPosition2D = glm::vec2(vertexPosition.x, vertexPosition.y);
		float distance = glm::distance(vertexPosition2D, hitPosition);
		if (distance < maxDistance)
		{
			terrain->vertices[i].pos.z -= Lerp(maxHeightChange, 0.0f, distance / maxDistance);
		}
	}	
}

void FTerrainEditor::Raise(glm::vec2 hitPosition, float maxHeightChange)
{

	for (int i = 0; i < terrain->vertices.size(); i++)
	{
		auto vertexPosition = terrain->vertices[i].pos;
		auto vertexPosition2D = glm::vec2(vertexPosition.x, vertexPosition.y);
		float distance = glm::distance(vertexPosition2D, hitPosition);
		if (distance < maxDistance)
		{
			terrain->vertices[i].pos.z += Lerp(maxHeightChange, 0.0f, distance / maxDistance);
		}
	}
}

glm::vec2 FTerrainEditor::GetHitPosition()
{
	auto xPosition = (inputManager->currentXMousePos / 800) * 16 - 8;
	auto yPosition = (inputManager->currentYMousePos / 600) * -16 + 8;
	return glm::vec2(xPosition, yPosition);
}

float FTerrainEditor::MoveTo(float current, float target, float maxDelta)
{
	auto delta = target - current;
	if (glm::abs(delta) <= maxDelta)
	{
		return target;
	}
	else
	{
		return current + glm::sign(delta) * maxDelta;
	}
}

float FTerrainEditor::Lerp(float from, float to, float t)
{
	return from * (1 - t) + to*t;
}