#include "SceneCalculator.h"
#include "Scene.h"
#include "Mesh.h"
#include "Camera.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <unordered_map>
#include "Vertex.h"
#include "Terrain.h"

void FSceneCalculator::LoadScene(FScene* scene, int width, int height)
{
	scene->mesh = LoadMesh();
	scene->camera = LoadCamera(width, height);
	//scene->terrain = LoadTerrain();
}

FMesh* FSceneCalculator::LoadMesh()
{
	auto mesh = new FMesh();
	
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string Err;


	std::string MODEL_PATH2 = "Models/chalet.obj";
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &Err, MODEL_PATH2.c_str()))
	{
		throw std::runtime_error(Err);
	}

	std::unordered_map<FVertex, uint32_t> uniqueVertices = {};
	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			FVertex vertex = {};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 0.0f, 0.0f, 0.0f };

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>((uint32_t)mesh->vertices.size());
				mesh->vertices.push_back(vertex);
			}

			mesh->indices.push_back(uniqueVertices[vertex]);
		}
	}

	return mesh;
}

FCamera* FSceneCalculator::LoadCamera(int width, int height)
{
	auto camera = new FCamera(width, height);

	return camera;
}
