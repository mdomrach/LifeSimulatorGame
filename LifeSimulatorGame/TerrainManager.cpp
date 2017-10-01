#include "TerrainManager.h"

#include "GameManager.h"
#include <glm/glm.hpp>
#include "PerlinNoise.h"
#include "Mesh.h"
#include "MeshVertex.h"
#include "TerrainCalculator.h"

void FTerrainManager::Initialize(FGameManager* gameManager)
{
	terrain = gameManager->terrain;
	LoadAssets();
}

void FTerrainManager::LoadAssets()
{
	PerlinNoise Noise;

	const float noiseAmplitude = 1.2f;
	const float noiseFrequency = 2.5f / numberOfVertices;

	for (int x = 0; x < numberOfVertices; x++)
	{
		for (int y = 0; y < numberOfVertices; y++)
		{
			float height = (float) (noiseAmplitude * Noise.noise(x * noiseFrequency, y * noiseFrequency));

			FMeshVertex Vertex;
			Vertex.pos = { x, y, 0 };
			Vertex.normal = glm::vec3(0, 0, 1);
			terrain->vertices.push_back(Vertex);
		}
	}

	for (int x = 1; x < numberOfVertices - 1; x++)
	{
		for (int y = 1; y < numberOfVertices - 1; y++)
		{
			int index11 = GetVertexIndex(x + 0, y + 0);
			glm::vec3 height11 = terrain->vertices[index11].pos;

			int index01 = GetVertexIndex(x - 1, y + 0);
			int index10 = GetVertexIndex(x + 0, y - 1);
			int index21 = GetVertexIndex(x + 1, y + 0);
			int index12 = GetVertexIndex(x + 0, y + 1);

			glm::vec3 height01 = terrain->vertices[index01].pos;
			glm::vec3 height21 = terrain->vertices[index21].pos;
			glm::vec3 height10 = terrain->vertices[index10].pos;
			glm::vec3 height12 = terrain->vertices[index12].pos;

			glm::vec3 normal1 = glm::cross(height01 - height11, height10 - height11);
			glm::vec3 normal2 = glm::cross(height21 - height11, height12 - height11);
			terrain->vertices[index11].normal = normal1 + normal2;
		}
	}

	for (int x = 0; x < numberOfQuads; x++)
	{
		for (int y = 0; y < numberOfQuads; y++)
		{
			int index00 = GetVertexIndex(x + 0, y + 0);
			int index01 = GetVertexIndex(x + 0, y + 1);
			int index10 = GetVertexIndex(x + 1, y + 0);
			int index11 = GetVertexIndex(x + 1, y + 1);

			terrain->indices.push_back(index00);
			terrain->indices.push_back(index10);

			terrain->indices.push_back(index01);

			terrain->indices.push_back(index10);
			terrain->indices.push_back(index11);
			terrain->indices.push_back(index01);
		}
	}

	RecalculateNormals();
}


void FTerrainManager::RecalculateNormals()
{
	for (int x = 1; x < FTerrainCalculator::numberOfVertices - 1; x++)
	{
		for (int y = 1; y < FTerrainCalculator::numberOfVertices - 1; y++)
		{
			int index11 = FTerrainCalculator::GetVertexIndex(x + 0, y + 0);
			glm::vec3 height11 = terrain->vertices[index11].pos;

			int index01 = FTerrainCalculator::GetVertexIndex(x - 1, y + 0);
			int index10 = FTerrainCalculator::GetVertexIndex(x + 0, y - 1);
			int index21 = FTerrainCalculator::GetVertexIndex(x + 1, y + 0);
			int index12 = FTerrainCalculator::GetVertexIndex(x + 0, y + 1);

			glm::vec3 height01 = terrain->vertices[index01].pos;
			glm::vec3 height21 = terrain->vertices[index21].pos;
			glm::vec3 height10 = terrain->vertices[index10].pos;
			glm::vec3 height12 = terrain->vertices[index12].pos;

			glm::vec3 normal1 = glm::cross(height01 - height11, height10 - height11);
			glm::vec3 normal2 = glm::cross(height21 - height11, height12 - height11);
			terrain->vertices[index11].normal = normal1 + normal2;
		}
	}
}

int FTerrainManager::GetVertexIndex(int x, int y)
{
	return x * numberOfVertices + y;
}