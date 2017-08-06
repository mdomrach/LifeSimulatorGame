#include "QueueFamilyIndices.h"

bool FQueueFamilyIndices::IsComplete()
{
	return graphicsFamily >= 0 && presentFamily >= 0;
}
