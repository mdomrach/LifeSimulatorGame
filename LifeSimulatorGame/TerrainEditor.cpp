#include "TerrainEditor.h"

#include "GameManager.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "VulkanApplication.h"
#include "Mesh.h"
#include "Scene.h"
#include "TerrainDisplayManager.h"
#include "TerrainManager.h"

void FTerrainEditor::Initialize(FGameManager* GameManager)
{
	inputManager = GameManager->inputManager;
	terrain = GameManager->terrain;
	timeManager = GameManager->timeManager;
	scene = GameManager->scene;
	terrainDisplayManager = GameManager->terrainDisplayManager;
	terrainManager = GameManager->terrainManager;

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


	glm::vec3 hitPosition = GetHitPosition();
	if (inputManager->GetMouseState(GLFW_MOUSE_BUTTON_LEFT))
	{
		float maxHeightChange = timeManager->deltaFrameTime;

		switch (terraformMode)
		{
		case TerraformMode::lower:
			Lower(hitPosition, 2*maxHeightChange);
			break;
		case TerraformMode::raise:
			Raise(hitPosition, 2*maxHeightChange);
			break;
		case TerraformMode::flatten:
			Flatten(hitPosition, 2*maxHeightChange);
			break;
		}

		terrainManager->RecalculateNormals();
		terrainDisplayManager->UpdateTerrainDisplayFromTerrain();
	}
}

void FTerrainEditor::Flatten(glm::vec3 hitPosition, float maxHeightChange)
{
	int flattenVertCount = 0;
	float flattenHeightSum = 0.0f;
	
	for (int i = 0; i < terrain->vertices.size(); i++)
	{
		auto vertexPosition = terrain->vertices[i].pos;
		float distance = glm::distance(vertexPosition, hitPosition);
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
		float distance = glm::distance(vertexPosition, hitPosition);
		if (distance < maxDistance)
		{
			terrain->vertices[i].pos.z = MoveTo(terrain->vertices[i].pos.z, flattenAverage, maxHeightChange * (1 - distance / maxDistance));
		}
	}
}

void FTerrainEditor::Lower(glm::vec3 hitPosition, float maxHeightChange)
{
	for (int i = 0; i < terrain->vertices.size(); i++)
	{
		auto vertexPosition = terrain->vertices[i].pos;
		float distance = glm::distance(vertexPosition, hitPosition);
		if (distance < maxDistance)
		{
			terrain->vertices[i].pos.z -= Lerp(maxHeightChange, 0.0f, distance / maxDistance);
		}
	}	
}

void FTerrainEditor::Raise(glm::vec3 hitPosition, float maxHeightChange)
{

	for (int i = 0; i < terrain->vertices.size(); i++)
	{
		auto vertexPosition = terrain->vertices[i].pos;
		float distance = glm::distance(vertexPosition, hitPosition);
		if (distance < maxDistance)
		{
			terrain->vertices[i].pos.z += Lerp(maxHeightChange, 0.0f, distance / maxDistance);
		}
	}
}

glm::vec3 FTerrainEditor::GetHitPosition()
{
	scene->position = inputManager->HitPoint;

	return glm::vec3(inputManager->HitPoint);
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