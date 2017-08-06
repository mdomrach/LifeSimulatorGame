#include "SleepCalculator.h"
#include "SleepCalculator.h"
#include <wtypes.h>
#include <iostream>

void FSleepCalculator::SleepUntilWaitTime(float deltaFrameTime)
{
	double targetFrameRate = 120;
	double waitTime = 1.0 / targetFrameRate;
	double duration = 1000.0f * (waitTime - deltaFrameTime);// +0.5;
	if (duration <= 0)
		return;

	DWORD durDW = DWORD(duration);
	if (durDW > 0)
	{
		Sleep(durDW);
	}
}