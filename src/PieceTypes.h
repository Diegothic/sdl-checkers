#pragma once

#include <glm/glm.hpp>

enum PieceType
{
	Light,
	Dark
};

enum PieceState
{
	Neutral,
	Movable,
	Selected,
	Capture
};

struct Move
{
	glm::ivec2 destination = {};
	glm::ivec2 capture = {};
	bool captures = false;

	bool operator==(const glm::ivec2& vec2) const
	{
		return vec2 == destination;
	}
};
