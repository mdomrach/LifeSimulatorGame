#pragma once

struct FQueueFamilyIndices
{
public:
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool IsComplete();
};

