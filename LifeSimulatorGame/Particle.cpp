#include "Particle.h"
#include "vulkan/vulkan.h"

VkVertexInputBindingDescription FParticle::GetVertexBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription = {};

	bindingDescription.binding = VERTEX_BUFFER_BIND_ID;
	bindingDescription.stride = sizeof(FParticle);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 6> FParticle::GetVertexAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 6> attributeDesciptions = {};

	// Location 0: Position
	attributeDesciptions[0].binding = 0;
	attributeDesciptions[0].location = 0;
	attributeDesciptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDesciptions[0].offset = offsetof(FParticle, pos);

	// Location 1: Color
	attributeDesciptions[1].binding = 0;
	attributeDesciptions[1].location = 1;
	attributeDesciptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDesciptions[1].offset = offsetof(FParticle, color);

	// Location 2: Alpha
	attributeDesciptions[2].binding = 0;
	attributeDesciptions[2].location = 2;
	attributeDesciptions[2].format = VK_FORMAT_R32_SFLOAT;
	attributeDesciptions[2].offset = offsetof(FParticle, alpha);

	// Location 3: Size
	attributeDesciptions[3].binding = 0;
	attributeDesciptions[3].location = 3;
	attributeDesciptions[3].format = VK_FORMAT_R32_SFLOAT;
	attributeDesciptions[3].offset = offsetof(FParticle, size);

	// Location 4: Rotation
	attributeDesciptions[4].binding = 0;
	attributeDesciptions[4].location = 4;
	attributeDesciptions[4].format = VK_FORMAT_R32_SFLOAT;
	attributeDesciptions[4].offset = sizeof(float) * 11;

	// Location 5: Particle type		
	attributeDesciptions[5].binding = 0;
	attributeDesciptions[5].location = 5;
	attributeDesciptions[5].format = VK_FORMAT_R32_SINT;
	attributeDesciptions[5].offset = offsetof(FParticle, type);

	return attributeDesciptions;


}
