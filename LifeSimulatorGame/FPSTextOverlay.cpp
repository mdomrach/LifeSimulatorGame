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

	textOverlay = new FTextOverlay();
	textOverlay->x = 5.0f;
	textOverlay->y = 5.0f;
	textOverlay->align = ETextAlign::alignLeft;

	gameManager->textOverlay.push_back(textOverlay);
}

void FFPSTextOverlay::UpdateFrame()
{
	frameCount++;
	if (timeManager->startFrameTime > nextFPSUpdateTime)
	{
		int fps = frameCount;
		textOverlay->text = "FPS: "+std::to_string(fps);
		vulkanTextOverlay->UpdateTextOverlay();
		frameCount = 0;
		nextFPSUpdateTime++;
	}
}