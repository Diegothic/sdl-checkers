#pragma once

#include "ManPiece.h"

class KingPiece : public ManPiece
{
public:
	KingPiece(PieceType type) : ManPiece(type)
	{
	}

	KingPiece(const Piece& other) : ManPiece(other)
	{
	}

	virtual ~KingPiece() override = default;

public:
	virtual void update(float deltaTime) override
	{
		ManPiece::update(deltaTime);
	}

	virtual void render(const Renderer& renderer) const override
	{
		ManPiece::render(renderer);
		const std::vector<Triangle> mesh = renderer.getMeshPrototypes()
		                                           .getPiece(getType(), getState());

		Transform transform = getTransform();
		transform.position.y += 0.2f;
		renderer.drawTriangles(mesh, transform);
	}

	virtual void recalculateMoves(
		const glm::ivec2& coords,
		Piece*** board,
		int boardSize
	) override
	{
		Piece::recalculateMoves(coords, board, boardSize);
		const int forward = getType() == PieceType::Light ? -1 : 1;

		calculateMovesDiagonal(coords, board, boardSize, forward, 1, 99);
		calculateMovesDiagonal(coords, board, boardSize, forward, -1, 99);
		calculateMovesDiagonal(coords, board, boardSize, -forward, 1, 99);
		calculateMovesDiagonal(coords, board, boardSize, -forward, -1, 99);

		calculateCapturesDiagonal(coords, board, boardSize, forward, 1, 99);
		calculateCapturesDiagonal(coords, board, boardSize, forward, -1, 99);
		calculateCapturesDiagonal(coords, board, boardSize, -forward, 1, 99);
		calculateCapturesDiagonal(coords, board, boardSize, -forward, -1, 99);

		if (getCapturesCount() > 0)
		{
			m_viableMoves.erase(
				std::remove_if(
					m_viableMoves.begin(),
					m_viableMoves.end(),
					[](const Move& move) -> bool
					{
						return !move.captures;
					}),
				m_viableMoves.end()
			);
		}
	}
};
