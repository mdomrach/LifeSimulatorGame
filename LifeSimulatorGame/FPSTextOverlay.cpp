#include "FPSTextOverlay.h"

#include "GameManager.h"
#include "VulkanApplication.h"
#include "TimeManager.h"
#include "TextOverlay.h"
#include "VulkanTextOverlay.h"

void FFPSTextOverlay::Initialize(FGameManager* gameManager)
{
	vulkanTextOverlay = gameManager->vulkanApplication->textOverlay;
	timeManager = gameManager->timeManager;
	textOverlay = gameManager->textOverlay;


	textOverlay->x = 5.0f;
	textOverlay->y = 5.0f;
	textOverlay->align = ETextAlign::alignLeft;
}

void FFPSTextOverlay::UpdateFrame()
{
	frameCount++;
	if (timeManager->startFrameTime > nextFPSUpdateTime)
	{
		int fps = frameCount;
		textOverlay->text = std::to_string(fps);
		vulkanTextOverlay->UpdateTextOverlay();
		frameCount = 0;
		nextFPSUpdateTime++;
	}
}