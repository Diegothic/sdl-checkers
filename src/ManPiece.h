#pragma once

#include "Piece.h"

class ManPiece : public Piece
{
public:
	ManPiece(PieceType type) : Piece(type)
	{
	}

	virtual ~ManPiece() override = default;

public:
	virtual void update(float deltaTime) override
	{
		Piece::update(deltaTime);
	}

	virtual void render(const Renderer& renderer) const override
	{
		const std::vector<Triangle> mesh = renderer.getMeshPrototypes()
		                                           .getPiece(getType(), getState());

		renderer.drawTriangles(mesh, getTransform());
	}

	virtual std::vector<glm::ivec2> getViableMoves() const override
	{
		return {};
	}
};
