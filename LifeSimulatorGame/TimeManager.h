#pragma once
class FTimeManager
{
public:
	float deltaFrameTime = 0.0f;
	float startFrameTime = 0.0f;

	void UpdateTime();
};

