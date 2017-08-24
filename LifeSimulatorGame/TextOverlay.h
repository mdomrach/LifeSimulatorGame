#pragma once

#include <string>

enum ETextAlign { alignLeft, alignCenter, alignRight };

class FTextOverlay
{
public:
	std::string text;
	float x;
	float y;
	ETextAlign align;
};