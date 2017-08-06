#include "VulkanPipelineCalculator.h"

#include <vulkan/vulkan.h>
#include "VulkanInitializers.h"
#include "Vertex.h"
#include "VulkanSwapChain.h"

VkGraphicsPipelineCreateInfo* FVulkanPipelineCalculator::CreateGraphicsPipelineInfo(FVulkanSwapChain swapChain, VkDescriptorSetLayout descriptorSetLayout, VkDevice logicalDevice, VkRenderPass renderPass, VkPipelineLayout pipelineLayout)
{
	//VkPipelineInputAssemblyStateCreateInfo* inputAssembly = new VkPipelineInputAssemblyStateCreateInfo();
	//inputAssembly->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	//inputAssembly->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//inputAssembly->primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo* rasterizer = new VkPipelineRasterizationStateCreateInfo();
	rasterizer->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer->depthClampEnable = VK_FALSE;
	rasterizer->rasterizerDiscardEnable = VK_FALSE;
	rasterizer->polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer->lineWidth = 1.0f;
	rasterizer->cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer->depthBiasEnable = VK_FALSE;
	rasterizer->depthBiasConstantFactor = 0.0f;
	rasterizer->depthBiasClamp = 0.0f;
	rasterizer->depthBiasSlopeFactor = 0.0f;

	//VkPipelineColorBlendAttachmentState* colorBlendAttachment = new VkPipelineColorBlendAttachmentState();
	//colorBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_A_BIT;
	//colorBlendAttachment->blendEnable = VK_FALSE;
	//colorBlendAttachment->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachment->dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachment->colorBlendOp = VK_BLEND_OP_ADD;
	//colorBlendAttachment->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachment->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachment->alphaBlendOp = VK_BLEND_OP_ADD;

	//VkPipelineColorBlendStateCreateInfo* colorBlending = new VkPipelineColorBlendStateCreateInfo();
	//colorBlending->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	//colorBlending->logicOpEnable = VK_FALSE;
	//colorBlending->logicOp = VK_LOGIC_OP_COPY;
	//colorBlending->attachmentCount = 1;
	//colorBlending->pAttachments = colorBlendAttachment;
	//colorBlending->blendConstants[0] = 0.0f;
	//colorBlending->blendConstants[1] = 0.0f;
	//colorBlending->blendConstants[2] = 0.0f;
	//colorBlending->blendConstants[3] = 0.0f;

	//VkPipelineDepthStencilStateCreateInfo* depthStencil = new VkPipelineDepthStencilStateCreateInfo();
	//depthStencil->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	//depthStencil->depthTestEnable = VK_TRUE;
	//depthStencil->depthWriteEnable = VK_TRUE;
	//depthStencil->depthCompareOp = VK_COMPARE_OP_LESS;
	//depthStencil->depthBoundsTestEnable = VK_FALSE;
	//depthStencil->minDepthBounds = 0.0f; // Optional
	//depthStencil->maxDepthBounds = 1.0f; // Optional
	//depthStencil->stencilTestEnable = VK_FALSE;
	//depthStencil->front = {};
	//depthStencil->back = {};

	VkViewport* viewPort = new VkViewport();
	viewPort->x = 0.0f;
	viewPort->y = 0.0f;
	viewPort->width = (float)swapChain.extent.width;
	viewPort->height = (float)swapChain.extent.height;
	viewPort->minDepth = 0.0f;
	viewPort->maxDepth = 1.0f;

	VkRect2D* scissor = new VkRect2D();
	scissor->offset = { 0, 0 };
	scissor->extent = swapChain.extent;

	VkPipelineViewportStateCreateInfo* viewPortState = new VkPipelineViewportStateCreateInfo();
	viewPortState->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewPortState->viewportCount = 1;
	viewPortState->pViewports = viewPort;
	viewPortState->scissorCount = 1;
	viewPortState->pScissors = scissor;


	VkPipelineMultisampleStateCreateInfo* multisampling = new VkPipelineMultisampleStateCreateInfo();
	multisampling->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling->sampleShadingEnable = VK_FALSE;
	multisampling->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling->minSampleShading = 1.0f;
	multisampling->pSampleMask = nullptr;
	multisampling->alphaToCoverageEnable = VK_FALSE;
	multisampling->alphaToOneEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo* pipelineInfo = new VkGraphicsPipelineCreateInfo();
	pipelineInfo->sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo->stageCount = 2;
	//pipelineInfo->pInputAssemblyState = inputAssembly;
	pipelineInfo->pViewportState = viewPortState;
	pipelineInfo->pRasterizationState = rasterizer;
	pipelineInfo->pMultisampleState = multisampling;
	//pipelineInfo->pDepthStencilState = depthStencil;
	//pipelineInfo->pColorBlendState = colorBlending;
	pipelineInfo->pDynamicState = nullptr;
	pipelineInfo->layout = pipelineLayout;
	pipelineInfo->renderPass = renderPass;
	pipelineInfo->subpass = 0;
	pipelineInfo->basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo->basePipelineIndex = 1;

	return pipelineInfo;
}

void FVulkanPipelineCalculator::DeleteGraphicsPipelineInfo(VkGraphicsPipelineCreateInfo* pipelineInfo)
{
	//delete pipelineInfo->pInputAssemblyState;
	delete pipelineInfo->pRasterizationState;
	delete pipelineInfo->pMultisampleState;
	//delete pipelineInfo->pDepthStencilState;

	delete pipelineInfo->pViewportState->pViewports;
	delete pipelineInfo->pViewportState->pScissors;
	delete pipelineInfo->pViewportState;


	//delete pipelineInfo->pColorBlendState->pAttachments;
	//delete pipelineInfo->pColorBlendState;

	delete pipelineInfo;
}
