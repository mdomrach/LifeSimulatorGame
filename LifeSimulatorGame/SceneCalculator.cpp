#include "SceneCalculator.h"
#include "Scene.h"
#include "Mesh.h"
#include "Camera.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <unordered_map>
#include "MeshVertex.h"
#include "Mesh.h"

void FSceneCalculator::LoadScene(FScene* scene, int width, int height)
{
	scene->camera = new FCamera(width, height);
}