#pragma once

#include "Window.h"

class GameWindow : public Window
{
public:
	GameWindow(const char* title, const glm::uvec2& dimensions) : Window(title, dimensions)
	{
	}

protected:
	void init() override
	{
	}

	void update(const float& deltaTime) override
	{
	}

	void onResize(const glm::uvec2& windowDimensions) override
	{
	}
};
