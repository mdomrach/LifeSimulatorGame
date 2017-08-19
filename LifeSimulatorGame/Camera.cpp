#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

FCamera::FCamera(int width, int height)
{
	//view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	position = glm::vec3(-0.00840694644, 1.77852452, -2.28619218);
	rotation = glm::quat(-0.917997181, 0.357873142, 0.0174562167, 0.0438852571);

	glm::mat4 transform;
	transform = transform * glm::toMat4(rotation);
	transform = glm::translate(transform, position);
	view = transform;

	proj = glm::perspective(glm::radians(45.0f), width / (float)height, 1.0f, 200.0f);
}
