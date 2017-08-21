#pragma once

class FGameManager;
class FTimeManager;
class FTextOverlay;
class FVulkanTextOverlay;

class FFPSTextOverlay
{
public:
	void Initialize(FGameManager* gameManager);
	void UpdateFrame();

private:
	int frameCount;
	float nextFPSUpdateTime;

	FVulkanTextOverlay* vulkanTextOverlay;
	FTimeManager* timeManager;
	FTextOverlay* textOverlay;
};

